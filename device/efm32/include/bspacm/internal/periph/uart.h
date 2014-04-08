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

#ifndef BSPACM_DEVICE_EFM32_INTERNAL_PERIPH_UART_H
#define BSPACM_DEVICE_EFM32_INTERNAL_PERIPH_UART_H

/** @file
 *
 * @brief EFM32 series-specific UART interface for BSPACM
 *
 * Declares the public data types, objects, and functions that are
 * required to make use of a UART peripheral.
 *
 * There are three matching functional capabilities on EFM32: USART,
 * UART, and LEUART.  Any device that has at least one UART also has
 * at least one USART.  The USART functionality is a superset of UART
 * functionality, and the register map is identical except for
 * unimplemented functionality in UART.  We shall share much of the
 * USART implementation with the UART, and shall use the USART
 * designation for shared data structures and functions.  Individual
 * peripherals will be named uniquely and correctly representing their
 * module type.
 *
 * @warning Peripheral instance objects such as
 * #xBSPACMdeviceEFM32periphUSART0 will exist only if they are
 * supported on the device being used and a definition is provided in
 * the peripheral configuration source file identified in the @c
 * PERIPH_CONFIG_SRC make variable.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/** Device-specific information for an EFM32 USART/UART instances.
 *
 * Contains the information that cannot be easily deduced from the
 * peripheral base address. */
typedef struct sBSPACMdeviceEFM32periphUSARTdevcfg {
  uint32_t clock;           /**< The peripheral clock configuration */
  int16_t tx_irqn;          /**< Transmit interrupt offset in NVIC */
  int16_t rx_irqn;          /**< Receive interrupt offset in NVIC */
  uint32_t location;        /**< Routing selection for USART */
} sBSPACMdeviceEFM32periphUSARTdevcfg;

/** The operations table for USART/UART devices.
 *
 * This is used to initialize sBSPACMperiphUARTstate::ops.
 *
*/
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

/** State for USART0 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART0;
/** State for USART1 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART1;
/** State for USART2 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUSART2;
/** State for UART0 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUART0;
/** State for UART1 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphUART1;
/** State for LEUART0 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphLEUART0;
/** State for LEUART1 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceEFM32periphLEUART1;

#endif /* BSPACM_DEVICE_EFM32_INTERNAL_PERIPH_UART_H */
