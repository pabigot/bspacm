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
 *
 * @brief Implementation-specific support for LEDs on boards in the TM4C device series.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_TM4C_UTILITY_LED_H
#define BSPACM_DEVICE_TM4C_UTILITY_LED_H

#include <bspacm/core.h>

/* @cond DOXYGEN_EXCLUDE */

/* Include file to get names of each LED */
#define BSPACM_INC_EXPAND_LED_CONFIGURE(periph_,port_,bits_)
#define BSPACM_INC_EXPAND_LED_REFERENCE(port_,pin_)
#include <bspacm/internal/board/led.inc>
#undef BSPACM_INC_EXPAND_LED_REFERENCE
#undef BSPACM_INC_EXPAND_LED_CONFIGURE

/** Individual LEDs are represented as pointers to the bit-banded DOUT
 * field within the controlling port. */
typedef volatile uint32_t * xBSPACMled;

extern xBSPACMled const xBSPACMleds[];

static BSPACM_CORE_INLINE_FORCED
void
vBSPACMledSet_ (int idx, int mode)
{
  xBSPACMled const lp = xBSPACMleds[idx];
  /* Using a bit-banded reference so we don't care which pin gets
   * assigned. */
  if (0 > mode) {
    *lp ^= -1;
  } else if (0 == mode) {
    *lp = 0;
  } else {
    *lp = -1;
  }
}

/* @endcond */

#endif /* BSPACM_DEVICE_TM4C_UTILITY_LED_H */
