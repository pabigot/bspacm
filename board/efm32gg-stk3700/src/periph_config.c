/* BSPACM - Board pinmux/periph configuration for EFM32GG-STK3700
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

#if (BSPACM_CONFIG_ENABLE_USART1 - 0)

__attribute__((__weak__))
const sBSPACMdeviceEFM32periphUSARTdevcfg xBSPACMdeviceEFM32periphUSART1devcfg = {
  .common = {
    .uart_base = USART1_BASE,
    .rx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 1,
      .mode = gpioModeInput,
    },
    .tx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 0,
      .mode = gpioModePushPull,
    },
    .clock = cmuClock_USART1,
    .location = USART_ROUTE_LOCATION_LOC1
  },
  .clk_pinmux = {
    .port = GPIO->P + gpioPortD,
    .pin = 2,
    .mode = gpioModePushPull,
  },
  .cs_pinmux = {
    .port = GPIO->P + gpioPortD,
    .pin = 3,
    .mode = gpioModePushPull,
  },
  .tx_irqn = USART1_TX_IRQn,
  .rx_irqn = USART1_RX_IRQn
};

#ifdef BSPACM_PERIPH_USART1_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_USART1_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_USART1_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_USART1_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_USART1_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_USART1_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 1
#include <bspacm/internal/periph/usart.inc>
#undef BSPACM_INC_LOCATION
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_USART1 */

#if (BSPACM_CONFIG_ENABLE_UART0 - 0)

__attribute__((__weak__))
const sBSPACMdeviceEFM32periphUARTdevcfg xBSPACMdeviceEFM32periphUART0devcfg = {
  .common = {
    .uart_base = UART0_BASE,
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
    .clock = cmuClock_UART0,
    .location = UART_ROUTE_LOCATION_LOC1,
  },
  .tx_irqn = UART0_TX_IRQn,
  .rx_irqn = UART0_RX_IRQn
};

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

__attribute__((__weak__))
const sBSPACMdeviceEFM32periphLEUARTdevcfg xBSPACMdeviceEFM32periphLEUART0devcfg = {
  .common = {
    .uart_base = LEUART0_BASE,
    .rx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 5,
      .mode = gpioModeInput,
    },
    .tx_pinmux = {
      .port = GPIO->P + gpioPortD,
      .pin = 4,
      .mode = gpioModePushPull,
    },
    .clock = cmuClock_LEUART0,
    .location = USART_ROUTE_LOCATION_LOC0
  },
  .irqn = LEUART0_IRQn,
  .lfbsel = cmuSelect_LFXO
};

#ifdef BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE
#define BSPACM_INC_TX_BUFFER_SIZE BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE */
#ifdef BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE
#define BSPACM_INC_RX_BUFFER_SIZE BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE
#endif /* BSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE */
#define BSPACM_INC_PERIPHNUM 0
#include <bspacm/internal/periph/leuart.inc>
#undef BSPACM_INC_PERIPHNUM
#undef BSPACM_INC_RX_BUFFER_SIZE
#undef BSPACM_INC_TX_BUFFER_SIZE

#endif /* BSPACM_CONFIG_ENABLE_LEUART0 */

/** Override the weak default that references no peripheral */
const hBSPACMperiphUART hBSPACMdefaultUART =
#if (BSPACM_CONFIG_DEFAULT_USART0 - 0)
  &xBSPACMdeviceEFM32periphUSART0
#elif (BSPACM_CONFIG_DEFAULT_USART1 - 0)
  &xBSPACMdeviceEFM32periphUSART1
#elif (BSPACM_CONFIG_DEFAULT_USART2 - 0)
  &xBSPACMdeviceEFM32periphUSART2
#elif (BSPACM_CONFIG_DEFAULT_UART0 - 0)
  &xBSPACMdeviceEFM32periphUART0
#elif (BSPACM_CONFIG_DEFAULT_UART1 - 0)
  &xBSPACMdeviceEFM32periphUART1
#elif (BSPACM_CONFIG_DEFAULT_LEUART0 - 0)
  &xBSPACMdeviceEFM32periphLEUART0
#elif (BSPACM_CONFIG_DEFAULT_LEUART1 - 0)
  &xBSPACMdeviceEFM32periphLEUART1
#else
#error no specified default UART
#endif
  ;

#endif /* BSPACM_CONFIG_ENABLE_UART */
