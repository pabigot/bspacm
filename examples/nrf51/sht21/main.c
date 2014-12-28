/* BSPACM - nRF51 i2c stuff
 *
 * Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/utility/misc.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <string.h>
#include "nrf_delay.h"

#ifndef PIN_SDA
#if (BSPACM_BOARD_NRF51_PCA10028 - 0)
#define PIN_SDA 7
#else /* PCA10028 */
#define PIN_SDA 25
#endif /* PCA10028 */
#endif /* PIN_SDA */
#ifndef PIN_SCL
#if (BSPACM_BOARD_NRF51_PCA10028 - 0)
#define PIN_SCL 30
#else /* PCA10028 */
#define PIN_SCL 24
#endif /* PCA10028 */
#endif /* PIN_SCL */

#define BIT_SDA (1UL << PIN_SDA)
#define BIT_SCL (1UL << PIN_SCL)

#define SHT21_ADDRESS 0x40

/* These constants define the bits for componentized basic 8-bit
 * commands described in the data sheet. */
#define SHT21_CMDBIT_BASE 0xE0
#define SHT21_CMDBIT_READ 0x01
#define SHT21_CMDBIT_TEMP 0x02
#define SHT21_CMDBIT_RH 0x04
#define SHT21_CMDBIT_UR 0x06
#define SHT21_CMDBIT_NOHOLD 0x10

#define SHT21_TRIGGER_T_HM (SHT21_CMDBIT_BASE | SHT21_CMDBIT_TEMP | SHT21_CMDBIT_READ)
#define SHT21_TRIGGER_RH_HM (SHT21_CMDBIT_BASE | SHT21_CMDBIT_RH | SHT21_CMDBIT_READ)

const uint32_t UPTIME_Hz = 32768;
NRF_RTC_Type * const uptime_rtc = NRF_RTC0;
const int uptime_ccidx_1Hz = 0;
const int uptime_ccidx_alarm = 1;

volatile uint32_t uptime_s;
volatile uint32_t rtc_overflows;
uint32_t rtc_now_last_overflow_;

inline uint64_t rtc_now ()
{
  uint32_t prev_ofl;
  uint32_t ctr24;

  /* Get a consistent pair of counter and overflow values */
  do {
    prev_ofl = rtc_now_last_overflow_;
    ctr24 = uptime_rtc->COUNTER;
    rtc_now_last_overflow_ = rtc_overflows;
  } while (prev_ofl != rtc_now_last_overflow_);
  return (((uint64_t)rtc_now_last_overflow_) << 24) | ctr24;
}

typedef struct uptime_alarm {
  struct uptime_alarm * next;   /** Next alarm to schedule, or NULL to terminate alarm sequence */
  uint32_t delta_utt;           /** Ticks until next alarm should go off */
} uptime_alarm;

/** Pending alarm */
const uptime_alarm * volatile uptime_alarms_;
/** Last alarm to fire */
const uptime_alarm * volatile uptime_alarm_;

/* Return a pointer to the last alarm that fired, or a null pointer if
 * no alarm has fired since the last call to this function. */
const uptime_alarm * uptime_get_alarm ()
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  const uptime_alarm * rv;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    rv = uptime_alarm_;
    uptime_alarm_ = NULL;
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

/** Register a sequence of alarms with the first scheduled at @p
 * alarm_time_utt.  Pass a null pointer as @p sp to disable existing
 * alarms. */
void uptime_set_alarms (const uptime_alarm * sp,
                        uint32_t alarm_time_utt)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    uptime_alarms_ = sp;
    if (uptime_alarms_) {
      uptime_rtc->CC[uptime_ccidx_alarm] = alarm_time_utt;
      uptime_rtc->INTENSET = (RTC_INTENSET_COMPARE0_Enabled << (RTC_INTENSET_COMPARE0_Pos + uptime_ccidx_alarm));
    } else {
      uptime_rtc->INTENCLR = (RTC_INTENSET_COMPARE0_Enabled << (RTC_INTENSET_COMPARE0_Pos + uptime_ccidx_alarm));
    }
    uptime_alarm_ = NULL;
    uptime_rtc->EVENTS_COMPARE[uptime_ccidx_alarm] = 0;
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
}

