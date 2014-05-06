/* Copyright 2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 * @brief Demonstrate TM4C edge time captures and ping-pong DMA
 *
 * This utility measures the high and low pulse widths resulting from
 * button presses, using a 16/32 GPTimer peripheral in dual 16-bit
 * mode.  TIMERA captures edge changes, and ping-pong DMA transfers
 * each capture into a buffer for batch processing. TIMERB is used as
 * an alarm to detect cessation of pulse activity. */

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/utility/misc.h>
#include <bspacm/periph/gpio.h>
#include <driverlib/sysctl.h>
#include <inc/hw_udma.h>
#include <inc/hw_timer.h>
#include <inc/hw_gpio.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>

/** The instance of the GPTimer module that will be used for captures.
 * This affects which DMA channel is used, and which pin the signal is
 * connected to. */
#define TIMER0_MODULE 0

/** GPIO used to capture the signal that will be monitored.  This must
 * be a pin on which the capture/compare signal for TIMER0A is
 * present, and it must also have a corresponding DMA channel. */
static const sBSPACMdeviceTM4Cpinmux signal = {
#if (BSPACM_BOARD_EK_TM4C123GXL - 0)
  /* PB6(7) is T0CCP0 on DMA channel 18.0 */
  .port = GPIOB, .pin = 6, .pctl = 7,
#define TIMER0A_DMA_CHANNEL 18
#define TIMER0A_DMA_SELECT 0
#endif /* BOARD */
};

/** There's only one DMA module on the device, but make clear what 0
 * means when it's being configured. */
#define UDMA_MODULE 0

/** Capture values are 24-bit, count downwards, and wrap on overflow.
 * Since we don't have a 24-bit unsigned type we need to use modular
 * signed arithmetic. */
typedef int capture_type;

/** Valid capture values will be non-negative and strictly less than
 * this value. */
#define CAPTURE_RANGE ((capture_type)(1U << 24))

/** Conversion factor between undivided timer ticks and
 * microseconds */
static unsigned int ticks_per_us;

/** Determine the number of ticks between capture value @p c0 and a
 * later capture value @p c1, taking into account the possibility the
 * counter wrapped between the captures. */
static unsigned int
capture_delta (capture_type c0,
               capture_type c1)
{
  capture_type cd = c0 - c1;

  /* Normally c0 > c1 because the counter decreases, but if there was
   * a timer wrap then the reverse will be true and the calculation
   * must be adjusted. */
  if (0 > cd) {
    cd += CAPTURE_RANGE;
  }
  return cd;
}

/** Rounded number of microseconds between two capture times measured
 * in ticks. */
static unsigned int
capture_delta_us (capture_type c0,
                  capture_type c1)
{
  unsigned int cd = capture_delta(c0, c1);
  return (cd + ticks_per_us / 2) / ticks_per_us;
}

/** Avoid the magic number 32 representing the number of supported DMA
 * channels in the peripheral interface. */
#define DMA_CHANNEL_COUNT 32

/* Allocate space to hold the configurations for the primary and
 * alternative DMA channels.  The channel table must be aligned to a 1
 * KiBy boundary. */
BSPACM_CORE_ALIGNED_OBJECT(1024)
static UDMA_CHANNEL_Type dma_channel_table[DMA_CHANNEL_COUNT * 2];

/** The DMA channel control bit settings exclusive of transfer size.
 * Captures are edge times measured in 24-bit counters (with bits
 * 31..25 zeroed).  Each sample is read from the timer TAR register
 * and transferred into a buffer.  When all samples in a batch have
 * been collected, they're processed while the DMA goes on to fill the
 * other half of the buffer. */
#define CAPTURE_CHCTL (UDMA_CHCTL_DSTINC_32 /* Destination word increment */ \
                       | UDMA_CHCTL_SRCINC_NONE /* Source no increment  */ \
                       | UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_SRCSIZE_32 /* Word copy */ \
                       | UDMA_CHCTL_ARBSIZE_1 /* One transfer per arbitration */ \
                       | UDMA_CHCTL_XFERMODE_PINGPONG /* One request, one transfer */ \
                       )

