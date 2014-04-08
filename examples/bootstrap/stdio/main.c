/* BSPACM - bootstrap/stdio demonstration application
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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void main ()
{
  int ctr;

  vBSPACMledConfigure();

  BSPACM_CORE_ENABLE_INTERRUPT();

  /* NB: setvbuf() has no effect when using newlib-nano */
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  ctr = 0;
  while (1) {
    const int MAX_PER_LINE = 40;
    volatile uint32_t delay = 1000001;

    ++ctr;
    if (MAX_PER_LINE == (ctr % (2 + MAX_PER_LINE))) {
      int ch;

      putchar('\n');
      vBSPACMledSet(BSPACM_LED_GREEN, -1);
      while (0 <= ((ch = getchar()))) {
        vBSPACMledSet(BSPACM_LED_RED, -1);
        printf("read 0x%02x '%c'\n", ch, ch);
      }
      while (--delay) {
      }
      delay = 1000001;
      ctr = 0;
    } else {
      putchar('0' + (ctr % 10));
    }
  }
}
