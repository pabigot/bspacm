/* BSPACM - bootstrap/stdio demonstration application
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

/* This application simply configures some number of buttons, and
 * increments a counter each time one is pressed. */

#include <bspacm/utility/led.h>
#include <bspacm/periph/gpio.h>
#include <string.h>
#include <em_gpio.h>
#include <em_emu.h>
#include <stdio.h>

/* Configure as plain input with no filter (DOUT=0).  For STK boards,
 * there's a pull-up, and a 1ms RC filter, so we don't need either of
 * those features.  If there was no filter, then set DOUT=1 so the
 * built-in 50ns filter is available for deglitch.  If there's also no
 * pull-up, then gpioModeInputPullFilter should be used.
 *
 * gpioModeInputPull would be used if you want pullup/pulldown without
 * a filter.  Not sure why you'd want that for an external
 * interrupt. */
const sBSPACMdeviceEFM32pinmux button[] = {
#if (BSPACM_BOARD_EFM32GG_STK3700 - 0)
  { .port = GPIO->P + gpioPortB, .pin = 9, .mode = gpioModeInput },
  { .port = GPIO->P + gpioPortB, .pin = 10, .mode = gpioModeInput },
#else
#error no button configuration available
#endif
};
static const unsigned int nbutton = sizeof(button)/sizeof(*button);

volatile unsigned int count_v[sizeof(button)/sizeof(*button)];
volatile uint32_t flags_v;

static void
irqHandler ()
{
  unsigned int pending = GPIO->IF & GPIO->IEN;
  int i;
  for (i = 0; i < nbutton; ++i) {
    const int pin = button[i].pin;
    const uint32_t mask = 1U << pin;
    if (mask & pending) {
      GPIO->IFC = mask;
      BSPACM_CORE_BITBAND_SRAM32(flags_v, i) = 1;
      count_v[i] += 1;
    }
  }
}

void GPIO_ODD_IRQHandler(void) { irqHandler(); }
void GPIO_EVEN_IRQHandler(void) { irqHandler(); }

void main ()
{
  vBSPACMledConfigure();
  SystemCoreClockUpdate();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  printf("There are %u buttons.  Press them.\n", nbutton);

  {
    BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
    int i;

    BSPACM_CORE_DISABLE_INTERRUPT();
    for (i = 0; i < nbutton; ++i) {
      const unsigned int port = button[i].port - GPIO->P;
      const unsigned int pin = button[i].pin;
      const uint32_t mask = 1U << pin;

      vBSPACMdeviceEFM32pinmuxConfigure(button+i, 1, 0);
      vBSPACMcoreSetPinNybble(&GPIO->EXTIPSELL, pin, port);
      GPIO->EXTIFALL |= mask;
      GPIO->EXTIRISE &= ~mask;
      GPIO->IFC = mask;
      GPIO->IEN |= mask;
      NVIC_ClearPendingIRQ((1 & pin) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn);
      NVIC_EnableIRQ((1 & pin) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn);
    }
    BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  }
  BSPACM_CORE_ENABLE_CYCCNT();
  printf("SCR SCB %lx\n", SCB->SCR);

  while (1) {
    unsigned int count[sizeof(count_v)/sizeof(*count_v)];
    unsigned int flags;
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      memcpy(count, (void*)count_v, sizeof(count_v));
      flags = flags_v;
      flags_v = 0;
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();
    if (flags) {
      printf("Count %u %u ; flags %x\n", count[0], count[1], flags);
    }
    BSPACM_CORE_SLEEP();
  }
}
