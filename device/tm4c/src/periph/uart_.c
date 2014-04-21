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

/* Implementation for TM4C device series UART peripheral interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/uart.h>
#include <bspacm/device/periphs.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/internal/utility/fifo.h>
#include <inc/hw_sysctl.h>
#include <inc/hw_ints.h>
#include <inc/hw_uart.h>
#include <inc/hw_gpio.h>

static
int
uart_configure (sBSPACMperiphUARTstate * usp,
                const sBSPACMperiphUARTconfiguration * cfgp)
{
  const sBSPACMdeviceTM4CperiphUARTdevcfg * devcfgp;
  UART0_Type * uart;
  unsigned int uart_instance;
  uint32_t rx_port_mask = 0;
  uint32_t tx_port_mask = 0;
  uint32_t rts_port_mask = 0;
  uint32_t cts_port_mask = 0;

  if (! (usp && usp->uart)) {
    return -1;
  }
  uart = (UART0_Type *)usp->uart;
  devcfgp = (const sBSPACMdeviceTM4CperiphUARTdevcfg *)usp->devcfg.ptr;

  /* Determine the instance of the UART.  This relies on the fact that
   * each UART instance is given 0x1000 bytes of peripheral memory,
   * and UART0 through UART7 are contiguous in that memory.  This
   * consistency is not present for other peripherals like I2C or
   * WTIMER, or GPIO, but that's no reason not to take advantage of it
   * here. */
  uart_instance = ((uintptr_t)uart - (uintptr_t)UART0) / 0x1000;

  if (devcfgp->rx_pinmux.port) {
    rx_port_mask = 1U << iBSPACMdeviceTM4CgpioPortShift(devcfgp->rx_pinmux.port);
  }
  if (devcfgp->tx_pinmux.port) {
    tx_port_mask = 1U << iBSPACMdeviceTM4CgpioPortShift(devcfgp->tx_pinmux.port);
  }
  if (devcfgp->rts_pinmux.port) {
    rts_port_mask = 1U << iBSPACMdeviceTM4CgpioPortShift(devcfgp->rts_pinmux.port);
  }
  if (devcfgp->cts_pinmux.port) {
    cts_port_mask = 1U << iBSPACMdeviceTM4CgpioPortShift(devcfgp->cts_pinmux.port);
  }

  /* If enabling configuration, enable clocks to the GPIOs and UART.
   * Do GPIO first, so the minimum 3-cycle delay between enabling
   * clocks and manipulating the module is consumed while we're
   * turning on the UART.
   *
   * If disabling configuration, reset the UART (if it's enabled) and
   * remove interrupts. */
  if (cfgp) {
    uint32_t rcgcgpio = rx_port_mask | tx_port_mask;
    if (devcfgp->rts_pinmux.pctl) {
      rcgcgpio |= rts_port_mask;
    }
    if (devcfgp->cts_pinmux.pctl) {
      rcgcgpio |= cts_port_mask;
    }
    SYSCTL->RCGCGPIO |= rcgcgpio;
    BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCUART, uart_instance) = 1;
  } else {
    /* Disable and clear interrupts at the NVIC, then shut down the
     * UART. */
    NVIC_DisableIRQ(devcfgp->irqn);
    NVIC_ClearPendingIRQ(devcfgp->irqn);
    BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCUART, uart_instance) = 0;
  }

  /* Enable or disable GPIO pins used by UART, as long as the GPIO
   * module is on.  Note that RTS and CTS may not be available. */
  if (SYSCTL->PRGPIO & rx_port_mask) {
    vBSPACMdeviceTM4CpinmuxConfigure(&devcfgp->rx_pinmux, !!cfgp, 1);
  }
  if (SYSCTL->PRGPIO & tx_port_mask) {
    vBSPACMdeviceTM4CpinmuxConfigure(&devcfgp->tx_pinmux, !!cfgp, 0);
  }
  if (devcfgp->rts_pinmux.pctl && (SYSCTL->PRGPIO & rts_port_mask)) {
    vBSPACMdeviceTM4CpinmuxConfigure(&devcfgp->rts_pinmux, !!cfgp, 0);
  }
  if (devcfgp->cts_pinmux.pctl && (SYSCTL->PRGPIO & cts_port_mask)) {
    vBSPACMdeviceTM4CpinmuxConfigure(&devcfgp->cts_pinmux, !!cfgp, 0);
  }

  /* Reset the FIFOs */
  if (usp->rx_fifo_ni_) {
    fifo_reset(usp->rx_fifo_ni_);
  }
  if (usp->tx_fifo_ni_) {
    fifo_reset(usp->tx_fifo_ni_);
  }
  usp->tx_state_ = 0;

  /* Configure UART as requested and bring it online. */
  if (cfgp) {
    uint32_t baud_rate = cfgp->speed_baud;
    uint32_t uart_clk_Hz = SystemCoreClock;
    uint32_t baud_rate_f64;

    if (0 == baud_rate) {
      baud_rate = 115200;
    }

    /* System clock divided by the UART divisor (16) and the baud
     * rate, scaled to represent in units of 1/64 bit (6-bit
     * fraction), plus 1/2 for rounding.  Convert this to an
     * integral and fractional portion. */
    baud_rate_f64 = ((baud_rate / 2) + 4 * uart_clk_Hz) / baud_rate;
    uart->IBRD = baud_rate_f64 / 64;
    uart->FBRD = baud_rate_f64 % 64;
    /* 8-bit, no parity, one stop bit, enable FIFOs*/
    uart->LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /* Clear all interrupts at the module and at the NVIC; enable at
     * the NVIC, then enable the UART */
    uart->ICR = uart->RIS;
    NVIC_ClearPendingIRQ(devcfgp->irqn);
    NVIC_EnableIRQ(devcfgp->irqn);
    uart->IM = UART_IM_RXIM | UART_IM_RTIM;
    uart->CTL = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
  }
  return 0;
}

