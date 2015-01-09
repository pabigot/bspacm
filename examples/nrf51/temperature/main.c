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
#include <bspacm/periph/dietemp.h>
#include <bspacm/newlib/ioctl.h>
#include <stdio.h>
#include <fcntl.h>

#include "nrf51_bitfields.h"

void main ()
{
  vBSPACMledConfigure();

  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  /* TEMP is accurate only if clock source is HFCLK.  This is true and
   * can be confirmed by disabling the conditional startup. */
  if (1) {
    vBSPACMnrf51_HFCLKSTART();
  }
  bool rc = bBSPACMdietempInitialize();
  printf("Die temp initialization: %s\n", rc ? "good" : "BAD");

  while (1) {
    int temp_cCel = iBSPACMdietemp_cCel();
    int temp_cFahr = (3200 + (9 * temp_cCel) / 5);
    printf("Temp: %d cCel, %d c[Fahr]\n", temp_cCel, temp_cFahr);

    /* NB: If HFCLK is not the crystal oscillator, the "temperature"
     * will rise, perhaps to over 50 Cel.  That this happens less with
     * pca10031 than the older pca10000 suggests a revision 3 fix.
     *
     * Also, the reading is [not
     * calibrated](https://devzone.nordicsemi.com/question/14794/). */
    BSPACM_CORE_DELAY_CYCLES(SystemCoreClock);
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
