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

/* Implementation for nRF51 device series UART peripheral interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/uart.h>
#include <bspacm/internal/utility/fifo.h>
#include "nrf_gpio.h"

/* Hardware flow control has not yet been validated */
#define ENABLE_HW_FLOW_CONTROL 0

/** A flag in sBSPACMperiphUARTstate::peripheral_state_ni that records
 * whether we're allowed to write to TXD. */
#define PERIPHERAL_FLAG_TXDRDY 0x01

/** A flag in sBSPACMperiphUARTstate::peripheral_state_ni that records
 * whether the TX task is running. */
#define PERIPHERAL_FLAG_TXTASK 0x02

/** Structure to map from a baud rate to the register configuration
 * value required to achieve that rate. */
typedef struct sNRF51baudMapEntry {
  uint32_t speed_baud;
  uint32_t baud_value;
} sNRF51baudMapEntry;

#ifndef BSPACM_NRF51_UART_EXTENDED_RATES
#define BSPACM_NRF51_UART_EXTENDED_RATES 0
#endif /* BSPACM_NRF51_UART_EXTENDED_RATES */

/* First entry is default.  Non-standard entries require special request.
 * WARNING: Code assumes this array has at least one entry. */
static const sNRF51baudMapEntry xNRF51baudMap[] = {
#define MAP_ENTRY(_r) { .speed_baud = _r, .baud_value = UART_BAUDRATE_BAUDRATE_Baud##_r },
  MAP_ENTRY(115200)
  MAP_ENTRY(1200)
  MAP_ENTRY(2400)
  MAP_ENTRY(9600)
  MAP_ENTRY(38400)
#if (BSPACM_NRF51_UART_EXTENDED_RATES - 0)
  MAP_ENTRY(4800)
  MAP_ENTRY(14400)
  MAP_ENTRY(19200)
  MAP_ENTRY(28800)
  MAP_ENTRY(57600)
  MAP_ENTRY(76800)
  MAP_ENTRY(230500)
  MAP_ENTRY(250000)
  MAP_ENTRY(460800)
  MAP_ENTRY(921600)
  { .speed_baud = 1000000U, .baud_value = UART_BAUDRATE_BAUDRATE_Baud1M }
#endif /* BSPACM_NRF51_UART_EXTENDED_RATES */
#undef MAP_ENTRY
};

