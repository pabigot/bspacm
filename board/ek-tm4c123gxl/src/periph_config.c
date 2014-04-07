/* BSPACM - Board pin mux configuration for EK-TM4C123GXL
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

#include <bspacm/periph/gpio.h>
#include <bspacm/periph/uart.h>
#include <bspacm/internal/utility/fifo.h>

/* PA 0    PB 1    PC 2    PD 3
 * PE 4    PF 5    PG 6    PH 7
 * *PJ 8   PK 9    PL 10   PM 11
 * PN 12   *PP 13  PQ 14   PR 15
 * PS 16   PT 17
 */
const sBSPACMdeviceTM4CpinmuxUART xBSPACMdeviceTM4CpinmuxUART[] = {
  /* UART0 is connected to the virtual COM port via PA0 and PA1 */
  {
    .uart_base = UART0_BASE,
    .rx_pinmux = { .port = GPIOA, .port_shift = 0, .pin = 0, .pctl = 1 },
    .tx_pinmux = { .port = GPIOA, .port_shift = 0, .pin = 1, .pctl = 1 },
    .irqn = UART0_IRQn,
  },
  /* UART1 is available on A.3(RX=PB0), A.4(TX=PB1), D.4(RTS=PC4), D.5(CTS=PC5).
   * Alternatively some functions are on D.4 and D.5 */
  {
    .uart_base = UART1_BASE,
    .rx_pinmux = { .port = GPIOB, .port_shift = 1, .pin = 0, .pctl = 1 },
    .tx_pinmux = { .port = GPIOB, .port_shift = 1, .pin = 1, .pctl = 1 },
    .rts_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 4, .pctl = 8 },
    .cts_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 5, .pctl = 8 },
    .irqn = UART1_IRQn,
  },
  /* UART2 is available on D.8(RX=PD6), D.9(TX=PD7). */
  {
    .uart_base = UART2_BASE,
    .rx_pinmux = { .port = GPIOD, .port_shift = 3, .pin = 6, .pctl = 1 },
    .tx_pinmux = { .port = GPIOD, .port_shift = 3, .pin = 7, .pctl = 1 }, /* NMI */
    .irqn = UART2_IRQn,
  },
  /* UART3 is available on D.6(RX=PC6), D.7(TX=PC7). */
  {
    .uart_base = UART3_BASE,
    .rx_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 6, .pctl = 1 },
    .tx_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 7, .pctl = 1 },
    .irqn = UART3_IRQn,
  },
  /* UART4 is available on D.4(RX=PC4), D.5(TX=PC5). */
  {
    .uart_base = UART4_BASE,
    .rx_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 4, .pctl = 1 },
    .tx_pinmux = { .port = GPIOC, .port_shift = 2, .pin = 5, .pctl = 1 },
    .irqn = UART4_IRQn,
  },
  /* UART5 is available on A.5(RX=PE4), A.6(TX=PE5). */
  {
    .uart_base = UART5_BASE,
    .rx_pinmux = { .port = GPIOE, .port_shift = 4, .pin = 4, .pctl = 1 },
    .tx_pinmux = { .port = GPIOE, .port_shift = 4, .pin = 5, .pctl = 1 },
    .irqn = UART5_IRQn,
  },
  /* UART6 is on PD4 and PD5 which are not brought out to boosterpack headers. */
  /* UART7 is available on B.3(RX=PE0), C.7(TX=PE1). */
  {
    .uart_base = UART7_BASE,
    .rx_pinmux = { .port = GPIOE, .port_shift = 4, .pin = 0, .pctl = 1 },
    .tx_pinmux = { .port = GPIOE, .port_shift = 4, .pin = 1, .pctl = 1 },
    .irqn = UART7_IRQn,
  },
};
const uint8_t nBSPACMdeviceTM4CpinmuxUART = sizeof(xBSPACMdeviceTM4CpinmuxUART)/sizeof(*xBSPACMdeviceTM4CpinmuxUART);

#ifdef BSPACM_PERIPH_UART0_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART0_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART0_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART0_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART0_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART0_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 0
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

/** Override the weak default that references no peripheral */
hBSPACMperiphUART hBSPACMutilityCONSOLEuart = &xBSPACMdeviceTM4CperiphUART0;
