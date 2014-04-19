/* BSPACM - config header for generic TM4C boards
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
 * @brief Common TM4C STK default application configuration.
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

#ifndef BSPACM_DEVICE_TM4C_BOARD_DEFAULT_APPCONF_H
#define BSPACM_DEVICE_TM4C_BOARD_DEFAULT_APPCONF_H

/* Use UART0 as the default UART unless one of the defaults is
 * explicitly configured. */
#if ! (defined(BSPACM_CONFIG_DEFAULT_UART0)        \
       || defined(BSPACM_CONFIG_DEFAULT_UART1)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART2)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART3)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART4)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART5)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART6)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART7))
#define BSPACM_CONFIG_DEFAULT_UART0 1
#endif /* BSPACM_CONFIG_DEFAULT */

/* Propagate to each supported peripheral.  USART1 is the default
 * UART, and is the only one implicitly enabled. */
#ifndef BSPACM_CONFIG_ENABLE_UART0
#define BSPACM_CONFIG_ENABLE_UART0 (BSPACM_CONFIG_DEFAULT_UART0 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART0 */
#ifndef BSPACM_CONFIG_ENABLE_UART1
#define BSPACM_CONFIG_ENABLE_UART1 (BSPACM_CONFIG_DEFAULT_UART1 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART1 */
#ifndef BSPACM_CONFIG_ENABLE_UART2
#define BSPACM_CONFIG_ENABLE_UART2 (BSPACM_CONFIG_DEFAULT_UART2 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART2 */
#ifndef BSPACM_CONFIG_ENABLE_UART3
#define BSPACM_CONFIG_ENABLE_UART3 (BSPACM_CONFIG_DEFAULT_UART3 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART3 */
#ifndef BSPACM_CONFIG_ENABLE_UART4
#define BSPACM_CONFIG_ENABLE_UART4 (BSPACM_CONFIG_DEFAULT_UART4 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART4 */
#ifndef BSPACM_CONFIG_ENABLE_UART5
#define BSPACM_CONFIG_ENABLE_UART5 (BSPACM_CONFIG_DEFAULT_UART5 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART5 */
#ifndef BSPACM_CONFIG_ENABLE_UART6
#define BSPACM_CONFIG_ENABLE_UART6 (BSPACM_CONFIG_DEFAULT_UART6 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART6 */
#ifndef BSPACM_CONFIG_ENABLE_UART7
#define BSPACM_CONFIG_ENABLE_UART7 (BSPACM_CONFIG_DEFAULT_UART7 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART7 */

/* Default enable BSPACM_CONFIG_ENABLE_UART if any specific UART is
 * enabled. */
#ifndef BSPACM_CONFIG_ENABLE_UART
#define BSPACM_CONFIG_ENABLE_UART ((BSPACM_CONFIG_ENABLE_UART0 - 0)     \
                                   || (BSPACM_CONFIG_ENABLE_UART1 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART2 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART3 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART4 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART5 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART6 - 0)  \
                                   || (BSPACM_CONFIG_ENABLE_UART7 - 0))
#endif /* BSPACM_CONFIG_ENABLE_UART */

/* Propagate default UART constants to UART0, UART1, etc.  */

#if defined(BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE)
#if (BSPACM_CONFIG_DEFAULT_UART0 - 0) && ! defined(BSPACM_PERIPH_UART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART1 - 0) && ! defined(BSPACM_PERIPH_UART1_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART1_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART2 - 0) && ! defined(BSPACM_PERIPH_UART2_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART2_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART3 - 0) && ! defined(BSPACM_PERIPH_UART3_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART3_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART4 - 0) && ! defined(BSPACM_PERIPH_UART4_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART4_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART5 - 0) && ! defined(BSPACM_PERIPH_UART5_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART5_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART6 - 0) && ! defined(BSPACM_PERIPH_UART6_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART6_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART7 - 0) && ! defined(BSPACM_PERIPH_UART7_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART7_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#endif /* select default UART */
#endif /* propagate default TX_BUFFER_SIZE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE)
#if (BSPACM_CONFIG_DEFAULT_UART0 - 0) && ! defined(BSPACM_PERIPH_UART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART1 - 0) && ! defined(BSPACM_PERIPH_UART1_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART1_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART2 - 0) && ! defined(BSPACM_PERIPH_UART2_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART2_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART3 - 0) && ! defined(BSPACM_PERIPH_UART3_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART3_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART4 - 0) && ! defined(BSPACM_PERIPH_UART4_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART4_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART5 - 0) && ! defined(BSPACM_PERIPH_UART5_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART5_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART6 - 0) && ! defined(BSPACM_PERIPH_UART6_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART6_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART7 - 0) && ! defined(BSPACM_PERIPH_UART7_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART7_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#endif /* select default UART */
#endif /* propagate default RX_BUFFER_SIZE */


#endif /* BSPACM_DEVICE_TM4C_BOARD_DEFAULT_APPCONF_H */
