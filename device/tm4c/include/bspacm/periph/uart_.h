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

#ifndef BSPACM_DEVICE_TM4C_INTERNAL_PERIPH_UART_H
#define BSPACM_DEVICE_TM4C_INTERNAL_PERIPH_UART_H

#include <bspacm/periph/gpio.h>

/** @file
 *
 * @brief TM4C series-specific UART interface for BSPACM
 *
 * Declares the public objects and functions that are required to make
 * use of a UART peripheral.
 *
 * @warning Peripheral instance objects such as
 * #xBSPACMdeviceTM4CperiphUART0 will exist only if they are supported
 * on the device being used and a definition is provided in the
 * peripheral configuration source file identified in the @c
 * PERIPH_CONFIG_SRC make variable.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/** Pin assignment structure for UART devices. */
typedef struct sBSPACMdeviceTM4CperiphUARTdevcfg {
  /** The base address of the UART peripheral register map to which
   * the mapping applies.  For example, @c UART0_BASE.  The ordinal
   * peripheral index may be determined using
   * #wBSPACMdeviceTM4CperiphUART. */
  uint32_t uart_base;

  /** The port pin mux configuration for the UnRx signal. */
  sBSPACMdeviceTM4Cpinmux rx_pinmux;

  /** The port pin mux configuration for the UnTx signal. */
  sBSPACMdeviceTM4Cpinmux tx_pinmux;

  /** The port pin mux configuration for the UnRTS signal.  Leave as
   * zeros if not configured. */
  sBSPACMdeviceTM4Cpinmux rts_pinmux;

  /** The port pin mux configuration for the UnCTS signal.  Leave as
   * zeros if not configured. */
  sBSPACMdeviceTM4Cpinmux cts_pinmux;

  /** The IRQ number corresponding to #uart_base.
   *
   * @note Values are truly interrupt numbers (e.g. @c UART0_IRQn from
   * the CMSIS device header), not exception numbers (e.g. @c
   * INT_UART0 from TivaWare <int/hw_ints.h>).  Using the latter
   * numbering will result in values offset by 16. */
  uint16_t irqn;

  /** The ordinal instance number of the UART within its device. */
  uint8_t instance;
} sBSPACMdeviceTM4CperiphUARTdevcfg;

/** The operations table for UART devices.
 *
 * This is used to initialize sBSPACMperiphUARTstate::ops. */
extern const sBSPACMperiphUARToperations xBSPACMdeviceTM4CperiphUARToperations;

/** The IRQHandler for UART interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceTM4CperiphUARTirqhandler (sBSPACMperiphUARTstate * const usp);

/** Device configuration information for UART0 peripheral, where this
 * exists.  The board @c periph_cfg.c file provides a weak definition
 * of this that may be overridden by the application. */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART0devcfg;
/** UART1 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART1devcfg;
/** UART2 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART2devcfg;
/** UART3 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART3devcfg;
/** UART4 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART4devcfg;
/** UART5 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART5devcfg;
/** UART6 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART6devcfg;
/** UART7 version of #xBSPACMdeviceTM4CperiphUART0devcfg */
extern const sBSPACMdeviceTM4CperiphUARTdevcfg xBSPACMdeviceTM4CperiphUART7devcfg;

/** State for UART0 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART0;
/** State for UART1 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART1;
/** State for UART2 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART2;
/** State for UART3 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART3;
/** State for UART4 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART4;
/** State for UART5 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART5;
/** State for UART6 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART6;
/** State for UART7 peripheral, where this exists. */
extern sBSPACMperiphUARTstate xBSPACMdeviceTM4CperiphUART7;

#endif /* BSPACM_DEVICE_TM4C_INTERNAL_PERIPH_UART_H */
