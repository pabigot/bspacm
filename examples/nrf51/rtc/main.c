/* BSPACM - nRF51 rtc stuff
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
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#if (BSPACM_NRF_USE_SD - 0)
#error Application does not support soft-device
#endif /* BSPACM_NRF_USE_SD */

const uint32_t UPTIME_Hz = 32768;
NRF_RTC_Type * const uptime_rtc = NRF_RTC0;
const int uptime_ccidx_1Hz = 0;

volatile uint32_t uptime_s;
volatile uint32_t rtc_overflows;

uint32_t rtc_now_last_overflow;
inline uint64_t rtc_now ()
{
  uint32_t prev_ofl;
  uint32_t ctr24;
  do {
    prev_ofl = rtc_now_last_overflow;
    ctr24 = uptime_rtc->COUNTER;
    rtc_now_last_overflow = rtc_overflows;
  } while (prev_ofl != rtc_now_last_overflow);
  return (((uint64_t)rtc_now_last_overflow) << 24) | ctr24;
}

void RTC0_IRQHandler ()
{
  if (uptime_rtc->EVENTS_OVRFLW) {
    uptime_rtc->EVENTS_OVRFLW = 0;
    ++rtc_overflows;
  }
  if (uptime_rtc->EVENTS_COMPARE[uptime_ccidx_1Hz]) {
    uptime_rtc->EVENTS_COMPARE[uptime_ccidx_1Hz] = 0;
    uptime_rtc->CC[uptime_ccidx_1Hz] += UPTIME_Hz;
    ++uptime_s;
  }
}

void main ()
{
  uint32_t uptime = ~0;
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("Initial stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* LFCLK starts as the RC oscillator.  Start the crystal . */
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (! NRF_CLOCK->EVENTS_LFCLKSTARTED) {
  }

  printf("Post start stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* RTC only works on LFCLK.  Set one up using a zero prescaler (runs
   * at 32 KiHz).  Clock's 24 bits, so track overflow in a 32-bit
   * external counter, giving us 56 bits for the full clock (span 2^41
   * seconds, roughly 69 thousand years).
   *
   * Also enable a comparison that will fire an event at 1Hz.  We
   * don't use these events in PPI so don't enable event routing; we
   * do need the interrupts, though. */
  uptime_rtc->TASKS_STOP = 1;
  uptime_rtc->TASKS_CLEAR = 1;
  uptime_rtc->EVTENCLR = ~0;
  uptime_rtc->INTENCLR = ~0;
  uptime_rtc->CC[uptime_ccidx_1Hz] = UPTIME_Hz;
  uptime_rtc->INTENSET = ((RTC_INTENSET_OVRFLW_Enabled << RTC_INTENSET_OVRFLW_Pos)
                          | (RTC_INTENSET_COMPARE0_Enabled << (RTC_INTENSET_COMPARE0_Pos + uptime_ccidx_1Hz)));

  NVIC_ClearPendingIRQ(RTC0_IRQn);
  NVIC_EnableIRQ(RTC0_IRQn);

  uptime_rtc->TASKS_START = 1;

  while (1) {
    uint64_t now;
    uint32_t new_uptime = uptime_s;

    while (new_uptime == uptime) {
      __WFE();
      new_uptime = uptime_s;
    }
    now = rtc_now();
    uptime = new_uptime;
    printf("Uptime %lu counter %" PRIx32 " counts %lu\n", uptime, (uint32_t)now, (uint32_t)(now / UPTIME_Hz));
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
