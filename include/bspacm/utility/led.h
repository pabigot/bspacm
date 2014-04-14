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
 * @brief Generic interface to LEDs
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_UTILITY_LED_H
#define BSPACM_UTILITY_LED_H

#include <bspacm/core.h>

/* The actual implementation is board/device/series/line-specific.
 * This include file should declare the type xBSPACMled, the array
 * xBSPACMleds, and the inline function vBSPACMledSet_(). */
#include <bspacm/utility/led_.h>

/** The number of LEDs supported on the board */
extern const uint8_t nBSPACMleds;

/** Configure the board LEDs.
 *
 * This enables any necessary peripheral clocks and configures GPIOs
 * for all LEDs supported by the board. */
void
vBSPACMledConfigure ();

/** Set, clear, or toggle an LED identified by its index.
 *
 * @note This interface validates the LED index then delegates to
 * vBSPACMledSet_() for the implementation.
 *
 * @param idx the index of the LED, from zero to #nBSPACMleds-1.
 *
 * @param mode how the LED should change: negative values toggle the
 * LED, zero turns it off, and positive values turn it on. */
static BSPACM_CORE_INLINE_FORCED
void
vBSPACMledSet (int idx, int mode)
{
  if ((0 <= idx) && (idx < nBSPACMleds)) {
    vBSPACMledSet_(idx, mode);
  }
}

#if (BSPACM_DOXYGEN - 0)
/** The type for LED instances.
 *
 * This is board-specific and non-public; you don't need to reference it. */
typedef void xBSPACMled;

/** The array of LED instances.
 *
 * This is non-public API; do not use this array except through vBSPACMledSet(). */
extern const xBSPACMled xBSPACMleds[];

/** Implements the API of vBSPACMledSet() but bypassing the check for
 * a valid @p idx. */
static BSPACM_CORE_INLINE_FORCED
void
vBSPACMledSet_ (int idx, int mode) { }
#endif /* BSPACM_DOXYGEN */

#endif /* BSPACM_UTILITY_LED_H */