/** Buffer for stored edge times.  This is split into two segments,
 * with DMA ping-pong used to fill one half while the other half is
 * processed. */
capture_type captures[256];

/** Number of samples in each DMA capture */
static unsigned int const capture_batch_length = (sizeof(captures)/sizeof(*captures)) / 2;

/** Number of consecutive batches in the capture of the current
 * series. */
unsigned int volatile dma_batch_count;

/** Pointers into the DMA channel table for the primary (0) and
 * alternative (1) configurations for the channel used to collect
 * capture times. */
UDMA_CHANNEL_Type * const capture_swch[2] = {
  dma_channel_table + TIMER0A_DMA_CHANNEL,
  dma_channel_table + DMA_CHANNEL_COUNT + TIMER0A_DMA_CHANNEL,
};

/** The index to the #capture_swch element that is currently active
 * (or has just completed).  When the data collected in the referenced
 * half has been processed and the half re-enabled for DMA, the value
 * is updated to refer to the other half. */
static uint8_t capture_half_idx;

/** TIMERA in edge-mode has 24 bits available (and cannot be scaled),
 * but TIMERB in one-shot mode has only 16 (but can be scaled).  Use a
 * /128 divider so that the maximum idle period we can support is half
 * the maximum pulse width we could detect.
 *
 * 80 MHz : max TB duration 104 ms with /128
 * 5 MHz : minimum TB duration 25 us with /128
 *
 * Note: The 16-bit timer we're using allows a divider up to 256,
 * which would result in exactly matching the representation range of
 * the 24-bit edge timer.  Don't go that high: we want to make sure
 * that the interrupt handler can calculate the time-since-last-edge
 * without overlapping the edge timer.
 */
#define TIMER0B_DIVIDER 128

/** Collection of a series stops when there have been @p idle_span_us
 * microseconds with no edge transitions. */
static const int idle_span_us = 30000;

#define EVENT_CAPTURE_READY_S 0
#define EVENT_CAPTURE_OVERRUN_S 1
#define EVENT_CAPTURE_IDLE_S 2
#define EVENT_CAPTURE_READY (1U << EVENT_CAPTURE_READY_S)
#define EVENT_CAPTURE_OVERRUN (1U << EVENT_CAPTURE_OVERRUN_S)
#define EVENT_CAPTURE_IDLE (1U << EVENT_CAPTURE_IDLE_S)
static volatile unsigned int events_v;
static volatile unsigned int captimer_irq_count;

/** The edge time at which we decided the series had completed */
static volatile capture_type idle_timestamp;