static
int
uart_configure (sBSPACMperiphUARTstate * usp,
                const sBSPACMperiphUARTconfiguration * cfgp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  const sBSPACMdeviceNRF51periphUARTdevcfg * devcfgp;

  if (! (usp && (NRF_UART0 == usp->uart))) {
    return -1;
  }
  devcfgp = (const sBSPACMdeviceNRF51periphUARTdevcfg *)usp->devcfg.ptr;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if (cfgp) {
      nrf_gpio_cfg_input(devcfgp->rx_pin, NRF_GPIO_PIN_NOPULL);
      NRF_UART0->PSELRXD = devcfgp->rx_pin;
      nrf_gpio_cfg_output(devcfgp->tx_pin);
      NRF_UART0->PSELTXD = devcfgp->tx_pin;
#if (ENABLE_HW_FLOW_CONTROL - 0)
      if ((0 <= devcfgp->rts_pin) && (0 <= devcfgp->cts_pin)) {
        nrf_gpio_cfg_output(devcfgp->cts_pin);
        NRF_UART0->PSELCTS = devcfgp->cts_pin;
        nrf_gpio_cfg_input(devcfgp->rts_pin, NRF_GPIO_PIN_NOPULL);
        NRF_UART0->PSELRTS = devcfgp->rts_pin;
        NRF_UART0->CONFIG = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
      }
#endif /* ENABLE_HW_FLOW_CONTROL */
    } else {
      NRF_UART0->TASKS_STOPRX = 1;
      if (PERIPHERAL_FLAG_TXTASK & usp->peripheral_state_ni) {
        NRF_UART0->TASKS_STOPTX = 1;
        usp->peripheral_state_ni &= ~PERIPHERAL_FLAG_TXTASK;
      }

#if (ENABLE_HW_FLOW_CONTROL - 0)
      if ((0 <= devcfgp->rts_pin) && (0 <= devcfgp->cts_pin)) {
        nrf_gpio_cfg_output(devcfgp->rts_pin);
        NRF_GPIO->OUTSET = 1 << devcfgp->rts_pin;
        NRF_UART0->PSELCTS = -1;
        NRF_UART0->PSELRTS = -1;
        NRF_UART0->CONFIG &= ~(UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
      }
#endif /* ENABLE_HW_FLOW_CONTROL */

      NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

      /* Clear the driver state flags */
      usp->peripheral_state_ni = 0;
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
      const sNRF51baudMapEntry * mep = xNRF51baudMap + sizeof(xNRF51baudMap)/sizeof(*xNRF51baudMap) ;

      do {
        --mep;
        if (cfgp->speed_baud == mep->speed_baud) {
          break;
        }
      } while (xNRF51baudMap < mep);

      NRF_UART0->BAUDRATE = (mep->baud_value << UART_BAUDRATE_BAUDRATE_Pos);
      NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
      NRF_UART0->EVENTS_RXDRDY = 0;
      NRF_UART0->EVENTS_TXDRDY = 0;
      NRF_UART0->INTENCLR = ~0UL;
      NRF_UART0->INTENSET = ((UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos)
                             | (UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos)
                             | (UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos));
      NVIC_ClearPendingIRQ(UART0_IRQn);
      NVIC_EnableIRQ(UART0_IRQn);

      /* Record that we're allowed to write to TXD. */
      usp->peripheral_state_ni = PERIPHERAL_FLAG_TXDRDY;
      NRF_UART0->TASKS_STARTRX = 1;
    }
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return 0;
}

static int
uart_hw_transmit (sBSPACMperiphUARTstate * usp,
                  uint8_t v)
{
  int rv = -1;

  NRF_UART0->TASKS_STARTTX = 1;
  NRF_UART0->TXD = v;
  while (!NRF_UART0->EVENTS_TXDRDY);
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->TASKS_STOPTX = 1;
  usp->tx_count += 1;
  rv = v;
  return rv;
}

static void
uart_hw_txien (sBSPACMperiphUARTstate * usp,
               int enablep)
{
  /* The BSPACM layer will call this to enable TX interrupts when it
   * thinks it's queued data for the first time.  That doesn't work
   * for this device because enabling interrupts will not cause an
   * interrupt if the TXD has space.  We manage interrupts by
   * communication between fifo_state and the interrupt handler.
   *
   * Oh, and "interrupt" in this case means whether or not the TX task
   * has been started.  TXDRDY interrupts are always on. */
}

static int
uart_fifo_state (sBSPACMperiphUARTstate * usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  NRF_UART_Type * const uart = (NRF_UART_Type *)usp->uart;
  int rv = 0;

  BSPACM_CORE_DISABLE_INTERRUPT();
  (void)uart;
  do {
    if (! (PERIPHERAL_FLAG_TXDRDY & usp->peripheral_state_ni)) {
      rv |= eBSPACMperiphUARTfifoState_HWTX;
    }
    if (usp->tx_fifo_ni_ && (! fifo_empty(usp->tx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWTX;
    }
    if (usp->rx_fifo_ni_ && (! fifo_empty(usp->rx_fifo_ni_))) {
      rv |= eBSPACMperiphUARTfifoState_SWRX;
    }
    if (NRF_UART0->EVENTS_RXDRDY) {
      rv |= eBSPACMperiphUARTfifoState_HWRX;
    }
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

void
vBSPACMdeviceNRF51periphUARTirqhandler (sBSPACMperiphUARTstate * const usp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  uint32_t errorsrc;

  BSPACM_CORE_DISABLE_INTERRUPT();
  if (NRF_UART0->EVENTS_ERROR) {
    NRF_UART0->EVENTS_ERROR = 0;
    errorsrc = NRF_UART0->ERRORSRC;
    NRF_UART0->ERRORSRC = errorsrc;
    if (UART_ERRORSRC_BREAK_Present & errorsrc) {
      usp->rx_break_errors += 1;
    }
    if (UART_ERRORSRC_FRAMING_Present & errorsrc) {
      usp->rx_frame_errors += 1;
    }
    if (UART_ERRORSRC_PARITY_Present & errorsrc) {
      usp->rx_parity_errors += 1;
    }
    if (UART_ERRORSRC_OVERRUN_Present & errorsrc) {
      usp->rx_overrun_errors += 1;
    }
  }
  if (NRF_UART0->EVENTS_RXDRDY) {
    NRF_UART0->EVENTS_RXDRDY = 0;
    uint8_t rxd = NRF_UART0->RXD;
    if ((! usp->rx_fifo_ni_)
        || (0 > fifo_push_head(usp->rx_fifo_ni_, rxd))) {
      usp->rx_dropped_errors += 1;
    }
    usp->rx_count += 1;
  }
  if (NRF_UART0->EVENTS_TXDRDY) {
    usp->peripheral_state_ni |= PERIPHERAL_FLAG_TXDRDY;
    NRF_UART0->EVENTS_TXDRDY = 0;
    if (usp->tx_fifo_ni_ && (! fifo_empty(usp->tx_fifo_ni_))) {
      usp->peripheral_state_ni &= ~PERIPHERAL_FLAG_TXDRDY;
      if (! (PERIPHERAL_FLAG_TXTASK & usp->peripheral_state_ni)) {
        usp->peripheral_state_ni |= PERIPHERAL_FLAG_TXTASK;
        NRF_UART0->TASKS_STARTTX = 1;
      }
      NRF_UART0->TXD = fifo_pop_tail(usp->tx_fifo_ni_, 0);
      usp->tx_count += 1;
    } else {
      NRF_UART0->TASKS_STOPTX = 1;
      usp->peripheral_state_ni &= ~PERIPHERAL_FLAG_TXTASK;
    }
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

const sBSPACMperiphUARToperations xBSPACMdeviceNRF51periphUARToperations = {
  .configure = uart_configure,
  .hw_transmit = uart_hw_transmit,
  .hw_txien = uart_hw_txien,
  .fifo_state = uart_fifo_state,
};
