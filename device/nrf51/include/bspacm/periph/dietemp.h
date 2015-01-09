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

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_DIETEMP_H
#define BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_DIETEMP_H

#include <bspacm/core.h>

/** @file
 *
 * @brief NRF51 series-specific DIETEMP interface for BSPACM
 *
 * Provides helper functions to read the die temperature taking into
 * account various errata.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/** Configure to measure the die temperature using the TEMP peripheral.
 *
 * @return true iff the high-frequency clock is enabled and using the
 * crystal oscillator, a condition the reference manual states is
 * required for the die temperature to be accurate. */
bool
bBSPACMdietempInitialize ();

#if (BSPACM_NRF_USE_SD - 0)
__STATIC_INLINE
#else /* BSPACM_NRF_USE_SD */
inline
#endif /* BSPACM_NRF_USE_SD */
int
iBSPACMdietemp_cCel ()
{
  int32_t temp_qCel;
#if (BSPACM_NRF_USE_SD - 0)
  sd_temp_get(&temp_qCel);
#else /* BSPACM_NRF_USE_SD */
  NRF_TEMP->EVENTS_DATARDY = 0;

  NRF_TEMP->TASKS_START = 1;
  while (! NRF_TEMP->EVENTS_DATARDY) {
  }
  NRF_TEMP->EVENTS_DATARDY = 0;

  /* PAN-29: STOP task clears TEMP register */
  uint32_t temp_raw = NRF_TEMP->TEMP;

  /* PAN-30: TEMP module analog front end does not power down when
   * DATARDY event occurs */
  NRF_TEMP->TASKS_STOP = 1;

  /* PAN-28: Negative measured values are not represented correctly.
   * Sign extension does not go higher than bit 9.
   *
   * Value is 10-bit 2's complement.  Convert to 32-bit 2's
   * complement. */
  const uint32_t sign_bit = 0x0200;
  if (temp_raw & sign_bit) {
    temp_raw |= ~(sign_bit - 1);
  }
  temp_qCel = (int32_t)temp_raw;
#endif /* BSPACM_NRF_USE_SD */

  return 25 * (int)temp_qCel;
}

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_DIETEMP_H */
