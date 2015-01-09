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

/* Implementation for nRF51 device series TWI (I2C) peripheral interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/twi.h>
#include <bspacm/utility/hires.h>
#include <bspacm/utility/uptime.h>
#include <string.h>

#include "nrf51_bitfields.h"

hBSPACMi2cBus
hBSPACMi2cConfigureBus (sBSPACMi2cBus * tpp,
                        NRF_TWI_Type * twi,
                        int sda_pin,
                        int scl_pin,
                        int ppi_chidx,
                        uint32_t frequency,
                        unsigned int timeout_us)
{
  if (! ((NULL != tpp)
         && (NULL != twi)
         && (0 <= sda_pin) && (sda_pin <= 31)
         && (0 <= scl_pin) && (scl_pin <= 31)
         && ((0 > ppi_chidx) || (ppi_chidx < (sizeof(NRF_PPI->CH)/sizeof(*NRF_PPI->CH)))))) {
    return NULL;
  }

  /* Reject ppi_chidx configuation values that are inconsistent with
   * the compile-time decision regarding PAN #36. */
  if (
#if (BSPACM_NRF_APPLY_PAN_36 - 0)
      0 > ppi_chidx
#else /* BSPACM_NRF_APPLY_PAN_36 */
      -1 != ppi_chidx
#endif /* BSPACM_NRF_APPLY_PAN_36 */
      ) {
    return NULL;
  }

  memset(tpp, 0, sizeof(*tpp));
  tpp->twi = twi;
  tpp->sda_pin = sda_pin;
  tpp->scl_pin = scl_pin;
#if (BSPACM_NRF_APPLY_PAN_36 - 0)
  tpp->ppi_chidx = ppi_chidx;
#endif /* BSPACM_NRF_APPLY_PAN_36 */
  tpp->frequency = frequency;

  if (BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us > timeout_us) {
    timeout_us = BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us;
  }
  tpp->timeout_utt = uiBSPACMuptimeConvert_us_utt(timeout_us);
  return tpp;
}

/** The GPIO configuration used for TWI pins when clearing the bus.
 * The important part is to configure them for output, as we need to
 * toggle SCL, and to enable the input buffer, as we need to read both
 * SDA and SCL. */
#define TWI_GPIO_PIN_CNF ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
                           | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) \
                           | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) \
                           | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) \
                           | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) )

/** Reset the I2C bus to idle state if a secondary device is holding
 * it active.
 *
 * The bus is enabled if and only if the return value is zero (no
 * error).
 *
 * @return zero if bus is left in the idle state (SDA and SCL not
 * pulled low), and #BSPACM_NRF_TWI_BUS_ERROR_CLEAR_FAILED if not.
 */
static int
twi_bus_clear (hBSPACMi2cBus tpp)
{
  uint32_t const sda_bit = (1U << tpp->sda_pin);
  uint32_t const scl_bit = (1U << tpp->scl_pin);
  uint32_t const bits = scl_bit | sda_bit;
  unsigned int const half_cycle_us = 5; /* Half cycle at 100 kHz */
  bool cleared;

  /* Pull up SDA and SCL then turn off TWI and wait a cycle to
   * settle before sampling the signals. */
  NRF_GPIO->OUTSET = bits;
  NRF_GPIO->PIN_CNF[tpp->sda_pin] = TWI_GPIO_PIN_CNF;
  NRF_GPIO->PIN_CNF[tpp->scl_pin] = TWI_GPIO_PIN_CNF;
  tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);

  vBSPACMhiresSleep_us(2 * half_cycle_us);
  cleared = (bits == (bits & NRF_GPIO->IN));

  if (! cleared) {
    /* At least one of SDA or SCL is being held low by a follower
     * device.  Toggle the clock enough to flush both leader and
     * follower bytes, then see if it's let go. */
    int cycles = 18;
    while (0 < cycles--) {
      NRF_GPIO->OUTCLR = scl_bit;
      vBSPACMhiresSleep_us(half_cycle_us);
      NRF_GPIO->OUTSET = scl_bit;
      vBSPACMhiresSleep_us(half_cycle_us);
    }
    cleared = (bits == (bits & NRF_GPIO->IN));
  }

  if (cleared) {
    tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos);
  }

  /* Return success if the bus is cleared. */
  return cleared ? 0 : BSPACM_NRF_TWI_BUS_ERROR_CLEAR_FAILED;
}

