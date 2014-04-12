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

#include <bspacm/periph/uart.h>
#include <bspacm/internal/utility/fifo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

__attribute__((__weak__))
const hBSPACMperiphUART hBSPACMdefaultUART = 0;

int
iBSPACMperiphUARTread (sBSPACMperiphUARTstate * usp, void * buf, size_t count)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  uint8_t * const bps = (uint8_t *)buf;
  int rv = -1;

  if (usp->rx_fifo_ni_) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      rv = fifo_pop_into_buffer(usp->rx_fifo_ni_, bps, bps+count);
    } while (0);
    BSPACM_CORE_REENABLE_INTERRUPT(istate);
  }
  return rv;
}

int
iBSPACMperiphUARTwrite (sBSPACMperiphUARTstate * usp, const void * buf,  size_t count)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  const uint8_t * const bps = (const uint8_t *)buf;
  const uint8_t * bp = bps;
  const uint8_t * const bpe = bp + count;
  bool did_transmit = true;
  uint8_t state;

  state = usp->tx_state_;
  while (did_transmit
         && ((0 != state)       /* partial sequence in progress */
             || (bp < bpe))) {  /* more to write */
    uint8_t bp_adjustment = 1;
    uint8_t next_state;
    uint8_t to_transmit;

    /* If we're not in a conversion state, see if there's
     * conversion to be done on the next character. */
    if (0 == state) {
      /* Maybe convert output LF to CR+LF. */
      if ((BSPACM_PERIPH_UART_FLAG_ONLCR & usp->flags) && ('\n' == *bp)) {
        state = '\r';
      }
    }
    if ('\r' == state) {
      to_transmit = state;
      bp_adjustment = 0;
      next_state = '\n';
    } else if ('\n' == state) {
      to_transmit = state;
      bp_adjustment = 1;
      next_state = 0;
    } else {
      to_transmit = *bp;
      bp_adjustment = 1;
      next_state = 0;
    }

    /* tx_fifo_ni_ is shared between this module and IRQHandler.  Enforce
     * mutex while putting the output onto the SW fifo it there's
     * already data there, or onto the hardware fifo with a fallback
     * to the SW fifo. */
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      int fifo_state = usp->ops->fifo_state(usp);
      if ((eBSPACMperiphUARTfifoState_SWTX & fifo_state)
          || (0 > usp->ops->hw_transmit(usp, to_transmit))) {
        if (usp->tx_fifo_ni_) {
          bool empty_on_entry = fifo_empty(usp->tx_fifo_ni_);
          if (fifo_full(usp->tx_fifo_ni_)) {
            did_transmit = false;
          } else {
            did_transmit = (0 <= fifo_push_head(usp->tx_fifo_ni_, to_transmit));
            if (did_transmit && empty_on_entry) {
              /* TX interrupts enabled as long as there's material in
               * the SW fifo. */
              usp->ops->hw_txien(usp, 1);
            }
          }
        } else {
          did_transmit = false;
        }
      }
    } while (0);
    if (did_transmit) {
      bp += bp_adjustment;
      state = next_state;
    }
    usp->tx_state_ = state;
    BSPACM_CORE_REENABLE_INTERRUPT(istate);
  }
  return bp - bps;
}

int iBSPACMperiphUARTflush (hBSPACMperiphUART usp,
                            int fifo_mask)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  unsigned int ena_mask = fifo_mask & (eBSPACMperiphUARTfifoState_SWTX | eBSPACMperiphUARTfifoState_RX);
  unsigned int dis_mask = fifo_mask & eBSPACMperiphUARTfifoState_HWTX;
  int rv;

  /* Phase 1: block on unsatisfied SW transmit and SW/HW RX.  These
   * conditions can only change during periods when interrupts are
   * enabled. */
  while (1) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    rv = iBSPACMperiphUARTfifoState(usp);
    if (0 > rv) {
      break;
    }
    if (0 == (ena_mask & rv)) {
      break;
    }
    BSPACM_CORE_SLEEP();
    BSPACM_CORE_ENABLE_INTERRUPT();
  }
  /* Phase 2: block on unsatisfied HW transmit.  This condition will
   * change without enabling interrupts, and if we tried to enable
   * interrupts we might deadlock because it had already changed. */
  while ((0 < rv) && (0 != (dis_mask & rv))) {
    rv = iBSPACMperiphUARTfifoState(usp);
  }
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}
