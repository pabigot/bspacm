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
#include <bspacm/utility/hires.h>
#include <bspacm/utility/uptime.h>
#include <bspacm/periph/dietemp.h>
#include <bspacm/periph/twi.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <string.h>
#include "nrf51_bitfields.h"

#ifndef SDA_PIN
#if (BSPACM_BOARD_NRF51_PCA10028 - 0)
#define SDA_PIN 7
#else /* PCA10028 */
#define SDA_PIN 25
#endif /* PCA10028 */
#endif /* SDA_PIN */
#ifndef SCL_PIN
#if (BSPACM_BOARD_NRF51_PCA10028 - 0)
#define SCL_PIN 30
#else /* PCA10028 */
#define SCL_PIN 24
#endif /* PCA10028 */
#endif /* SCL_PIN */

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

int sht21_read_eic (hBSPACMi2cBus tp,
                    uint8_t * eic)
{
  uint8_t data[16];
  uint8_t * dp;
  int rc;

  dp = data;
  *dp++ = 0xFA;
  *dp++ = 0x0F;
  rc = iBSPACMi2cWrite(tp, SHT21_ADDRESS, data, dp-data);
  if (0 <= rc) {
    rc = iBSPACMi2cRead(tp, SHT21_ADDRESS, data, 8);
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
      rc = iBSPACMi2cWrite(tp, SHT21_ADDRESS, data, dp-data);
    } else {
      rc = -1;
    }
  }
  if (0 <= rc) {
    rc = iBSPACMi2cRead(tp, SHT21_ADDRESS, data, 6);
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

/* What we're doing here is using the generic uptime infrastructure to
 * wake up once per second and start a sequence of timed I2C
 * operations: first intiate a temperature read; after 100 ms read the
 * temperature and initiate a humidity read; after 50 ms read the
 * humidity and display the results; then start again in 850 ms. */

typedef struct alarm_stage {
  /* The alarm that fired to initiate this stage */
  sBSPACMuptimeAlarm alarm;
  /* The function that implements this stage */
  int (* callback) (const struct alarm_stage * asp);
  /* The next stage to run */
  struct alarm_stage * next;
  /* I2C parameters required to execute the stage */
  hBSPACMi2cBus tpp;
  /* The raw uptime clock value when this stage fired */
  uint32_t last_fired;
  /* The delta from #last_fired to when the next stage should be
   * started */
  uint16_t delta_utt;
  /* An I2C command parameter used within the stage */
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

int show_results (const alarm_stage * asp)
{
  uint64_t now = ullBSPACMuptime();
  uint64_t elapsed = now - epoch;

  (void)asp;

  printf("%lu: %u wakeups ; uptime %lu ; awake %lu ; duty %lu [ppth]\n",
         (unsigned long)(now / BSPACM_UPTIME_Hz),
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
    int dt_cCel = iBSPACMdietemp_cCel();
    int dt_cFahr = (3200 + (9 * dt_cCel) / 5);
    printf("\tT[die]: %d cCel ; %d c[Fahr]\n", dt_cCel, dt_cFahr);
  }
#endif /* EXCLUDE_DIE_TEMP */

  return 0;
}

int send_read (const alarm_stage * asp)
{
  uint8_t cmd = (asp->cmd
                 | SHT21_CMDBIT_BASE
                 | SHT21_CMDBIT_READ
                 | SHT21_CMDBIT_NOHOLD);
  return iBSPACMi2cWrite(asp->tpp, SHT21_ADDRESS, &cmd, sizeof(cmd));
}

int read_result (const alarm_stage * asp)
{
  uint8_t data[3];
  int rc;

  rc = iBSPACMi2cRead(asp->tpp, SHT21_ADDRESS, data, sizeof(data));
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

static const alarm_stage * volatile pending_stage_;
void stage_flih (int ccidx,
                 hBSPACMuptimeAlarm ap)
{
  alarm_stage * asp = (alarm_stage *)ap;
  asp->last_fired = BSPACM_UPTIME_RTC->CC[ccidx];
  pending_stage_ = asp;
}

/* Return a pointer to the next stage to execute, or a null pointer if
 * no stage has become due since the last call to this function. */
const alarm_stage * get_pending_stage ()
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  const alarm_stage * rv;

  BSPACM_CORE_DISABLE_INTERRUPT();
  do {
    rv = pending_stage_;
    pending_stage_ = NULL;
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

void main ()
{
  vBSPACMledConfigure();
  vBSPACMuptimeStart();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("SHT21: Connect SDA to P0.%u, SCL to P0.%u\n",
         SDA_PIN, SCL_PIN);

  do {
    sBSPACMi2cBus tp;
    hBSPACMi2cBus tpp;
    alarm_stage alarm_stages[4];
    alarm_stage * asp;
    unsigned int remaining_delay;
    uint8_t buf[16];
    int rc;

    /* High-resolution clock for microsecond-resolution sleeps. */
    if (0 != iBSPACMhiresInitialize(1000U * 1000U)) {
      printf("ERR: Failed to initialize high-resolution timer\n");
      break;
    }
    (void)iBSPACMhiresSetEnabled(true);

#if ! (EXCLUDE_DIE_TEMP - 0)
    /* Die temperature depends on HFCLK for accuracy. */
    {
      bool dtok = bBSPACMdietempInitialize();
      printf("Die temperature enabled: expect %s results\n", dtok ? "good" : "BAD");
    }
#endif /* EXCLUDE_DIE_TEMP */

    /* Now configure TWI (I2C) for a SHT21 */
    tpp = hBSPACMi2cConfigureBus(&tp, NRF_TWI0, SDA_PIN, SCL_PIN,
#if (BSPACM_NRF_APPLY_PAN_36 - 0)
                                 0
#else /* BSPACM_NRF_APPLY_PAN_36 */
                                 -1
#endif /* BSPACM_NRF_APPLY_PAN_36 */
                                 , (TWI_FREQUENCY_FREQUENCY_K400 << TWI_FREQUENCY_FREQUENCY_Pos)
                                 , BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us );
    printf("I2C handle %p\n", tpp);
    if (! tpp) {
      printf("ERR: Failed to configure I2C bus characteristics\n");
      break;
    }
    iBSPACMi2cSetEnabled(&tp, true);

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

    remaining_delay = BSPACM_UPTIME_Hz;

    memset(alarm_stages, 0, sizeof(alarm_stages));

    asp = alarm_stages;

    asp->alarm.callback_flih = stage_flih;
    asp->callback = send_read;
    asp->tpp = &tp;
    asp->cmd = SHT21_CMDBIT_TEMP;
    /* High-resolution temperature read takes up to 85 ms.  Sleep for
     * 100 ms. */
    asp->delta_utt = BSPACM_UPTIME_Hz / 10;
    remaining_delay -= asp->delta_utt;
    asp->next = asp+1;
    ++asp;

    asp->alarm.callback_flih = stage_flih;
    asp->callback = read_result;
    asp->tpp = &tp;
    asp->cmd = SHT21_CMDBIT_RH;
    /* High-resolution humidity read takes up to 29 ms.  Sleep for 50
     * ms. */
    asp->delta_utt = BSPACM_UPTIME_Hz / 20;
    remaining_delay -= asp->delta_utt;
    asp->next = asp+1;
    ++asp;

    asp->alarm.callback_flih = stage_flih;
    asp->callback = read_result;
    asp->tpp = &tp;
    asp->delta_utt = remaining_delay;
    asp->next = alarm_stages;

    uint64_t last_wake = ullBSPACMuptime();
    wake_count = 0;
    epoch = last_wake;
    rc = iBSPACMuptimeAlarmSet(1, BSPACM_UPTIME_Hz, &alarm_stages[0].alarm);
    while (1) {
      const alarm_stage * asp;
      while (! ((asp = get_pending_stage()))) {
        uint64_t last_sleep = ullBSPACMuptime();
        wake_total += (last_sleep - last_wake);
        __WFE();
        last_wake = ullBSPACMuptime();
        /* NB: We can get wakeups not only because the alarm is ready
         * but also because of UART interrupts as the transmit buffer
         * drains. */
        ++wake_count;
      }
      asp->callback(asp);
      if (asp->next) {
        rc = iBSPACMuptimeAlarmSet(1, asp->last_fired + asp->delta_utt, &asp->next->alarm);
      }
    }
  } while (0);
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
