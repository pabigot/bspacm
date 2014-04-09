/* BSPACM - LED configuration include file for efm32zg-stk3200
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

/* LED locations for EFM32ZG-STK3200 "Zero Gecko" starter kit */
#ifndef BSPACM_LED_YELLOW
#define BSPACM_LED_YELLOW 0
#define BSPACM_LED_YELLOW1 1
/* No green or red LEDs, but standard API expects something to serve
 * this role */
#define BSPACM_LED_RED BSPACM_LED_YELLOW
#define BSPACM_LED_GREEN BSPACM_LED_YELLOW1
#endif /* BSPACM_LED_YELLOW */
/* Yellow LEDs on PC10 and PC11 */
BSPACM_INC_EXPAND_LED_REFERENCE(gpioPortC, 10)
BSPACM_INC_EXPAND_LED_REFERENCE(gpioPortC, 11)

/*
 * Local Variables:
 * mode:c
 * End:
 */
