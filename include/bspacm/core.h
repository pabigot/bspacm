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
#include <cstddef>
#else /* __cplusplus */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif /* __cplusplus */

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

/** Declare an aligned object a toolchain-specific manner.
 *
 * @param sz_ alignment requirements in bytes.  E.g. to ensure the
 * lower 10 bits of the address are zero, use 1024.
 *
 * This expands to a toolchain-specific directive that controls the
 * alignment of the following object definition.  It is used for
 * structures that are placed in system memory but accessed through
 * peripherals, such as the micro-DMA peripheral. */
#if defined(BSPACM_DOXYGEN) || (BSPACM_CORE_TOOLCHAIN_GCC - 0)
#define BSPACM_CORE_ALIGNED_OBJECT(sz_) __attribute__((__aligned__(sz_)))
#endif /* TOOLCHAIN */

/* Device-specific material.  This includes device vendor and CMSIS
 * headers, and provides any necessary bridging declarations.  The
 * correct content should be located by the prioritization of include
 * paths, but is generally found in the device/SERIES include
 * hierarchy. */
#include <bspacm/device.h>

/* Application/board-specific material.  This corresponds to the
 * material in @c periph_config.c, and has things such as the handle
 * to the default system UART peripheral.  A default is provided in
 * each board-specific include hierarchy, but it can be overridden by
 * providing a path to another file in the providing a higher-priority
 * path in the application-specific Makefile. */
#include <bspacm/config.h>

/** Version identifier for the BSPACM infrastructure
 *
 * A monotonically non-decreasing integer reflecting the version of
 * BSPACM that is being used.  The value represents a development
 * freeze date in the form YYYYMMDD as a decimal number. */
#define BSPACM_VERSION 20150114

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

#if defined(BSPACM_DOXYGEN) || (! defined(BSPACM_CORE_SLEEP))
/** Enter the Cortex-M sleep mode.
 *
 * The specific effect of this is vendor-specific, but in concept it
 * turns off the processor clock and leaves all other clocks running.
 * The implementation may map to a vendor-specific function or macro,
 * but is expected to be equivalent to invoking the ARM instruction @c
 * WFI (or @c WFE) with the @c SLEEPDEEP bit of System Control
 * Register clear.
 *
 * @warning The implementation of this operation is device-specific.
 *
 * @note The user should be aware of the effect of this operation on
 * the hardware an application targets.  Interrupts that cause wakeup
 * will, if not masked through @c PRIMASK, be executed prior to
 * control returning to the point following the @c __WFI() operation,
 * meaning that any clocks disabled by the operation will not be
 * restored until interrupt processing completes and control returns
 * to the remainder of this macro.
 *
 * @see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0497a/CHDJJHJI.html */
#define BSPACM_CORE_SLEEP() do { \
    __WFI();                     \
  } while(0)
#endif /* BSPACM_CORE_SLEEP */

#if defined(BSPACM_DOXYGEN) || (! defined(BSPACM_CORE_DEEP_SLEEP))
/** Enter the Cortex-M deep sleep mode.
 *
 * The specific effect of this is vendor-specific, but in concept it
 * turns off the processor clock and any number of other clocks.  The
 * implementation may map to a vendor-specific function or macro, but
 * is expected to be roughly equivalent to invoking the ARM
 * instruction @c WFI (or @c WFE) with the @c SLEEPDEEP bit of the
 * System Control Register set.
 *
 * @warning The implementation of this operation is device-specific.
 * In particular, it may include restoration of some or all peripheral
 * clocks.
 *
 * @note The user should be aware of the effect of this operation on
 * the hardware an application targets.  Interrupts that cause wakeup
 * will, if not masked through @c PRIMASK, be executed prior to
 * control returning to the point following the @c __WFI() operation,
 * meaning that any clocks disabled by the operation will not be
 * restored until interrupt processing completes and control returns
 * to the remainder of this macro.
 *
 * @warning Although some vendor implementations may leave the @c
 * SLEEPDEEP bit set, it SHOULD be restored to its original state in
 * the BSPACM implementation of this macro so that subsequent use of
 * @c __WFI() and @c __WFE() executes regular or deep sleep in
 * accordance with the original setting.
 *
 * @see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0497a/CHDJJHJI.html */
