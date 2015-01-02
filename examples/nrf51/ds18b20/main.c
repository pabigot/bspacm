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
#include <string.h>
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
   * use #iBSPACMonewireReadBit to determine whether the conversion
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

typedef struct sBSPACMonewireBus {
  int8_t dq_pin;
  int8_t pwr_pin;
  uint32_t dq_bit;
  uint32_t pwr_bit;
} sBSPACMonewireBus;

typedef const struct sBSPACMonewireBus * hBSPACMonewireBus;

hBSPACMonewireBus
hBSPACMonewireConfigureBus (sBSPACMonewireBus * bp,
                            int dq_pin,
                            int pwr_pin)
{
  memset(bp, 0, sizeof(*bp));
  if ((0 > dq_pin) || (31 < dq_pin)) {
    return NULL;
  }
  bp->dq_pin = dq_pin;
  bp->dq_bit = 1UL << dq_pin;
  bp->pwr_pin = -1;
  if ((0 <= pwr_pin) && (pwr_pin <= 31)) {
    bp->pwr_pin = pwr_pin;
    bp->pwr_bit = 1UL << pwr_pin;
  } else if (0 < pwr_pin) {
    return NULL;
  }

  return bp;
}

volatile bool cc0_timeout;

/* Configure to output high, so the device can detect the pull low
 * for reset.  We'll toggle direction with DIR and output with OUT
 * as necessary; the main thing this does is ensure the input buffer
 * is connected.  (The pull-up is probably unnecessary assuming you
 * wired a pull-up resistor to the data line as the datasheet
 * instructs.) */
#define ONEWIRE_CNF_ACTIVE ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
                           | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) \
                           | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) \
                           | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) \
                           | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) \
                           )

/* Configure to reset state: input, but buffer disconnected */
#define ONEWIRE_CNF_INACTIVE ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
                             | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) \
                             | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) \
                             | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) \
                             | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) \
                             )

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
  /* The compare initialization sequence takes about 27 (16 MHz) ticks
   * using an undivided clock, so if the desired delay is less than 3
   * us we could miss the compare.  Use the busy-wait delay if a
   * conservative check for duration fails. */
  const uint32_t min_wfe_delay_us = 4;

  if (min_wfe_delay_us > count_us) {
    nrf_delay_us(count_us);
    return;
  }
  cc0_timeout = false;
  NRF_TIMER0->TASKS_CAPTURE[0] = 1;
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  NRF_TIMER0->CC[0] += count_us << 4;
  while (! cc0_timeout) {
    __WFE();
  }
  NRF_TIMER0->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk;
  return;
}

#ifndef ONEWIRE_DQ_PIN
#define ONEWIRE_DQ_PIN 29
#endif /* ONEWIRE_DQ_PIN */
#ifndef ONEWIRE_PWR_PIN
#define ONEWIRE_PWR_PIN 0
#endif /* ONEWIRE_PWR_PIN */

int
iBSPACMonewireReset (hBSPACMonewireBus bus)
{
  int present;

  /* Set bus high so device can detect start of reset. */
  NRF_GPIO->OUTSET = bus->dq_bit;
  NRF_GPIO->PIN_CNF[bus->dq_pin] = ONEWIRE_CNF_ACTIVE;

  if (bus->pwr_bit) {
    /* In some situations it may be necessary to charge C_PP so the
     * device detects the falling edge that starts the reset.  The
     * hold period probably doesn't need to be RSTH, but that value
     * was confirmed to work when this particular problem was observed
     * using a different MCU. */
    delay_us(OWT_RSTH_us);
  }

  /* Hold bus low for OWT_RESET_us */
  NRF_GPIO->OUTCLR = bus->dq_bit;
  delay_us(OWT_RSTL_us);

  /* Release bus and switch to input until presence pulse should be
   * visible. */
  NRF_GPIO->DIRCLR = bus->dq_bit;
  delay_us(OWT_PDHIGH_us);

  /* Record presence if bus is low (DS182x is holding it there) */
  present = !(NRF_GPIO->IN & bus->dq_bit);

  /* Wait for reset cycle to complete */
  delay_us(OWT_RSTH_us - OWT_PDHIGH_us);

  return present;
}

void
vBSPACMonewireParasitePower (hBSPACMonewireBus bus,
                             bool powered)
{
  if (bus->pwr_bit) {
    if (powered) {
      NRF_GPIO->OUTSET = bus->pwr_bit;
      NRF_GPIO->PIN_CNF[bus->pwr_pin] = ONEWIRE_CNF_ACTIVE;
    } else {
      NRF_GPIO->OUTCLR = bus->pwr_bit;
      NRF_GPIO->PIN_CNF[bus->pwr_pin] = ONEWIRE_CNF_INACTIVE;
    }
  }
}

void
vBSPACMonewireShutdown (hBSPACMonewireBus bus)
{
  /* Configure to power-up defaults */
  NRF_GPIO->OUTCLR = bus->dq_bit;
  NRF_GPIO->PIN_CNF[bus->dq_pin] = ONEWIRE_CNF_INACTIVE;
  if (bus->pwr_bit) {
    NRF_GPIO->OUTCLR = bus->pwr_bit;
    NRF_GPIO->PIN_CNF[bus->pwr_pin] = ONEWIRE_CNF_INACTIVE;
  }
}

