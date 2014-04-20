/* BSPACM - tm4c/rtc demonstration application
 *
 * Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <inc/hw_hibernate.h>
#include <inc/hw_sysctl.h>
#include <stdio.h>
#include <fcntl.h>

#ifndef WAKEUP_INTERVAL_S
#define WAKEUP_INTERVAL_S 10
#endif /* WAKEUP_INTERVAL_S */

volatile uint32_t hib_mis_v;
volatile unsigned int hib_irqc_v;
void HIBERNATE_IRQHandler (void)
{
  uint32_t hib_mis;

  ++hib_irqc_v;
  hib_mis_v = hib_mis = HIB->MIS;
  HIB->IC = hib_mis;
}

typedef struct sResetCause {
  uint32_t mask;
  const char * name;
} sResetCause;
static const sResetCause xResetCause[] = {
  { SYSCTL_RESC_MOSCFAIL, "MOSC Failure" },
  { SYSCTL_RESC_HSSR, "HSSR Reset" },
  { SYSCTL_RESC_HIB, "HIB" },
  { SYSCTL_RESC_WDT1, "Watchdog Timer 1" },
  { SYSCTL_RESC_SW, "Software Reset" },
  { SYSCTL_RESC_WDT0, "Watchdog Timer 0" },
  { SYSCTL_RESC_BOR, "Brown-Out" },
  { SYSCTL_RESC_POR, "Power-On" },
  { SYSCTL_RESC_EXT, "External" },
};

#define MAGIC 0x14235342
/** NOTE: Any write into this structure must be followed by a wait for
 * WRC to clear. */
typedef struct sRetainedState {
  uint32_t magic;
  uint32_t boots;
  uint32_t hibdelay;
} sRetainedState;
sRetainedState * const retained_state = (sRetainedState *)&HIB->DATA;

