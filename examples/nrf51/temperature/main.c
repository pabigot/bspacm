/* BSPACM - nRF51 die temperature example
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

void main ()
{
  vBSPACMledConfigure();

  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  /* TEMP is accurate only if clock source is HFCLK.  Confirm this. */
  {
    uint32_t hfclkstat = NRF_CLOCK->HFCLKSTAT;
    printf("HFCLK %08lx: Source %u MHz, %srunning\n", hfclkstat,
           (unsigned int)(16 << ((hfclkstat & CLOCK_HFCLKSTAT_SRC_Msk) / CLOCK_HFCLKRUN_STATUS_Msk)),
           (hfclkstat & (CLOCK_HFCLKSTAT_STATE_Running << CLOCK_HFCLKSTAT_STATE_Pos)) ? "" : "not ");
  }

  while (1) {

    /* PAN-31: Temperature offset value has to be manually loaded.
     * Address is 0x4000C504, the word before TEMP.  The struct does
     * not have a field for this.
     *
     * Not clear whether this needs to be done before each read.  */
    ((__O uint32_t *)&NRF_TEMP->TEMP)[-1] = 0;

    NRF_TEMP->EVENTS_DATARDY = 0;

    NRF_TEMP->TASKS_START = 1;
    while (! NRF_TEMP->EVENTS_DATARDY) {
    }
    NRF_TEMP->EVENTS_DATARDY = 0;

    int temp_cCel = 0;
    int temp_cFahr = 0;
    {
      /* PAN-29: STOP task clears TEMP register */
      uint32_t temp_raw = NRF_TEMP->TEMP;

      /* PAN-30: TEMP module analog front end does not power down when
       * DATARDY event occurs */
      NRF_TEMP->TASKS_STOP = 1;

      /* PAN-28: Negative measured values are not represented correctly.
       * Sign extension does not go higher than bit 9.
       *
       * Value is 10-bit 2's complement.  Convert to 32-bit 2's
       * complement. */
      const uint32_t sign_bit = 0x0200;
      if (temp_raw & sign_bit) {
        temp_raw |= ~(sign_bit - 1);
      }

      temp_cCel = 25 * (int)(int32_t)temp_raw;
      temp_cFahr = (3200 + (9 * temp_cCel) / 5);
    }
    printf("Temp: %d cCel, %d c[Fahr]\n", temp_cCel, temp_cFahr);

    /* NB: Staying active like this tends to cause the temperature to
     * rise, perhaps to over 50 Cel.  That this happens less with
     * pca10031 than the older pca10000 suggests a revision 3 fix or a
     * better heat sink on the newer board.
     *
     * Also, the reading is [not
     * calibrated](https://devzone.nordicsemi.com/question/14794/). */
    BSPACM_CORE_DELAY_CYCLES(SystemCoreClock);
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
