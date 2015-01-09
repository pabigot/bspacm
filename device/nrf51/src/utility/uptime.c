/* BSPACM - nRF51 low-resolution uptime support
 *
 * Copyright 2015, Peter A. Bigot
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

#include <bspacm/utility/uptime.h>
#include <string.h>

#include "nrf51_bitfields.h"

#if NRF_RTC0_BASE == BSPACM_UPTIME_RTC_BASE
#define BSPACM_UPTIME_RTC_IRQn RTC0_IRQn
#define BSPACM_UPTIME_RTC_IRQHandler RTC0_IRQHandler
#elif NRF_RTC1_BASE == BSPACM_UPTIME_RTC_BASE
#define BSPACM_UPTIME_RTC_IRQn RTC1_IRQn
#define BSPACM_UPTIME_RTC_IRQHandler RTC1_IRQHandler
#else /* BSPACM_UPTIME_RTC_BASE */
#error Unrecognized uptime RTC
#endif /* BSPACM_UPTIME_RTC_BASE */

#define RTC_COUNTER_BITS 24
#define RTC_COUNTER_MASK ((1U << RTC_COUNTER_BITS) - 1)
#define SLEEP_CCIDX 0
#define SLEEP_COMPARE_BIT (RTC_INTENSET_COMPARE0_Enabled << (SLEEP_CCIDX + RTC_INTENSET_COMPARE0_Pos))

sBSPACMuptimeState xBSPACMuptimeState_;

int
iBSPACMuptimeAlarmSet (int ccidx,
                       unsigned int when_utt,
                       hBSPACMuptimeAlarm ap)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  int rv = -1;

  if (! bBSPACMuptimeEnabled()) {
    return -1;
  }
  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if ((0 > ccidx)
        || (SLEEP_CCIDX == ccidx)
        || (BSPACM_UPTIME_CC_COUNT <= ccidx)
        || (NULL != xBSPACMuptimeState_.alarm[ccidx])
        || (NULL == ap)) {
      break;
    }
    BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx] = 0;
    BSPACM_UPTIME_RTC->CC[ccidx] = when_utt;
    BSPACM_UPTIME_RTC->INTENSET = (RTC_INTENSET_COMPARE0_Enabled << (ccidx + RTC_INTENSET_COMPARE0_Pos));
    xBSPACMuptimeState_.alarm[ccidx] = ap;
    rv = 0;
  } while (0);
  BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rv;
}

hBSPACMuptimeAlarm
hBSPACMuptimeAlarmClear (int ccidx,
                         bool * pendingp)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  hBSPACMuptimeAlarm rv = NULL;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    if ((1 > ccidx)
        || (BSPACM_UPTIME_CC_COUNT <= ccidx)) {
      break;
    }
    /* Disable the interrupt.  If there's one pending, record that
     * fact and clear it. */
    BSPACM_UPTIME_RTC->INTENCLR = (RTC_INTENCLR_COMPARE0_Enabled << (ccidx + RTC_INTENCLR_COMPARE0_Pos));
    bool pending = BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx];
    BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx] = 0;
    if (pendingp) {
      *pendingp = pending;
    }
    rv = xBSPACMuptimeState_.alarm[ccidx];
    xBSPACMuptimeState_.alarm[ccidx] = NULL;
  } while (0);
  BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rv;
}