#define BSPACM_CORE_DEEP_SLEEP() do {                           \
    uint32_t in_sleepdeep = (SCB->SCR & SCB_SCR_SLEEPDEEP_Msk); \
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;                          \
    __WFI();                                                    \
    SCB->SCR &= ~in_sleepdeep;                                  \
  } while(0)
#endif /* BSPACM_CORE_DEEP_SLEEP */

/** Defined to true value if cycle-counting is supported on the
 * architecture.  This is false on a Cortex-M0+ device. */
#if defined(DWT_CTRL_NOCYCCNT_Msk)
#define BSPACM_CORE_SUPPORTS_CYCCNT 1
#else /* DWT_CTRL_NOCYCCNT_Msk */
#define BSPACM_CORE_SUPPORTS_CYCCNT 0
#endif /* DWT_CTRL_NOCYCCNT_Msk */

/** Functional macro to enable the cycle-counting capability of the
 * core Data Watchpoint and Trace unit.
 *
 * Note that this functionality is optional, so may not be present on
 * your device.  This is a no-op in that case.
 *
 * @dependency #BSPACM_CORE_SUPPORTS_CYCCNT */
#if (BSPACM_CORE_SUPPORTS_CYCCNT - 0)
#define BSPACM_CORE_ENABLE_CYCCNT() do {              \
    if (! (DWT_CTRL_NOCYCCNT_Msk & DWT->CTRL)) {      \
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
      DWT->CYCCNT = 0;                                \
      DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            \
    }                                                 \
  } while (0)
#else /* BSPACM_CORE_SUPPORTS_CYCCNT */
#define BSPACM_CORE_ENABLE_CYCCNT() do { } while (0)
#endif /* BSPACM_CORE_SUPPORTS_CYCCNT */

/** Functional macro to disable the cycle-counting capability of the
 * core Data Watchpoint and Trace unit. */
#if (BSPACM_CORE_SUPPORTS_CYCCNT - 0)
#define BSPACM_CORE_DISABLE_CYCCNT() do {              \
    if (! (DWT_CTRL_NOCYCCNT_Msk & DWT->CTRL)) {       \
      DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;            \
      CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; \
    }                                                  \
  } while (0)
#else /* BSPACM_CORE_SUPPORTS_CYCCNT */
#define BSPACM_CORE_DISABLE_CYCCNT() do { } while (0)
#endif /* BSPACM_CORE_SUPPORTS_CYCCNT */

/** Returns the current value of the cycle counter.
 *
 * This returns a constant 0 if the cycle counter is not supported.
 * (It returns an arbitrary constant if the cycle counter is supported
 * but you didn't invoke BSPACM_CORE_ENABLE_CYCCNT()).
 *
 * @dependency BSPACM_CORE_SUPPORTS_CYCCNT
 * @see BSPACM_CORE_ENABLE_CYCCNT() */
#if (BSPACM_CORE_SUPPORTS_CYCCNT - 0)
#define BSPACM_CORE_CYCCNT() DWT->CYCCNT
#else /* BSPACM_CORE_SUPPORTS_CYCCNT */
#define BSPACM_CORE_CYCCNT() 0
#endif /* BSPACM_CORE_SUPPORTS_CYCCNT */

/** Delay for a specified number of cycles.
 *
 * This does not attempt to account for the overhead of the loop.
 *
 * @warning On architectures that do not support a cycle counter, this
 * is a rough estimate based on cycles-per-iteration of a loop. */
