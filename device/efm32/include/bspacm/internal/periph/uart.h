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

#ifndef BSPACM_INTERNAL_EFM32_PERIPH_UART_H
#define BSPACM_INTERNAL_EFM32_PERIPH_UART_H

/** @file
 *
 * @brief EFM32 series-specific UART interface for BSPACM
 *
 * Declares the public data types, objects, and functions that are
 * required to make use of a UART peripheral.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/** Device-specific information for an EFM32 UART.
 *
 * Contains the information that cannot be easily deduced from the
 * peripheral base address. */
typedef struct sBSPACMdeviceEFM32periphUARTdevcfg {
  uint32_t clock;           /**< The peripheral clock configuration */
  int16_t tx_irqn;          /**< Transmit interrupt offset in NVIC */
  int16_t rx_irqn;          /**< Receive interrupt offset in NVIC */
  uint32_t location;        /**< Routing selection for UART */
} sBSPACMdeviceEFM32periphUARTdevcfg;

/** The operations table for UART devices.
 *
 * This is used to initialize sBSPACMperiphUARTstate::ops.
 *
 * @note EFM32 uses UART and USART.  Any device that has at least one
 * UART also has at least one USART, the USART functionality is a
 * superset of UART functionality, and the register map is identical
 * except for unimplemented functionality in UART.  We shall refer to
 * the device as USART.*/
extern const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphUSARToperations;

/** The IRQHandler for UART RX interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceEFM32periphUSARTrxirqhandler (sBSPACMperiphUARTstate * const usp);

/** The IRQHandler for UART TX interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceEFM32periphUSARTtxirqhandler (sBSPACMperiphUARTstate * const usp);

extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART0;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART1;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART2;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUART0;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUART1;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphLEUART0;
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphLEUART1;

#endif /* BSPACM_INTERNAL_EFM32_PERIPH_UART_H */
