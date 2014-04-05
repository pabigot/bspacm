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

/** @file
 *
 * @brief Generic UART interface for BSPACM
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_PERIPH_UART_H
#define BSPACM_PERIPH_UART_H

#include <bspacm/core.h>

/* Forward declaration */
struct sBSPACMperiphUARToperations;
struct sFIFO;

/** State associated with a UART device.
 *
 * An instance of this structure is uniquely associated with each UART
 * peripheral that is supported by an application. */
typedef struct sBSPACMperiphUARTstate {
  /** Reference to the device.  This is a pointer to the
   * peripheral-specific base CMSIS device structure; interpreted as a
   * @c uint32_t is is the peripheral base address. */
  void * const uart;

  /** A field that contains device-specific auxiliary information that
   * cannot be inferred from the base address.  For example, an
   * identifier for the module clock. */
  union {
    uint32_t u32;               /**< some integral value */
    void * ptr;                 /**< null, or a pointer to an internal structure */
  } const devcfg;

  /** Pointer to the operations table that implements UART
   * functionality for this device.  Users mostly won't need to touch
   * this. */
  const struct sBSPACMperiphUARToperations * const ops;

  /** Pointer to a device-specific software FIFO to hold data to be
   * transmitted. */
  struct sFIFO * const tx_fifo;

  /** Pointer to a device-specific software FIFO to hold data that has
   * been received but not accepted by the application. */
  struct sFIFO * const rx_fifo;

  /** Flags controlling the behavior of the UART at the BSPACM
   * layer. */
  unsigned int flags;

  /** The total number of characters received at the hardware
   * interface.  This includes characters that were dropped due to
   * lack of space in the software fifo (#rx_dropped_errors). */
  unsigned int rx_count;

  /** The total number of characters transmitted over the hardware
   * interface. */
  unsigned int tx_count;

  /** The number of times a newly received character at the hardware
   * interface caused a previously received character to be dropped
   * from the software FIFO. */
  uint16_t rx_dropped_errors;

  /** The number of framing errors detected by hardware */
  uint8_t rx_frame_errors;

  /** The number of parity errors detected by hardware */
  uint8_t rx_parity_errors;

  /** The number of break conditions ("errors") detected by
   * hardware */
  uint8_t rx_break_errors;

  /** The number of overrun errors detected by hardware */
  uint8_t rx_overrun_errors;
} sBSPACMperiphUARTstate;

/** Collected configuration information used to enable a UART.
 *
 * @note At the moment, configuration options are sparse. */
typedef struct sBSPACMperiphUARTconfiguration {
  /** The speed at which the UART should run, in baud (symbols to
   * second; think 9600, 38400, 115200, etc.) */
  unsigned int speed_baud;
} sBSPACMperiphUARTconfiguration;

/** Bits set in the return code of iBSPACMperiphUARTfifoState() to
 * indicate where there is unflushed material. */
typedef enum eBSPACMperiphUARTfifoState {
  /** Indicates there is material waiting to be read from the hardware
   * receive buffer */
  eBSPACMperiphUARTfifoState_HWRX = 0x01,

  /** Indicates there is material waiting to be written in the
   * hardware transmit buffer.  This includes material that is still
   * in a shift register; the bit clears only when the transmission is
   * fully complete to the point where the UART can be shut down
   * without loss of data. */
  eBSPACMperiphUARTfifoState_HWTX = 0x02,

  /** Indicates there is material waiting to be read from the software
   * receive FIFO */
  eBSPACMperiphUARTfifoState_SWRX = 0x04,

  /** Indicates there is material waiting to be written in the
   * software transmit FIFO */
  eBSPACMperiphUARTfifoState_SWTX = 0x08,
} eBSPACMperiphUARTfifoState;

/** The set of operations supported by UARTs.
 *
 * The underlying implementation is specific to a vendor peripheral
 * capable of acting like a UART.
 *
 * @note The contents of this structure are not intended to be public
 * API. */
typedef struct sBSPACMperiphUARToperations {
  /** Configure (or deconfigure) a UART.
   *
   * @param usp the UART peripheral state
   *
   * @param cfgp if non-null, a pointer to information used to
   * configure the peripheral.  If null, a sign that the peripheral is
   * no longer being used and should be shut down/deconfigured.
   *
   * @return zero on successful (de-)configuration, otherwise a
   * negative error code. */
  int (* configure) (sBSPACMperiphUARTstate * usp,
                     const sBSPACMperiphUARTconfiguration * cfgp);

  /** Attempt to transmit an octet by adding it to the hardware
   * transmit FIFO.
   *
   * @param usp the UART abstraction being used
   * @param v the octet to be transmitted
   * @return @p v if the transmission was accepted by the hardware,
   * otherwise a negative error code.
   */
  int (* hw_transmit) (sBSPACMperiphUARTstate * usp, uint8_t v);

  /** Enable or disable the interrupt associated with UART transmission.
   *
   * The management model of this implementation is that the UART
   * transmit interrupt is enabled only when there is pending material
   * in the software transmit fifo.  Thus it is the caller's
   * responsibility to queue data and enable the interrupt when
   * #hw_transmit fails.
   *
   * Under normal circumstances the transmit interrupt is disabled
   * within the interrupt handler when the software FIFO is
   * emptied.
   *
   * @note On some platforms (include TM4C) space in the hardware fifo
   * *must* be filled before enabling the TX interrupt will work,
   * because that interrupt is signalled by a full-to-not-full
   * transmission.  On other platforms (including EFM32) the interrupt
   * is constant buffer-space-available signal, for which filling the
   * hardware fifo is fine albeit not necessary.
   *
   * @param usp the UART abstraction being used
   *
   * @param enablep a non-zero value if the interrupt is to be
   * enabled; a zero value will disable the interrupt (not a normal
   * way to use this function). */
  void (* hw_txien) (sBSPACMperiphUARTstate * usp, int enablep);

  /** Determine whether there is anything pending in the device:
   * material that has been received but not consumed by the
   * application, or material that has been submitted for transmission
   * but has not yet gone out over the channel.
   *
   * @param usp the UART abstraction being used
   *
   * @return a combination of bits defined in
   * #eBSPACMperiphUARTfifoState.  A value of zero indicates that no
   * material is pending.  A negative value indicates an error,
   * e.g. that the UART is unconfigured. */
  int (* fifo_state) (sBSPACMperiphUARTstate * usp);

} sBSPACMperiphUARToperations;