#if (BSPACM_CORE_SUPPORTS_CYCCNT - 0)
#define BSPACM_CORE_DELAY_CYCLES(cycles_) do {  \
    uint32_t const delta = (cycles_);           \
    uint32_t const cc0 = DWT->CYCCNT;           \
    while ((DWT->CYCCNT - cc0) < delta) {       \
      /* spin */                                \
    }                                           \
  } while (0)
#else /* BSPACM_CORE_SUPPORTS_CYCCNT */
#define BSPACM_CORE_DELAY_CYCLES(cycles_) do {  \
    uint32_t const cycles_per_iter = 3;         \
    uint32_t remaining = (cycles_);             \
    while (remaining > cycles_per_iter) {       \
      remaining -= cycles_per_iter;             \
    }                                           \
  } while (0)
#endif /* BSPACM_CORE_SUPPORTS_CYCCNT */

/* Cortex-M3 devices support bit-band access to SRAM and peripherals,
 * but Cortex-M0+ does not, so the following features are optional. */

#if defined(BSPACM_DOXYGEN) || (defined(BSPACM_CORE_SRAM_BASE) && defined(BSPACM_CORE_SRAM_BITBAND_BASE))
/* @cond DOXYGEN_EXCLUDE */
#define BSPACM_CORE_BITBAND_SRAM_(object_, bit_) (BSPACM_CORE_SRAM_BITBAND_BASE + 4 * ((bit_) + 8 * ((uintptr_t)&(object_) - BSPACM_CORE_SRAM_BASE)))
/* @endcond */

/** Bit-band reference into an 32-bit object in SRAM.
 *
 * This is particularly useful when manipulating a single flag in a
 * volatile object aggregating event flags while interrupts are
 * enabled.
 *
 * @note Only supported on processors that use bit-banding
 * (e.g. Cortex-M3, Cortex-M4).  This macro will not be defined if the
 * feature is not supported.
 *
 * @param object_ a reference to (not the address of) a 32-bit object
 * located in SRAM
 *
 * @param bit_ the bit of interest within @p object_ (0 to 31,
 * depending on size of object)
 *
 * @return an lvalue reference to a volatile @c bool object in which
 * bit 0 aliases bit @p bit_ of @p object_ and no other bits are
 * used. */
#define BSPACM_CORE_BITBAND_SRAM32(object_, bit_) (*(volatile uint32_t *)BSPACM_CORE_BITBAND_SRAM_(object_, bit_))

/** Bit-band reference into an 16-bit object in SRAM.
 *
 * @see #BSPACM_CORE_BITBAND_SRAM32 */
#define BSPACM_CORE_BITBAND_SRAM16(object_, bit_) (*(volatile uint16_t *)BSPACM_CORE_BITBAND_SRAM_(object_, bit_))

/** Bit-band reference into an 8-bit object in SRAM.
 *
 * @see #BSPACM_CORE_BITBAND_SRAM32 */
#define BSPACM_CORE_BITBAND_SRAM8(object_, bit_) (*(volatile uint8_t *)BSPACM_CORE_BITBAND_SRAM_(object_, bit_))

#endif /* SRAM bitband supported */

#if defined(BSPACM_DOXYGEN) || (defined(BSPACM_CORE_PERIPH_BASE) && defined(BSPACM_CORE_PERIPH_BITBAND_BASE))
/** Bit-band reference into a peripheral register.
 *
 * This is particularly useful to reduce code size when the bit and
 * the peripheral register address are compile-time constants, as a
 * read-modify-write instruction sequence will be replaced by a single
 * store.
 *
 * @note Only supported on processors that use bit-banding
 * (e.g. Cortex-M3, Cortex-M4).  This macro will not be defined if the
 * feature is not supported.
 *
 * @warning Not all peripherals will support bit-banding.
 *
 * @param object_ a reference to the peripheral register (not its address).
 *
 * @param bit_ the bit of interest within @p object_ (0 to 31)
 *
 * @return an lvalue reference to a volatile @c uint32_t object in
 * which bit 0 aliases bit @p bit_ of @p object_ and no other bits are
 * used. */
