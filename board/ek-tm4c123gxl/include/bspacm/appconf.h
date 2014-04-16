/* BSPACM - config header for ek-tm4c123gxl
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
 * @brief Board-specific configuration header for ek-tm4c123gxl
 *
 * This file is included by <bspacm/config.h> which defines general
 * defaults that are overridden/extended/used here to apply
 * information to the board-specific configuration. Its contents
 * should be consistent with the peripheral configuration identified
 * in the @c PERIPH_CONFIG_SRC make variable.
 *
 * This file may be copied, modified, and installed in an
 * application-specific include directory to supersede the copy in the
 * board-specific include hierarchy.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @publicdomain @cc0_doc
 */

#ifndef BSPACM_APPCONF_H
#define BSPACM_APPCONF_H

/* Propagate to each supported peripheral.  UART0 is the default
 * UART, and is the only one implicitly enabled. */
#ifndef BSPACM_CONFIG_ENABLE_UART0
#define BSPACM_CONFIG_ENABLE_UART0 (BSPACM_CONFIG_ENABLE_UART - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART0 */
#ifndef BSPACM_CONFIG_ENABLE_UART1
#define BSPACM_CONFIG_ENABLE_UART1 0
#endif /* BSPACM_CONFIG_ENABLE_UART1 */
#ifndef BSPACM_CONFIG_ENABLE_UART2
#define BSPACM_CONFIG_ENABLE_UART2 0
#endif /* BSPACM_CONFIG_ENABLE_UART2 */
#ifndef BSPACM_CONFIG_ENABLE_UART3
#define BSPACM_CONFIG_ENABLE_UART3 0
#endif /* BSPACM_CONFIG_ENABLE_UART3 */
#ifndef BSPACM_CONFIG_ENABLE_UART4
#define BSPACM_CONFIG_ENABLE_UART4 0
#endif /* BSPACM_CONFIG_ENABLE_UART4 */
#ifndef BSPACM_CONFIG_ENABLE_UART5
#define BSPACM_CONFIG_ENABLE_UART5 0
#endif /* BSPACM_CONFIG_ENABLE_UART5 */
#ifndef BSPACM_CONFIG_ENABLE_UART6
#define BSPACM_CONFIG_ENABLE_UART6 0
#endif /* BSPACM_CONFIG_ENABLE_UART6 */
#ifndef BSPACM_CONFIG_ENABLE_UART7
#define BSPACM_CONFIG_ENABLE_UART7 0
#endif /* BSPACM_CONFIG_ENABLE_UART7 */

/* Propagate default UART constants to UART0 */

#if defined(BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#endif /* Copy TX_BUFFER_SIZE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#endif /* Copy RX_BUFFER_SIZE */

#endif /* BSPACM_APPCONF_H */