void RTC0_IRQHandler ()
{
  if (uptime_rtc->EVENTS_OVRFLW) {
    uptime_rtc->EVENTS_OVRFLW = 0;
    ++rtc_overflows;
  }
  if (uptime_rtc->EVENTS_COMPARE[uptime_ccidx_1Hz]) {
    uptime_rtc->EVENTS_COMPARE[uptime_ccidx_1Hz] = 0;
    uptime_rtc->CC[uptime_ccidx_1Hz] += UPTIME_Hz;
    ++uptime_s;
  }
  if (uptime_rtc->EVENTS_COMPARE[uptime_ccidx_alarm]) {
    uptime_rtc->EVENTS_COMPARE[uptime_ccidx_alarm] = 0;
    if (uptime_alarms_) {
      uptime_alarm_ = uptime_alarms_;
      uptime_alarms_ = uptime_alarm_->next;
    }
    if (uptime_alarm_) {
      uptime_rtc->CC[uptime_ccidx_alarm] += uptime_alarm_->delta_utt;
    } else {
      uptime_rtc->INTENCLR = (RTC_INTENSET_COMPARE0_Enabled << (RTC_INTENSET_COMPARE0_Pos + uptime_ccidx_alarm));
    }
  }
}

/* TWI bus error returns are negative integers with absolute value
 * taken from the ERRORSRC register augmented by several flags for
 * other error conditions. */

#define TWI_BUS_ERROR_PAN_56 0x100
#define TWI_BUS_ERROR_CLEAR_FAILED 0x200
#define TWI_BUS_ERROR_UNKNOWN 0x400

#ifndef ENABLE_PAN_36
/** PAN-36: Shortcuts are not functional
 * Use PPI channel to enforce BB to SUSPEND and STOP tasks */
#define ENABLE_PAN_36 1
#endif /* ENABLE_PAN_36 */

#ifndef ENABLE_PAN_56
/** PAN-56: module lock-up
 *
 * Various conditions require a full power-cycle of the TWI module to
 * restore functionality.  The likelihood of occurrence can be
 * lessened by certain behaviors, but in the end we need to alarm if
 * an expected RXDREADY or TXDREADY signal is not received in a timely
 * manner. */
#define ENABLE_PAN_56 1
#endif /* ENABLE_PAN_56 */

typedef struct twi_periphs_type {
  NRF_TWI_Type * twi;
#if (ENABLE_PAN_36 - 0)
  NRF_PPI_Type * ppi;
  unsigned int chidx;
#endif /* ENABLE_PAN_36 */
#if (ENABLE_PAN_56 - 0)
  /* If the block for TXDREADY or RXDREADY takes more than this number
   * of iterations then assume the TWI is locked up and power-cycle
   * it.  A value of zero uses a non-zero internal default. */
  unsigned int pan56_loop_limit;

#ifndef DEFAULT_PAN56_LOOP_LIMIT
  /* Default loop limit if pan56_loop_limit is zero.  Ignoring delays
   * due to devices being slow, we need enough TWI clock cycles to
   * process one octet.  Give it a little leeway, too.  Let's do 100
   * SCLK cycles at 100 kHz, or 1 ms, and assume 16 MHz MCU clock and
   * 5 clocks per iteration. */
#define DEFAULT_PAN56_LOOP_LIMIT (16000 / 5)
#endif /* DEFAULT_PAN56_LOOP_LIMIT */

#define PAN56_DECLARE_AND_INITIALIZE(tpp_)       \
  int loop_limit_ = (tpp_)->pan56_loop_limit;    \
  if (0 == loop_limit_) {                        \
    loop_limit_ = DEFAULT_PAN56_LOOP_LIMIT;      \
  } else if (0 > loop_limit_) {                  \
    loop_limit_ = INT_MAX;                       \
  }
#define PAN56_CHECK_EXCEEDED() (0 >= --loop_limit_)
#define PAN56_MAYBE_RETURN_RESET(tpp_) do {     \
    if (0 > loop_limit_) {                      \
      return - (TWI_BUS_ERROR_PAN_56 | - twi_pan56_reset(tpp_)); \
    }                                           \
  } while (0)

