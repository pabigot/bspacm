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

/** @file
 *
 * @brief nRF51-specific support for high-resolution timers
 *
 * Timer #BSPACM_HIRES_TIMER is reserved for use by BSPACM to support
 * short-duration sleeps and high-precision timing.
 *
 * Capture/compare register 0 is reserved for timed sleeps via
 * vBSPACMhiresSleep_us().
 *
 * Other capture/compare registers may be used for high-precision
 * timing by the application by issuing a capture task then (at an
 * appropriate time) reading the corresponding CC register.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_HIRES_H
#define BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_HIRES_H

#include <bspacm/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef BSPACM_HIRES_TIMER_BASE
/** The integral address of the TIMER peripheral to be used for BSPACM
 * high-resolution timing.
 *
 * You probably don't want this to be NRF_TIMER0_BASE, because that's
 * used by soft devices. */
#define BSPACM_HIRES_TIMER_BASE NRF_TIMER1_BASE
#endif /* BSPACM_HIRES_TIMER_BASE */

/** @def BSPACM_HIRES_TIMER
 *
 * One of the NRF TIMER peripherals reserved for use by BSPACM
 * infrastructure.
 *
 * @defaulted
 * @dependency #BSPACM_HIRES_TIMER_BASE
 */
#if NRF_TIMER0_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER NRF_TIMER0
#elif NRF_TIMER1_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER NRF_TIMER1
#elif NRF_TIMER2_BASE == BSPACM_HIRES_TIMER_BASE
#define BSPACM_HIRES_TIMER NRF_TIMER2
#else  /* BSPACM_HIRES_TIMER_BASE */
#error Unrecognized high-resolution timer
#endif /* BSPACM_HIRES_TIMER_BASE */

/** Initialize #BSPACM_HIRES_TIMER to run at the specified frequency.
 *
 * @param freq_Hz the desired high-resolution frequency.  The 16 MHz
 * core system clock must be a power-of-two multiple of this value.
 *
 * @return 0 if clock configured successfully; a negative error code
 * if @p freq_Hz is unacceptable or a hardware problem is detected.
 *
 * @note This only configures the timer; it does not enable it.
 * @see vBSPACMhiresSetEnabled() */
int
iBSPACMhiresInitialize (unsigned int freq_Hz);

/** Return @c true if and only if the high resolution timer has been
 * initialized and is currently enabled. */
bool
bBSPACMhiresEnabled (void);

/** Enable or disable the high-resolution timer.
 *
 * @param enabled if true, enable the timer; if false, disable it.
 *
 * @return -1 if the high-resolution timer has not been initialized,
 * otherwise the value of bBSPACMhiresEnabled() at the time of the
 * call. */
int
iBSPACMhiresSetEnabled (bool enabled);

/** Convert from ticks of the 16 MHz core system clock to ticks of
 * #BSPACM_HIRES_TIMER */
inline
unsigned int
uiBSPACMhiresConvert_hfclk_hrt (unsigned int dur_hfclk)
{
  return dur_hfclk >> BSPACM_HIRES_TIMER->PRESCALER;
}

/** Convert from ticks of #BSPACM_HIRES_TIMER to ticks of the 16 MHz
 * core system clock. */
inline
unsigned int
uiBSPACMhiresConvert_hrt_hfclk (unsigned int dur_hrt)
{
  return dur_hrt << BSPACM_HIRES_TIMER->PRESCALER;
}

/** Convert from microseconds to ticks of the 16 MHz core system clock. */
inline
unsigned int
uiBSPACMhiresConvert_us_hfclk (unsigned int dur_us)
{
  return dur_us << 4;
}

/** Convert from microseconds to ticks of #BSPACM_HIRES_TIMER. */
inline
unsigned int
uiBSPACMhiresConvert_us_hrt (unsigned int dur_us)
{
  int shift = 4 - (int)BSPACM_HIRES_TIMER->PRESCALER;
  if (0 < shift) {
    return dur_us << shift;
  }
  return dur_us >> -shift;
}

/** Convert from ticks of #BSPACM_HIRES_TIMER to microseconds. */
inline
unsigned int
uiBSPACMhiresConvert_hrt_us (unsigned int dur_hrt)
{
  int shift = 4 - (int)BSPACM_HIRES_TIMER->PRESCALER;
  if (0 < shift) {
    return dur_hrt >> shift;
  }
  return dur_hrt << -shift;
}

/** Sleep for the desired duration.
 *
 * This will enter basic sleep mode if the duration is long enough;
 * otherwise it will busy-wait.  The busy-wait solution is vulnerable
 * to extending too long due to processed interrupts; the duration
 * when sleeping will be more accurate.
 *
 * @param count_us the duration to sleep, expressed in microseconds
 *
 * @warning The infrastructure does not check for overflow in
 * converting @p count_us to hires clock ticks.  Unless you are using
 * TIMER0, the timer has only 16 bits and will overflow at
 * divisor*4.096 ms.  This means the maximum delay for a 1 MHz timer
 * is 65.535 ms.
 *
 * @warning If you invoke this when bBSPACMhiresEnabled() returns
 * false it will hang.  This is a bigger clue that your program is
 * incorrect than any other reasonable behavior.
 *
 * @warning Do not invoke this from first-level interrupt handlers.
 * Even if PAN #6 is fixed in your hardware, you may be overwriting
 * the deadline for a user-level delay. */
void
vBSPACMhiresSleep_us (unsigned long count_us);

/** Sleep for the desired duration.
 *
 * Thin wrapper around vBSPACMhiresSleep_us() that scales milliseconds
 * to microseconds.
 *
 * @note These are true kHz milliseconds (1s / 1000) not KiHz
 * "milliseconds" (1s / 1024).
 *
 * @warning The infrastructure does not check for overflow in
 * converting @p count_ms to hires clock ticks.  See delay limit
 * discussion at vBSPACMhiresSleep_us(). */
inline void
vBSPACMhiresSleep_ms (unsigned long count_ms)
{
  vBSPACMhiresSleep_us(1000 * count_ms);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_HIRES_H */
