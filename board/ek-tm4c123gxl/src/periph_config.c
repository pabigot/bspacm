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

#if (BSPACM_CONFIG_ENABLE_UART - 0)

/* PA 0    PB 1    PC 2    PD 3
 * PE 4    PF 5    PG 6    PH 7
 * *PJ 8   PK 9    PL 10   PM 11
 * PN 12   *PP 13  PQ 14   PR 15
 * PS 16   PT 17
 */

#if (BSPACM_CONFIG_ENABLE_UART0 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART0devcfg = {
  /* UART0 is connected to the virtual COM port via PA0 and PA1 */
  .uart_base = UART0_BASE,
  .rx_pinmux = { .port = GPIOA, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOA, .pin = 1, .pctl = 1 },
  .irqn = UART0_IRQn,
  .instance = 0
};

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

#endif /* BSPACM_CONFIG_ENABLE_UART0 */

#if (BSPACM_CONFIG_ENABLE_UART1 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART1devcfg = {
  /* UART1 is available on A.3(RX=PB0), A.4(TX=PB1), D.4(RTS=PC4), D.5(CTS=PC5).
   * Alternatively some functions are on D.4 and D.5 */
  .uart_base = UART1_BASE,
  .rx_pinmux = { .port = GPIOB, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOB, .pin = 1, .pctl = 1 },
  .rts_pinmux = { .port = GPIOC, .pin = 4, .pctl = 8 },
  .cts_pinmux = { .port = GPIOC, .pin = 5, .pctl = 8 },
  .irqn = UART1_IRQn,
  .instance = 1
};

#ifdef BSPACM_PERIPH_UART1_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART1_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART1_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART1_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART1_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART1_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 1
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART1 */

#if (BSPACM_CONFIG_ENABLE_UART2 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART2devcfg = {
  /* UART2 is available on D.8(RX=PD6), D.9(TX=PD7). */
  .uart_base = UART2_BASE,
  .rx_pinmux = { .port = GPIOD, .pin = 6, .pctl = 1 },
  .tx_pinmux = { .port = GPIOD, .pin = 7, .pctl = 1 }, /* NMI */
  .irqn = UART2_IRQn,
  .instance = 2
};

#ifdef BSPACM_PERIPH_UART2_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART2_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART2_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART2_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART2_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART2_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 2
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART2 */

#if (BSPACM_CONFIG_ENABLE_UART3 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART3devcfg = {
  /* UART3 is available on D.6(RX=PC6), D.7(TX=PC7). */
  .uart_base = UART3_BASE,
  .rx_pinmux = { .port = GPIOC, .pin = 6, .pctl = 1 },
  .tx_pinmux = { .port = GPIOC, .pin = 7, .pctl = 1 },
  .irqn = UART3_IRQn,
  .instance = 3
};

#ifdef BSPACM_PERIPH_UART3_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART3_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART3_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART3_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART3_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART3_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 3
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART3 */

#if (BSPACM_CONFIG_ENABLE_UART4 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART4devcfg = {
  /* UART4 is available on D.4(RX=PC4), D.5(TX=PC5). */
  .uart_base = UART4_BASE,
  .rx_pinmux = { .port = GPIOC, .pin = 4, .pctl = 1 },
  .tx_pinmux = { .port = GPIOC, .pin = 5, .pctl = 1 },
  .irqn = UART4_IRQn,
  .instance = 4
};

#ifdef BSPACM_PERIPH_UART4_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART4_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART4_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART4_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART4_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART4_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 4
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART4 */

#if (BSPACM_CONFIG_ENABLE_UART5 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART5devcfg = {
  /* UART5 is available on A.5(RX=PE4), A.6(TX=PE5). */
  .uart_base = UART5_BASE,
  .rx_pinmux = { .port = GPIOE, .pin = 4, .pctl = 1 },
  .tx_pinmux = { .port = GPIOE, .pin = 5, .pctl = 1 },
  .irqn = UART5_IRQn,
  .instance = 5
};

#ifdef BSPACM_PERIPH_UART5_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART5_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART5_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART5_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART5_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART5_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 5
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART5 */

/* UART6 is on PD4 and PD5 which are not brought out to boosterpack headers. */
#if (BSPACM_CONFIG_ENABLE_UART7 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART7devcfg = {
  /* UART7 is available on B.3(RX=PE0), C.7(TX=PE1). */
  .uart_base = UART7_BASE,
  .rx_pinmux = { .port = GPIOE, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOE, .pin = 1, .pctl = 1 },
  .irqn = UART7_IRQn,
  .instance = 7
};

#ifdef BSPACM_PERIPH_UART7_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART7_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART7_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART7_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART7_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART7_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 7
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART7 */

/** Override the weak default that references no peripheral */
const hBSPACMperiphUART hBSPACMdefaultUART =
#if (BSPACM_CONFIG_DEFAULT_UART0 - 0)
  &xBSPACMdeviceTM4CperiphUART0
#elif (BSPACM_CONFIG_DEFAULT_UART1 - 0)
  &xBSPACMdeviceTM4CperiphUART1
#elif (BSPACM_CONFIG_DEFAULT_UART2 - 0)
  &xBSPACMdeviceTM4CperiphUART2
#elif (BSPACM_CONFIG_DEFAULT_UART3 - 0)
  &xBSPACMdeviceTM4CperiphUART3
#elif (BSPACM_CONFIG_DEFAULT_UART4 - 0)
  &xBSPACMdeviceTM4CperiphUART4
#elif (BSPACM_CONFIG_DEFAULT_UART5 - 0)
  &xBSPACMdeviceTM4CperiphUART5
#elif (BSPACM_CONFIG_DEFAULT_UART6 - 0)
  &xBSPACMdeviceTM4CperiphUART6
#elif (BSPACM_CONFIG_DEFAULT_UART7 - 0)
  &xBSPACMdeviceTM4CperiphUART7
#else
#error no specified default UART
#endif
  ;

#endif /* BSPACM_CONFIG_ENABLE_UART */
