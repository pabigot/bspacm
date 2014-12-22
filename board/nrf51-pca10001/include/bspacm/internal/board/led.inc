/* BSPACM - LED configuration include file for nRF51822-EK (PCA10001)
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

/* LED locations for PCA10001 "nRF51822-EK" */
#ifndef BSPACM_LED_YELLOW
#define BSPACM_LED_YELLOW 0
#define BSPACM_LED_YELLOW1 1
/* No green or red LEDs, but standard API expects something to serve
 * this role */
#define BSPACM_LED_RED BSPACM_LED_YELLOW
#define BSPACM_LED_GREEN BSPACM_LED_YELLOW1
#endif /* BSPACM_LED_YELLOW */

/* Yellow LED on P0.18 */
BSPACM_INC_EXPAND_LED_REFERENCE(18)
/* Yellow LED on P0.19 */
BSPACM_INC_EXPAND_LED_REFERENCE(19)

/*
 * Local Variables:
 * mode:c
 * End:
 */
