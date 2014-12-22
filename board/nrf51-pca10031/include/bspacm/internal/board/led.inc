/* BSPACM - LED configuration include file for nRF51-Dongle (PCA10031)
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

/* LED locations for PCA10031 "nRF51-Dongle" */
#ifndef BSPACM_LED_RED
#define BSPACM_LED_RED 0
#define BSPACM_LED_GREEN 1
#define BSPACM_LED_BLUE 2
#endif /* BSPACM_LED_RED */

/* Red LED on P0.21 */
BSPACM_INC_EXPAND_LED_REFERENCE(21)
/* Green LED on P0.22 */
BSPACM_INC_EXPAND_LED_REFERENCE(22)
/* Blue LED on P0.23 */
BSPACM_INC_EXPAND_LED_REFERENCE(23)

/*
 * Local Variables:
 * mode:c
 * End:
 */