#else /* ENABLE_PAN_56 */
#define PAN56_DECLARE_AND_INITIALIZE(tpp_)  do { } while (0)
#define PAN56_CHECK_EXCEEDED() (0)
#define PAN56_MAYBE_RETURN_RESET(tpp_) do { } while (0)
#endif /* ENABLE_PAN_56 */
} twi_periphs_type;

/** Reset the I2C bus to idle state if a secondary device is holding
 * it active.
 *
 * The return value is zero if bus is left in the idle state (SDA and
 * SCL not pulled low), and #TWI_BUS_ERROR_CLEAR_FAILED if not. */
int twi_bus_clear (const twi_periphs_type * tpp)
{
  const uint32_t pin_cnf = (0
    | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
    | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
    | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
    | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
    );
  int pin_scl = 0x1f & tpp->twi->PSELSCL;
  int pin_sda = 0x1f & tpp->twi->PSELSDA;
  uint32_t const bit_scl = (1U << pin_scl);
  uint32_t const bit_sda = (1U << pin_sda);
  uint32_t const bits = bit_scl | bit_sda;
  uint32_t pin_cnf_scl = NRF_GPIO->PIN_CNF[pin_scl];
  uint32_t pin_cnf_sda = NRF_GPIO->PIN_CNF[pin_sda];
  uint32_t enable = tpp->twi->ENABLE;
  unsigned int const half_cycle_us = 5; /* Half cycle at 100 kHz */
  bool cleared;

  /* Pull up SDA and SCL then turn off TWI and wait a cycle to
   * settle before sampling the signals. */
  NRF_GPIO->PIN_CNF[pin_scl] = pin_cnf;
  NRF_GPIO->PIN_CNF[pin_sda] = pin_cnf;
  NRF_GPIO->OUTSET = bits;
  tpp->twi->ENABLE = 0;
  nrf_delay_us(2 * half_cycle_us);
  cleared = (bits == (bits & NRF_GPIO->IN));

  if (! cleared) {
    /* At least one of SDA or SCL is being held low by a follower
     * device.  Toggle the clock enough to flush both leader and
     * follower bytes, then see if it's let go. */
    int cycles = 18;
    while (0 < cycles--) {
      NRF_GPIO->OUTCLR = bit_scl;
      nrf_delay_us(half_cycle_us);
      NRF_GPIO->OUTSET = bit_scl;
      nrf_delay_us(half_cycle_us);
    }
    cleared = (bits == (bits & NRF_GPIO->IN));
  }

  /* Put the pins back to inputs and restore the TWI peripheral. */
  NRF_GPIO->PIN_CNF[pin_sda] = pin_cnf_sda;
  NRF_GPIO->PIN_CNF[pin_scl] = pin_cnf_scl;
  tpp->twi->ENABLE = enable;

  /* Return success if the bus is cleared. */
  return cleared ? 0 : TWI_BUS_ERROR_CLEAR_FAILED;
}

/** Invoked when a read or write operation detects a bus error.
 *
 * The error cause(s) are read and returned as a bit set.  If no error
 * is detected, #TWI_BUS_ERROR_UNKNOWN is set.  An attempt is made to
 * restore the bus to a cleared state; failure to do so is also
 * indicated in the return value.
 *
 * The return value will always be a positive integer representing one
 * or more error flags. */
