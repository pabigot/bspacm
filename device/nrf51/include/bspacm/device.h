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
 * @brief Common device header for all nRF51 series devices.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_NRF51_H
#define BSPACM_DEVICE_NRF51_H

#if ! (BSPACM_DEVICE_SERIES_NRF51 - 0)
#error NRF51 device header in non-NRF51 device
#endif /* BSPACM_DEVICE_SERIES_NRF51 */

/* We need the peripheral structures for inline use in headers, but
 * try not to contaminate the visible identifiers with everything
 * that's in nrf51_bitfields.h */
#include "nrf51.h"

#ifndef BSPACM_NRF_USE_SD
/** Define to a true value to use the nRF soft-device wrapper
 * functions. */
#define BSPACM_NRF_USE_SD 0
#endif /* BSPACM_NRF_USE_SD */

#if (BSPACM_NRF_USE_SD - 0)
/* And of course this pulls in nrf51_bitfields.h, dammit. */
#include "nrf_soc.h"
#endif /* BSPACM_NRF_USE_SD */

/** Conditionally set priority for non-soft-device interrupts.
 *
 * The ARM Cortex-M0 supports four interrupt levels from 0 (highest)
 * through 3 (lowest), with 0 being the power-up default.
 *
 * The Nordic soft device architecture allows applications to use
 * interrupt levels 1 (high-priority application) and 3 (low-priority
 * application).  Attempts to enable the soft-device if interrupts at
 * other priorities are already enabled result in an error
 * NRF_ERROR_SDM_INCORRECT_INTERRUPT_CONFIGURATION.
 *
 * This function can be used wherever interrupts are configured for
 * peripherals that are not restricted by the soft-device.  It has no
 * effect when #BSPACM_NRF_USE_SD is false, but assigns a
 * SD-acceptable priority when #BSPACM_NRF_USE_SD is true.
 *
 * @param irqn the IRQ number.
 * @note @p irqn should reflect a peripheral that is not restricted by
 * the soft device.  For restricted peripherals you should use
 * vBSPACMnrf_NVIC_SetPriority().
 *
 * @param high if @c true use high priority; if false use low priority. */
__STATIC_INLINE void
vBSPACMnrf51NVICsetApplicationPriority (IRQn_Type irqn,
                                        bool high)
{
#if (BSPACM_NRF_USE_SD - 0)
  /* NB: Use non-sd interface for unrestricted peripherals */
  NVIC_SetPriority(irqn, high ? 1 : 3);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_EnableIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_EnableIRQ (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NVIC_EnableIRQ(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_DisableIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_DisableIRQ (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NVIC_DisableIRQ(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_GetPendingIRQ */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_NVIC_GetPendingIRQ (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  return NVIC_GetPendingIRQ(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_SetPendingIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SetPendingIRQ (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NVIC_SetPendingIRQ(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_ClearPendingIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_ClearPendingIRQ (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NVIC_ClearPendingIRQ(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_SetPriority */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SetPriority (IRQn_Type irqn,
                               uint32_t priority)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NVIC_SetPriority(irqn, priority);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_GetPriority */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_NVIC_SetPriority (IRQn_Type irqn)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  return NVIC_GetPriority(irqn);
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware version of NVIC_SystemReset */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SystemReset (void)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  return NVIC_SystemReset();
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware configuration of NRF_PPI->CH */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CH (uint8_t channel_num,
                     const volatile void * evt_endpoint,
                     const volatile void * task_endpoint)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NRF_PPI->CH[channel_num].EEP = (uintptr_t)evt_endpoint;
  NRF_PPI->CH[channel_num].TEP = (uintptr_t)task_endpoint;
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware assignment to NRF_PPI->CHENCLR */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CHENCLR (uint32_t mask)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NRF_PPI->CHENCLR = mask;
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware assignment to NRF_PPI->CHENSET */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CHENSET (uint32_t mask)
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NRF_PPI->CHENSET = mask;
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware read of to NRF_PPI->CHEN */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_PPI_CHEN ()
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  return NRF_PPI->CHEN;
#endif /* BSPACM_NRF_USE_SD */
}

/** Soft-device--aware start of HFCLK */
__STATIC_INLINE void
vBSPACMnrf51_HFCLKSTART ()
{
#if (BSPACM_NRF_USE_SD - 0)
  #error not supported
#else /* BSPACM_NRF_USE_SD */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while (! NRF_CLOCK->EVENTS_HFCLKSTARTED) {
  }
#endif /* BSPACM_NRF_USE_SD */
}

#endif /* BSPACM_DEVICE_NRF51_H */