void
vBSPACMonewireWriteByte (hBSPACMonewireBus bus,
                         uint8_t byte)
{
  int bp;

  for (bp = 0; bp < 8; ++bp) {
    NRF_GPIO->OUTCLR = bus->dq_bit;
    NRF_GPIO->DIRSET = bus->dq_bit;
    if (byte & 0x01) {
      delay_us(OWT_LOW1_us);
      NRF_GPIO->DIRCLR = bus->dq_bit;
      delay_us(OWT_SLOT_us - OWT_LOW1_us + OWT_REC_us);
    } else {
      delay_us(OWT_LOW0_us);
      NRF_GPIO->DIRCLR = bus->dq_bit;
      delay_us(OWT_SLOT_us - OWT_LOW0_us + OWT_REC_us);
    }
    byte >>= 1;
  }
}

int
iBSPACMonewireReadBit (hBSPACMonewireBus bus)
{
  int rv;

  NRF_GPIO->OUTCLR = bus->dq_bit;
  NRF_GPIO->DIRSET = bus->dq_bit;
  delay_us(OWT_INT_us);
  NRF_GPIO->DIRCLR = bus->dq_bit;
  delay_us(OWT_RDV_us);
  vBSPACMledSet(0, 1);
  rv = !!(NRF_GPIO->IN & bus->dq_bit);
  vBSPACMledSet(0, 0);
  delay_us(OWT_SLOT_us - OWT_RDV_us - OWT_INT_us + OWT_REC_us);
  return rv;
}

int
iBSPACMonewireReadByte (hBSPACMonewireBus bus)
{
  int byte = 0;
  int bit = 1;

  do {
    if (iBSPACMonewireReadBit(bus)) {
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
iBSPACMonewireReadSerialNumber (hBSPACMonewireBus bus,
                                sBSPACMonewireSerialNumber * snp)
{
  uint8_t rom[8];
  int i;
  int rv = -1;

  do {
    if (! iBSPACMonewireReset(bus)) {
      break;
    }
    vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_READ_ROM);
    for (i = 0; i < sizeof(rom); ++i) {
      rom[i] = iBSPACMonewireReadByte(bus);
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
iBSPACMonewireRequestTemperature (hBSPACMonewireBus bus)
{
  if (! iBSPACMonewireReset(bus)) {
    return -1;
  }
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_SKIP_ROM);
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_CONVERT_T);
  return 0;
}

int
iBSPACMonewireReadPowerSupply (hBSPACMonewireBus bus)
{
  int rv = -1;

  if (iBSPACMonewireReset(bus)) {
    vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_SKIP_ROM);
    vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_READ_POWER_SUPPLY);
    rv = iBSPACMonewireReadBit(bus);
  }
  return rv;
}

int
iBSPACMonewireReadTemperature (hBSPACMonewireBus bus,
                               int * temp_xCel)
{
  int t;

  if (! iBSPACMonewireReset(bus)) {
    return -1;
  }
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_SKIP_ROM);
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_READ_SCRATCHPAD);
  t = iBSPACMonewireReadByte(bus);
  t |= (iBSPACMonewireReadByte(bus) << 8);
  if (0 > t) {
    return -1;
  }
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

  /* Timers only work on HFCLK.  Set up to clock at full speed 16 MHz,
   * using a 32-bit timer so we can handle delays that are much longer
   * than we really need for this application. */
  NRF_TIMER0->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  NRF_TIMER0->PRESCALER = 0;
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

  do {
    sBSPACMonewireBus bus_config;
    hBSPACMonewireBus bus = hBSPACMonewireConfigureBus(&bus_config, ONEWIRE_DQ_PIN, ONEWIRE_PWR_PIN);

    if (! iBSPACMonewireReset(bus)) {
      printf("ERR: No DS18B20 present on P0.%u\n", ONEWIRE_DQ_PIN);
      break;
    }

    static const char * const supply_type[] = { "parasitic", "external" };

    int external_power = iBSPACMonewireReadPowerSupply(bus);
    printf("Power supply: %s\n", supply_type[external_power]);
    if (0 > external_power) {
      printf("ERROR: Device not present?\n");
      break;
    }

    sBSPACMonewireSerialNumber serial;
    int rc = iBSPACMonewireReadSerialNumber(bus, &serial);
    printf("Serial got %d: ", rc);
    vBSPACMconsoleDisplayOctets(serial.id, sizeof(serial.id));
    putchar('\n');

    while (0 == iBSPACMonewireRequestTemperature(bus)) {
      if (external_power) {
        /* Wait for read to complete.  Conversion time can be as long as
         * 750 ms if 12-bit resolution is used (this resolution is the
         * default). Timing will be wrong unless interrupts are enabled
         * so uptime overflow events can be handled.  Sleep for 600ms,
         * then test at 10ms intervals until the result is ready. */
        delay_us(600 * 1000UL);
        while (! iBSPACMonewireReadBit(bus)) {
          delay_us(10 * 1000UL);
        }
      } else {
        /* Output high on the parasitic power boost line for 750ms, to
         * power the conversion.  Then switch that signal back to
         * input so the data can flow over the same circuit. */
        vBSPACMonewireParasitePower(bus, true);
        delay_us(750 * 1000UL);
        vBSPACMonewireParasitePower(bus, false);
      }
      int t_raw = -1;
      rc = iBSPACMonewireReadTemperature(bus, &t_raw);
      vBSPACMonewireShutdown(bus);
      int t_dCel = (10 * t_raw) / 16;
      int t_dFahr = 320 + (9 * t_dCel) / 5;
      printf("Got %d dCel, %d d[Fahr]\n", t_dCel, t_dFahr);
      delay_us(1000 * 1000);
    }

  } while (0);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
