/* BSPACM - Board pin mux configuration for EK-TM4C1294XL
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

/** @file
 *
 * @brief Device pin muxing to support EK-TM4C1294XL Connected Launchpad */

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

/* NB: Boosterpack headers are AB_CD (A/D outermost, B/C innermost). */

#if (BSPACM_CONFIG_ENABLE_UART0 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART0devcfg = {
  .uart_base = UART0_BASE,
  /* JP4/JP5 in UART configuration: virtual COM
   * JP4/JP5 in CAN configuration: A2.3, A2.4 */
  .rx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 1, .pctl = 1 },
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
/* NB: UART1 is not brought out to boosterpack headers. */

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART1devcfg = {
  .uart_base = UART1_BASE,
  /* X11.58 PB0 RX, X11.60 PB1 TX */
  .rx_pinmux = { .port = GPIOB_AHB, .port_shift = 1, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOB_AHB, .port_shift = 1, .pin = 1, .pctl = 1 },
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
  .uart_base = UART2_BASE,
  /* JP4/JP5 in UART configuration: A2.5 PD4 RX, A2.6 PD5 TX
   * JP4/JP5 in CAN configuration: N/A */
  .rx_pinmux = { .port = GPIOD_AHB, .port_shift = 3, .pin = 4, .pctl = 1 },
  .tx_pinmux = { .port = GPIOD_AHB, .port_shift = 3, .pin = 5, .pctl = 1 },
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
  .uart_base = UART3_BASE,
  /* B2.9 PA4 RX, B2.10 PA5 TX */
  .rx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 4, .pctl = 1 },
  .tx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 5, .pctl = 1 },
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
  .uart_base = UART4_BASE,
  /* B2.5 PK0 RX, B2.6 PK1 TX */
  .rx_pinmux = { .port = GPIOK, .port_shift = 9, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOK, .port_shift = 9, .pin = 1, .pctl = 1 },
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
  .uart_base = UART5_BASE,
  /* A1.5 PC6 RX, A1.8 PC7 TX */
  .rx_pinmux = { .port = GPIOC_AHB, .port_shift = 2, .pin = 6, .pctl = 1 },
  .tx_pinmux = { .port = GPIOC_AHB, .port_shift = 2, .pin = 7, .pctl = 1 },
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

#if (BSPACM_CONFIG_ENABLE_UART6 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART6devcfg = {
  .uart_base = UART6_BASE,
  /* A2.3 PP0 RX, A2.4 PP1 TX */
  .rx_pinmux = { .port = GPIOP, .port_shift = 13, .pin = 0, .pctl = 1 },
  .tx_pinmux = { .port = GPIOP, .port_shift = 13, .pin = 1, .pctl = 1 },
  .irqn = UART6_IRQn,
  .instance = 6
};

#ifdef BSPACM_PERIPH_UART6_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART6_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART6_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART6_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART6_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART6_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 6
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_UART6 */

#if (BSPACM_CONFIG_ENABLE_UART7 - 0)

__attribute__((__weak__))
const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART7devcfg = {
  .uart_base = UART7_BASE,
  /* A1.3 PC4 RX, A1.4 PC5 TX */
  .rx_pinmux = { .port = GPIOC_AHB, .port_shift = 2, .pin = 4, .pctl = 1 },
  .tx_pinmux = { .port = GPIOC_AHB, .port_shift = 2, .pin = 5, .pctl = 1 },
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