#define BSPACM_CORE_BITBAND_PERIPH(object_, bit_) (*(volatile uint32_t *)(BSPACM_CORE_PERIPH_BITBAND_BASE + 4 * ((bit_) + 8 * ((uintptr_t)&(object_) - BSPACM_CORE_PERIPH_BASE))))
#endif /* PERIPH bitband supported */

/** Convert a bit mask to a bit number.
 *
 * Useful for bitband object references.  Some vendors (SiLabs)
 * provide macros for bit positions; some (TI) only provide the mask
 * values.
 *
 * @warning If the passed value does not have exactly one bit set,
 * this function will hang.
 *
 * @param mask a bit mask.  Exactly one bit must be set.  In normal
 * use the parameter is a compile-time constant so this function
 * reduces to a compile-time constant too.
 *
 * @return the offset of the single set bit in @p mask. */
static BSPACM_CORE_INLINE
unsigned int
uiBSPACMcoreBitFromMask (uint32_t mask)
{
  if (0x1U == mask) {
    return 0;
  } else  if (0x2U == mask) {
    return 1;
  } else  if (0x4U == mask) {
    return 2;
  } else  if (0x8U == mask) {
    return 3;
  } else  if (0x10U == mask) {
    return 4;
  } else  if (0x20U == mask) {
    return 5;
  } else  if (0x40U == mask) {
    return 6;
  } else  if (0x80U == mask) {
    return 7;
  } else  if (0x100U == mask) {
    return 8;
  } else  if (0x200U == mask) {
    return 9;
  } else  if (0x400U == mask) {
    return 10;
  } else  if (0x800U == mask) {
    return 11;
  } else  if (0x1000U == mask) {
    return 12;
  } else  if (0x2000U == mask) {
    return 13;
  } else  if (0x4000U == mask) {
    return 14;
  } else  if (0x8000U == mask) {
    return 15;
  } else  if (0x10000U == mask) {
    return 16;
  } else  if (0x20000U == mask) {
    return 17;
  } else  if (0x40000U == mask) {
    return 18;
  } else  if (0x80000U == mask) {
    return 19;
  } else  if (0x100000U == mask) {
    return 20;
  } else  if (0x200000U == mask) {
    return 21;
  } else  if (0x400000U == mask) {
    return 22;
  } else  if (0x800000U == mask) {
    return 23;
  } else  if (0x1000000U == mask) {
    return 24;
  } else  if (0x2000000U == mask) {
    return 25;
  } else  if (0x4000000U == mask) {
    return 26;
  } else  if (0x8000000U == mask) {
    return 27;
  } else  if (0x10000000U == mask) {
    return 28;
  } else  if (0x20000000U == mask) {
    return 29;
  } else  if (0x40000000U == mask) {
    return 30;
  } else  if (0x80000000U != mask) {
    /* Not exactly one bit set.  This is an error in usage. */
    while(1);
  }
  return 31;
}

/** Function to set the value of a nybble in a word array.
 *
 * This is the API for things like GPIO pin alternative function
 * selection and interrupt source port selection on the EFM32,
 * and DMA channel source selection on the TM4C.
 *
 * @param regp pointer to a consecutive sequence of one or more words
 * that hold packed nybble values.  Each word holds eight nybbles.
 *
 * @param pin the pin number is the ordinal of the nybble to be set.
 *
 * @param value the nybble value to be stored.  Only the four low bits
 * are used.
 */
static BSPACM_CORE_INLINE
void vBSPACMcoreSetPinNybble (volatile uint32_t * basep,
                              unsigned int pin,
                              unsigned int value)
{
  volatile uint32_t * const wordp = basep + (pin / 8);
  const unsigned int shift = 4 * (pin % 8);
  *wordp = (*wordp & ~(0x0F << shift)) | ((0x0F & value) << shift);
}

#endif /* BSPACM_CORE_H */
