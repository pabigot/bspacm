/* BSPACM - config header for ek-tm4c1294xl
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

#ifndef BSPACM_CONFIG_H
#define BSPACM_CONFIG_H

/** The default UART device on this board */
#define BSPACM_CONFIG_DEFAULT_UART_HANDLE (&xBSPACMdeviceTM4CperiphUART0)

#if defined(BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#endif /* Copy TX_BUFFER_SIZE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#endif /* Copy RX_BUFFER_SIZE */

#endif /* BSPACM_CONFIG_H */
