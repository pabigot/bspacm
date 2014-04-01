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

int
iBSPACMperiphUARTread (sBSPACMperiphUARTstate * usp, void * buf, size_t nbyte)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  uint8_t * const bps = (uint8_t *)buf;
  int rv;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    rv = fifo_pop_into_buffer(usp->rx_fifo, bps, bps+nbyte);
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  if (0 == rv) {
    errno = EAGAIN;
    rv = -1;
  }
  return rv;
}

int
iBSPACMperiphUARTwrite (sBSPACMperiphUARTstate * usp, const void * buf,  size_t nbyte)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  const uint8_t * const bps = (const uint8_t *)buf;
  const uint8_t * bp = bps;
  const uint8_t * const bpe = bp + nbyte;
  int state = -1;
  unsigned int flags = usp->flags;

  while ((0 <= state)           /* partial sequence in progress */
         || ((bp < bpe)         /* more to write */
             && (bp == bps)     /* haven't written anything */
             && ! (BSPACM_PERIPH_UART_FLAG_NONBLOCK & flags) /* have to write something */
             )) {
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      uint16_t head = usp->tx_fifo->head;
      uint16_t const tail = usp->tx_fifo->tail;

      /* Loop doing output until done or the FIFO fills (break inside
       * loop) */
      while ((0 <= state) || (bp < bpe)) {
        uint16_t next_head = FIFO_ADJUST_OFFSET(usp->tx_fifo, head + 1U);
        int to_transmit = -1;

        /* If there's no space in the SW FIFO we have to stop. */
        if (next_head == tail) {
          break;
        }

        /* If we're not in a conversion state, see if there's
         * conversion to be done on the next character. */
        if (0 > state) {
          /* Maybe convert output LF to CR+LF */
          if ((BSPACM_PERIPH_UART_FLAG_ONLCR & flags) && ('\n' == *bp)) {
            state = '\r';
            ++bp;
          }
        }
        if ('\r' == state) {
          to_transmit = state;
          state = '\n';
        } else if ('\n' == state) {
          to_transmit = state;
          state = -1;
        } else {
          to_transmit = *bp++;
        }
        /* Add to end of SW FIFO if there's data already in the SW
         * FIFO or if the HW FIFO doesn't have room. */
        if ((head != tail)
            || (0 > usp->ops->hw_transmit(usp, to_transmit))) {
          usp->tx_fifo->cell[head] = to_transmit;
          if (head == tail) {
            usp->ops->hw_txien(usp, 1);
          }
          head = next_head;
        }
      }
      /* Write back the updated head. */
      usp->tx_fifo->head = head;
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();
  }
  BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  if (bp == bps) {
    errno = EAGAIN;
    return -1;
  }
  return bp - bps;
}
