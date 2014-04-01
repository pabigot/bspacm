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
  const sBSPACMdeviceTM4CpinmuxUART * pmp = xBSPACMdeviceTM4CpinmuxUART;
  const sBSPACMdeviceTM4CpinmuxUART * const pmpe = pmp + nBSPACMdeviceTM4CpinmuxUART;
  UART0_Type * uart;
  uint32_t uart_base;
  unsigned int uart_idx;

  if (! (usp && usp->uart)) {
    return -1;
  }
  uart = (UART0_Type *)usp->uart;
  uart_base = (uint32_t)usp->uart;

  /* Find the pinmux configuration for the selected UART.  Fail if
   * there isn't one, or if it doesn't provide at least RX and TX
   * pins. */
  while ((pmp < pmpe) && (pmp->uart_base != uart_base)) {
    ++pmp;
  }
  if (pmp >= pmpe) {
    return -1;
  }
  if (! (pmp->rx_pinmux.pctl && pmp->tx_pinmux.pctl)) {
    return -1;
  }

  /* Determine the UART index.  Fail if we can't figure it out. */
  uart_idx = 0;
  while (uart_idx < nBSPACMdeviceTM4CperiphUART) {
    if ((uint32_t)uart == wBSPACMdeviceTM4CperiphUART[uart_idx]) {
      break;
    }
    ++uart_idx;
  }
  if (nBSPACMdeviceTM4CperiphUART == uart_idx) {
    return -1;
  }

  /* If enabling configuration, enable clocks to the GPIOs and UART.
   * Do GPIO first, so the minimum 3-cycle delay between enabling
   * clocks and manipulating the module is consumed while we're
   * turning on the UART.
   *
   * If disabling configuration, reset the UART (if it's enabled) and
   * remove interrupts. */
  if (cfgp) {
    uint32_t rcgcgpio = (SYSCTL_RCGCGPIO_R0 << pmp->rx_pinmux.port_shift);
    rcgcgpio |= (SYSCTL_RCGCGPIO_R0 << pmp->tx_pinmux.port_shift);
    if (pmp->rts_pinmux.pctl) {
      rcgcgpio |= (SYSCTL_RCGCGPIO_R0 << pmp->rts_pinmux.port_shift);
    }
    if (pmp->cts_pinmux.pctl) {
      rcgcgpio |= (SYSCTL_RCGCGPIO_R0 << pmp->cts_pinmux.port_shift);
    }
    SYSCTL->RCGCGPIO |= rcgcgpio;
    SYSCTL->RCGCUART |= (SYSCTL_RCGCUART_R0 << uart_idx);
  } else {
    /* Disable the UART, disable its interrupt vector, then turn off its clock. */
    if (SYSCTL->RCGCUART & (SYSCTL_RCGCUART_R0 << uart_idx)) {
      uart->CTL = 0;
      SYSCTL->RCGCUART &= ~(SYSCTL_RCGCUART_R0 << uart_idx);
    }
    NVIC->ISER[pmp->irqn / 32] &= ~(1U << (pmp->irqn % 32));
  }

  /* Enable or disable GPIO pins used by UART, as long as the GPIO
   * module is on.  Note that RTS and CTS may not be available. */
  if (SYSCTL->RCGCGPIO & (1U << pmp->rx_pinmux.port_shift)) {
    vBSPACMdeviceTM4CpinmuxConfigure(&pmp->rx_pinmux, !!cfgp, 1);
  }
  if (SYSCTL->RCGCGPIO & (1U << pmp->tx_pinmux.port_shift)) {
    vBSPACMdeviceTM4CpinmuxConfigure(&pmp->tx_pinmux, !!cfgp, 0);
  }
  if (pmp->rts_pinmux.pctl
      && (SYSCTL->RCGCGPIO & (1U << pmp->rts_pinmux.port_shift))) {
    vBSPACMdeviceTM4CpinmuxConfigure(&pmp->rts_pinmux, !!cfgp, 0);
  }
  if (pmp->cts_pinmux.pctl
      && (SYSCTL->RCGCGPIO & (1U << pmp->cts_pinmux.port_shift))) {
    vBSPACMdeviceTM4CpinmuxConfigure(&pmp->cts_pinmux, !!cfgp, 0);
  }

  /* Reset the FIFOs */
  fifo_reset(usp->rx_fifo);
  fifo_reset(usp->tx_fifo);

  /* Configure UART as requested and bring it online. */
  if (cfgp) {
    const uint32_t baud_rate = cfgp->speed_baud;
    uint32_t uart_clk_Hz = SystemCoreClock;
    uint32_t baud_rate_f64;

    /* System clock divided by the UART divisor (16) and the baud
     * rate, scaled to represent in units of 1/64 bit (6-bit
     * fraction), plus 1/2 for rounding.  Convert this to an
     * integral and fractional portion. */
    baud_rate_f64 = ((baud_rate / 2) + 4 * uart_clk_Hz) / baud_rate;
    uart->IBRD = baud_rate_f64 / 64;
    uart->FBRD = baud_rate_f64 % 64;
    /* 8-bit, no parity, one stop bit, enable FIFOs*/
    uart->LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /* Enable the UART */
    NVIC->ISER[pmp->irqn / 32] |= (1U << (pmp->irqn % 32));
    uart->IM = UART_IM_RXIM | UART_IM_RTIM;
    uart->CTL = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
  }
  return 0;
}

static int
uart_hw_transmit (sBSPACMperiphUARTstate * usp,
                  uint8_t v)
{
  UART0_Type * const uart = (UART0_Type *)usp->uart;
  if (UART_FR_TXFF & uart->FR) {
    return -1;
  }
  uart->DR = v;
  usp->tx_count += 1;
  return v;
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

void
vBSPACMdeviceTM4CperiphUARTirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  UART0_Type * const uart = (UART0_Type *)usp->uart;

  BSPACM_CORE_DISABLE_INTERRUPT();
  uart->ICR = (~ UART_MIS_TXMIS) & uart->MIS;
  while(! (UART_FR_RXFE & uart->FR)) {
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
      if (0 > fifo_push_head(usp->rx_fifo, dr)) {
        usp->rx_dropped_errors += 1;
      }
      usp->rx_count += 1;
    }
  }
  while(! (fifo_empty(usp->tx_fifo) || (UART_FR_TXFF & uart->FR))) {
    uart->DR = fifo_pop_tail(usp->tx_fifo, 0);
    usp->tx_count += 1;
  }
  if (fifo_empty(usp->tx_fifo)) {
    uart->IM &= ~UART_IM_TXIM;
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

const sBSPACMperiphUARToperations xBSPACMdeviceTM4CperiphUARToperations = {
  .configure = uart_configure,
  .hw_transmit = uart_hw_transmit,
  .hw_txien = uart_hw_txien,
};