int twi_error_clear (const twi_periphs_type * tpp)
{
  int rv = tpp->twi->ERRORSRC;
  if (0 == rv) {
    rv |= TWI_BUS_ERROR_UNKNOWN;
  }
  tpp->twi->ERRORSRC = 0;
  rv |= twi_bus_clear(tpp);
  return rv;
}

/** Configure and enable the TWI peripheral.
 *
 * @param pin_scl the pin to use as the serial clock signal.  nRF51
 * default is 24 but note this is shared with an LED on some boards.
 *
 * @param pin_sda the pin to use as the serial data signal.  nRF51
 * default is 25.
 *
 * @param frequency the setting for the peripheral FREQUENCY register.
 * For example, #TWI_FREQUENCY_FREQUENCY_K400.
 *
 * @return Zero if configuration was successful, otherwise a negative
 * error code. */
int twi_initialize (const twi_periphs_type * tpp,
                    int pin_scl,
                    int pin_sda,
                    uint32_t frequency)
{
  const uint32_t pin_cnf = (0
    | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
    | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
    | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
    | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
    );

  NRF_GPIO->PIN_CNF[pin_scl] = pin_cnf;
  NRF_GPIO->PIN_CNF[pin_sda] = pin_cnf;

  tpp->twi->EVENTS_RXDREADY = 0;
  tpp->twi->EVENTS_TXDSENT = 0;
  tpp->twi->PSELSCL = pin_scl;
  tpp->twi->PSELSDA = pin_sda;
  tpp->twi->FREQUENCY = frequency;

#if (ENABLE_PAN_36 - 0)
  tpp->ppi->CHENCLR = PPI_CHENCLR_CH0_Msk << tpp->chidx;
#endif /* ENABLE_PAN_36 */

  tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos);

  return - twi_bus_clear(tpp);
}

/** Power cycle the peripheral and re-initialize it.
 *
 * Returns zero if the reset was successful, otherwise a negative error code. */
int twi_pan56_reset (const twi_periphs_type * tpp)
{
  uint32_t pin_scl = tpp->twi->PSELSCL;
  uint32_t pin_sda = tpp->twi->PSELSDA;
  uint32_t frequency = tpp->twi->FREQUENCY;
  uint16_t address = tpp->twi->ADDRESS;
  int rv;

  printf("PAN56 Workaround!\n");
  ((twi_periphs_type*)tpp)->pan56_loop_limit = 0;
  tpp->twi->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);
  tpp->twi->POWER = 0;
  nrf_delay_us(5);
  tpp->twi->POWER = 1;
  rv = twi_initialize(tpp, pin_scl, pin_sda, frequency);
  if (0 == rv) {
    tpp->twi->ADDRESS = address;
  }
  return rv;
}