int
iBSPACMi2cSetEnabled (hBSPACMi2cBus tpp,
                      bool enabled)
{
  int rv = 0;

  tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);
  if (enabled) {
    tpp->twi->PSELSCL = tpp->scl_pin;
    tpp->twi->PSELSDA = tpp->sda_pin;
    tpp->twi->FREQUENCY = tpp->frequency;

#if (BSPACM_NRF_APPLY_PAN_36 - 0)
    NRF_PPI->CHENCLR = PPI_CHENCLR_CH0_Msk << tpp->ppi_chidx;
#endif /* BSPACM_NRF_APPLY_PAN_36 */
    rv = - twi_bus_clear(tpp);
  }
  return rv;
}

/** Invoked when a read or write operation detects a bus error.
 *
 * The error cause(s) are read and returned as a bit set.  If no error
 * is detected, #BSPACM_NRF_TWI_BUS_ERROR_UNKNOWN is set.  An attempt is made to
 * restore the bus to a cleared state; failure to do so is also
 * indicated in the return value.
 *
 * Callers may combine the return value with other error indicators
 * but should negate the result prior to returning it as an error from
 * public API functions.
 *
 * @return a positive integer representing one or more error flags,
 * possibly including a failure to clear the bus, or an indication
 * that the cause of the aerror could not be determined. */
static int
twi_error_clear (hBSPACMi2cBus tpp)
{
  int rv = tpp->twi->ERRORSRC;
  if (0 == rv) {
    rv |= BSPACM_NRF_TWI_BUS_ERROR_UNKNOWN;
  }
  tpp->twi->ERRORSRC = 0;
  rv |= twi_bus_clear(tpp);
  return rv;
}

#if (BSPACM_NRF_APPLY_PAN_56 - 0)
/** Power cycle the peripheral and re-initialize it.
 *
 * Returns zero if the reset was successful, otherwise a negative error code. */
static int
twi_pan56_reset (hBSPACMi2cBus tpp)
{
  uint16_t address = tpp->twi->ADDRESS;
  int rv;

#if 0
  printf("PAN56 Workaround!\n");
  /* Use this to reset the loop limit to something that works when
   * verifying the behavior of a too-short limit */
  ((sBSPACMi2cBus*)tpp)->timeout_utt = 100;
#endif
  tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);
  tpp->twi->POWER = 0;
  vBSPACMhiresSleep_us(5);
  tpp->twi->POWER = 1;
  rv = - iBSPACMi2cSetEnabled(tpp, true);
  if (0 == rv) {
    tpp->twi->ADDRESS = address;
  }
  return rv;
}
#endif /* BSPACM_NRF_APPLY_PAN_56 */

