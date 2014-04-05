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
 * TM4C peripherals are mapped to specific GPIO pins; the specific
 * peripheral function supported by a GPIO pin is determined by the
 * value written to the corresponding GPIOPCTL port control register.
 *
 * The mapping between pin and peripheral function is constrained by
 * the specific device.  In some cases the same function can be mapped
 * to multiple pins; in that situation, the desired mapping may be
 * specific to the board or even to the application.
 *
 * This file declares arrays that provide pin mapping definitions for
 * all peripherals that are used by the application.  These mappings
 * are used by the generic TM4C peripheral configuration routines.
 * The application, board, or device should provide definitions for
 * the structure corresponding to each required peripheral in a source
 * file named @c periph_config.c with an absolute path set in the make
 * variable @c PERIPH_CONFIG_SRC.  By default the board-specific @c
 * periph_config.c is used.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_TM4C_PERIPH_GPIO_H
#define BSPACM_DEVICE_TM4C_PERIPH_GPIO_H

#include <bspacm/core.h>

/** Core structure associating a GPIO port, pin, and port control
 * function.
 *
 * @note An all-zero pinmux denotes an unconfigured GPIO due to #pctl
 * being zero. */
typedef struct sBSPACMdeviceTM4Cpinmux {
  /** The register pointer for the port to be configured.  The
   * specific device is GPIOA_Type or GPIOA_AHB_Type, depending on
   * device line, though the two types are structurally equivalent. */
  void * port;

  /** The index of the port's bit within registers such as @c
   * SYSCTL->RCGCGPIO.  Note that @c PORTI and @c PORTO do not exist;
   * use this table:
   *
   * @code
   * // PA 0    PB 1    PC 2    PD 3
   * // PE 4    PF 5    PG 6    PH 7
   * // *PJ 8   PK 9    PL 10   PM 11
   * // PN 12   *PP 13  PQ 14   PR 15
   * // PS 16   PT 17
   * @endcode
   */
  uint8_t port_shift;

  /** The pin number, ranging from 0 through 7. */
  uint8_t pin;

  /** The port mux control nybble, with 0 denoting the GPIO function
   * and higher values indicating a specific peripheral function.  A
   * valid value is in the range 1 through 15, and must be shifted
   * left by 4*#pin in order to place it in the correct location in
   * the corresponding GPIOPCTL register.
   *
   * @note A zero value for #pctl indicates that the pinmux is not
   * valid and the corresponding GPIO should not be configured for its
   * alternate function.  This is used in cases where a particular
   * peripheral supports multiple functions, and a specific function
   * is not needed/supported by the mapping (e.g. the RTS/CTS signals
   * on a UART). */
  uint8_t pctl;
} sBSPACMdeviceTM4Cpinmux;

/** Set the AFSEL bit and other bits required to configure a pin for a
 * peripheral function according to @p *cfgp, or to reconfigure as a
 * GPIO.
 *
 * NB: Pins that happen to be commit-control locked will be unlocked
 * to perform this reconfiguration.
 *
 * @param cfgp pointer to the configuration structure.  If @link
 * sBSPACMdeviceTM4Cpinmux::port cfgp->port@endlink is null, no
 * configuration will be performed.
 *
 * @param enablep nonzero if the pin is to be configured based on @p
 * cfgp->mode; zero if the pin is to be disabled.
 *
 * @param initial_high zero to clear the pin, positive to set the pin,
 * negative to leave the pin setting unchanged.
 */
void vBSPACMdeviceTM4CpinmuxConfigure (const sBSPACMdeviceTM4Cpinmux * cfgp,
                                       int enablep,
                                       int initial_high);

/** Pin assignment structure for UART devices. */
typedef struct sBSPACMdeviceTM4CpinmuxUART {
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
} sBSPACMdeviceTM4CpinmuxUART;

/** The port pin mux mapping for UART peripherals in the
 * application/board/device. */
extern const sBSPACMdeviceTM4CpinmuxUART xBSPACMdeviceTM4CpinmuxUART[];

/** The number of entries in the #xBSPACMdeviceTM4CpinmuxUART
 * array. */
extern const uint8_t nBSPACMdeviceTM4CpinmuxUART;

#endif /* BSPACM_DEVICE_TM4C_PERIPH_GPIO_H */