int twi_read (const twi_periphs_type * tpp,
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
#if (ENABLE_PAN_36 - 0)
  tpp->ppi->CH[tpp->chidx].EEP = (uintptr_t)&tpp->twi->EVENTS_BB;
  if (1 == len) {
    tpp->ppi->CH[tpp->chidx].TEP = (uintptr_t)&tpp->twi->TASKS_STOP;
  } else {
    tpp->ppi->CH[tpp->chidx].TEP = (uintptr_t)&tpp->twi->TASKS_SUSPEND;
  }
  tpp->ppi->CHENSET = PPI_CHENSET_CH0_Msk;
#else /* */
  if (1 == len) {
    tpp->twi->SHORTS = (TWI_SHORTS_BB_STOP_Enabled << TWI_SHORTS_BB_STOP_Pos);
  } else {
    tpp->twi->SHORTS = (TWI_SHORTS_BB_SUSPEND_Enabled << TWI_SHORTS_BB_SUSPEND_Pos);
  }
#endif /* ENABLE_PAN_36 */

  PAN56_DECLARE_AND_INITIALIZE(tpp);
  tpp->twi->EVENTS_ERROR = 0;
  tpp->twi->EVENTS_STOPPED = 0;
  tpp->twi->EVENTS_RXDREADY = 0;
  tpp->twi->TASKS_STARTRX = 1;
  while (dp < dpe) {
    while (! (tpp->twi->EVENTS_RXDREADY
              || PAN56_CHECK_EXCEEDED()
              || tpp->twi->EVENTS_ERROR)) {
    }
    PAN56_MAYBE_RETURN_RESET(tpp);
    if (tpp->twi->EVENTS_ERROR) {
      tpp->twi->EVENTS_STOPPED = 0;
      tpp->twi->TASKS_STOP = 1;
      rv = -1;
      break;
    }
    if (tpp->twi->EVENTS_RXDREADY) {
      *dp++ = tpp->twi->RXD;
      if ((dp+1) >= dpe) {
#if (ENABLE_PAN_36 - 0)
        tpp->ppi->CH[tpp->chidx].TEP = (uintptr_t)&tpp->twi->TASKS_STOP;
#else /* ENABLE_PAN_36 */
        tpp->twi->SHORTS = (TWI_SHORTS_BB_STOP_Enabled << TWI_SHORTS_BB_STOP_Pos);
#endif /* ENABLE_PAN_36 */
      }
      tpp->twi->EVENTS_RXDREADY = 0;
    }
#if (ENABLE_PAN_56 - 0)
    /* PAN-56: module lock-up
     *
     * Delay RESUME for a least two TWI clock periods after RXD read
     * to ensure clock-stretched ACK completes.  100 kHz = 20us, 400 kHz = 5us. */
    nrf_delay_us(20);
#endif /* ENABLE_PAN_56 */
    tpp->twi->TASKS_RESUME = 1;
  }
#if (ENABLE_PAN_36 - 0)
  tpp->ppi->CHENCLR = PPI_CHENCLR_CH0_Msk;
#else /* ENABLE_PAN_36 */
  tpp->twi->SHORTS = 0;
#endif /* ENABLE_PAN_36 */
  while (! tpp->twi->EVENTS_STOPPED) {
  }
  tpp->twi->EVENTS_STOPPED = 0;
  if (0 > rv) {
    rv = twi_error_clear(tpp);
  }
  return rv;
}

int twi_write (const twi_periphs_type * tpp,
               const uint8_t * sp,
               size_t len)
{
  const uint8_t * const spe = sp + len;
  int rv = len;

  tpp->twi->EVENTS_ERROR = 0;
  tpp->twi->EVENTS_TXDSENT = 0;
  tpp->twi->TXD = *sp;
  tpp->twi->TASKS_STARTTX = 1;

  PAN56_DECLARE_AND_INITIALIZE(tpp);
  while (true) {
    while (! (tpp->twi->EVENTS_TXDSENT
              || PAN56_CHECK_EXCEEDED()
              || tpp->twi->EVENTS_ERROR)) {
    }
    PAN56_MAYBE_RETURN_RESET(tpp);
    if (tpp->twi->EVENTS_ERROR) {
      rv = -1;
      break;
    }
    if (tpp->twi->EVENTS_TXDSENT) {
      tpp->twi->EVENTS_TXDSENT = 0;
      if (++sp < spe) {
        tpp->twi->TXD = *sp;
      } else {
        break;
      }
    }
  }
  tpp->twi->EVENTS_STOPPED = 0;
  tpp->twi->TASKS_STOP = 1;
  while (! tpp->twi->EVENTS_STOPPED) {
  }
  tpp->twi->EVENTS_STOPPED = 0;
  if (0 > rv) {
    rv = twi_error_clear(tpp);
  }
  return rv;
}

