/* BSPACM - nRF51 high-resolution timer support
 *
 * Copyright 2015, Peter A. Bigot
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

#include <bspacm/utility/hires.h>
#include "nrf_delay.h"

#if NRF_TIMER0_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER_IRQn TIMER0_IRQn
#define BSPACM_HIRES_TIMER_IRQHandler TIMER0_IRQHandler
#elif NRF_TIMER1_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER_IRQn TIMER1_IRQn
#define BSPACM_HIRES_TIMER_IRQHandler TIMER1_IRQHandler
#elif NRF_TIMER2_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER_IRQn TIMER2_IRQn
#define BSPACM_HIRES_TIMER_IRQHandler TIMER2_IRQHandler
#else  /* BSPACM_HIRES_TIMER_BASE */
#error Unrecognized high-resolution timer
#endif /* BSPACM_HIRES_TIMER_BASE */

static volatile bool cc0_timeout;

void BSPACM_HIRES_TIMER_IRQHandler ()
{
  if (BSPACM_HIRES_TIMER->EVENTS_COMPARE[0]) {
    cc0_timeout = true;
    BSPACM_HIRES_TIMER->EVENTS_COMPARE[0] = 0;
  }
}

int
iBSPACMhiresInitialize (unsigned int freq_Hz)
{
  unsigned int divisor = SystemCoreClock / freq_Hz;

  /* Desired frequency must divide SystemCoreClock, and do so with a
   * multiple that is a power of two. */
  if (((divisor * freq_Hz) != SystemCoreClock)
      || (0 != (divisor & (divisor - 1)))) {
    return -1;
  }
  /* Calculate the bit shift required to produce divisor. */
  int prescaler = -1;
  do {
    ++prescaler;
    divisor >>= 1;
  } while (divisor);

  /* High-resolution timer uses the high-frequency clock (go figure).
   * Start it if nobody's done so already. */
  if ((CLOCK_HFCLKSTAT_SRC_Xtal << CLOCK_HFCLKSTAT_SRC_Pos)
      != (CLOCK_HFCLKSTAT_SRC_Msk & NRF_CLOCK->HFCLKSTAT)) {
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (! NRF_CLOCK->EVENTS_HFCLKSTARTED) {
    }
  }

  /* Use a the biggest timer base available.  Pretty feeble except for
   * TIMER0. */
  BSPACM_HIRES_TIMER->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  BSPACM_HIRES_TIMER->PRESCALER = prescaler;
#if (NRF_TIMER0_BASE == BSPACM_HIRES_TIMER_BASE)
  BSPACM_HIRES_TIMER->BITMODE = (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos);
#else /* BSPACM_HIRES_TIMER_BASE */
  BSPACM_HIRES_TIMER->BITMODE = (TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos);
#endif /* BSPACM_HIRES_TIMER_BASE */

  return 0;
}

void
vBSPACMhiresSetEnabled (bool enabled)
{
  if (enabled) {
    /* Enable interrupts (thus event wakeup?) at the peripheral, but not
     * at the NVIC */
    BSPACM_HIRES_TIMER->INTENCLR = ~0;
    BSPACM_HIRES_TIMER->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
    NVIC_ClearPendingIRQ(BSPACM_HIRES_TIMER_IRQn);
    NVIC_EnableIRQ(BSPACM_HIRES_TIMER_IRQn);

    /* Clear the counter and start things going */
    BSPACM_HIRES_TIMER->TASKS_CLEAR = 1;
    BSPACM_HIRES_TIMER->TASKS_START = 1;
  } else {
    BSPACM_HIRES_TIMER->TASKS_STOP = 1;
    NVIC_DisableIRQ(BSPACM_HIRES_TIMER_IRQn);
    BSPACM_HIRES_TIMER->INTENCLR = ~0;
    NVIC_ClearPendingIRQ(BSPACM_HIRES_TIMER_IRQn);
  }
}

void
vBSPACMhiresSleep_us (unsigned long count_us)
{
  /* The optimized compare initialization sequence takes about 27 (16
   * MHz) ticks, or a little under 2 us.  If the delay period isn't at
   * least 5 us and doesn't require at least two tick increments, we
   * could miss the compare.  In that situation fall back to the nRF
   * delay implementation (which you did patch so it's correct for
   * gcc, didn't you?) */
  const unsigned long min_wfe_delay_us = 5;
  const unsigned long min_wfe_delay_hrt = 2;
  unsigned int count_hrt = uiBSPACMhiresConvert_us_hrt(count_us);
  if ((min_wfe_delay_us > count_us)
      || (min_wfe_delay_hrt > count_hrt)) {

    /* nrf_delay_us will delay for 2^32 us if passed zero, so don't do
     * that. */
    if (0 < count_us) {
      nrf_delay_us(count_us);
    }
    return;
  }
  cc0_timeout = false;
  BSPACM_HIRES_TIMER->TASKS_CAPTURE[0] = 1;
  BSPACM_HIRES_TIMER->EVENTS_COMPARE[0] = 0;
  BSPACM_HIRES_TIMER->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  BSPACM_HIRES_TIMER->CC[0] += count_hrt;
  while (! cc0_timeout) {
    __WFE();
  }
  BSPACM_HIRES_TIMER->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk;
  return;
}
