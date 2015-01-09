/* BSPACM - demonstrate buttons and uptime sleep w/interrupt
 *
 * Even buttons toggle and display at 1 Hz.  Odd buttons same but
 * wakeup immediately.
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

#include <bspacm/utility/led.h>
#include <bspacm/utility/uptime.h>
#include <bspacm/newlib/ioctl.h>
#include <stdio.h>
#include <fcntl.h>

#include "nrf51_bitfields.h"

const uint8_t button_pin[] = {
#if (BSPACM_BOARD_NRF51_PCA10028 - 0)
  17, 18, 19, 20
#elif (BSPACM_BOARD_NRF51_PCA10031 - 0)
#error PCA10031 has no buttons
#elif (BSPACM_BOARD_NRF51_PCA20006 - 0)
  8, 18
#elif (BSPACM_BOARD_NRF51_PCA10001 - 0)
  16, 17
#elif (BSPACM_BOARD_NRF51_PCA10000 - 0)
#error PCA10000 has no buttons
#else /* BSPACM_BOARD */
#error No button configuration available
#endif /* BSPACM_BOARD */
};
const uint8_t nbuttons = sizeof(button_pin) / sizeof(*button_pin);
volatile uint8_t button_changed;
volatile uint8_t button_state;

#define MAX_GPIOTE (sizeof(NRF_GPIOTE->CONFIG) / sizeof(*NRF_GPIOTE->CONFIG))

volatile unsigned int irqs;

void
GPIOTE_IRQHandler (void)
{
  int b;
  vBSPACMledSet(0, -1);
  ++irqs;
  for (b = 0; b < MAX_GPIOTE; ++b) {
    if (NRF_GPIOTE->EVENTS_IN[b]) {
      uint8_t bbit = (1U << b);
      uint32_t pbit = (1U << button_pin[b]);
      NRF_GPIOTE->EVENTS_IN[b] = 0;
      button_changed |= bbit;
      if (NRF_GPIO->IN & pbit) { /* active low */
        button_state &= ~bbit;
      } else {
        button_state |= bbit;
      }
      if (1 & b) {
        vBSPACMuptimeSleepCancel();
      }
    }
  }
}

void main ()
{
  int b;

  vBSPACMledConfigure();
  vBSPACMuptimeStart();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  NRF_GPIOTE->INTENCLR = ~0U;
  NRF_GPIOTE->EVENTS_PORT = 0;
  for (b = 0; (b < nbuttons) && (b < MAX_GPIOTE); ++b) {
    int pin = 0x1F & button_pin[b];
    uint8_t bbit = (1UL << b);
    printf("Button %d on P0.%02u bit %x\n", b, pin, bbit);
    NRF_GPIO->PIN_CNF[pin] = 0
      | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
      ;
    NRF_GPIOTE->CONFIG[b] = 0
      | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)
      | (pin << GPIOTE_CONFIG_PSEL_Pos)
      | (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)
      ;
    NRF_GPIOTE->EVENTS_IN[b] = 0;
    NRF_GPIOTE->INTENSET = bbit;
  }
  /* Enable the interrupts at the NVIC */
  vBSPACMnrf51_NVIC_ClearPendingIRQ(GPIOTE_IRQn);
  vBSPACMnrf51_NVIC_EnableIRQ(GPIOTE_IRQn);

  while (1) {
    bool fns = bBSPACMuptimeSleep(BSPACM_UPTIME_Hz);
    /* Yes, this is a race condition.  No, I don't care; it's an
     * example. */
    uint8_t chgd = button_changed;
    button_changed = 0;
    for (b = 0; (b < nbuttons) && (b < MAX_GPIOTE); ++b) {
      uint32_t pbit = (1UL << button_pin[b]);

      putchar(NRF_GPIO->IN & pbit ? ' ' : 'P');
    }
    printf(" -- st %x ; chg %x ; irqs %u ; full sleep %d\n",
           button_state, chgd, irqs, fns);
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
