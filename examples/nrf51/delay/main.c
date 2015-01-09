/* BSPACM - nRF51 timer stuff
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
#include <stdlib.h>
#include <fcntl.h>
#include "nrf_delay.h"

#if (BSPACM_NRF_USE_SD - 0)
#error Application not updated for soft-device use
#endif /* BSPACM_NRF_USE_SD */

volatile unsigned int irqs;
volatile bool do_wake;
void TIMER0_IRQHandler ()
{
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  if (0 == (0x03 & ++irqs)) {
    do_wake = true;
  }
  vBSPACMledSet(irqs & 3, -1);
}

void main ()
{
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("Initial stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* HFCLK starts as the RC oscillator.  Start the crystal for HFCLK,
   * and start one for LFCLK. */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (! (NRF_CLOCK->EVENTS_HFCLKSTARTED && NRF_CLOCK->EVENTS_LFCLKSTARTED)) {
  }

  printf("Post start stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* Timers only work on HFCLK.  Set one up using a zero prescaler
   * (runs at HFCLK), to wake at 1 Hz. */

  /* 32-bit compare in timer mode using undivided HFCLK */
  NRF_TIMER0->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  NRF_TIMER0->PRESCALER = 0;
  NRF_TIMER0->BITMODE = (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos);

  /* Clear previous events, reset counter, raise event at 1Hz */
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->EVENTS_COMPARE[1] = 0;
  NRF_TIMER0->EVENTS_COMPARE[2] = 0;
  NRF_TIMER0->EVENTS_COMPARE[3] = 0;
  NRF_TIMER0->TASKS_CLEAR = 1;

  /* Turn off all shorts */
  NRF_TIMER0->SHORTS = 0;

  /* Start the whole thing off. */
  NRF_TIMER0->TASKS_START = 1;

  do {
    __IO uint32_t * const tc0 = NRF_TIMER0->TASKS_CAPTURE + 0;
    __IO uint32_t * const tc1 = NRF_TIMER0->TASKS_CAPTURE + 1;
    __IO uint32_t * const cc0 = NRF_TIMER0->CC + 0;
    __IO uint32_t * const cc1 = NRF_TIMER0->CC + 1;
    const uint32_t count_limit = (1UL << 11);
    int32_t count = 0;
    uint32_t delta_clk;
    uint32_t delta_us;
    uint32_t delta_ms;
    uint32_t err_ppth;

#define SET_DELTAS(_count,_measure) do {                          \
      uint32_t exp = (_count);                                    \
      delta_clk = *cc1 - *cc0;                                    \
      delta_us = delta_clk >> 4;                                  \
      delta_ms = delta_us / 1000;                                 \
      err_ppth = exp ? (1000 * labs(_measure - exp)) / exp : exp; \
  } while (0)
#define FORMAT_DELTAS "%lu clk ; %lu us ; %lu ms ; err %lu [ppth]"
#define PASS_DELTAS delta_clk, delta_us, delta_ms, err_ppth

    count = 0;
    *tc0 = 1;
    *tc1 = 1;
    SET_DELTAS(0, delta_us);
    printf("No delay: " FORMAT_DELTAS "\n", PASS_DELTAS);

    count = 1;
    *tc0 = 1;
    nrf_delay_us(1);
    *tc1 = 1;
    SET_DELTAS(count, delta_us);
    printf("Inline 1us: " FORMAT_DELTAS "\n", PASS_DELTAS);

    count = 8;
    *tc0 = 1;
    nrf_delay_us(8);
    *tc1 = 1;
    SET_DELTAS(count, delta_us);
    printf("Inline %lu us: " FORMAT_DELTAS "\n", count, PASS_DELTAS);

    count = 64;
    *tc0 = 1;
    nrf_delay_us(64);
    *tc1 = 1;
    SET_DELTAS(count, delta_us);
    printf("Inline %lu us: " FORMAT_DELTAS "\n", count, PASS_DELTAS);

    count = 512;
    *tc0 = 1;
    nrf_delay_us(512);
    *tc1 = 1;
    SET_DELTAS(count, delta_us);
    printf("Inline %lu us: " FORMAT_DELTAS "\n", count, PASS_DELTAS);

    /* nrf_delay_us does not work with zero count */
    count = 1;
    do {
      *tc0 = 1;
      nrf_delay_us(count);
      *tc1 = 1;
      SET_DELTAS(count, delta_us);
      printf("delay %lu us : " FORMAT_DELTAS "\n", count, PASS_DELTAS);
      if (0 == count) {
        count = 1;
      } else {
        count <<= 1;
      }
    } while (count <= count_limit);

    count = 1;
    do {
      *tc0 = 1;
      nrf_delay_ms(count);
      *tc1 = 1;
      SET_DELTAS(count, delta_ms);
      printf("delay %lu ms : " FORMAT_DELTAS "\n", count, PASS_DELTAS);
      if (0 == count) {
        count = 1;
      } else {
        count <<= 1;
      }
    } while (count <= count_limit);

  } while (0);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
