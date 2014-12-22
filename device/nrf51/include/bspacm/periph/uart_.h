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

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_UART_H
#define BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_UART_H

/** @file
 *
 * @brief NRF51 series-specific UART interface for BSPACM
 *
 * Declares the public objects and functions that are required to make
 * use of the UART0 peripheral.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/** Pin assignment structure for UART devices. */
typedef struct sBSPACMdeviceNRF51periphUARTdevcfg {
  int8_t rx_pin;               /**< Pin number for RX function */
  int8_t tx_pin;               /**< Pin number for TX function */
  int8_t rts_pin;              /**< Pin number for RTS function, negative if disabled */
  int8_t cts_pin;              /**< Pin number for CTS function, negative if disabled */
} sBSPACMdeviceNRF51periphUARTdevcfg;

/** The operations table for UART devices.
 *
 * This is used to initialize sBSPACMperiphUARTstate::ops. */
extern const sBSPACMperiphUARToperations xBSPACMdeviceNRF51periphUARToperations;

/** The IRQHandler for UART interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceNRF51periphUARTirqhandler (sBSPACMperiphUARTstate * const usp);

/** Device configuration information for UART0 peripheral, where this
 * exists.  The board @c periph_cfg.c file provides a weak definition
 * of this that may be overridden by the application. */
extern const sBSPACMdeviceNRF51periphUARTdevcfg xBSPACMdeviceNRF51periphUART0devcfg;

/** State for UART0 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceNRF51periphUART0;

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_UART_H */
