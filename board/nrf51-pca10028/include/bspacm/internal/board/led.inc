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

/* LED locations for PCA10028 "nRF51-DK" */
#ifndef BSPACM_LED_YELLOW
#define BSPACM_LED_YELLOW 0
#define BSPACM_LED_YELLOW1 1
#define BSPACM_LED_YELLOW2 2
#define BSPACM_LED_YELLOW3 3
/* No green or red LEDs, but standard API expects something to serve
 * this role */
#define BSPACM_LED_RED BSPACM_LED_YELLOW
#define BSPACM_LED_GREEN BSPACM_LED_YELLOW1
#endif /* BSPACM_LED_YELLOW */

/* Yellow LED on P0.21 */
BSPACM_INC_EXPAND_LED_REFERENCE(21)
/* Yellow LED on P0.22 */
BSPACM_INC_EXPAND_LED_REFERENCE(22)
/* Yellow LED on P0.23 */
BSPACM_INC_EXPAND_LED_REFERENCE(23)
/* Yellow LED on P0.24 */
BSPACM_INC_EXPAND_LED_REFERENCE(24)

/*
 * Local Variables:
 * mode:c
 * End:
 */
