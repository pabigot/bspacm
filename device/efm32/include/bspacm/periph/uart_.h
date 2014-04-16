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

#include <bspacm/periph/gpio.h>

/** The intersection of configuration information relevant to all
 * EFM32 devices that support UART functionality: USART, UART, and
 * LEUART. */
typedef struct sBSPACMdeviceEFM32periphXRTdevcfg {
  /** The base address of the UART peripheral register map to which
   * the mapping applies.  From the CMSIS header, e.g. @c
   * USART1_BASE. */
  uint32_t uart_base;

  /** The port pin mux configuration for the RX (MISO) signal. */
  sBSPACMdeviceEFM32pinmux rx_pinmux;

  /** The port pin mux configuration for the TX (MOSI) signal. */
  sBSPACMdeviceEFM32pinmux tx_pinmux;

  /** The peripheral clock configuration that must be enabled.  From
   * the <em_cmu.h> header, e.g. @c cmuClock_USART1 */
  uint32_t clock;

  /** Routing selection for USART.  From the CMSIS header, e.g. @c
   * USART_ROUTE_LOCATION_LOC1 */
  uint16_t location;
} sBSPACMdeviceEFM32periphXRTdevcfg;

/** Pin assignment structure for USART devices.
 *
 * These support both asynchronous (UART) and synchronous
 * (SPI/SSI/I2S) capabilities. */
typedef struct sBSPACMdeviceEFM32periphUSARTdevcfg {
  /** Configuration for which code can be shared with other devices. */
  sBSPACMdeviceEFM32periphXRTdevcfg common;

  /** The port pin mux configuration for the CLK signal. */
  sBSPACMdeviceEFM32pinmux clk_pinmux;

  /** The port pin mux configuration for the CS signal.  Used only
   * when the USART controls the CS line. */
  sBSPACMdeviceEFM32pinmux cs_pinmux;

  /** Transmit interrupt offset in NVIC.  From the CMSIS header,
   * e.g. @c USART1_TX_IRQn */
  uint8_t tx_irqn;

  /** Receive interrupt offset in NVIC.  From the CMSIS header,
   * e.g. @c USART1_RX_IRQn */
  uint8_t rx_irqn;
} sBSPACMdeviceEFM32periphUSARTdevcfg;

/** Device-specific information for an EFM32 UART device.
 *
 * The UART is functionally and structurally equivalent to the
 * asynchronous portion of the USART. */
typedef struct sBSPACMdeviceEFM32periphUARTdevcfg {
  /** Configuration for which code can be shared with other devices. */
  sBSPACMdeviceEFM32periphXRTdevcfg common;

  /** Transmit interrupt offset in NVIC.  From the CMSIS header,
   * e.g. @c USART1_TX_IRQn */
  uint8_t tx_irqn;

  /** Receive interrupt offset in NVIC.  From the CMSIS header,
   * e.g. @c USART1_RX_IRQn */
  uint8_t rx_irqn;
} sBSPACMdeviceEFM32periphUARTdevcfg;

/** Device-specific information for an EFM32 LEUART instance.
 *
 * Contains the information that cannot be easily deduced from the
 * peripheral base address. */
typedef struct sBSPACMdeviceEFM32periphLEUARTdevcfg {
  /** Configuration for which code can be shared with other devices. */
  sBSPACMdeviceEFM32periphXRTdevcfg common;

  /** Interrupt offset in NVIC.  From the CMSIS header, e.g. @c
   * LEART1_IRQn */
  uint8_t irqn;

  /** Clock source selection for LFBCLK.  From the <em_cmu.h> header,
   * e.g. @c cmuSelect_ULFRCO.  The value zero (mapping to @c
   * cmuSelect_Error) means to leave LFB in its existing
   * configuration. */
  uint8_t lfbsel;
} sBSPACMdeviceEFM32periphLEUARTdevcfg;

/** The operations table for USART devices. */
extern const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphUSARToperations;

/** The operations table for UART devices. */
extern const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphUARToperations;

/** The operations table for LEUART devices. */
extern const sBSPACMperiphUARToperations xBSPACMdeviceEFM32periphLEUARToperations;

/** The IRQHandler for USART/UART RX interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceEFM32periphUSARTrxirqhandler (sBSPACMperiphUARTstate * const usp);

/** The IRQHandler for USART/UART TX interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceEFM32periphUSARTtxirqhandler (sBSPACMperiphUARTstate * const usp);

/** The IRQHandler for LEUART interrupts.
 *
 * The handler in the vector table invokes this with the appropriate
 * state reference. */
void vBSPACMdeviceEFM32periphLEUARTirqhandler (sBSPACMperiphUARTstate * const usp);

/** Device configuration information for USART0 peripheral, where this
 * exists.  The board @c periph_cfg.c file provides a weak definition
 * of this that may be overridden by the application. */
extern const sBSPACMdeviceEFM32periphUSARTdevcfg xBSPACMdeviceEFM32periphUSART0devcfg;
/** USART1 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphUSARTdevcfg xBSPACMdeviceEFM32periphUSART1devcfg;
/** USART2 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphUSARTdevcfg xBSPACMdeviceEFM32periphUSART2devcfg;
/** UART0 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphUARTdevcfg xBSPACMdeviceEFM32periphUART0devcfg;
/** UART1 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphUARTdevcfg xBSPACMdeviceEFM32periphUART1devcfg;
/** LEUART0 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphLEUARTdevcfg xBSPACMdeviceEFM32periphLEUART0devcfg;
/** LEUART1 version of #xBSPACMdeviceEFM32periphUSART0devcfg */
extern const sBSPACMdeviceEFM32periphLEUARTdevcfg xBSPACMdeviceEFM32periphLEUART1devcfg;

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