void
vBSPACMuptimeStart ()
{
  int ccidx;

  /* RTC peripherals uses the low frequency clock.  Start it (in its
   * crystal oscillator form) if nobody's done so already. */
  if ((CLOCK_LFCLKSTAT_STATE_Running << CLOCK_LFCLKSTAT_STATE_Pos)
      != (CLOCK_LFCLKSTAT_STATE_Msk & NRF_CLOCK->LFCLKSTAT)) {
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while (! NRF_CLOCK->EVENTS_LFCLKSTARTED) {
    }
  }

  /* Stop the clock, reset it and the overflow count, clear all
   * events, disable PPI event forwarding, disable all interrupts
   * except overflow. */
  BSPACM_UPTIME_RTC->TASKS_STOP = 1;
  memset(&xBSPACMuptimeState_, 0, sizeof(xBSPACMuptimeState_));
  BSPACM_UPTIME_RTC->TASKS_CLEAR = 1;
  BSPACM_UPTIME_RTC->EVENTS_TICK = 0;
  BSPACM_UPTIME_RTC->EVENTS_OVRFLW = 0;
  for (ccidx = 0; ccidx < BSPACM_UPTIME_CC_COUNT; ++ccidx) {
    BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx] = 0;
  }
  BSPACM_UPTIME_RTC->EVTENCLR = ~0;
  BSPACM_UPTIME_RTC->INTENCLR = ~0;
  BSPACM_UPTIME_RTC->INTENSET = (RTC_INTENSET_OVRFLW_Enabled << RTC_INTENSET_OVRFLW_Pos);

  /* Enable the interrupts at the NVIC */
  NVIC_ClearPendingIRQ(BSPACM_UPTIME_RTC_IRQn);
  vBSPACMnrf51NVICsetApplicationPriority(BSPACM_UPTIME_RTC_IRQn, true);
  NVIC_EnableIRQ(BSPACM_UPTIME_RTC_IRQn);

  /* And start the clock */
  BSPACM_UPTIME_RTC->TASKS_START = 1;
  xBSPACMuptimeState_.enabled = true;
}

static volatile bool sleep_aborted;
static volatile bool sleep_wakeup;

void
vBSPACMuptimeSleepCancel (void)
{
  sleep_aborted = true;
}

bool
bBSPACMuptimeSleep (unsigned int duration_utt)
{
  while (! bBSPACMuptimeEnabled()) {
    /* If you get here you forgot to start or re-enable the clock. */
  }

  /* Mask off the bits that aren't supported by the counter, so we
   * check for "now" using the value the peripheral will use. */
  duration_utt &= RTC_COUNTER_MASK;
  if (BSPACM_UPTIME_SLEEP_MINIMUM > duration_utt) {
    return true;
  }

  /* We are expecting the following instructions up through INTENSET
   * take less than BSPACM_UPTIME_SLEEP_MINIMUM-1 uptime clock
   * ticks. */
  BSPACM_UPTIME_RTC->CC[SLEEP_CCIDX] = BSPACM_UPTIME_RTC->COUNTER + duration_utt;
  BSPACM_UPTIME_RTC->EVENTS_COMPARE[SLEEP_CCIDX] = 0;
  sleep_aborted = sleep_wakeup = false;
  BSPACM_UPTIME_RTC->INTENSET = SLEEP_COMPARE_BIT;
  while (! (sleep_wakeup || sleep_aborted)) {
    __WFE();
  }
  BSPACM_UPTIME_RTC->INTENCLR = SLEEP_COMPARE_BIT;
  BSPACM_UPTIME_RTC->EVENTS_COMPARE[SLEEP_CCIDX] = 0;
  return sleep_wakeup;
}

void
BSPACM_UPTIME_RTC_IRQHandler ()
{
  int ccidx;
  if (BSPACM_UPTIME_RTC->EVENTS_OVRFLW) {
    BSPACM_UPTIME_RTC->EVENTS_OVRFLW = 0;
    ++xBSPACMuptimeState_.overflows;
  }
  if (BSPACM_UPTIME_RTC->EVENTS_COMPARE[SLEEP_CCIDX]) {
    BSPACM_UPTIME_RTC->EVENTS_COMPARE[SLEEP_CCIDX] = 0;
    sleep_wakeup = true;
    BSPACM_UPTIME_RTC->INTENCLR = SLEEP_COMPARE_BIT;
  }
  for (ccidx = 0; ccidx < BSPACM_UPTIME_CC_COUNT; ++ccidx) {
    if (BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx]) {
      hBSPACMuptimeAlarm ap = xBSPACMuptimeState_.alarm[ccidx];
      BSPACM_UPTIME_RTC->EVENTS_COMPARE[ccidx] = 0;
      if (ap) {
        if (ap->interval_utt) {
          BSPACM_UPTIME_RTC->CC[ccidx] += ap->interval_utt;
        } else {
          (void)hBSPACMuptimeAlarmClear(ccidx, NULL);
        }
        if (ap->callback_flih) {
          ap->callback_flih(ccidx, ap);
        }
      }
    }
  }
}
