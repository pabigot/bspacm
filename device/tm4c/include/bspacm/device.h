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
 * @brief Common device header for all TM4C series devices.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_TM4C_H
#define BSPACM_DEVICE_TM4C_H

#if ! (BSPACM_DEVICE_SERIES_TM4C - 0)
#error TM4C device header in non-TM4C device
#endif /* BSPACM_DEVICE_SERIES_TM4C */

#if ! (__cplusplus - 0)
/* TI headers require (C99) bool but do not include a definition */
#include <stdbool.h>
#endif /* __cplusplus */
#if (BSPACM_CMSIS - 0)
#include <TIVA.h>
#endif /* BSPACM_CMSIS */

/** Type alias for the native struct used for the GPIO module on
 * specific lines of the TM4C series.  GPIOA_Type from TM4C123 is a
 * prefix of GPIOA_AHB_Type from TM4C129, and it appears not all GPIOs
 * on a TM4C129 device necessarily support the AHB interface
 * (specifically, GPIOK and higher are publicized by name @c GPIOK
 * rather than @c GPIOK_AHB).  Nonetheless, duplicating all the code
 * that only cares about the material available on non-AHB instances
 * is unacceptable. */
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
typedef GPIOA_Type GPIOCommon_Type;
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
typedef GPIOA_AHB_Type GPIOCommon_Type;
#endif /* DEVICE_LINE */

/* @cond DOXYGEN_EXCLUDE */
/* SRAM and peripheral bitband addresses are in standard Cortex-M3/M4
 * locations. */
#define BSPACM_CORE_SRAM_BASE ((uintptr_t)0x20000000)
#define BSPACM_CORE_SRAM_BITBAND_BASE ((uintptr_t)0x22000000)
#define BSPACM_CORE_PERIPH_BASE ((uintptr_t)0x40000000)
#define BSPACM_CORE_PERIPH_BITBAND_BASE ((uintptr_t)0x42000000)
/* @endcond */

#endif /* BSPACM_DEVICE_TM4C_H */