void main ()
{
  uint32_t reset_cause;
  unsigned int hib_ris;
  bool need_reset;
  int sleep_mode = 0;

#if (BSPACM_DEVICE_LINE_TM4C129 - 0)
  /* TM4C129 has GPIO retention which must be explicitly cleared so
   * that pins work. */
  do { } while (! (HIB_CTL_WRC & HIB->CTL));
  HIB->CTL &= ~HIB_CTL_RETCLR;
  do { } while (! (HIB_CTL_WRC & HIB->CTL));
#endif /* TM4C129 */

  reset_cause = SYSCTL->RESC;
  SYSCTL->RESC = reset_cause;
  hib_ris = HIB->RIS;

  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  printf("System clock %lu Hz\n", SystemCoreClock);
  {
    int i = 0;
    printf("HIB RIS %x, reset cause %lx:", hib_ris, reset_cause);
    while (i < (sizeof(xResetCause)/sizeof(*xResetCause))) {
      const sResetCause * const rcp = xResetCause + i;
      if (rcp->mask & reset_cause) {
        printf(" %s", rcp->name);
      }
      ++i;
    }
    putchar('\n');
  }
  do { } while (! (HIB_CTL_WRC & HIB->CTL));
  if (MAGIC == retained_state->magic) {
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    ++retained_state->boots;
    printf("Retained state %d at %p: %lx magic, %lu boots, %lu hibdelay\n",
           (MAGIC == retained_state->magic),
           retained_state, retained_state->magic,
           retained_state->boots, retained_state->hibdelay);
  }
  printf("Powerup HIB: RTCC %lx RTCM0 %lx RTCLD %lx\n"
         "CTL %lx RTCT %lx RTCSS %lx IM %lx\n",
         HIB->RTCC, HIB->RTCM0, HIB->RTCLD,
         HIB->CTL, HIB->RTCT, HIB->RTCSS, HIB->IM);
#if (BSPACM_DEVICE_LINE_TM4C129 - 0)
  printf("CALCTL %lx HIBLOCK %lx\n", HIB->CALCTL, HIB->LOCK);
#endif /* TM4C129 */
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  need_reset = ((SYSCTL_RESC_SW | SYSCTL_RESC_EXT) & reset_cause) || ! (HIB_CTL_CLK32EN & HIB->CTL);
  if (need_reset) {
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    retained_state->magic = MAGIC;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    retained_state->boots = 1;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    retained_state->hibdelay = -1;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    /* Enable and configure the clock */
    HIB->CTL = HIB_CTL_CLK32EN;
#if (BSPACM_DEVICE_LINE_TM4C129 - 0)
    /* On TM4C129, there's a calendar mode which, if enabled, will
     * prevent RTCC from updating.  Clear it since it may have been
     * set by a previous application. */
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->CALCTL = 0;
    /* Also on TM4C129 there's a lock on resetting the counter */
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->LOCK = HIB_LOCK_HIBLOCK_KEY;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->RTCLD = 0;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->LOCK = 1;
#else /* TM4C129 */
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->RTCLD = 0;
#endif /* TM4C129 */
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->RTCM0 = WAKEUP_INTERVAL_S;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->IC = HIB_IC_RTCALT0;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    HIB->IM = HIB_IM_RTCALT0;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    /* Start it running */
    HIB->CTL |= HIB_CTL_RTCEN;
    do { } while (! (HIB_CTL_WRC & HIB->CTL));
    printf("Configured HIB: RTCC %lx RTCM0 %lx RTCLD %lx\n"
           "CTL %lx RTCT %lx RTCSS %lx\n",
           HIB->RTCC, HIB->RTCM0, HIB->RTCLD,
           HIB->CTL, HIB->RTCT, HIB->RTCSS);
  }

  NVIC_ClearPendingIRQ(HIB_IRQn);
  NVIC_EnableIRQ(HIB_IRQn);
  while (1) {
    BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
    uint32_t s0;
    uint32_t ss0;
    unsigned int rtcc;
    unsigned int hib_mis;
    unsigned int hib_irqc;
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      hib_irqc = hib_irqc_v;
      hib_mis = hib_mis_v;
      hib_mis_v = 0;
      do { } while (! (HIB_CTL_WRC & HIB->CTL));
      do {
        s0 = HIB->RTCC;
        ss0 = HIB->RTCSS;
      } while ((HIB->RTCSS != ss0) || (HIB->RTCC != s0));
      do { } while (! (HIB_CTL_WRC & HIB->CTL));
      HIB->RTCM0 = s0 + WAKEUP_INTERVAL_S;
      do { } while (! (HIB_CTL_WRC & HIB->CTL));
      HIB->IC = HIB_IC_RTCALT0;
    } while (0);
    BSPACM_CORE_REENABLE_INTERRUPT(istate);
    rtcc = (s0 << 15) | (0x7fff & ss0);
    (void)hib_mis;
    printf("%u RTC %x, sleep mode %u\n", hib_irqc, rtcc, sleep_mode);
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      fflush(stdout);
      ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
      switch (sleep_mode) {
        case 0:
          while (! (HIB_RIS_RTCALT0 & HIB->RIS)) {
          }
          ++sleep_mode;
          break;
        case 1:
          BSPACM_CORE_SLEEP();
          ++sleep_mode;
          break;
        case 2:
          BSPACM_CORE_DEEP_SLEEP();
          ++sleep_mode;
          break;
        case 3:
          /* Put in a 2-second delay before going to hibernate, since
           * regaining control in hibernate--to, say, reprogram the
           * board--can be tricky. */
          vBSPACMledSet(1, 1);
          BSPACM_CORE_DELAY_CYCLES(2 * SystemCoreClock);
          vBSPACMledSet(1, 0);
          do {
            if (! (HIB_CTL_VDD3ON & HIB->CTL)) {
              /* To control hibernation and to wakeup with the wake
               * button or RTC, need these additional bits.  Note that
               * RETCLR is a TM4C129 bit that must be set when VDD3ON
               * is set; it does not exist but does not hurt on
               * TM4C123. */
              do { } while (! (HIB_CTL_WRC & HIB->CTL));
              HIB->CTL |= HIB_CTL_VBATSEL_2_5V
                | HIB_CTL_VDD3ON | HIB_CTL_RETCLR
                | HIB_CTL_PINWEN | HIB_CTL_RTCWEN;
            }
            do { } while (! (HIB_CTL_WRC & HIB->CTL));
            HIB->IC = ~0U;
            do { } while (! (HIB_CTL_WRC & HIB->CTL));

            /* NB: You MUST wait for the write t complete after
             * setting the request.  Otherwise the MCU will continue
             * to execute code, including any GPIO settings. */
            HIB->CTL |= HIB_CTL_HIBREQ;
            do { } while (! (HIB_CTL_WRC & HIB->CTL));

          } while (0);
          /* Supposedly unreachable */
          break;
      }
    } while (0);
    BSPACM_CORE_REENABLE_INTERRUPT(istate);
  }
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
