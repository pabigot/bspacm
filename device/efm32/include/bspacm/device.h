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
 * @brief Common device header for all EFM32 series devices.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_EFM32_H
#define BSPACM_DEVICE_EFM32_H

#if ! (BSPACM_DEVICE_SERIES_EFM32 - 0)
#error EFM32 device header in non-EFM32 device
#endif /* BSPACM_DEVICE_SERIES_EFM32 */

#include <em_device.h>
#include <em_emu.h>

/** Function to turn on the EFM32 Serial Wire Output (SWO) feature,
 * required when using the Trace Port Interface Unit to feed back PC
 * sample and interrupt information to the energyAware Profiler
 * application.
 *
 * @note SWO is only available on Cortex-M3 and Cortex-M4 device
 * lines, not on the Zero Gecko. */
void vBSPACMdeviceEFM32setupSWO(void);

/** Function to set the value of a nybble in a two-word array.
 *
 * This is the API for things like GPIO pin alternative function
 * selection and interrupt source port selection.
 *
 * @param regp pointer to a consecutive sequence of two words that
 * hold packed nybble values.
 *
 * @param pin the pin number is the ordinal of the nybble to be set
 *
 * @param value the nybble value to be stored
 */
static BSPACM_CORE_INLINE
void vBSPACMdeviceEFM32setPinNybble (volatile uint32_t * regp,
                                     unsigned int pin,
                                     int value)
{
  volatile uint32_t * const psel = regp + ((8 <= pin) ? 1 : 0);
  const unsigned int shift = 4 * (0x07 & pin);
  *psel = (*psel & ~(0x0F << shift)) | ((0x0F & value) << shift);
}

/** Bypass default implementation in favor of EFM32 standard. */
#define BSPACM_CORE_SLEEP() do { \
    EMU_EnterEM1();              \
  } while(0)

/** Bypass default implementation in favor of EFM32 standard, invoked
 * in a way that causes it to restore clock configurations, and
 * augmented by clearing the @c SLEEPDEEP bit which EMU_EnterEM2()
 * leaves set. */
#define BSPACM_CORE_DEEP_SLEEP() do {   \
    EMU_EnterEM2(true);                 \
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; \
  } while(0)

/* @cond DOXYGEN_EXCLUDE */
/* SRAM and peripheral bitband addresses are in standard Cortex-M3/M4
 * locations for everything except the Cortex-M0+ Zero Gecko line. */
#if ! (_EFM32_ZERO_FAMILY - 0)
#define BSPACM_CORE_SRAM_BASE ((uintptr_t)0x20000000)
#define BSPACM_CORE_SRAM_BITBAND_BASE ((uintptr_t)0x22000000)
#define BSPACM_CORE_PERIPH_BASE ((uintptr_t)0x40000000)
#define BSPACM_CORE_PERIPH_BITBAND_BASE ((uintptr_t)0x42000000)
#endif /* ! Zero Gecko */
/* @endcond */

#endif /* BSPACM_DEVICE_EFM32_H */
