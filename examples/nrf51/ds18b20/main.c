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
#include <bspacm/utility/hires.h>
#include <bspacm/utility/onewire.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifndef ONEWIRE_DQ_PIN
#define ONEWIRE_DQ_PIN 29
#endif /* ONEWIRE_DQ_PIN */
#ifndef ONEWIRE_PWR_PIN
#define ONEWIRE_PWR_PIN 0
#endif /* ONEWIRE_PWR_PIN */

void main ()
{
  int rc;

  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  /* Configure high-resolution timer at 1 MHz */
  rc = iBSPACMhiresInitialize(1000U * 1000U);
  printf("Hires initialize got %d, divides %u\n", rc, 1U << BSPACM_HIRES_TIMER->PRESCALER);
  printf("1 hrt = %u hfclk = %u ns\n", uiBSPACMhiresConvert_hrt_hfclk(1), uiBSPACMhiresConvert_hrt_us(1000));
  printf("128 us = %u hrt\n", uiBSPACMhiresConvert_us_hrt(128));
  vBSPACMhiresSetEnabled(true);

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
        vBSPACMhiresSleep_ms(600);
        while (! iBSPACMonewireReadBit(bus)) {
          vBSPACMhiresSleep_ms(10);
        }
      } else {
        /* Output high on the parasitic power boost line for 750ms, to
         * power the conversion.  Then switch that signal back to
         * input so the data can flow over the same circuit. */
        vBSPACMonewireParasitePower(bus, true);
        vBSPACMhiresSleep_ms(750);
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
      vBSPACMhiresSleep_ms(1000);
    }

  } while (0);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
