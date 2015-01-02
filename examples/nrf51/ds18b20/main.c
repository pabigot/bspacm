/* BSPACM - nRF51 DS18B20 OneWire example
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
#include <bspacm/utility/misc.h>
#include <bspacm/utility/onewire.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

volatile bool cc0_timeout;

void TIMER0_IRQHandler ()
{
  if (NRF_TIMER0->EVENTS_COMPARE[0]) {
    cc0_timeout = true;
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  }
}

void
delay_us (uint32_t count_us)
{
  /* The compare initialization sequence takes about 27 (16 MHz) ticks
   * using an undivided clock, so if the desired delay is less than 3
   * us we could miss the compare.  Use the busy-wait delay if a
   * conservative check for duration fails. */
  const uint32_t min_wfe_delay_us = 4;

  if (min_wfe_delay_us > count_us) {
    nrf_delay_us(count_us);
    return;
  }
  cc0_timeout = false;
  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  NRF_TIMER0->CC[0] += count_us << 4;
  while (! cc0_timeout) {
    __WFE();
  }
  NRF_TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk;
  return;
}

#ifndef ONEWIRE_DQ_PIN
#define ONEWIRE_DQ_PIN 29
#endif /* ONEWIRE_DQ_PIN */
#ifndef ONEWIRE_PWR_PIN
#define ONEWIRE_PWR_PIN 0
#endif /* ONEWIRE_PWR_PIN */

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

  /* Timers only work on HFCLK.  Set up to clock at full speed 16 MHz,
   * using a 32-bit timer so we can handle delays that are much longer
   * than we really need for this application. */
  NRF_TIMER0->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  NRF_TIMER0->PRESCALER = 0;
  NRF_TIMER0->BITMODE = (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos);

  /* Enable interrupts (thus event wakeup?) at the peripheral, but not
   * at the NVIC */
  NRF_TIMER0->INTENCLR = ~0;
  NRF_TIMER0->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);

  /* Clear the counter and start things going */
  NRF_TIMER0->TASKS_CLEAR = 1;
  NRF_TIMER0->TASKS_START = 1;

  do {
    sBSPACMonewireBus bus_config;
    hBSPACMonewireBus bus = hBSPACMonewireConfigureBus(&bus_config, ONEWIRE_DQ_PIN, ONEWIRE_PWR_PIN);

    if (! iBSPACMonewireReset(bus)) {
      printf("ERR: No DS18B20 present on P0.%u\n", ONEWIRE_DQ_PIN);
      break;
    }

    static const char * const supply_type[] = { "parasitic", "external" };

    int external_power = iBSPACMonewireReadPowerSupply(bus);
    printf("Power supply: %s\n", supply_type[external_power]);
    if (0 > external_power) {
      printf("ERROR: Device not present?\n");
      break;
    }

    sBSPACMonewireSerialNumber serial;
    int rc = iBSPACMonewireReadSerialNumber(bus, &serial);
    printf("Serial got %d: ", rc);
    vBSPACMconsoleDisplayOctets(serial.id, sizeof(serial.id));
    putchar('\n');

    while (0 == iBSPACMonewireRequestTemperature(bus)) {
      if (external_power) {
        /* Wait for read to complete.  Conversion time can be as long as
         * 750 ms if 12-bit resolution is used (this resolution is the
         * default). Timing will be wrong unless interrupts are enabled
         * so uptime overflow events can be handled.  Sleep for 600ms,
         * then test at 10ms intervals until the result is ready. */
        delay_us(600 * 1000UL);
        while (! iBSPACMonewireReadBit(bus)) {
          delay_us(10 * 1000UL);
        }
      } else {
        /* Output high on the parasitic power boost line for 750ms, to
         * power the conversion.  Then switch that signal back to
         * input so the data can flow over the same circuit. */
        vBSPACMonewireParasitePower(bus, true);
        delay_us(750 * 1000UL);
        vBSPACMonewireParasitePower(bus, false);
      }
      int16_t t_xCel = -1;
      rc = iBSPACMonewireReadTemperature(bus, &t_xCel);
      vBSPACMonewireShutdown(bus);
      printf("Got %d xCel, %d dCel, %d d[degF], %d dK\n",
             t_xCel,
             BSPACM_ONEWIRE_xCel_TO_dCel(t_xCel),
             BSPACM_ONEWIRE_xCel_TO_ddegF(t_xCel),
             BSPACM_ONEWIRE_xCel_TO_dK(t_xCel));
      delay_us(1000 * 1000);
    }

  } while (0);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