/* TIMERA and TIMERB delegate to this shared implementation. */
static void
captimer_IRQHandler (void)
{
  unsigned int mis = TIMER0->MIS;
  bool set_idle_alarm = false;
  bool abort_capture = false;

  ++captimer_irq_count;
  TIMER0->ICR = mis;

  /* CAE will be set as soon as there's a capture, but even though it
   * shows up in MIS the NVIC won't signal the interrupt until the
   * corresponding DMA completes.  (This is why we need the
   * first-DMA-buffer-one-sample trick.)  Thus, when we're entering
   * the interrupt because of idle timer checks we'll see a CAE that
   * is not authoritative for completion of the DMA.
   *
   * Use the CHIS indicator to detect DMA completion.  NB: On TM4C129
   * it appears this register will disappear and be replaced by
   * TIMER_MIS_DMAAMIS. */
  if (BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, TIMER0A_DMA_CHANNEL)) {
    BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, TIMER0A_DMA_CHANNEL) = 1;

    /* The first capture signal comes after the alternate
     * configuration captured one sample to cue start of the idle
     * timer.  We need to move that capture into its proper location
     * at the beginning of the first buffer, and reconfigure the
     * alternate configuration so the next capture is full-length. */
    if (0 == dma_batch_count) {
      captures[0] = captures[2*capture_batch_length - 1];
      capture_swch[1]->chctl = CAPTURE_CHCTL | ((capture_batch_length - 1) << UDMA_CHCTL_XFERSIZE_S);
      vBSPACMledSet(BSPACM_LED_GREEN, 1);
    } else {
      /* Anything after the first sample is a complete buffer */
      BSPACM_CORE_BITBAND_SRAM32(events_v, EVENT_CAPTURE_READY_S) = 1;
    }
    ++dma_batch_count;
  }

  /* If uDMA disabled itself we didn't process data fast enough and
   * have produced an overrun.  Stop the capture and record the need
   * to clean up. */
  if (! BSPACM_CORE_BITBAND_PERIPH(UDMA->ENASET, TIMER0A_DMA_CHANNEL)) {
    abort_capture = true;
    BSPACM_CORE_BITBAND_SRAM32(events_v, EVENT_CAPTURE_OVERRUN_S) = 1;
  }

  /* If we know there's been a capture since the last time we were
   * here, even if the DMA is ongoing, we should reschedule the idle
   * timer, unless we already know the series collection must stop. */
  if (TIMER_MIS_CAEMIS & mis) {
    mis &= ~TIMER_MIS_CAEMIS;
    set_idle_alarm = (! abort_capture);
  }

  if (set_idle_alarm) {
    int since_capture_us;
    int until_idle_us;

    /* Ignore and disable any current idle alarm, and schedule a new
     * alarm to wake when the time since the last capture will exceed
     * the maximum idle time.  If through some external delays that's
     * already happened, consider the alarm to have expired already.
     * (NB: Correct functioning requires that idle_span_us be
     * representable in 16-bits for TIMERB using the TIMER0B_DIVIDER
     * and in 23 bits (half the representable range) for the undivided
     * TIMERA.) */
    BSPACM_CORE_BITBAND_PERIPH(TIMER0->CTL, uiBSPACMcoreBitFromMask(TIMER_CTL_TBEN)) = 0;
    mis &= ~TIMER_MIS_TBTOMIS;
    since_capture_us = capture_delta(TIMER0->TAR, TIMER0->TAV) / ticks_per_us;
    until_idle_us = idle_span_us - since_capture_us;
    if (0 > until_idle_us) {
      abort_capture = 1;
      BSPACM_CORE_BITBAND_SRAM32(events_v, EVENT_CAPTURE_IDLE_S) = 1;
    } else {
      TIMER0->TBILR = (until_idle_us * ticks_per_us) / TIMER0B_DIVIDER;
      TIMER0->ICR = TIMER_ICR_TBTOCINT;
      BSPACM_CORE_BITBAND_PERIPH(TIMER0->CTL, uiBSPACMcoreBitFromMask(TIMER_CTL_TBEN)) = 1;
    }
  } else if (TIMER_MIS_TBTOMIS & mis) {
    /* No activity and the idle timer fired.  Stop the collection. */
    mis &= ~TIMER_MIS_TBTOMIS;
    BSPACM_CORE_BITBAND_SRAM32(events_v, EVENT_CAPTURE_IDLE_S) = 1;
    abort_capture = true;
  }

  if (abort_capture) {
    /* Disable the idle timer, the DMA, and all timer interrupts.
     * Mark a (partial) capture ready in case there's trailing data in
     * the uncompleted DMA. */
    BSPACM_CORE_BITBAND_PERIPH(TIMER0->CTL, uiBSPACMcoreBitFromMask(TIMER_CTL_TBEN)) = 0;
    idle_timestamp = TIMER0->TAV;
    BSPACM_CORE_BITBAND_PERIPH(UDMA->ENACLR, TIMER0A_DMA_CHANNEL) = 1;
    TIMER0->IMR = 0;
    BSPACM_CORE_BITBAND_SRAM32(events_v, EVENT_CAPTURE_READY_S) = 1;
    vBSPACMledSet(BSPACM_LED_GREEN, 0);
  }
}

void
TIMER0A_IRQHandler (void)
{
  captimer_IRQHandler();
}

void
TIMER0B_IRQHandler (void)
{
  captimer_IRQHandler();
}

#define TICKS_FROM_us(us_) ((us_) * (SystemCoreClock / 1000000U))