/** If set, iBSPACMperiphUARTwrite() will translate any newline (ASCII
 * @c LF, hex @c 0x0a) in the output buffer output into a synthesized
 * sequence <tt>CR LF</tt> (<tt>0x0d 0x0a</tt>). */
#define BSPACM_PERIPH_UART_FLAG_ONLCR 0x01

/** Control whether the iBSPACMperiphUARTwrite() is permitted to
 * return immediately if it cannot output any data.
 *
 * This flag has an effect only when the underlying UART has no more
 * space in its hardware or software FIFOs.
 *
 * If clear (default), iBSPACMperiphUARTwrite() invoked with a
 * positive @p count will enable interrupts and block until at least
 * one byte is written; if no space is available after that, it will
 * return with a partial write.
 *
 * If set, iBSPACMperiphUARTwrite() may return having written no data
 * if there is no space.
 *
 * In any case, if #BSPACM_PERIPH_UART_FLAG_ONLCR or other flags
 * require iBSPACMperiphUARTwrite() to translate a single byte into a
 * multi-byte sequence, it will block with interrupts enabled until an
 * error or the complete synthesized sequence has been queued for
 * transmission.
 */
#define BSPACM_PERIPH_UART_FLAG_NONBLOCK 0x02

/** Configure (or deconfigure) a UART.
 *
 * @param usp the UART peripheral state
 *
 * @param cfgp if non-null, a pointer to information used to
 * configure the peripheral.  If null, a sign that the peripheral is
 * no longer being used and should be shut down/deconfigured.
 *
 * @return @p usp on successful (de-)configuration, otherwise a null
 * pointer value to indicate an error. */
static BSPACM_CORE_INLINE
sBSPACMperiphUARTstate *
hBSPACMperiphUARTconfigure (sBSPACMperiphUARTstate * usp,
                            const sBSPACMperiphUARTconfiguration * cfgp) {
  if (!!usp && (0 == usp->ops->configure(usp, cfgp))) {
    return usp;
  }
  return 0;
}

/** Read data from a UART.
 *
 * This call attempts to read up to @p count bytes of data into the
 * buffer at @p buf.
 *
 * @param usp the UART peripheral state.  The peripheral must be
 * configured and active.
 *
 * @param buf location into which data should be stored
 *
 * @param count the number of bytes that should be read
 *
 * @return the number of bytes actually read (which may be zero
 * depending on receiver state), or a negative error code. */
int iBSPACMperiphUARTread (sBSPACMperiphUARTstate * usp, void * buf, size_t count);

/** Write data to a UART.
 *
 * @warning This function will enable interrupts at certain points to
 * allow queued data to be transmitted.  Be aware that other
 * interrupts may be serviced during these times, and that an
 * arbitrary amount of data queued through this function may be
 * transmitted prior to the function's return.
 *
 * @warning If interrupts were disabled when the function was entered,
 * they will be disabled before it returns, and it is the caller's
 * responsibility to ensure they are re-enabled if necessary to
 * complete transmission of queued data.
 *
 * @param usp the UART peripheral state
 *
 * @param buf location from which transmitted data is read
 *
 * @param count the number of bytes that should be written
 *
 * @return the number of bytes actually written (which may be zero
 * depending on UART configuration flags and transmitter state), or a
 * negative error code. */
int iBSPACMperiphUARTwrite (sBSPACMperiphUARTstate * usp, const void * buf,  size_t count);

/** Determine whether there is anything pending in the device:
 * material that has been received but not consumed by the
 * application, or material that has been submitted for transmission
 * but has not yet gone out over the channel.
 *
 * @param usp the UART abstraction
 *
 * @return a combination of bits defined in
 * #eBSPACMperiphUARTfifoState.  A value of zero indicates that no
 * material is pending.  A negative value indicates an error,
 * e.g. that the UART is unconfigured. */
static BSPACM_CORE_INLINE
int iBSPACMperiphUARTfifoState (sBSPACMperiphUARTstate * usp) {
  if (! usp) {
    return -1;
  }
  return usp->ops->fifo_state(usp);
}


/** Include the device-specific file that declares the objects and
 * functions that provide UART capability on the board. */
#include <bspacm/internal/periph/uart.h>

#endif /* BSPACM_PERIPH_UART_H */
