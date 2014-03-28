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
