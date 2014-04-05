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
 * @brief Support for peripheral pin multiplexing configuration
 *
 * EFM32 peripherals are mapped to specific GPIO pins; the specific
 * peripheral function supported by a GPIO pin is determined by the
 * value written to the corresponding ROUTE register within the
 * peripheral device register block.
 *
 * The mapping between pin and peripheral function is constrained by
 * the specific device.  In some cases the same function can be mapped
 * to multiple pins; in that situation, the desired mapping may be
 * specific to the board or even to the application.
 *
 * This file declares arrays that provide pin mapping definitions for
 * all peripherals that are used by the application.  These mappings
 * are used by the generic EFM32 peripheral configuration routines.
 * The application, board, or device should provide definitions for
 * the structure corresponding to each required peripheral in a source
 * file named @c periph_config.c with an absolute path set in the make
 * variable @c PERIPH_CONFIG_SRC.  By default the board-specific @c
 * periph_config.c is used.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_EFM32_PERIPH_GPIO_H
#define BSPACM_DEVICE_EFM32_PERIPH_GPIO_H

#include <bspacm/core.h>

/** Core structure associating a GPIO port, pin, and port control
 * function.
 *
 * @note An all-zero pinmux denotes an unconfigured GPIO due to #port
 * being null.
 *
 * Example to configure @c PD0 as output (push/pull):
 * @code
 *  .tx_pinmux = {
 *    .port = GPIO->P + gpioPortD,
 *    .pin = 0,
 *    .mode = gpioModePushPull,
 *  },
 * @endcode
 */
typedef struct sBSPACMdeviceEFM32pinmux {
  /** The port that is responsible for the pin.  Leave this null if
   * the pinmux is unused for the specific configuration that contains
   * it. Example:
   **/
  GPIO_P_TypeDef * port;

  /** The pin number, ranging from 0 through 15. */
  uint8_t pin;

  /** The mode to which the pin should be configured.
   *
   * @note This is the 4-bit value independent of #pin; the value will
   * be shifted and placed in either the @c MODEL or @c MODEH register
   * depending on #pin.  As all pins use the same value/behavior map,
   * and EMLIB is good enough to provide an equivalent enumeration
   * that is pin-independent, use the constants from <em_gpio.h>
   * GPIO_Mode_TypeDef. */
  uint8_t mode;
} sBSPACMdeviceEFM32pinmux;

/** Pin assignment structure for UART devices. */
typedef struct sBSPACMdeviceEFM32pinmuxUART {
  /** The base address of the UART peripheral register map to which
   * the mapping applies.  For example, @c USART1_BASE. */
  uint32_t uart_base;

  /** The port pin mux configuration for the UnRx signal. */
  sBSPACMdeviceEFM32pinmux rx_pinmux;

  /** The port pin mux configuration for the UnTx signal. */
  sBSPACMdeviceEFM32pinmux tx_pinmux;

  /** The port pin mux configuration for the UnRTS signal.  Leave as
   * zeros if not configured. */
  sBSPACMdeviceEFM32pinmux rts_pinmux;

  /** The port pin mux configuration for the UnCTS signal.  Leave as
   * zeros if not configured. */
  sBSPACMdeviceEFM32pinmux cts_pinmux;

  /** The RX_IRQ number corresponding to #uart_base. */
  int16_t rx_irqn;

  /** The TX_IRQ number corresponding to #uart_base. */
  int16_t tx_irqn;
} sBSPACMdeviceEFM32pinmuxUART;

/** Function to configure a GPIO according to @p *cfgp and provide an
 * initial value.
 *
 * @param cfgp pointer to the configuration structure.  If @link
 * sBSPACMdeviceEFM32pinmux::port cfgp->port@endlink is null, no
 * configuration will be performed.
 *
 * @param enablep nonzero if the pin is to be configured based on @p
 * cfgp->mode; zero if the pin is to be disabled.
 *
 * @param initial_high zero to clear the pin, positive to set the pin,
 * negative to leave the pin setting unchanged.
 *
 * @note This differs from EMLIB's GPIO_PinModeSet() by allowing
 * representation of a pin that should not be configured, and
 * separating the configuration data from the initial value (which may
 * depend on how the peripheral intends to use the pin).
 */
void vBSPACMdeviceEFM32pinmuxConfigure (const sBSPACMdeviceEFM32pinmux * cfgp,
                                        int enablep,
                                        int initial_high);

/** The port pin mux mapping for UART peripherals in the
 * application/board/device. */
extern const sBSPACMdeviceEFM32pinmuxUART xBSPACMdeviceEFM32pinmuxUART[];

/** The number of entries in the #xBSPACMdeviceEFM32pinmuxUART
 * array. */
extern const uint8_t nBSPACMdeviceEFM32pinmuxUART;

#endif /* BSPACM_DEVICE_EFM32_PERIPH_GPIO_H */
