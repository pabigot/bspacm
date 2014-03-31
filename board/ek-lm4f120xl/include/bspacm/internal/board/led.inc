/* BSPACM - LED configuration include file for ek-lm4f120xl
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

/* LED locations for EK-LM4F120XL "Stellaris LaunchPad" */
#ifndef BSPACM_LED_RED
#define BSPACM_LED_RED 0
#define BSPACM_LED_BLUE 1
#define BSPACM_LED_GREEN 2
#endif /* BSPACM_LED_RED */
/* LEDs on PF1, PF2, PF3 */
BSPACM_INC_EXPAND_LED_CONFIGURE(GPIOF, GPIOF, (1U << 1) | (1U << 2) | (1U << 3))
BSPACM_INC_EXPAND_LED_REFERENCE(GPIOF,1) /* Red on PF1 */
BSPACM_INC_EXPAND_LED_REFERENCE(GPIOF,2) /* Blue on PF2 */
BSPACM_INC_EXPAND_LED_REFERENCE(GPIOF,3) /* Green on PF3 */

/*
 * Local Variables:
 * mode:c
 * End:
 */
