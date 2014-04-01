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
 * @brief Mapping between peripheral base address and shift offset
 *
 * There are a variety of peripherals in the TM4C series for which
 * more than two instances may be present on any given device.
 * Unfortunately, even though a particular peripheral instance has the
 * same address on any device that supports it, there is no clean
 * mechanism to calculate the base address from the instance number or
 * vice versa (especially for GPIOs, of which there are two flavors
 * with discontinuous strides).
 *
 * This matters because on some cases we know the base address, but
 * need to figure out which bit in a register such as
 * <tt>SYSCTL->RCGCGUART</tt> corresponds to the instance.  In most cases, the
 * ordinal of the peripheral (3 for @c UART3) is the same as the shift
 * amount to locate the corresponding bit.
 *
 * For GPIOs it is more complex.  These peripherals do not use index
 * numbers, but index letters.  While @c GPIOC might have been called
 * @c GPIO2 and uniformly uses shift 2, @c GPIOI and @c GPIOO do not
 * exist and are skipped in the ordering.  Thus ports map to instances
 * (shifts) as:
 *
 * @code
 * // PA 0    PB 1    PC 2    PD 3
 * // PE 4    PF 5    PG 6    PH 7
 * // *PJ 8   PK 9    PL 10   PM 11
 * // PN 12   *PP 13  PQ 14   PR 15
 * // PS 16   PT 17
 * @endcode
 *
 * This file declares arrays, indexed by shift count or instance
 * number, holding for each peripheral type the base address of each
 * peripheral available on the device for which the array was
 * constructed.  So obtaining the base address of a given instance is
 * a simple lookup, and calculating the instance from the base address
 * is a linear search.
 *
 * Be aware that it is technically possible that some instances are
 * skipped on some devices (e.g., GPIOG may be followed by GPION).  At
 * the time these data were created the shift counts were not adjusted
 * for any such case, so if this can happen there will be zero-valued
 * base addresses in the arrays.
 *
 * These arrays can also be used to iterate through the available
 * peripherals on a device to inspect their configuration.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_TM4C_DEVICE_PERIPHS_H
#define BSPACM_DEVICE_TM4C_DEVICE_PERIPHS_H

#include <bspacm/core.h>

/** Ordered BASE addresses of GPIO peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphGPIO[];
/** Number of GPIO peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphGPIO;

/** Ordered BASE addresses of I2C peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphI2C[];
/** Number of I2C peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphI2C;

/** Ordered BASE addresses of SSI peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphSSI[];
/** Number of SSI peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphSSI;

/** Ordered BASE addresses of TIMER peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphTIMER[];
/** Number of TIMER peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphTIMER;

/** Ordered BASE addresses of UART peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphUART[];
/** Number of UART peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphUART;

/** Ordered BASE addresses of WTIMER peripherals */
extern const uint32_t wBSPACMdeviceTM4CperiphWTIMER[];
/** Number of WTIMER peripherals available on specific device */
extern const uint8_t nBSPACMdeviceTM4CperiphWTIMER;

#endif /* BSPACM_DEVICE_TM4C_DEVICE_PERIPHS_H */