int
iBSPACMi2cRead (hBSPACMi2cBus tpp,
                unsigned int addr,
                uint8_t * dp,
                size_t len)
{
  uint8_t * const dps = dp;
  uint8_t * const dpe = dps + len;
  int rv = len;

/* The nRF51 RM demonstrates BB->SUSPEND and BB->STOP connections
 * during TWI reads, but fails to motivate this except for the
 * BB->STOP required for the last byte read.  A compelling discusion
 * of why the non-final byte connections are also needed is at:
 * https://devzone.nordicsemi.com/question/17472
 *
 * Select the appropriate mechanism to provide this coordination. */
#if (BSPACM_NRF_APPLY_PAN_36 - 0)
  NRF_PPI->CH[tpp->ppi_chidx].EEP = (uintptr_t)&tpp->twi->EVENTS_BB;
  if (1 == len) {
    NRF_PPI->CH[tpp->ppi_chidx].TEP = (uintptr_t)&tpp->twi->TASKS_STOP;
  } else {
    NRF_PPI->CH[tpp->ppi_chidx].TEP = (uintptr_t)&tpp->twi->TASKS_SUSPEND;
  }
  NRF_PPI->CHENSET = PPI_CHENSET_CH0_Msk << tpp->ppi_chidx;
#else /* */
  if (1 == len) {
    tpp->twi->SHORTS = (TWI_SHORTS_BB_STOP_Enabled << TWI_SHORTS_BB_STOP_Pos);
  } else {
    tpp->twi->SHORTS = (TWI_SHORTS_BB_SUSPEND_Enabled << TWI_SHORTS_BB_SUSPEND_Pos);
  }
#endif /* BSPACM_NRF_APPLY_PAN_36 */

  tpp->twi->ADDRESS = addr;
  tpp->twi->EVENTS_ERROR = 0;
  tpp->twi->EVENTS_STOPPED = 0;
  tpp->twi->EVENTS_RXDREADY = 0;
  tpp->twi->TASKS_STARTRX = 1;

  while (! bBSPACMuptimeEnabled()) {
    /* If you block here, you didn't enable the uptime clock */
  }
  unsigned int start_utt = uiBSPACMuptime();

  while (dp < dpe) {
    while (! (tpp->twi->EVENTS_RXDREADY
              || tpp->twi->EVENTS_ERROR)) {
      if ((uiBSPACMuptime() - start_utt) >= tpp->timeout_utt) {
        break;
      }
    }
    if (tpp->twi->EVENTS_ERROR) {
      tpp->twi->EVENTS_STOPPED = 0;
      tpp->twi->TASKS_STOP = 1;
      rv = -1;
      break;
    } else if (tpp->twi->EVENTS_RXDREADY) {
      *dp++ = tpp->twi->RXD;
      if ((dp+1) >= dpe) {
#if (BSPACM_NRF_APPLY_PAN_36 - 0)
        NRF_PPI->CH[tpp->ppi_chidx].TEP = (uintptr_t)&tpp->twi->TASKS_STOP;
#else /* BSPACM_NRF_APPLY_PAN_36 */
        tpp->twi->SHORTS = (TWI_SHORTS_BB_STOP_Enabled << TWI_SHORTS_BB_STOP_Pos);
#endif /* BSPACM_NRF_APPLY_PAN_36 */
      }
      tpp->twi->EVENTS_RXDREADY = 0;
      start_utt = uiBSPACMuptime();
    } else {
      rv = BSPACM_NRF_TWI_BUS_ERROR_TIMEOUT;
#if (BSPACM_NRF_APPLY_PAN_56 - 0)
      rv |= twi_pan56_reset(tpp);
#endif /* BSPACM_NRF_APPLY_PAN_56 */
      return -rv;
    }
#if (BSPACM_NRF_APPLY_PAN_56 - 0)
    /* PAN-56: module lock-up
     *
     * Delay RESUME for a least two TWI clock periods after RXD read
     * to ensure clock-stretched ACK completes.
     *
     * 100 kHz = 20us, 400 kHz = 5us. */
    vBSPACMhiresSleep_us(20);
#endif /* BSPACM_NRF_APPLY_PAN_56 */
    tpp->twi->TASKS_RESUME = 1;
  }

#if (BSPACM_NRF_APPLY_PAN_36 - 0)
  NRF_PPI->CHENCLR = PPI_CHENCLR_CH0_Msk << tpp->ppi_chidx;
#else /* BSPACM_NRF_APPLY_PAN_36 */
  tpp->twi->SHORTS = 0;
#endif /* BSPACM_NRF_APPLY_PAN_36 */

  while (! tpp->twi->EVENTS_STOPPED) {
  }
  tpp->twi->EVENTS_STOPPED = 0;

  if (0 > rv) {
    /* Obtain details on error cause */
    rv = - twi_error_clear(tpp);
  }
  return rv;
}

int
iBSPACMi2cWrite (hBSPACMi2cBus tpp,
                 unsigned int addr,
                 const uint8_t * sp,
                 size_t len)
{
  const uint8_t * const spe = sp + len;
  int rv = len;

  tpp->twi->ADDRESS = addr;
  tpp->twi->EVENTS_ERROR = 0;
  tpp->twi->EVENTS_TXDSENT = 0;
  tpp->twi->TXD = *sp;
  tpp->twi->TASKS_STARTTX = 1;

  while (! bBSPACMuptimeEnabled()) {
    /* If you block here, you didn't enable the uptime clock */
  }
  unsigned int start_utt = uiBSPACMuptime();

  while (true) {
    while (! (tpp->twi->EVENTS_TXDSENT
              || tpp->twi->EVENTS_ERROR)) {
      if ((uiBSPACMuptime() - start_utt) >= tpp->timeout_utt) {
        break;
      }
    }
    if (tpp->twi->EVENTS_ERROR) {
      rv = -1;
      break;
    } else if (tpp->twi->EVENTS_TXDSENT) {
      tpp->twi->EVENTS_TXDSENT = 0;
      if (++sp < spe) {
        tpp->twi->TXD = *sp;
      } else {
        break;
      }
    } else {
      rv = BSPACM_NRF_TWI_BUS_ERROR_TIMEOUT;
#if (BSPACM_NRF_APPLY_PAN_56 - 0)
      rv |= twi_pan56_reset(tpp);
#endif /* BSPACM_NRF_APPLY_PAN_56 */
      return -rv;
    }
  }
  tpp->twi->EVENTS_STOPPED = 0;
  tpp->twi->TASKS_STOP = 1;
  while (! tpp->twi->EVENTS_STOPPED) {
  }

  if (0 > rv) {
    /* Obtain details on error cause */
    rv = - twi_error_clear(tpp);
  }
  return rv;
}
