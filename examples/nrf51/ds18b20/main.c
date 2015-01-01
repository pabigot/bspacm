/* BSPACM - nRF51 DS18B20 OneWire interface
 *
 * Copyright 2012-2015, Peter A. Bigot
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

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/utility/misc.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <stdio.h>
#include <fcntl.h>

/** Structure holding a 1-wire serial number. */
typedef struct sBSPACMonewireSerialNumber {
  /** The serial number in order MSB to LSB */
  uint8_t id[6];
} sBSPACMonewireSerialNumber;

enum {
  /** Read 64-bit ROM code without using search procedure */
  BSPACM_ONEWIRE_CMD_READ_ROM = 0x33,

  /** Skip ROM sends the following command to all bus devices */
  BSPACM_ONEWIRE_CMD_SKIP_ROM = 0xcc,

  /** Determine whether device is parasite-powered or
   * external-powered */
  BSPACM_ONEWIRE_CMD_READ_POWER_SUPPLY = 0xb4,

  /** Store data from EEPROM into RAM */
  BSPACM_ONEWIRE_CMD_RECALL_EE = 0xb8,

  /** Read the RAM area. */
  BSPACM_ONEWIRE_CMD_READ_SCRATCHPAD = 0xbe,

  /** Initiate a temperature conversion.
   *
   * Be aware that temperature conversion can take up to 750ms at the
   * default 12-bit resolution.
   *
   * For an externally (non-parasite) powered sensor, the caller may
   * use #iBSPACMonewireReadBit_ni to determine whether the conversion
   * has completed.  Completion is indicated when the device responds
   * with a 1. */
  BSPACM_ONEWIRE_CMD_CONVERT_T = 0x44,
};

/** Define protocol state times in microseconds.
 *
 * @note Since all these times are far less than any sane watchdog
 * interval, and the timing can be important, BSPACM_CORE_DELAY_CYCLES
 * is not used in this module. */
enum {
  /** Minimum time to hold bus low to ensure reset */
  OWT_RSTL_us = 480,

  /** Time to wait for presence detection after reset to quiesce */
  OWT_RSTH_us = 480,

  /** Delay before presence pulse is known to be valid (15us..60us) */
  OWT_PDHIGH_us = 60,

  /** Minimum time to hold bus low when writing a zero */
  OWT_LOW0_us = 60,

  /** Minimum time to hold bus low when writing a one */
  OWT_LOW1_us = 1,

  /** Recovery delay between write/read transaction cycles */
  OWT_REC_us = 1,

  /** Time to hold bus low when initiating a read slot */
  OWT_INT_us = 1,

  /** Point at which read value should be sampled */
  OWT_RDV_us = 15 - OWT_INT_us,

  /** Minimum duration of a read or write slot */
  OWT_SLOT_us = 60,
};

volatile bool cc0_timeout;

void TIMER0_IRQHandler ()
{
  if (NRF_TIMER0->EVENTS_COMPARE[0]) {
    cc0_timeout = true;
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  }
}

void
delay_us (uint32_t count_us)
{

  /* Don't muck with timers if the duration is so short we might miss
   * the compare value wrap. */
  if (2 >= count_us) {
    nrf_delay_us(count_us);
    return;
  }
  cc0_timeout = false;
  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  NRF_TIMER0->CC[0] += count_us;
  while (! cc0_timeout) {
    __WFE();
  }
  NRF_TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk;
  return;
}

#ifndef ONEWIRE_PIN
#define ONEWIRE_PIN 29
#endif /* ONEWIRE_PIN */
#define ONEWIRE_BIT (1ULL << ONEWIRE_PIN)

inline void
dirset ()
{
  NRF_GPIO->DIRSET = ONEWIRE_BIT;
}

inline void
dirclr ()
{
  NRF_GPIO->PIN_CNF[ONEWIRE_PIN] = 0
    | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
    | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
    | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
    | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
}

int
iBSPACMonewireReset_ni ()
{
  int present;

  /* Non-standard: Hold bus high for OWT_RESET_us.  This provides
   * enough parasitic power for the device to signal presence.
   * Without this, effective RSTL duration may exceed the maximum
   * 960us before device reset. */
  dirset();
  NRF_GPIO->OUTSET = ONEWIRE_BIT;
  delay_us(OWT_RSTH_us);

  /* Hold bus low for OWT_RESET_us */
  NRF_GPIO->OUTCLR = ONEWIRE_BIT;
  delay_us(OWT_RSTL_us);

  /* Release bus and switch to input until presence pulse should be
   * visible. */
  dirclr();
  delay_us(OWT_PDHIGH_us);

  /* Record presence if bus is low (DS182x is holding it there) */
  present = !(NRF_GPIO->IN & ONEWIRE_BIT);

  /* Wait for reset cycle to complete */
  delay_us(OWT_RSTH_us - OWT_PDHIGH_us);

  return present;
}

void
vBSPACMonewireShutdown_ni ()
{
  NRF_GPIO->OUTCLR = ONEWIRE_BIT;
  dirclr();
}

void
vBSPACMonewireWriteByte_ni (int byte)
{
  int bp;

  for (bp = 0; bp < 8; ++bp) {
    NRF_GPIO->OUTCLR = ONEWIRE_BIT;
    dirset();
    if (byte & 0x01) {
      delay_us(OWT_LOW1_us);
      dirclr();
      delay_us(OWT_SLOT_us - OWT_LOW1_us + OWT_REC_us);
    } else {
      delay_us(OWT_LOW0_us);
      dirclr();
      delay_us(OWT_SLOT_us - OWT_LOW0_us + OWT_REC_us);
    }
    byte >>= 1;
  }
}

