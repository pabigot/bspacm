/**************************************************************************//**
 * @file     system_ARMCM4.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM4 Device Series
 * @version  V1.08
 * @date     23. November 2012
 *
 * @note     This version modified from the original to support use in
 *           Tiva&trade; C Series Cortex-M4 devices using
 *           BSPACM.  See: http://github.com/pabigot/bspacm
 *
 ******************************************************************************/
/* Copyright (c) 2011 - 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/


#include <TIVA.h>

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/

/* TM4C devices power-up using PIOSC which is nominally 16 MHz.  On
 * TM4C123 this default is obtained using:
 *
 *  MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
 *
 * and the current frequency can be obtained using MAP_SysCtlClockGet().
 *
 * On TM4C129 this default is obtained using:
 *
 *  MAP_SysCtlClockFreqSet((SYSCTL_OSC_INT | SYSCTL_USE_OSC | SYSCTL_MAIN_OSC_DIS), 16000000);
 *
 * and the only way to get the current frequency is to cache the
 * return value from that function.
 */
#define __SYSTEM_CLOCK 16000000U


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = __SYSTEM_CLOCK;   /*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
  /* This function has no effect..
   *
   * CMSIS specifies that this should read the clock registers and
   * determine the current setting.  This can only be done on TM4C123
   * and requires headers that are not part of CMSIS, so we're
   * skipping it for now.  Recommended practice is to explicitly set
   * SystemCoreClock whenever the clock speed is changed. */
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
#else
  SystemCoreClock = __SYSTEM_CLOCK;
#endif
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
  /* The standard toolchain flags enable floating point, which
   * introduces hidden dependencies even if there appears no use of
   * the FPU in an application.  Make sure faults aren't generated if
   * the FPU is referenced. */
  SCB->CPACR |= (3UL << (10*2)) | (3U << (11*2));
  FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;
}
