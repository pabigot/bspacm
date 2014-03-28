/* LED locations for EFM32TG-STK3300 "Tiny Gecko" starter kit */
#ifndef BSPACM_LED_YELLOW
#define BSPACM_LED_YELLOW 0
/* No green or red LEDs, but standard API expects something to serve
 * this role.  Unfortunate there's only one LED so they can't be
 * distinguished. */
#define BSPACM_LED_RED BSPACM_LED_YELLOW
#define BSPACM_LED_GREEN BSPACM_LED_YELLOW
#endif /* BSPACM_LED_YELLOW */
/* Yellow LED on PD7 */
BSPACM_INC_EXPAND_LED_REFERENCE(gpioPortD, 7)

/*
 * Local Variables:
 * mode:c
 * End:
 */
