/* BSPACM - config header for generic EFM32 boards
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
 * @brief Common EFM32 STK default application configuration.
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

#ifndef BSPACM_DEVICE_EFM32_BOARD_DEFAULT_APPCONF_H
#define BSPACM_DEVICE_EFM32_BOARD_DEFAULT_APPCONF_H

/* Use USART1 as the default UART unless one of the defaults is
 * explicitly configured. */
#if ! (defined(BSPACM_CONFIG_DEFAULT_USART0)        \
       || defined(BSPACM_CONFIG_DEFAULT_USART1)     \
       || defined(BSPACM_CONFIG_DEFAULT_USART2)     \
       || defined(BSPACM_CONFIG_DEFAULT_UART0)      \
       || defined(BSPACM_CONFIG_DEFAULT_UART1)      \
       || defined(BSPACM_CONFIG_DEFAULT_LEUART0)    \
       || defined(BSPACM_CONFIG_DEFAULT_LEUART1))
#define BSPACM_CONFIG_DEFAULT_USART1 1
#endif /* BSPACM_CONFIG_DEFAULT */

/* Propagate to each supported peripheral.  USART1 is the default
 * UART, and is the only one implicitly enabled. */
#ifndef BSPACM_CONFIG_ENABLE_USART1
#define BSPACM_CONFIG_ENABLE_USART1 (BSPACM_CONFIG_DEFAULT_USART1 - 0)
#endif /* BSPACM_CONFIG_ENABLE_USART1 */
#ifndef BSPACM_CONFIG_ENABLE_UART0
#define BSPACM_CONFIG_ENABLE_UART0 (BSPACM_CONFIG_DEFAULT_UART0 - 0)
#endif /* BSPACM_CONFIG_ENABLE_UART0 */
#ifndef BSPACM_CONFIG_ENABLE_LEUART0
#define BSPACM_CONFIG_ENABLE_LEUART0 (BSPACM_CONFIG_DEFAULT_LEUART0 - 0)
#endif /* BSPACM_CONFIG_ENABLE_LEUART0 */

/* Propagate default UART constants to USART1, UART0, and LEUART0, the
 * three most likely candidates for the default UART.  */

#if defined(BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE)
#if (BSPACM_CONFIG_DEFAULT_USART1 - 0) && ! defined(BSPACM_PERIPH_USART1_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_USART1_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART0 - 0) && ! defined(BSPACM_PERIPH_UART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_LEUART0 - 0) && ! defined(BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#endif /* select default UART */
#endif /* propagate default TX_BUFFER_SIZE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE)
#if (BSPACM_CONFIG_DEFAULT_USART1 - 0) && ! defined(BSPACM_PERIPH_USART1_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_USART1_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_UART0 - 0) && ! defined(BSPACM_PERIPH_UART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#elif (BSPACM_CONFIG_DEFAULT_LEUART0 - 0) && ! defined(BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#endif /* select default UART */
#endif /* propagate default TX_BUFFER_SIZE */

#endif /* BSPACM_DEVICE_EFM32_BOARD_DEFAULT_APPCONF_H */
