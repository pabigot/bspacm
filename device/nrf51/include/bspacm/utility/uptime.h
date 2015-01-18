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
 * @brief nRF51-specific support for low-resolution uptime clock
 *
 * RTC peripheral #BSPACM_UPTIME_RTC is reserved for use by BSPACM to
 * support long-duration sleeps and user-defined alarms.
 *
 * Capture/compare register 0 is reserved for timed sleeps via
 * iBSPACMuptimeSleep().
 *
 * Other capture/compare registers may be used for user alarms via
 * iBSPACMuptimeAlarmSet().
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_UPTIME_H
#define BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_UPTIME_H

#include <bspacm/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef BSPACM_UPTIME_RTC_BASE
/** The integral address of the RTC peripheral to be used for BSPACM
 * uptime clock.
 *
 * You probably don't want this to be NRF_RTC0_BASE, because that's
 * used by soft devices. */
#define BSPACM_UPTIME_RTC_BASE NRF_RTC1_BASE
#endif /* BSPACM_UPTIME_RTC_BASE */

/** @def BSPACM_UPTIME_RTC
 *
 * One of the NRF RTC peripherals reserved for use by BSPACM
 * infrastructure.
 *
 * @defaulted
 * @dependency #BSPACM_UPTIME_RTC_BASE
 */

/** @def BSPACM_UPTIME_CC_COUNT
 *
 * The number of capture/compare registers available on the selected
 * RTC peripheral.
 *
 * @defaulted
 * @dependency #BSPACM_UPTIME_RTC_BASE
 */

#if NRF_RTC0_BASE == BSPACM_UPTIME_RTC_BASE
#define BSPACM_UPTIME_RTC NRF_RTC0
#define BSPACM_UPTIME_CC_COUNT 3
#elif NRF_RTC1_BASE == BSPACM_UPTIME_RTC_BASE
#define BSPACM_UPTIME_RTC NRF_RTC1
#define BSPACM_UPTIME_CC_COUNT 4
#else /* BSPACM_UPTIME_RTC_BASE */
#error Unrecognized uptime RTC
#endif /* BSPACM_UPTIME_RTC_BASE */

/** The frequency of the BSPACM_UPTIME_RTC peripheral in Hz. */
#define BSPACM_UPTIME_Hz 32768U

/** The minimum duration in uptime ticks that will cause
 * bBSPACMuptimeSleep() to actually sleep. */
#define BSPACM_UPTIME_SLEEP_MINIMUM 2

/** Convert a time from microseconds to uptime ticks, rounding up. */
inline
unsigned int
uiBSPACMuptimeConvert_us_utt (unsigned int dur_us)
{
  const unsigned int us_per_s = 1000000U;
  return (dur_us * BSPACM_UPTIME_Hz + us_per_s - 1) / us_per_s;
}

/** Convert a time from milliseconds to uptime ticks, rounding up. */
inline
unsigned int
uiBSPACMuptimeConvert_ms_utt (unsigned int dur_ms)
{
  const unsigned int ms_per_s = 1000U;
  return (dur_ms * BSPACM_UPTIME_Hz + ms_per_s - 1) / ms_per_s;
}

/** Convert a time from uptime ticks to microseconds, rounding up. */
inline
unsigned int
uiBSPACMuptimeConvert_utt_us (unsigned int dur_utt)
{
  return ((1000000ULL * dur_utt) + BSPACM_UPTIME_Hz - 1) / BSPACM_UPTIME_Hz;
}

/** Convert a time from uptime ticks to milliseconds, rounding up. */
inline
unsigned int
uiBSPACMuptimeConvert_utt_ms (unsigned int dur_utt)
{
  return ((1000ULL * dur_utt) + BSPACM_UPTIME_Hz - 1) / BSPACM_UPTIME_Hz;
}

/* Forward declaration */
struct sBSPACMuptimeAlarm;

/** Type for an alarm callback function.
 *
 * It is explicitly permitted to invoke iBSPACMuptimeAlarmSet() and
 * iBSPACMuptimeAlarmCancel() from within the callback.
 *
 * @param ap pointer to the alarm structure.  This may be embedded in
 * a larger structure to pass context to the callback.
 *
 * @note The callback is invoked from the first level interrupt
 * handler and is subject to all the usual caveats about what should
 * be done in that context.
 *
 * @note If the registered alarm is scheduled for repeated invocation,
 * the corresponding compare register will have been updated to the
 * next alarm time prior to invoking the callback. */
