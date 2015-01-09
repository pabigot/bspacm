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
 * Primarily this contains wrappers that allow code to be generically
 * capable of running with or without a soft device.
 *
 * First, the soft-device wrapper functions should not be used for
 * peripherals that are not restricted by the soft-device: doing so
 * can result in hard faults.
 *
 * Second, the soft-device wrapper functions are unnecessary if soft
 * device support is disabled (#BSPACM_NRF_USE_SD).
 *
 * Third, even when compiled-in support is present the softdevice
 * might not enabled at the time the operation is invoked.  In that
 * situation we need to fall back to the non-SD implementation.
 *
 * Because non-SD functionality generally does not indicate errors,
 * any errors evoked by the soft-device wrappers that can't be
 * processed immediately causes the application to block.  The
 * expectation is that these errors are due to mis-use of a restricted
 * peripheral, and will be discovered during application testing.
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
#define NOSD_BODY() do { \
    NVIC_EnableIRQ(irqn); \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_EnableIRQ(irqn);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_DisableIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_DisableIRQ (IRQn_Type irqn)
{
#define NOSD_BODY() do { \
    NVIC_DisableIRQ(irqn); \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_DisableIRQ(irqn);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_GetPendingIRQ */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_NVIC_GetPendingIRQ (IRQn_Type irqn)
{
  uint32_t pending_irq;

#define NOSD_BODY() do {                    \
    pending_irq = NVIC_GetPendingIRQ(irqn); \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_GetPendingIRQ(irqn, &pending_irq);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */
  return pending_irq;

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_SetPendingIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SetPendingIRQ (IRQn_Type irqn)
{
#define NOSD_BODY() do {      \
    NVIC_SetPendingIRQ(irqn); \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_SetPendingIRQ(irqn);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_ClearPendingIRQ */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_ClearPendingIRQ (IRQn_Type irqn)
{
#define NOSD_BODY() do {         \
    NVIC_ClearPendingIRQ(irqn);  \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_ClearPendingIRQ(irqn);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_SetPriority */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SetPriority (IRQn_Type irqn,
                               uint32_t priority)
{
#define NOSD_BODY() do {              \
    NVIC_SetPriority(irqn, priority); \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_SetPriority(irqn, priority);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware version of NVIC_GetPriority */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_NVIC_GetPriority (IRQn_Type irqn)
{
#define NOSD_FN() NVIC_GetPriority(irqn)

#if (BSPACM_NRF_USE_SD - 0)
  nrf_app_irq_priority_t priority;
  uint32_t ec = sd_nvic_GetPriority(irqn, &priority);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    priority = NOSD_FN();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
  return priority;
#else /* BSPACM_NRF_USE_SD */
  return NOSD_FN();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_FN
}

/** Soft-device--aware version of NVIC_SystemReset */
__STATIC_INLINE void
vBSPACMnrf51_NVIC_SystemReset (void)
{
#define NOSD_BODY() do {  \
    NVIC_SystemReset();   \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_nvic_SystemReset();
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware configuration of NRF_PPI->CH */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CH (uint8_t channel_num,
                     const volatile void * evt_endpoint,
                     const volatile void * task_endpoint)
{
#define NOSD_BODY() do {                                        \
    NRF_PPI->CH[channel_num].EEP = (uintptr_t)evt_endpoint;     \
    NRF_PPI->CH[channel_num].TEP = (uintptr_t)task_endpoint;    \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_ppi_channel_assign(channel_num, evt_endpoint, task_endpoint);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware assignment to NRF_PPI->CHENCLR */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CHENCLR (uint32_t mask)
{
#define NOSD_BODY() do {     \
    NRF_PPI->CHENCLR = mask; \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_ppi_channel_enable_clr(mask);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware assignment to NRF_PPI->CHENSET */
__STATIC_INLINE void
vBSPACMnrf51_PPI_CHENSET (uint32_t mask)
{
#define NOSD_BODY() do {     \
    NRF_PPI->CHENSET = mask; \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_ppi_channel_enable_set(mask);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

/** Soft-device--aware read of to NRF_PPI->CHEN */
__STATIC_INLINE uint32_t
uiBSPACMnrf51_PPI_CHEN ()
{
#define NOSD_FN() NRF_PPI->CHEN

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t chen;
  uint32_t ec = sd_ppi_channel_enable_get(&chen);
  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    chen = NOSD_FN();
  } else if (NRF_SUCCESS != ec) {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
  return chen;
#else /* BSPACM_NRF_USE_SD */
  return NOSD_FN();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_FN
}

/** Soft-device--aware start of HFCLK */
__STATIC_INLINE void
vBSPACMnrf51_HFCLKSTART ()
{
#define NOSD_BODY() do {                        \
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;         \
    NRF_CLOCK->TASKS_HFCLKSTART = 1;            \
    while (! NRF_CLOCK->EVENTS_HFCLKSTARTED) {  \
    }                                           \
  } while (0)

#if (BSPACM_NRF_USE_SD - 0)
  uint32_t ec = sd_clock_hfclk_request();

  if (NRF_ERROR_SOFTDEVICE_NOT_ENABLED == ec) {
    NOSD_BODY();
  } else if (NRF_SUCCESS == ec) {
    uint32_t is_running;
    do {
      if (NRF_SUCCESS != sd_clock_hfclk_is_running(&is_running))
        while (1) {
          /* deadlock to indicate invalid use */
        }
    } while (! is_running);
  } else {
    while (1) {
      /* deadlock to indicate invalid use */
    }
  }
#else /* BSPACM_NRF_USE_SD */
  NOSD_BODY();
#endif /* BSPACM_NRF_USE_SD */

#undef NOSD_BODY
}

#endif /* BSPACM_DEVICE_NRF51_H */