int
iBSPACMonewireReadBit_ni ()
{
  int rv;

  NRF_GPIO->OUTCLR = ONEWIRE_BIT;
  dirset();
  delay_us(OWT_INT_us);
  dirclr();
  delay_us(OWT_RDV_us);
  vBSPACMledSet(0, 1);
  rv = !!(NRF_GPIO->IN & ONEWIRE_BIT);
  vBSPACMledSet(0, 0);
  delay_us(OWT_SLOT_us - OWT_RDV_us - OWT_INT_us + OWT_REC_us);
  return rv;
}

int
iBSPACMonewireReadByte_ni ()
{
  int byte = 0;
  int bit = 1;

  do {
    if (iBSPACMonewireReadBit_ni()) {
      byte |= bit;
    }
    bit <<= 1;
  } while (0x100 != bit);
  return byte;
}

int
iBSPACMonewireComputeCRC (const unsigned char * romp,
                          int len)
{
  static const unsigned char OW_CRC_POLY = 0x8c;
  unsigned char crc = 0;

  while (0 < len--) {
    int bi;

    crc ^= *romp++;
    for (bi = 0; bi < 8; ++bi) {
      if (crc & 1) {
        crc = (crc >> 1) ^ OW_CRC_POLY;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

int
iBSPACMonewireReadSerialNumber (sBSPACMonewireSerialNumber * snp)
{
  uint8_t rom[8];
  int i;
  int rv = -1;

  do {
    if (! iBSPACMonewireReset_ni()) {
      break;
    }
    vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_READ_ROM);
    for (i = 0; i < sizeof(rom); ++i) {
      rom[i] = iBSPACMonewireReadByte_ni();
    }
    rv = 0;
  } while (0);

  if (0 == rv) {
    if (0 != iBSPACMonewireComputeCRC(rom, sizeof(rom))) {
      rv = -1;
    } else {
      for (i = 0; i < sizeof(snp->id); ++i) {
        snp->id[i] = rom[sizeof(rom) - 2 - i];
      }
    }
  }
  return rv;
}

int
iBSPACMonewireRequestTemperature_ni ()
{
  if (! iBSPACMonewireReset_ni()) {
    return -1;
  }
  vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_SKIP_ROM);
  vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_CONVERT_T);
  return 0;
}

int
iBSPACMonewireReadPowerSupply ()
{
  int rv = -1;

  if (iBSPACMonewireReset_ni()) {
    vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_SKIP_ROM);
    vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_READ_POWER_SUPPLY);
    rv = iBSPACMonewireReadBit_ni();
  }
  return rv;
}

int
iBSPACMonewireReadTemperature_ni (int * temp_xCel)
{
  int t;

  if (! iBSPACMonewireReset_ni()) {
    return -1;
  }
  vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_SKIP_ROM);
  vBSPACMonewireWriteByte_ni(BSPACM_ONEWIRE_CMD_READ_SCRATCHPAD);
  t = iBSPACMonewireReadByte_ni();
  t |= (iBSPACMonewireReadByte_ni() << 8);
  *temp_xCel = t;
  return 0;
}

void main ()
{
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("Initial stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* HFCLK starts as the RC oscillator.  Start the crystal for HFCLK,
   * and start one for LFCLK. */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (! (NRF_CLOCK->EVENTS_HFCLKSTARTED && NRF_CLOCK->EVENTS_LFCLKSTARTED)) {
  }

  printf("Post start stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* Timers only work on HFCLK.  Set up to clock at 1 MHz, using a
   * 32-bit timer so it's easier to do incremental changes to the
     compare registers. */
  NRF_TIMER0->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  NRF_TIMER0->PRESCALER = 4;    /* /16 */
  NRF_TIMER0->BITMODE = (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos);

  /* Enable interrupts (thus event wakeup?) at the peripheral, but not
   * at the NVIC */
  NRF_TIMER0->INTENCLR = ~0;
  NRF_TIMER0->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);

  /* Clear the counter and start things going */
  NRF_TIMER0->TASKS_CLEAR = 1;
  NRF_TIMER0->TASKS_START = 1;

  dirclr();

  do {
    if (! iBSPACMonewireReset_ni()) {
      printf("ERR: No DS18B20 present on P0.%u\n", ONEWIRE_PIN);
      break;
    }

    static const char * const supply_type[] = { "parasitic", "external" };

    int external_power = iBSPACMonewireReadPowerSupply();
    printf("Power supply: %s\n", supply_type[external_power]);
    if (0 > external_power) {
      printf("ERROR: Device not present?\n");
      break;
    }

    sBSPACMonewireSerialNumber serial;
    int rc = iBSPACMonewireReadSerialNumber(&serial);
    printf("Serial got %d: ", rc);
    vBSPACMconsoleDisplayOctets(serial.id, sizeof(serial.id));
    putchar('\n');

    while (0 == iBSPACMonewireRequestTemperature_ni()) {
      int t_raw;
      delay_us(600000UL);
      while (! iBSPACMonewireReadBit_ni()) {
      }
      rc = iBSPACMonewireReadTemperature_ni(&t_raw);
      int t_dCel = (10 * t_raw) / 16;
      int t_dFahr = 320 + (9 * t_dCel) / 5;
      printf("Got %d dCel, %d d[Fahr]\n", t_dCel, t_dFahr);
    }


  } while (0);

  dirset();
  NRF_GPIO->OUTCLR = ONEWIRE_BIT;

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
