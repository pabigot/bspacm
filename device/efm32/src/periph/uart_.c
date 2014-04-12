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
#include <em_leuart.h>

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
  devcfgp = (const sBSPACMdeviceEFM32periphUSARTdevcfg *)usp->devcfg.ptr;

  /* If enabling configuration, enable the high-frequency peripheral
   * clock and the clock for the uart itself.
   *
   * If disabling configuration, disable the interrupts. */
  if (cfgp) {
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(devcfgp->clock, true);
  } else {
    NVIC_DisableIRQ(devcfgp->rx_irqn);
    NVIC_DisableIRQ(devcfgp->tx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->rx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->tx_irqn);
  }
  USART_Reset(usart);
  if (usp->rx_fifo_ni_) {
    fifo_reset(usp->rx_fifo_ni_);
  }
  if (usp->tx_fifo_ni_) {
    fifo_reset(usp->tx_fifo_ni_);
  }
  usp->tx_state_ = 0;

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
  } else {
    /* Done with device; turn it off */
    CMU_ClockEnable(devcfgp->clock, false);
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
  }

  return 0;
}

static int
usart_hw_transmit (sBSPACMperiphUARTstate * usp,
                   uint8_t v)
{
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;
  int rv = -1;

  if (USART_STATUS_TXBL & usart->STATUS) {
    usart->TXDATA = v;
    rv = v;
    usp->tx_count += 1;
  }
  return rv;
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
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;
  int rv = 0;
  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if (! (usart->STATUS & USART_STATUS_TXC)) {
      rv |= eBSPACMperiphUARTfifoState_HWTX;
    }
    if (usart->STATUS & USART_STATUS_RXDATAV) {
      rv |= eBSPACMperiphUARTfifoState_HWRX;
    }
    if (usp->tx_fifo_ni_ && (! fifo_empty(usp->tx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWTX;
    }
    if (usp->rx_fifo_ni_ && (! fifo_empty(usp->rx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWRX;
    }
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphUSARToperations = {
  .configure = usart_configure,
  .hw_transmit = usart_hw_transmit,
  .hw_txien = usart_hw_txien,
  .fifo_state = usart_fifo_state,
};

static int
leuart_configure (sBSPACMperiphUARTstate * usp,
                  const sBSPACMperiphUARTconfiguration * cfgp)
{
  const sBSPACMdeviceEFM32pinmuxUART * pmp = xBSPACMdeviceEFM32pinmuxUART;
  const sBSPACMdeviceEFM32pinmuxUART * const pmpe = pmp + nBSPACMdeviceEFM32pinmuxUART;
  LEUART_TypeDef * leuart;
  uint32_t leuart_base;
  const sBSPACMdeviceEFM32periphLEUARTdevcfg * devcfgp;

  if (! (usp && usp->uart)) {
    return -1;
  }
  leuart = (LEUART_TypeDef *)usp->uart;
  leuart_base = (uint32_t)usp->uart;

  /* Find the pinmux configuration for the selected UART.  Fail if
   * there isn't one, or if it doesn't provide at least RX and TX
   * pins. */
  while ((pmp < pmpe) && (pmp->uart_base != leuart_base)) {
    ++pmp;
  }
  if (pmp >= pmpe) {
    return -1;
  }
  if (! (pmp->rx_pinmux.port && pmp->tx_pinmux.port)) {
    return -1;
  }
  devcfgp = (const sBSPACMdeviceEFM32periphLEUARTdevcfg *)usp->devcfg.ptr;

  /* Configure LFB's source, enable the low-energy peripheral clock, and the clock for the
   * leuart itself */
  if (cfgp) {
    /* LFB is required for LEUART.  Power-up is LFRCO which doesn't
     * work so good; if we were told a source to use, override
     * whatever was there. */
    if (devcfgp->lfbsel) {
      CMU_ClockSelectSet(cmuClock_LFB, devcfgp->lfbsel);
    }
    CMU_ClockEnable(cmuClock_CORELE, true);
    CMU_ClockEnable(devcfgp->clock, true);
  } else {
    NVIC_DisableIRQ(devcfgp->irqn);
    NVIC_ClearPendingIRQ(devcfgp->irqn);
  }
  LEUART_Reset(leuart);
  leuart->FREEZE = LEUART_FREEZE_REGFREEZE;
  leuart->CMD = LEUART_CMD_RXDIS | LEUART_CMD_TXDIS;
  if (usp->rx_fifo_ni_) {
    fifo_reset(usp->rx_fifo_ni_);
  }
  if (usp->tx_fifo_ni_) {
    fifo_reset(usp->tx_fifo_ni_);
  }
  usp->tx_state_ = 0;

  if (cfgp) {
    unsigned int speed_baud = cfgp->speed_baud;
    if (0 == speed_baud) {
      speed_baud = 9600;
    }
    /* Configure the LEUART for rate at 8N1. */
    leuart->CTRL = LEUART_CTRL_DATABITS_EIGHT | LEUART_CTRL_PARITY_NONE | LEUART_CTRL_STOPBITS_ONE;
    LEUART_BaudrateSet(leuart, 0, speed_baud);
    CMU_ClockEnable(cmuClock_GPIO, true);
  }

  /* Enable or disable UART pins. To avoid false start, when enabling
   * configure TX as high.  This relies on a comment in the EMLIB code
   * that manipulating registers of disabled modules has no effect
   * (unlike TM4C where it causes a HardFault).  We'll see. */
  vBSPACMdeviceEFM32pinmuxConfigure(&pmp->rx_pinmux, !!cfgp, 1);
  vBSPACMdeviceEFM32pinmuxConfigure(&pmp->tx_pinmux, !!cfgp, 0);

  if (cfgp) {
    leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | devcfgp->location;

    /* Clear and enable RX interrupts at the device.  Device TX
     * interrupts are enabled at the peripheral when there's something
     * to transmit.  Clear then enable interrupts at the NVIC. */
    leuart->IFC = _LEUART_IF_MASK;
    leuart->IEN = LEUART_IF_RXDATAV;
    NVIC_ClearPendingIRQ(devcfgp->irqn);
    NVIC_EnableIRQ(devcfgp->irqn);

    /* Configuration complete; enable the LEUART, and release the
     * registers to synchronize. */
    leuart->CMD = LEUART_CMD_RXEN | LEUART_CMD_TXEN;
    leuart->FREEZE = 0;
  } else {
    CMU_ClockEnable(devcfgp->clock, false);
  }
  return 0;
}

static int
leuart_hw_transmit (sBSPACMperiphUARTstate * usp,
                    uint8_t v)
{
  LEUART_TypeDef * const leuart = (LEUART_TypeDef *)usp->uart;

  if (! (LEUART_STATUS_TXBL & leuart->STATUS)) {
    return -1;
  }
  while (LEUART_SYNCBUSY_TXDATA & leuart->SYNCBUSY) {
    /* wait */
  }
  leuart->TXDATA = v;
  usp->tx_count += 1;
  return v;
}

static void
leuart_hw_txien (sBSPACMperiphUARTstate * usp,
                 int enablep)
{
  LEUART_TypeDef * const leuart = (LEUART_TypeDef *)usp->uart;
  if (enablep) {
    leuart->IEN |= LEUART_IF_TXBL;
  } else {
    leuart->IEN &= ~LEUART_IF_TXBL;
  }
}

static int
leuart_fifo_state (sBSPACMperiphUARTstate * usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  LEUART_TypeDef * const leuart = (LEUART_TypeDef *)usp->uart;
  int rv = 0;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if (! (leuart->STATUS & LEUART_STATUS_TXC)) {
      rv |= eBSPACMperiphUARTfifoState_HWTX;
    }
    if (leuart->STATUS & LEUART_STATUS_RXDATAV) {
      rv |= eBSPACMperiphUARTfifoState_HWRX;
    }
    if (usp->tx_fifo_ni_ && (! fifo_empty(usp->tx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWTX;
    }
    if (usp->rx_fifo_ni_ && (! fifo_empty(usp->rx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWRX;
    }
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphLEUARToperations = {
  .configure = leuart_configure,
  .hw_transmit = leuart_hw_transmit,
  .hw_txien = leuart_hw_txien,
  .fifo_state = leuart_fifo_state,
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
        if ((! usp->rx_fifo_ni_)
            || (0 > fifo_push_head(usp->rx_fifo_ni_, usart->RXDATA))) {
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
vBSPACMdeviceEFM32periphUSARTtxirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  USART_TypeDef * const usart = (USART_TypeDef *)usp->uart;

  if (usp->tx_fifo_ni_
      && (USART_STATUS_TXBL & usart->STATUS)) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    while ((USART_STATUS_TXBL & usart->STATUS)
           && (! fifo_empty(usp->tx_fifo_ni_))) {
      usart->TXDATA = fifo_pop_tail(usp->tx_fifo_ni_, 0);
      usp->tx_count += 1;
    }
    if (fifo_empty(usp->tx_fifo_ni_)) {
      usart->IEN &= ~USART_IF_TXBL;
    }
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

void
vBSPACMdeviceEFM32periphLEUARTirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  LEUART_TypeDef * const leuart = (LEUART_TypeDef *)usp->uart;

  BSPACM_CORE_DISABLE_INTERRUPT();
  if (LEUART_STATUS_RXDATAV & leuart->STATUS) {
    while (LEUART_STATUS_RXDATAV & leuart->STATUS) {
      uint16_t rxdatax = leuart->RXDATAX;
      if (0 == ((LEUART_RXDATAX_PERR | LEUART_RXDATAX_FERR) & rxdatax)) {
        if ((! usp->rx_fifo_ni_)
            || (0 > fifo_push_head(usp->rx_fifo_ni_, leuart->RXDATA))) {
          usp->rx_dropped_errors += 1;
        }
        usp->rx_count += 1;
      } else {
        if (LEUART_RXDATAX_PERR & rxdatax) {
          usp->rx_parity_errors += 1;
        }
        if (LEUART_RXDATAX_FERR & rxdatax) {
          usp->rx_frame_errors += 1;
        }
      }
    };
  }
  if (usp->tx_fifo_ni_
      && (LEUART_STATUS_TXBL & leuart->STATUS)) {
    while ((LEUART_STATUS_TXBL & leuart->STATUS)
           && (! fifo_empty(usp->tx_fifo_ni_))) {
      leuart->TXDATA = fifo_pop_tail(usp->tx_fifo_ni_, 0);
      usp->tx_count += 1;
    }
    if (fifo_empty(usp->tx_fifo_ni_)) {
      leuart->IEN &= ~LEUART_IF_TXBL;
    }
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}
