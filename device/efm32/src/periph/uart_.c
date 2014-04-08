/* Copyright 2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Implementation for EFM32 device series UART peripheral interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/uart.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/internal/utility/fifo.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_usart.h>

static
int
usart_configure (sBSPACMperiphUARTstate * usp,
                 const sBSPACMperiphUARTconfiguration * cfgp)
{
  const sBSPACMdeviceEFM32pinmuxUART * pmp = xBSPACMdeviceEFM32pinmuxUART;
  const sBSPACMdeviceEFM32pinmuxUART * const pmpe = pmp + nBSPACMdeviceEFM32pinmuxUART;
  USART_TypeDef * usart;
  uint32_t usart_base;
  const sBSPACMdeviceEFM32periphUSARTdevcfg * devcfgp;

  if (! (usp && usp->uart)) {
    return -1;
  }
  usart = (USART_TypeDef *)usp->uart;
  usart_base = (uint32_t)usp->uart;
  /* Find the pinmux configuration for the selected UART.  Fail if
   * there isn't one, or if it doesn't provide at least RX and TX
   * pins. */
  while ((pmp < pmpe) && (pmp->uart_base != usart_base)) {
    ++pmp;
  }
  if (pmp >= pmpe) {
    return -1;
  }
  if (! (pmp->rx_pinmux.port && pmp->tx_pinmux.port)) {
    return -1;
  }
  (void)usart_base;
  (void)pmpe;
  devcfgp = (const sBSPACMdeviceEFM32periphUSARTdevcfg *)usp->devcfg.ptr;

  /* Enable the high-frequency peripheral clock, and the clock for the
   * uart itself */
  if (cfgp) {
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(devcfgp->clock, true);
  }
  USART_Reset(usart);
  fifo_reset(usp->rx_fifo);
  fifo_reset(usp->tx_fifo);

  if (cfgp) {
    unsigned int baud_rate = cfgp->speed_baud;

    if (0 == baud_rate) {
      baud_rate = 115200;
    }
    /* Configure the USART for 8N1.  Set TXBL at half-full. */
    usart->FRAME = USART_FRAME_DATABITS_EIGHT | USART_FRAME_PARITY_NONE | USART_FRAME_STOPBITS_ONE;
    usart->CTRL |= USART_CTRL_TXBIL_HALFFULL;
    USART_BaudrateAsyncSet(usart, 0, baud_rate, usartOVS16);
    CMU_ClockEnable(cmuClock_GPIO, true);
  }

  /* Enable or disable UART pins. To avoid false start, when enabling
   * configure TX as high.  This relies on a comment in the EMLIB code
   * that manipulating registers of disabled modules has no effect
   * (unlike TM4C where it causes a HardFault).  We'll see. */
  vBSPACMdeviceEFM32pinmuxConfigure(&pmp->rx_pinmux, !!cfgp, 1);
  vBSPACMdeviceEFM32pinmuxConfigure(&pmp->tx_pinmux, !!cfgp, 0);

  if (cfgp) {
    usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | devcfgp->location;

    /* Clear and enable RX interrupts.  TX interrupts are enabled at the
     * peripheral when there's something to transmit.  TX and RX are
     * enabled at the NVIC now. */
    usart->IFC = _USART_IF_MASK;
    usart->IEN = USART_IF_RXDATAV;
    NVIC_ClearPendingIRQ(devcfgp->rx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->tx_irqn);
    NVIC_EnableIRQ(devcfgp->rx_irqn);
    NVIC_EnableIRQ(devcfgp->tx_irqn);

    /* Configuration complete; enable the USART */
    usart->CMD = USART_CMD_RXEN | USART_CMD_TXEN;
  } else {
    CMU_ClockEnable(devcfgp->clock, false);
  }

  return 0;
}

static int
usart_hw_transmit (sBSPACMperiphUARTstate * usp,
                   uint8_t v)
{
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;

  if (! (USART_STATUS_TXBL & usart->STATUS)) {
    return -1;
  }
  usart->TXDATA = v;
  usp->tx_count += 1;
  return v;
}

static void
usart_hw_txien (sBSPACMperiphUARTstate * usp,
                int enablep)
{
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;
  if (enablep) {
    usart->IEN |= USART_IF_TXBL;
  } else {
    usart->IEN &= ~USART_IF_TXBL;
  }
}

static int
usart_fifo_state (sBSPACMperiphUARTstate * usp)
{
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;
  int rv = 0;
  if (! (usart->STATUS & USART_STATUS_TXC)) {
    rv |= eBSPACMperiphUARTfifoState_HWTX;
  }
  if (usart->STATUS & USART_STATUS_RXDATAV) {
    rv |= eBSPACMperiphUARTfifoState_HWRX;
  }
  if (! fifo_empty(usp->tx_fifo)) {
    rv |= eBSPACMperiphUARTfifoState_SWTX;
  }
  if (! fifo_empty(usp->rx_fifo)) {
    rv |= eBSPACMperiphUARTfifoState_SWRX;
  }
  return rv;
}

const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphUSARToperations = {
  .configure = usart_configure,
  .hw_transmit = usart_hw_transmit,
  .hw_txien = usart_hw_txien,
  .fifo_state = usart_fifo_state,
};

void
vBSPACMdeviceEFM32periphUSARTrxirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;

  if (USART_STATUS_RXDATAV & usart->STATUS) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    while (USART_STATUS_RXDATAV & usart->STATUS) {
      uint16_t rxdatax = usart->RXDATAX;
      if (0 == ((USART_RXDATAX_PERR | USART_RXDATAX_FERR) & rxdatax)) {
        if (0 > fifo_push_head(usp->rx_fifo, usart->RXDATA)) {
          usp->rx_dropped_errors += 1;
        }
        usp->rx_count += 1;
      } else {
        if (USART_RXDATAX_PERR & rxdatax) {
          usp->rx_parity_errors += 1;
        }
        if (USART_RXDATAX_FERR & rxdatax) {
          usp->rx_frame_errors += 1;
        }
      }
    };
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

void
vBSPACMdeviceEFM32periphUSARTtxirqhandler(sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;

  if (USART_STATUS_TXBL & usart->STATUS) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    while ((USART_STATUS_TXBL & usart->STATUS)
           && (! fifo_empty(usp->tx_fifo))) {
      usart->TXDATA = fifo_pop_tail(usp->tx_fifo, 0);
      usp->tx_count += 1;
    }
    if (fifo_empty(usp->tx_fifo)) {
      usart->IEN &= ~USART_IF_TXBL;
    }
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}
