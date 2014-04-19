/* BSPACM - tm4c/button demonstration application
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
#include <fcntl.h>
#include <bspacm/newlib/ioctl.h>
#include <string.h>
#include <stdio.h>

/* PA 0    PB 1    PC 2    PD 3
 * PE 4    PF 5    PG 6    PH 7
 * *PJ 8   PK 9    PL 10   PM 11
 * PN 12   *PP 13  PQ 14   PR 15
 * PS 16   PT 17
 */
const sBSPACMdeviceTM4Cpinmux button[] = {
#if (BSPACM_BOARD_EK_TM4C123GXL - 0)
  { .port = GPIOF, .port_shift = 5, .pin = 4, .pctl = 0, .irqn = GPIOF_IRQn },
  { .port = GPIOF, .port_shift = 5, .pin = 0, .pctl = 0, .irqn = GPIOF_IRQn },
#else
  { .port = GPIOJ_AHB, .port_shift = 8, .pin = 0, .pctl = 0, .irqn = GPIOJ_IRQn },
  { .port = GPIOJ_AHB, .port_shift = 8, .pin = 1, .pctl = 0, .irqn = GPIOJ_IRQn },
#endif
};
static const unsigned int nbutton = sizeof(button)/sizeof(*button);

volatile unsigned int count_v[sizeof(button)/sizeof(*button)];
volatile uint32_t flags_v;

static void
irqHandler (GPIOCommon_Type * gpio)
{
  unsigned int pending = gpio->MIS;
  gpio->ICR = pending;
  int i;
  for (i = 0; i < nbutton; ++i) {
    GPIOCommon_Type * const bgpio = (GPIOCommon_Type *)button[i].port;
    const int pin = button[i].pin;
    const uint32_t mask = 1U << pin;
    if ((bgpio == gpio) && (mask & pending)) {
      BSPACM_CORE_BITBAND_SRAM32(flags_v, i) = 1;
      count_v[i] += 1;
    }
  }
}

#if ((BSPACM_BOARD_EK_TM4C123GXL - 0) || (BSPACM_BOARD_EK_LM4F120XL - 0))
void GPIOF_IRQHandler(void) { irqHandler(GPIOF); }
#elif (BSPACM_BOARD_EK_TM4C1294XL - 0)
void GPIOJ_IRQHandler(void) { irqHandler(GPIOJ_AHB); }
#else
#error no button configuration available
#endif

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
      GPIOCommon_Type * const gpio = (GPIOCommon_Type *)button[i].port;
      const unsigned int pin = button[i].pin;
      const unsigned int irqn = button[i].irqn;

      BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, button[i].port_shift) = 1;
      vBSPACMdeviceTM4CpinmuxConfigure(button+i, 1, 0);
      BSPACM_CORE_BITBAND_PERIPH(gpio->IM, pin) = 0; /* mask */
      BSPACM_CORE_BITBAND_PERIPH(gpio->IS, pin) = 0; /* edge-sense */
      BSPACM_CORE_BITBAND_PERIPH(gpio->IEV, pin) = 0; /* falling edge */
      BSPACM_CORE_BITBAND_PERIPH(gpio->IBE, pin) = 0; /* not also rising edge */
      BSPACM_CORE_BITBAND_PERIPH(gpio->PUR, pin) = 1; /* pull-up enabled */
      BSPACM_CORE_BITBAND_PERIPH(gpio->ICR, pin) = 1; /* clear previous interrupt */
      BSPACM_CORE_BITBAND_PERIPH(gpio->IM, pin) = 1;  /* unmask interrupt */
      NVIC_ClearPendingIRQ(irqn);
      NVIC_EnableIRQ(irqn);
    }
    BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  }
  BSPACM_CORE_ENABLE_CYCCNT();

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
