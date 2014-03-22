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
 * @brief Common header included by all BSPACM leaf headers.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_CORE_H
#define BSPACM_CORE_H

#if (__cplusplus - 0)
#include <cstdint>
#else /* __cplusplus */
#include <stdint.h>
#endif /* __cplusplus */

#if defined(BSPACM_DEVICE_SERIES_TM4C)
#if ! (__cplusplus - 0)
/* TI headers require (C99) bool but do not include a definition */
#include <stdbool.h>
#endif /* __cplusplus */
#if (BSPACM_CMSIS - 0)
#include <TIVA.h>
#endif /* BSPACM_CMSIS */
#elif defined(BSPACM_DEVICE_SERIES_EFM32)
#include <em_device.h>
#else /* BSPACM_DEVICE_SERIES */
#error No support for device series
#endif /* BSPACM_DEVICE_SERIES */

/** Gratuitous alternative spelling of standard CMSIS core
 * intrinsic. */
#define BSPACM_CORE_DISABLE_INTERRUPT() __disable_irq()

/** Gratuitous alternative spelling of standard CMSIS core
 * intrinsic. */
#define BSPACM_CORE_ENABLE_INTERRUPT() __enable_irq()

/** Declare and initialize a const variable that records whether
 * interrupts are disabled.  The value is the PRIMASK register.
 *
 * @note This macro expands to a variable definition with an
 * initializer expression, so must be placed in an appropriate
 * context.
 *
 * @param var_ an identifier for a variable that will be defined to
 * hold the value of the PRIMASK register at the point of
 * definition. */
#define BSPACM_CORE_SAVED_INTERRUPT_STATE(var_) unsigned int const var_ = __get_PRIMASK()

/** Restore the interrupt enable state to what it was when @p var_ was
 * assigned.  This will invoke either BSPACM_CORE_ENABLE_INTERRUPT()
 * or BSPACM_CORE_DISABLE_INTERRUPT().  You may want
 * BSPACM_CORE_REENABLE_INTERRUPT() instead. */
#define BSPACM_CORE_RESTORE_INTERRUPT_STATE(var_) do { \
    if (1U & var_) {                                   \
      BSPACM_CORE_DISABLE_INTERRUPT();                 \
    } else {                                           \
      BSPACM_CORE_ENABLE_INTERRUPT();                  \
    }                                                  \
  } while (0)

/** If interrupts were enabled when @p var_ was declared (generally by
 * BSPACM_CORE_SAVED_INTERRUPT_STATE()), then enable them; otherwise
 * leave them in their current state. */
#define BSPACM_CORE_REENABLE_INTERRUPT(var_) do { \
    if (! (1U & var_)) {                          \
      BSPACM_CORE_ENABLE_INTERRUPT();             \
    }                                             \
  } while (0)

/** Mark a function to be inlined.
 *
 * Most toolchains support this feature, but the spelling of the
 * request varies.
 *
 * The toolchain is free to ignore the request, which is after all
 * only the developer's expert opinion.  When optimizing for size
 * toolchains are likely to ignore this if more than one call site is
 * in the translation unit.
 *
 * @see #BSPACM_CORE_INLINE_FORCED
 */
#if defined(BSPACM_DOXYGEN) || (BSPACM_CORE_TOOLCHAIN_GCC - 0)
#define BSPACM_CORE_INLINE __inline__
#elif BSPACM_CORE_TOOLCHAIN_TI - 0
#define BSPACM_CORE_INLINE __inline
#else /* TOOLCHAIN */
#define BSPACM_CORE_INLINE inline
#endif /* TOOLCHAIN */

/** Insist that a function be inlined.
 *
 * Use this when #BSPACM_CORE_INLINE is being ignored.  Not all
 * toolchains will support this; on those it should be treated as
 * #BSPACM_CORE_INLINE.
 */
#if defined(BSPACM_DOXYGEN) || (BSPACM_CORE_TOOLCHAIN_GCC - 0)
/* GCC wants both directives */
#define BSPACM_CORE_INLINE_FORCED BSPACM_CORE_INLINE __attribute__((__always_inline__))
#else /* TOOLCHAIN */
#define BSPACM_CORE_INLINE_FORCED BSPACM_CORE_INLINE
#endif /* TOOLCHAIN */

/** Declare a packed structure in a toolchain-specific manner.
 *
 * @param nm_ name of the structure to be declared
 *
 * This expands to @c struct @p nm_ annotated with toolchain-specific
 * directives to ensure the structure contents have no padding.  It is
 * used for binary messages that mix types which might normally
 * require padding to maintain MCU-standard alignment. */
#if defined(BSPACM_DOXYGEN) || (BSPACM_CORE_TOOLCHAIN_GCC - 0)
#define BSPACM_CORE_PACKED_STRUCT(nm_) struct __attribute__((__packed__)) nm_
#endif /* TOOLCHAIN */


#endif /* BSPACM_CORE_H */