typedef void (* vBSPACMuptimeAlarmCallback_flih) (int ccidx,
                                                  struct sBSPACMuptimeAlarm * ap);

/** State for an alarm driven by the uptime clock. */
typedef struct sBSPACMuptimeAlarm {
  /** An optional callback, invoked from the handler after applying
   * #interval_utt. */
  vBSPACMuptimeAlarmCallback_flih callback_flih;

  /** An optional interval between repeated alarms.
   *
   * If the value is zero the alarm is removed from the set of alarms.
   * If the value is not zero, a new alarm is scheduled for @p
   * interval_utt ticks after the previous alarm.  Both operations are
   * applied before the callback is invoked, so the callback may
   * override that behavior. */
  unsigned int interval_utt;
} sBSPACMuptimeAlarm;

/** A handle to an uptime alarm structure */
typedef sBSPACMuptimeAlarm * hBSPACMuptimeAlarm;

/** Schedule an alarm on the uptime clock.
 *
 * @param ccidx the capture/compare register to use for the alarm.  If
 * the passed value is not valid for the RTC peripheral being used as
 * #BSPACM_UPTIME_RTC, or if there is already an alarm attached to the
 * register, the call returns an error.  Note that ccidx 0 is reserved.
 *
 * @param when_utt the absolute time at which the alarm should go off.
 * @note Alarm times are limited by the 24-bit resolution of the RTC.
 *
 * @param ap a pointer to the alarm structure
 *
 * @return 0 if successfully scheduled, or a negative error code.
 * Attempting to schedule an alarm on an unstarted/disabled clock is
 * an error. */
int
iBSPACMuptimeAlarmSet (int ccidx,
                       unsigned int when_utt,
                       hBSPACMuptimeAlarm ap);

/** Remove an alarm from the uptime clock
 *
 * @param ccidx the capture/compare register from which the alarm
 * should be removed.  If this register is not valid for the RTC
 * peripheral an error is returned.
 *
 * @param pendingp optional pointer to a record of whether the alarm
 * was pending when it was removed.  Pass a null pointer if this
 * information is not relevant.
 *
 * @return the handle for the alarm that was removed, or a null
 * pointer if no alarm was present or the ccidx was invalid. */
hBSPACMuptimeAlarm
hBSPACMuptimeAlarmClear (int ccidx,
                         bool * pendingp);

/** Return the absolute uptime in its highest available precision */
unsigned long long
ullBSPACMuptime (void);

/** Return the current uptime truncated to the lowest 32 bits */
unsigned int
uiBSPACMuptime (void);

/** Configure and enable the uptime clock. */
void
vBSPACMuptimeStart (void);

/** Return @c true if and only if the uptime clock has been started
 * and is still running. */
bool
bBSPACMuptimeEnabled (void);

/** Function to cancel any in-progress bBSPACMuptimeSleep().
 *
 * This may be called from interrupt handlers to force an early wakeup
 * when a situation requires the application to react early. */
void
vBSPACMuptimeSleepCancel (void);

/** Function to sleep for a given duration.
 *
 * This allows sleeps up to 512 s (24 bits of a 32 KiHz timer), much
 * longer than vBSPACMhiresSleep_us().
 *
 * @warning If this function is invoked while bBSPACMuptimeEnabled()
 * returns false, it will hang.  This is a bigger clue that your
 * program is incorrect than any other reasonable behavior.
 *
 * @param duration_utt duration of the sleep, in #BSPACM_UPTIME_Hz
 * ticks.  Values are taken modulo 2^24.  Values less than
 * #BSPACM_UPTIME_SLEEP_MINIMUM cause immediate return @c true.
 *
 * @return @c true if the sleep ran to completion or was shorter than
 * #BSPACM_UPTIME_SLEEP_MINIMUM; @c false if
 * vBSPACMuptimeSleepCancel() was invoked during an interrupt handler
 * to force an early wakeup. */
bool
bBSPACMuptimeSleep (unsigned int duration_utt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_UTILITY_UPTIME_H */
