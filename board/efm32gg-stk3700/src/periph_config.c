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

const sBSPACMdeviceEFM32pinmuxUART xBSPACMdeviceEFM32pinmuxUART[] = {
  {
    .uart_base = USART1_BASE,
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
    .rx_irqn = USART1_RX_IRQn,
    .tx_irqn = USART1_TX_IRQn,
  },
};
const uint8_t nBSPACMdeviceEFM32pinmuxUART = sizeof(xBSPACMdeviceEFM32pinmuxUART)/sizeof(*xBSPACMdeviceEFM32pinmuxUART);

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

/** Override the weak default that references no peripheral */
hBSPACMperiphUART hBSPACMutilityCONSOLEuart = &xBSPACMdeviceEFM32periphUSART1;