void
main ()
{
  GPIOCommon_Type * const gpio = (GPIOCommon_Type *)signal.port;
  volatile uint32_t * const signalp = &BSPACM_CORE_BITBAND_PERIPH(gpio->DATA, signal.pin);
  uint8_t signal_idle_state;
  unsigned int displayed_sample_count = 0;
  int last_timestamp = -1;
  unsigned int captured_edge_count = 0;
  unsigned int num_series = 0;
  unsigned int num_sleeps = 0;
  unsigned int num_overruns = 0;

#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
  /* NOTE: Setting to 80 MHz requires a patch to SysCtlClockGet() when
   * using TivaWare_C_Series-2.1.0.12573.  See
   * http://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/p/330524/1156581.aspx#1156581 */
#if 1
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* 2*PLL/5 = 80 MHz */
#elif 1
  SysCtlClockSet(SYSCTL_SYSDIV_20 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/20 = 10 MHz */
#elif 1
  SysCtlClockSet(SYSCTL_SYSDIV_40 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/40 = 5 MHz */
#endif
  SystemCoreClock = SysCtlClockGet();
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
#endif /* LINE */
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");

  ticks_per_us = TICKS_FROM_us(1);

  printf("System clock %lu Hz (%lu ns per tick)\n",
         SystemCoreClock, 1000000000U / SystemCoreClock);
  printf("Maximum pulse duration: %u us\n", CAPTURE_RANGE / ticks_per_us);
  {
    unsigned int ns_per_dtick = 1000000000U / (SystemCoreClock / TIMER0B_DIVIDER);
    unsigned int dticks_per_ms = (SystemCoreClock / TIMER0B_DIVIDER) / 1000U;
    printf("Idle divider %u has resolution %u ns and range %u ms\n",
           TIMER0B_DIVIDER, ns_per_dtick, (1U << 16) / dticks_per_ms);
  }

  /* Enable signal GPIO in active and sleep, but not deep sleep.
   * Configure it for its timer CCP function.  Record its initial
   * state so we always start from there. */
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(gpio)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->SCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(gpio)) = 1;
  __NOP(); __NOP(); __NOP(); /* delay 3 cycles */
  BSPACM_CORE_BITBAND_PERIPH(gpio->DIR, signal.pin) = 0; /* input */
  BSPACM_CORE_BITBAND_PERIPH(gpio->AFSEL, signal.pin) = 1; /* ccp */
  vBSPACMcoreSetPinNybble(&gpio->PCTL, signal.pin, signal.pctl);
  BSPACM_CORE_BITBAND_PERIPH(gpio->PUR, signal.pin) = 1; /* pull-up enabled */
  BSPACM_CORE_BITBAND_PERIPH(gpio->DEN, signal.pin) = 1; /* enable digital function */
  signal_idle_state = *signalp;

  printf("Connect signal to P%c%u.  Signal idle state is %u.\n",
         iBSPACMdeviceTM4CgpioPortTagFromShift(iBSPACMdeviceTM4CgpioPortShift(gpio)),
         signal.pin, signal_idle_state);

  /* DMA available in active and sleep, but not in deep sleep */
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCDMA, UDMA_MODULE) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->SCGCDMA, UDMA_MODULE) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->DCGCDMA, UDMA_MODULE) = 0;

  /* Enable DMA module and specify the control table */
  UDMA->CFG = UDMA_CFG_MASTEN;
  UDMA->CTLBASE = (uintptr_t)dma_channel_table;

  /* Specify the source selector for the channel that will be used. */
  vBSPACMcoreSetPinNybble(&UDMA->CHMAP0, TIMER0A_DMA_CHANNEL, TIMER0A_DMA_SELECT);

  /* Set channel to known state: Allow peripheral requests,
   * single+burst, use normal priority, clear any past completion
   * signal.  This is all pretty much just setting it to power-up
   * state. */
  BSPACM_CORE_BITBAND_PERIPH(UDMA->REQMASKCLR, TIMER0A_DMA_CHANNEL) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->USEBURSTCLR, TIMER0A_DMA_CHANNEL) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->PRIOCLR, TIMER0A_DMA_CHANNEL) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, TIMER0A_DMA_CHANNEL) = 1;

  /* Both halves of the ping-pong DMA read from TIMER0A's capture
   * register.  The primary configuration always writes into the first
   * half; the secondary always writes into the second half. */
  capture_swch[0]->srcendp = &TIMER0->TAR;
  capture_swch[1]->srcendp = &TIMER0->TAR;
  capture_swch[0]->dstendp = captures + capture_batch_length - 1;
  capture_swch[1]->dstendp = captures + 2 * capture_batch_length - 1;

  /* Enable the timer module in active and sleep, but not in deep
   * sleep. */
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCTIMER, TIMER0_MODULE) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->SCGCTIMER, TIMER0_MODULE) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->DCGCTIMER, TIMER0_MODULE) = 0;

  /* Make it a pair of timers, counting down.  TA uses edge timer
   * captures; TB is a divided one-shot used to detect cessation of
   * pulse activity.
   *
   * TAILR and TAPR must be all ones to ensure that the full 24-bit
   * range is available for the edge capture.  The TAPR power-up value
   * is all zeros.
   *
   * Divide the periodic timer to increase the duration that we can
   * measure.  Without this, at 80MHz the 16-bit counter would wrap
   * before 820 us; with it we can get to 104 ms with a divider of
   * 128. */
  TIMER0->CFG = TIMER_CFG_16_BIT;
  TIMER0->TAILR = -1;
  TIMER0->TAPR = -1;
  TIMER0->TBPR = TIMER0B_DIVIDER - 1;
  TIMER0->TAMR = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP;
  TIMER0->TBMR = TIMER_TBMR_TBMR_1_SHOT;
  TIMER0->IMR = 0;
  TIMER0->CTL = TIMER_CTL_TAEVENT_BOTH | TIMER_CTL_TAEN;

  /* Enable timer interrupts */
  NVIC_ClearPendingIRQ(TIMER0A_IRQn);
  NVIC_ClearPendingIRQ(TIMER0B_IRQn);
  NVIC_EnableIRQ(TIMER0A_IRQn);
  NVIC_EnableIRQ(TIMER0B_IRQn);

  /* Start by processing an idle event which enables a fresh
   * capture. */
  events_v = EVENT_CAPTURE_IDLE;

  while (1) {
    bool need_restart = false;
    unsigned int events;

    fflush(stdout);
    ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      while (! events_v) {
        ++num_sleeps;
        BSPACM_CORE_SLEEP();
        BSPACM_CORE_ENABLE_INTERRUPT();
        BSPACM_CORE_DISABLE_INTERRUPT();
      }
      events = events_v;
      events_v = 0;
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();

    if (EVENT_CAPTURE_READY & events) {
      const UDMA_CHANNEL_Type * const swch = capture_swch[capture_half_idx];
      const int size_remaining = ((UDMA_CHCTL_XFERSIZE_M & swch->chctl) >> UDMA_CHCTL_XFERSIZE_S);
      capture_type * caps = captures + (capture_half_idx ? capture_batch_length : 0);
      const capture_type * const cape = (size_remaining ? -size_remaining : 1) + (const capture_type *)swch->dstendp;
      captured_edge_count += cape - caps;
#if 0
      {
        const capture_type * cap = caps;
        unsigned int n = 0;
        cap = caps;
        while (cap < cape) {
          if (0 == (0x07 & n++)) {
            putchar('\n');
          }
          printf(" %x", *cap++);
        }
        putchar('\n');
      }
#endif
#if 0
      printf("capture %u at %p %u to %u terminal %u span %u us\n", cape-caps, caps,
             caps[0], cape[-1], idle_timestamp,
             capture_delta_us(caps[0], cape[-1]));
#endif
#if 1
      {
        capture_type * dp = caps;
        const capture_type * cap = caps;
        if (0 > last_timestamp) {
          last_timestamp = *cap++;
        }
        /* Convert from absolute times to the width of each high and
         * low period. */
        while (cap < cape) {
          capture_type nts = *cap++;
          *dp++ = capture_delta_us(last_timestamp, nts);
          last_timestamp = nts;
        }
        /* Display the length of each period */
        cap = caps;
        while (cap < dp) {
          if (0 == (displayed_sample_count++ % 10)) {
            putchar('\n');
          }
          printf(" %5u,", *cap++);
        }
      }
#endif
      /* Re-enable DMA into the half we just processed */
      capture_swch[capture_half_idx]->chctl = CAPTURE_CHCTL | ((capture_batch_length - 1) << UDMA_CHCTL_XFERSIZE_S);
      capture_half_idx = ! capture_half_idx;
      events &= ~EVENT_CAPTURE_READY;
    }
    if (EVENT_CAPTURE_OVERRUN & events) {
      events &= ~EVENT_CAPTURE_OVERRUN;
      printf("\nOverrun after %u cap %u bufs %u intr %u sleep\n",
             captured_edge_count, dma_batch_count, captimer_irq_count, num_sleeps);
      ++num_overruns;
      need_restart = true;
    }
    if (EVENT_CAPTURE_IDLE & events) {
      printf("\nIdle after %u us\n%u edges %u samples %u batches %u interrupts %u sleep\n",
             capture_delta_us(last_timestamp, idle_timestamp),
             captured_edge_count, displayed_sample_count,
             dma_batch_count, captimer_irq_count, num_sleeps);
      need_restart = true;
    }
    if (need_restart) {
      printf("\nRestarting for series %u with %u overruns\n",
             num_series, num_overruns);
      memset(captures, 0, sizeof(captures));
      BSPACM_CORE_DISABLE_INTERRUPT();
      do {
        uint8_t ss0;            /* signal state when starting configuration */
        uint8_t ss1;            /* signal state after completing configuration */

        /* Loop enabling a capture until we do so with the signal
         * state being idle at the start, and not having changed while
         * we were configuring things. */
        do {
          /* Stop any DMA activity resulting from a signal change
           * during the last reconfiguration and wipe out any recorded
           * timer interrupts. */
          BSPACM_CORE_BITBAND_PERIPH(UDMA->ENACLR, TIMER0A_DMA_CHANNEL) = 1;
          TIMER0->ICR = TIMER0->RIS;

          /* Wait for the signal to return to its idle state. */
          do {
            ss0 = *signalp;
          } while (signal_idle_state != ss0);

          /* The first completed buffer will be the first half, but
           * we're going to start with a one-element capture in the
           * alternate configuration which will cause the idle timer to
           * be set.  While the primary configuration continues to fill
           * the remainder of the first buffer, the IRQ handler will
           * copy the first captured time into the correct position and
           * reconfigure the alternate configuration to take over when
           * the first half is filled.  This will ensure the idle timer
           * is set after the first capture, in case the pulse sequence
           * is shorter than capture_batch_length. */
          BSPACM_CORE_BITBAND_PERIPH(UDMA->ALTSET, TIMER0A_DMA_CHANNEL) = 1;
          capture_swch[1]->chctl = CAPTURE_CHCTL;
          capture_swch[0]->chctl = CAPTURE_CHCTL | ((capture_batch_length - 2) << UDMA_CHCTL_XFERSIZE_S);

          /* Clear the pending timer interrupts, and enable the capture
           * interrupt (which will fire only when the corresponding DMA
           * transfer completes).  Then clear the DMA interrupts and
           * enable the DMA. */
          BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, TIMER0A_DMA_CHANNEL) = 1;
          BSPACM_CORE_BITBAND_PERIPH(UDMA->ENASET, TIMER0A_DMA_CHANNEL) = 1;

          /* Get the signal state.  If it's not the same as it was
           * when we started, or the timer captured an edge transition
           * while we were working, we need to start over. */
          ss1 = *signalp;
        } while ((ss0 != ss1) || (TIMER_RIS_CAERIS & TIMER0->RIS));

        /* Enable interrupt on idle timeout and on capture, then reset
         * all the per-series state. */
        TIMER0->IMR = TIMER_IMR_TBTOIM | TIMER_IMR_CAEIM;
        capture_half_idx = 0;
        captimer_irq_count = 0;
        dma_batch_count = 0;
        captured_edge_count = 0;
        idle_timestamp = -1;
        last_timestamp = -1;
        displayed_sample_count = 0;
        ++num_series;
      } while (0);
      BSPACM_CORE_ENABLE_INTERRUPT();
    }
  }

  NVIC_DisableIRQ(TIMER0B_IRQn);
  NVIC_DisableIRQ(TIMER0A_IRQn);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
