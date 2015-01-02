/* Copyright 2015, Peter A. Bigot
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
 * @brief Implementation-specific support for onewire bus
 * configurations on boards in the Nordic Semiconductor nRF51 device
 * series.
 *
 * @warning This file is included generically by
 * <bspacm/utility/onewire.h> and will not be interpreted correctly in
 * other contexts.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_ONEWIRE_H
#define BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_ONEWIRE_H

/** nRF51 variant bus configuration for one-wire devices */
typedef struct sBSPACMonewireBus {
  /** Pin for DQ signal.
   *
   * Value must be in the range 0..31 inclusive. */
  int8_t dq_pin;

  /** Pin for parasitic power.
   *
   * Set to -1 if external power used, otherwise must be in the range 0..31. */
  int8_t pwr_pin;

  /* TODO: flag to enable active-low parasitic power?  does anyone care? */

  /** Pre-computed bit for DQ port */
  uint32_t dq_bit;

  /** Pre-computed bit for parasitic power.
   *
   * Zero if external power used. */
  uint32_t pwr_bit;
} sBSPACMonewireBus;

/** Configure the given bus structure for one-wire operation.
 *
 * @param bp pointer to device-specific information identifying the 1-wire bus
 *
 * @param dq_pin GPIO pin connected to device DQ signal.  This must be
 * an integer in the range 0..31.
 *
 * @param pwr_pin GPIO pin used to turn on parasitic power.  Assumed
 * to be active high.  Pass -1 to indicate that external power is
 * being used.
 *
 * @return the handle for the configured bus, or @c NULL if the
 * parameters were invalid. */
hBSPACMonewireBus
hBSPACMonewireConfigureBus (sBSPACMonewireBus * bp,
                            int dq_pin,
                            int pwr_pin);

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_ONEWIRE_H */
