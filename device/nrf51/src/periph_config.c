/* BSPACM - Board pinmux/periph configuration for nRF51-Dongle
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

/** Shared periph_config for nRF51 devices.
 *
 * There is no variation among peripherals available for this device
 * series; only which pins are used on which board. */

#include <bspacm/periph/uart.h>
#include <bspacm/internal/utility/fifo.h>
#include <bspacm/internal/board/uart0.inc>

#ifndef NRF51_UART0_RTS_PIN
#define NRF51_UART0_RTS_PIN -1
#endif /* NRF51_UART0_RTS_PIN */

#ifndef NRF51_UART0_CTS_PIN
#define NRF51_UART0_CTS_PIN -1
#endif /* NRF51_UART0_CTS_PIN */

#if (BSPACM_CONFIG_ENABLE_UART - 0)

__attribute__((__weak__))
const sBSPACMdeviceNRF51periphUARTdevcfg xBSPACMdeviceNRF51periphUART0devcfg = {
  .rx_pin = NRF51_UART0_RXD_PIN,
  .tx_pin = NRF51_UART0_TXD_PIN,
  .rts_pin = NRF51_UART0_RTS_PIN,
  .cts_pin = NRF51_UART0_CTS_PIN
};

#if (BSPACM_PERIPH_UART0_TX_BUFFER_SIZE - 0)
FIFO_DEFINE_ALLOCATION(tx_allocation_UART0, BSPACM_PERIPH_UART0_TX_BUFFER_SIZE);
#endif /* BSPACM_PERIPH_UART0_TX_BUFFER_SIZE */

#if (BSPACM_PERIPH_UART0_RX_BUFFER_SIZE - 0)
FIFO_DEFINE_ALLOCATION(rx_allocation_UART0, BSPACM_PERIPH_UART0_RX_BUFFER_SIZE);
#endif /* BSPACM_PERIPH_UART0_RX_BUFFER_SIZE */

sBSPACMperiphUARTstate xBSPACMdeviceNRF51periphUART0 = {
  .uart = NRF_UART0,
  .devcfg = { .ptr = &xBSPACMdeviceNRF51periphUART0devcfg },
  .ops = &xBSPACMdeviceNRF51periphUARToperations,
#if (BSPACM_PERIPH_UART0_TX_BUFFER_SIZE - 0)
  .tx_fifo_ni_ = FIFO_FROM_ALLOCATION(tx_allocation_UART0),
#endif /* BSPACM_PERIPH_UART0_TX_BUFFER_SIZE */
#if (BSPACM_PERIPH_UART0_RX_BUFFER_SIZE - 0)
  .rx_fifo_ni_ = FIFO_FROM_ALLOCATION(rx_allocation_UART0),
#endif /* BSPACM_PERIPH_UART0_RX_BUFFER_SIZE */
};

/** Override the weak default that references no peripheral */
const hBSPACMperiphUART hBSPACMdefaultUART =
  &xBSPACMdeviceNRF51periphUART0
  ;

void UART0_IRQHandler(void)
{
  vBSPACMdeviceNRF51periphUARTirqhandler(&xBSPACMdeviceNRF51periphUART0);
}

#endif /* BSPACM_CONFIG_ENABLE_UART */
