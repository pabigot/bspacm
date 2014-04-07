/* BSPACM - bootstrap/nop demonstration application
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

#include <bspacm/core.h>
#include <bspacm/utility/led.h>
#include <stdlib.h>

void * volatile ptr;

void main ()
{
  vBSPACMledConfigure();
  ptr = malloc(1016);
  vBSPACMledSet(BSPACM_LED_GREEN, 1);
}
