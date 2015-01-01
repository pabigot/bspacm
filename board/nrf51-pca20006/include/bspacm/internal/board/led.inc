/* BSPACM - LED configuration include file for nRF51-DK (PCA10028)
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

/* LED locations for PCA20006 "nRF51822-Beacon" */
#ifndef BSPACM_LED_RED
#define BSPACM_LED_RED 0
#define BSPACM_LED_GREEN 1
#define BSPACM_LED_BLUE 2
#endif /* BSPACM_LED_RED */

/* Red LED on P0.16 */
BSPACM_INC_EXPAND_LED_REFERENCE(16)
/* Green LED on P0.12 */
BSPACM_INC_EXPAND_LED_REFERENCE(12)
/* Blue LED on P0.15 */
BSPACM_INC_EXPAND_LED_REFERENCE(15)

/*
 * Local Variables:
 * mode:c
 * End:
 */
