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
 * This file may be copied, modified, and installed in an
 * application-specific include directory to supersede the board
 * default configuration.  Its contents should be consistent with the
 * peripheral configuration identified in the @c PERIPH_CONFIG_SRC
 * make variable.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @publicdomain @cc0_doc
 */

#ifndef BSPACM_CONFIG_H
#define BSPACM_CONFIG_H

/* @cond DOXYGEN_EXCLUDE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_TX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_TX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#endif /* Copy TX_BUFFER_SIZE */

#if defined(BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE) \
  && ! defined(BSPACM_PERIPH_UART0_RX_BUFFER_SIZE)
#define BSPACM_PERIPH_UART0_RX_BUFFER_SIZE BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#endif /* Copy RX_BUFFER_SIZE */

/* @endcond */

#endif /* BSPACM_CONFIG_H */
