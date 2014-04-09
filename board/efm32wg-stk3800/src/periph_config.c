/* BSPACM - Board pinmux/periph configuration for EFM32WG-STK3800
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
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>
#include <em_leuart.h>

#if (BSPACM_CONFIG_ENABLE_UART - 0)

const sBSPACMdeviceEFM32pinmuxUART xBSPACMdeviceEFM32pinmuxUART[] = {
#if (BSPACM_CONFIG_ENABLE_USART1 - 0)
  {
    .uart_base = USART1_BASE,
    /* LOC1 */
    .tx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 0,
      .mode = gpioModePushPull,
    },
    .rx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 1,
      .mode = gpioModeInput,
    },
  },
#endif /* BSPACM_CONFIG_ENABLE_USART1 */
#if (BSPACM_CONFIG_ENABLE_UART0 - 0)
  {
    .uart_base = UART0_BASE,
    /* LOC1 */
    .tx_pinmux = {
      .port = GPIO->P + gpioPortE,
      .pin = 0,
      .mode = gpioModePushPull,
    },
    .rx_pinmux = {
      .port = GPIO->P + gpioPortE,
      .pin = 1,
      .mode = gpioModeInput,
    },
  },
#endif /* BSPACM_CONFIG_ENABLE_UART0 */
#if (BSPACM_CONFIG_ENABLE_LEUART0 - 0)
  {
    .uart_base = LEUART0_BASE,
    /* LOC0 */
    .tx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 4,
      .mode = gpioModePushPull,
    },
    .rx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 5,
      .mode = gpioModeInput,
    },
  },
#endif /* BSPACM_CONFIG_ENABLE_LEUART0 */
};
const uint8_t nBSPACMdeviceEFM32pinmuxUART = sizeof(xBSPACMdeviceEFM32pinmuxUART)/sizeof(*xBSPACMdeviceEFM32pinmuxUART);

#if (BSPACM_CONFIG_ENABLE_USART1 - 0)
#ifdef BSPACM_PERIPH_USART1_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_USART1_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_USART1_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_USART1_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_USART1_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_USART1_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 1
#define BSPACM_INC_LOCATION USART_ROUTE_LOCATION_LOC1
#include <bspacm/internal/periph/usart.inc>
#undef BSPACM_INC_LOCATION
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE
#endif /* BSPACM_CONFIG_ENABLE_USART1 */

#if (BSPACM_CONFIG_ENABLE_UART0 - 0)
#ifdef BSPACM_PERIPH_UART0_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_UART0_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART0_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_UART0_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_UART0_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_UART0_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 0
#define BSPACM_INC_LOCATION UART_ROUTE_LOCATION_LOC1
#include <bspacm/internal/periph/uart.inc>
#undef BSPACM_INC_LOCATION
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE
#endif /* BSPACM_CONFIG_ENABLE_UART0 */

#if (BSPACM_CONFIG_ENABLE_LEUART0 - 0)
#ifdef BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 0
#define BSPACM_INC_LOCATION LEUART_ROUTE_LOCATION_LOC0
#include <bspacm/internal/periph/leuart.inc>
#undef BSPACM_INC_LOCATION
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE
#endif /* BSPACM_CONFIG_ENABLE_LEUART0 */

/** Override the weak default that references no peripheral */
const hBSPACMperiphUART hBSPACMdefaultUART = &xBSPACMdeviceEFM32periphUSART1;

#endif /* BSPACM_CONFIG_ENABLE_UART */
