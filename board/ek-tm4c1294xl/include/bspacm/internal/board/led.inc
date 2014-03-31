/* BSPACM - LED configuration include file for ek-tm4c1294xl
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

/* LED locations for EK-TM4C1294XL "Connected LaunchPad" */
#ifndef BSPACM_LED_GREEN
#define BSPACM_LED_GREEN 0
#define BSPACM_LED_GREEN1 1
#define BSPACM_LED_GREEN2 2
#define BSPACM_LED_GREEN3 3
/* No red LED, but standard API expects something to serve this role */
#define BSPACM_LED_RED BSPACM_LED_GREEN1
#endif /* BSPACM_LED_GREEN */

BSPACM_INC_EXPAND_LED_CONFIGURE(GPION, GPION, (1U << 1) | (1U << 0))
BSPACM_INC_EXPAND_LED_CONFIGURE(GPIOF, GPIOF_AHB, (1U << 4) | (1U << 0))
BSPACM_INC_EXPAND_LED_REFERENCE(GPION,1) /* green on PN1 */
BSPACM_INC_EXPAND_LED_REFERENCE(GPION,0) /* green on PN0 */
BSPACM_INC_EXPAND_LED_REFERENCE(GPIOF_AHB,4) /* green on PF4 */
BSPACM_INC_EXPAND_LED_REFERENCE(GPIOF_AHB,0) /* green on PF0 */

/*
 * Local Variables:
 * mode:c
 * End:
 */
