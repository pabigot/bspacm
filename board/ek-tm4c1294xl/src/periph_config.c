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

/* PA 0    PB 1    PC 2    PD 3
 * PE 4    PF 5    PG 6    PH 7
 * *PJ 8   PK 9    PL 10   PM 11
 * PN 12   *PP 13  PQ 14   PR 15
 * PS 16   PT 17
 */
const sBSPACMdeviceTM4CpinmuxUART xBSPACMdeviceTM4CpinmuxUART[] = {
  { .uart_base = UART0_BASE,
    /* JP4/JP5 in UART configuration: virtual COM
     * JP4/JP5 in CAN configuration: A2.3, A2.4 */
    .rx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 0, .pctl = 1 },
    .tx_pinmux = { .port = GPIOA_AHB, .port_shift = 0, .pin = 1, .pctl = 1 },
    .irqn = UART0_IRQn,
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