static int
uart_hw_transmit (sBSPACMperiphUARTstate * usp,
                  uint8_t v)
{
  int rv = -1;
  UART0_Type * const uart = (UART0_Type *)usp->uart;
  if (! (UART_FR_TXFF & uart->FR)) {
    uart->DR = v;
    usp->tx_count += 1;
    rv = v;
  }
  return rv;
}

static void
uart_hw_txien (sBSPACMperiphUARTstate * usp,
               int enablep)
{
  UART0_Type * const uart = (UART0_Type *)usp->uart;
  if (enablep) {
    uart->IM |= UART_IM_TXIM;
  } else {
    uart->IM &= ~UART_IM_TXIM;
  }
}

static int
uart_fifo_state (sBSPACMperiphUARTstate * usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  UART0_Type * const uart = (UART0_Type *)usp->uart;
  int rv = 0;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if (UART_FR_TXFE != ((UART_FR_TXFE | UART_FR_BUSY) & uart->FR)) {
      rv |= eBSPACMperiphUARTfifoState_HWTX;
    }
    if (! (UART_FR_RXFE & uart->FR)) {
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

void
vBSPACMdeviceTM4CperiphUARTirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  UART0_Type * const uart = (UART0_Type *)usp->uart;

  BSPACM_CORE_DISABLE_INTERRUPT();
  uart->ICR = (~ UART_MIS_TXMIS) & uart->MIS;
  while (! (UART_FR_RXFE & uart->FR)) {
    uint8_t dr = uart->DR;
    uint32_t rsr = uart->RSR;
    if (rsr) {
      /* Warning: this clears all errors, not just the ones we just
       * captured and are processing. */
      uart->RSR = rsr;
      if (UART_RSR_FE & rsr) {
        usp->rx_frame_errors += 1;
      }
      if (UART_RSR_PE & rsr) {
        usp->rx_parity_errors += 1;
      }
      if (UART_RSR_BE & rsr) {
        usp->rx_break_errors += 1;
      }
      if (UART_RSR_OE & rsr) {
        usp->rx_overrun_errors += 1;
      }
    } else {
      if ((! usp->rx_fifo_ni_)
          || (0 > fifo_push_head(usp->rx_fifo_ni_, dr))) {
        usp->rx_dropped_errors += 1;
      }
      usp->rx_count += 1;
    }
  }
  if (usp->tx_fifo_ni_) {
    while (! (fifo_empty(usp->tx_fifo_ni_) || (UART_FR_TXFF & uart->FR))) {
      uart->DR = fifo_pop_tail(usp->tx_fifo_ni_, 0);
      usp->tx_count += 1;
    }
    if (fifo_empty(usp->tx_fifo_ni_)) {
      uart->IM &= ~UART_IM_TXIM;
    }
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

const sBSPACMperiphUARToperations xBSPACMdeviceTM4CperiphUARToperations = {
  .configure = uart_configure,
  .hw_transmit = uart_hw_transmit,
  .hw_txien = uart_hw_txien,
  .fifo_state = uart_fifo_state,
};