int sht21_crc (const uint8_t * data,
               int len)
{
  static const uint16_t SHT_CRC_POLY = 0x131;
  uint8_t crc = 0;

  while (0 < len--) {
    int bi;

    crc ^= *data++;
    for (bi = 8; bi > 0; --bi) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ SHT_CRC_POLY;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

int sht21_read_eic (const twi_periphs_type * tp,
                    uint8_t * eic)
{
  uint8_t data[16];
  uint8_t * dp;
  int rc;

  dp = data;
  *dp++ = 0xFA;
  *dp++ = 0x0F;
  rc = twi_write(tp, data, dp-data);
  if (0 <= rc) {
    rc = twi_read(tp, data, 8);
  }
  if (0 <= rc) {
    if ((0 == sht21_crc(data+0, 2))
        && (0 == sht21_crc(data+2, 2))
        && (0 == sht21_crc(data+4, 2))
        && (0 == sht21_crc(data+6, 2))) {
      eic[2] = data[0];
      eic[3] = data[2];
      eic[4] = data[4];
      eic[5] = data[6];
      dp = data;
      *dp++ = 0xFC;
      *dp++ = 0xC9;
      rc = twi_write(tp, data, dp-data);
    } else {
      rc = -1;
    }
  }
  if (0 <= rc) {
    rc = twi_read(tp, data, 6);
  }
  if (0 <= rc) {
    if ((0 == sht21_crc(data+0, 3))
        && (0 == sht21_crc(data+3, 3))) {
      eic[0] = data[3];
      eic[1] = data[4];
      eic[6] = data[0];
      eic[7] = data[1];
      rc = 8;
    } else {
      rc = -1;
    }
  }
  return rc;
}

typedef struct alarm_stage {
  uptime_alarm alarm;
  int (* callback) (struct alarm_stage * asp);
  const twi_periphs_type * tpp;
  uint8_t cmd;
} alarm_stage;

unsigned int wake_count;
uint64_t epoch;
uint64_t wake_total;
uint16_t t_raw;
uint16_t rh_raw;

/** Convert a raw humidity value to parts-per-thousand as an unsigned
 * int */
/* RH_pph = -6 + 125 * S / 2^16 */
#define BSP430_SENSORS_SHT21_HUMIDITY_RAW_TO_ppth(raw_) (unsigned int)(((1250UL * (raw_)) >> 16) - 60)

/** Convert a raw temperature value to centi-degrees Kelvin as an
 * unsigned int */
/* T_dC = -46.85 + 175.72 * S / 2^16
 * T_cK = 27315 - 4685 + 17572 * S / 2^16
 *      = 22630 + 17572 * S / 2^16
 */
#define BSP430_SENSORS_SHT21_TEMPERATURE_RAW_TO_cK(raw_) (22630U + (unsigned int)((17572UL * (raw_)) >> 16))

int show_results (alarm_stage * asp)
{
  uint64_t now = rtc_now();
  uint64_t elapsed = now - epoch;

  (void)asp;

  printf("%lu: %u wakeups ; uptime %lu ; awake %lu ; duty %lu [ppth]\n",
         (unsigned long)(now / UPTIME_Hz),
         wake_count,
         (unsigned long)elapsed,
         (unsigned long)wake_total,
         (unsigned long)((1000 * wake_total) / elapsed));
#if ! (DUTY_CYCLE_ONLY - 0)
  /* If all output is disabled the duty cycle is less than 0.2%, as
   * opposed to 1.5% with output enabled.  This is about 2 ms per
   * measurement. */
  unsigned int t_cK = BSP430_SENSORS_SHT21_TEMPERATURE_RAW_TO_cK(t_raw);
  int t_cCel = t_cK - 27315;
  int t_cFahr = 3200 + (9 * t_cCel) / 5;
  unsigned int rh_ppth = BSP430_SENSORS_SHT21_HUMIDITY_RAW_TO_ppth(rh_raw);
  printf("\tT: %u raw ; %u cK ; %d cCel ; %d c[Fahr]\n", t_raw, t_cK, t_cCel, t_cFahr);
  printf("\tRH: %u raw %u ppth\n", rh_raw, rh_ppth);
  t_raw = rh_raw = ~0;
#endif /* ! DUTY_CYCLE_ONLY */

#if ! (EXCLUDE_DIE_TEMP - 0)
  {
    NRF_TEMP->EVENTS_DATARDY = 0;
    NRF_TEMP->TASKS_START = 1;
    while (! NRF_TEMP->EVENTS_DATARDY) {
    }
    NRF_TEMP->EVENTS_DATARDY = 0;

    /* PAN-29: STOP task clears TEMP register */
    uint32_t dt_raw = NRF_TEMP->TEMP;

    /* PAN-30: TEMP module analog front end does not power down when
     * DATARDY event occurs */
    NRF_TEMP->TASKS_STOP = 1;

    /* PAN-28: Negative measured values are not represented correctly.
     * Sign extension does not go higher than bit 9.
     *
     * Value is 10-bit 2's complement.  Convert to 32-bit 2's
     * complement. */
    const uint32_t sign_bit = 0x0200;
    if (dt_raw & sign_bit) {
      dt_raw |= ~(sign_bit - 1);
    }

    int dt_cCel = 25 * (int)(int32_t)dt_raw;
    int dt_cFahr = (3200 + (9 * dt_cCel) / 5);
    printf("\tT[die]: %d cCel ; %d c[Fahr]\n", dt_cCel, dt_cFahr);
  }
#endif /* EXCLUDE_DIE_TEMP */

  return 0;
}

int send_read (alarm_stage * asp)
{
  uint8_t cmd = (asp->cmd
                 | SHT21_CMDBIT_BASE
                 | SHT21_CMDBIT_READ
                 | SHT21_CMDBIT_NOHOLD);
  return twi_write(asp->tpp, &cmd, sizeof(cmd));
}

int read_result (alarm_stage * asp)
{
  uint8_t data[3];
  int rc;

  rc = twi_read(asp->tpp, data, sizeof(data));
  if ((0 <= rc) && (0 == sht21_crc(data, rc))) {
    rc = ((data[0] << 8) | data[1]) & ~0x03;
    if (asp->cmd) {
      t_raw = rc;
      rc = send_read(asp);
    } else {
      rh_raw = rc;
      show_results(asp);
    }
  }
  return rc;
}

void main ()
{
  twi_periphs_type tp;
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("SHT21: Connect SDA to P0.%u, SCL to P0.%u\n",
         PIN_SDA, PIN_SCL);

  /* LFCLK starts as the RC oscillator.  Start the crystal . */
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (! NRF_CLOCK->EVENTS_LFCLKSTARTED) {
  }

  /* RTC only works on LFCLK.  Set one up using a zero prescaler (runs
   * at 32 KiHz).  Clock's 24 bits, so track overflow in a 32-bit
   * external counter, giving us 56 bits for the full clock (span 2^41
   * seconds, roughly 69 thousand years).
   *
   * Also enable a comparison that will fire an event at 1Hz.  We
   * don't use these events in PPI so don't enable event routing; we
   * do need the interrupts, though. */
  uptime_rtc->TASKS_STOP = 1;
  uptime_rtc->TASKS_CLEAR = 1;
  uptime_rtc->EVTENCLR = ~0;
  uptime_rtc->INTENCLR = ~0;
  uptime_rtc->CC[uptime_ccidx_1Hz] = UPTIME_Hz;
  uptime_rtc->INTENSET = ((RTC_INTENSET_OVRFLW_Enabled << RTC_INTENSET_OVRFLW_Pos)
                          | (RTC_INTENSET_COMPARE0_Enabled << (RTC_INTENSET_COMPARE0_Pos + uptime_ccidx_1Hz)));

  NVIC_ClearPendingIRQ(RTC0_IRQn);
  NVIC_EnableIRQ(RTC0_IRQn);

  uptime_rtc->TASKS_START = 1;

  /* PAN-31: Temperature offset value has to be manually loaded.
   * Address is 0x4000C504, the word before TEMP.  The struct does
   * not have a field for this. */
  ((__O uint32_t *)&NRF_TEMP->TEMP)[-1] = 0;

  do {
    alarm_stage alarm_stages[4];
    alarm_stage * asp;
    unsigned int remaining_delay;
    uint8_t buf[16];
    int rc;

    /* Now configure TWI (I2C) for a SHT21 */
    memset(&tp, 0, sizeof(tp));
#if (ENABLE_PAN_36 - 0)
    tp.ppi = NRF_PPI;
    tp.chidx = 0;
#endif /* ENABLE_PAN_36 */
#if (ENABLE_PAN_56 - 0)
    /* 500 is enough for a responsive I2C device at 400 kHz; 800 is
     * enough at 100 kHz.  Use smaller values to verify the reset
     * works. */
    tp.pan56_loop_limit = 800;
#endif /* ENABLE_PAN_56 */
    tp.twi = NRF_TWI0;

    twi_initialize(&tp, PIN_SCL, PIN_SDA, (TWI_FREQUENCY_FREQUENCY_K400 << TWI_FREQUENCY_FREQUENCY_Pos));

    tp.twi->ADDRESS = SHT21_ADDRESS;

    /* Read the EIC */
    rc = sht21_read_eic(&tp, buf);
    printf("EIC %d: ", rc);
    if (0 <= rc) {
      vBSPACMconsoleDisplayOctets(buf, rc);
    }
    putchar('\n');
    if (0 > rc) {
      rc = sht21_read_eic(&tp, buf);
      printf("Retry EIC %d: ", rc);
      if (0 <= rc) {
        vBSPACMconsoleDisplayOctets(buf, rc);
        putchar('\n');
      }
    }

    remaining_delay = UPTIME_Hz;

    memset(alarm_stages, 0, sizeof(alarm_stages));

    asp = alarm_stages;

    asp->callback = send_read;
    asp->tpp = &tp;
    asp->cmd = SHT21_CMDBIT_TEMP;
    /* High-resolution temperature read takes up to 85 ms.  Sleep for
     * 100 ms. */
    asp->alarm.delta_utt = UPTIME_Hz / 10;
    remaining_delay -= asp->alarm.delta_utt;
    asp->alarm.next = &asp[1].alarm;
    ++asp;

    asp->callback = read_result;
    asp->tpp = &tp;
    asp->cmd = SHT21_CMDBIT_RH;
    /* High-resolution humidity read takes up to 29 ms.  Sleep for 50
     * ms. */
    asp->alarm.delta_utt = UPTIME_Hz / 20;
    remaining_delay -= asp->alarm.delta_utt;
    asp->alarm.next = &asp[1].alarm;
    ++asp;

    asp->callback = read_result;
    asp->tpp = &tp;
    asp->alarm.delta_utt = remaining_delay;
    remaining_delay -= asp->alarm.delta_utt;
    asp->alarm.next = &alarm_stages[0].alarm;

    uint64_t last_wake = rtc_now();
    wake_count = 0;
    epoch = last_wake;
    uptime_set_alarms(asp->alarm.next, UPTIME_Hz);
    while (1) {
      const uptime_alarm * ap;
      while (! ((ap = uptime_get_alarm()))) {
        uint64_t last_sleep = rtc_now();
        wake_total += (last_sleep - last_wake);
        __WFE();
        last_wake = rtc_now();
        /* NB: We can get wakeups not only because the alarm is ready
         * but also because of UART interrupts as the transmit buffer
         * drains. */
        ++wake_count;
      }
      asp = (alarm_stage *)ap;
      asp->callback(asp);
    }
  } while (0);
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}