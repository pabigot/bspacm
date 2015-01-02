/* BSPACM - nRF51 OneWire interface
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

#include <bspacm/utility/onewire.h>
#include <bspacm/utility/hires.h>
#include "nrf_gpio.h"
#include <string.h>

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

int
iBSPACMonewireReset (hBSPACMonewireBus bus)
{
  int present;

  /* Set bus high so device can detect start of reset. */
  NRF_GPIO->OUTSET = bus->dq_bit;
  NRF_GPIO->PIN_CNF[bus->dq_pin] = ONEWIRE_CNF_ACTIVE;

  if (bus->pwr_bit) {
    /* In some parasitic power situations involving repeated
     * operations it may be necessary to assert DQ long enough that
     * C_PP gets a little charge and the device detects the falling
     * edge that starts the reset, ensuring that the effective total
     * time in RSTL does not exceed 960 us which could induce a
     * power-on reset.  The hold period probably doesn't need to be as
     * long as RSTH, but that value was confirmed to work when this
     * particular problem was observed using a different MCU. */
    vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_RSTH_us);
  }

  /* Hold bus low for T_RESET us */
  NRF_GPIO->OUTCLR = bus->dq_bit;
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_RSTL_us);

  /* Release bus and switch to input until presence pulse should be
   * visible. */
  NRF_GPIO->DIRCLR = bus->dq_bit;
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_PDHIGH_us);

  /* Record presence if bus is low (DS182x is holding it there) */
  present = !(NRF_GPIO->IN & bus->dq_bit);

  /* Wait for reset cycle to complete */
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_RSTH_us - BSPACM_ONEWIRE_T_PDHIGH_us);

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
      vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_LOW1_us);
      NRF_GPIO->DIRCLR = bus->dq_bit;
      vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_SLOT_us - BSPACM_ONEWIRE_T_LOW1_us + BSPACM_ONEWIRE_T_REC_us);
    } else {
      vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_LOW0_us);
      NRF_GPIO->DIRCLR = bus->dq_bit;
      vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_SLOT_us - BSPACM_ONEWIRE_T_LOW0_us + BSPACM_ONEWIRE_T_REC_us);
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
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_INT_us);
  NRF_GPIO->DIRCLR = bus->dq_bit;
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_RDV_us);
  rv = !!(NRF_GPIO->IN & bus->dq_bit);
  vBSPACMhiresSleep_us(BSPACM_ONEWIRE_T_SLOT_us - BSPACM_ONEWIRE_T_RDV_us - BSPACM_ONEWIRE_T_INT_us + BSPACM_ONEWIRE_T_REC_us);
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
                               int16_t * temp_xCel)
{
  int16_t t;

  if (! iBSPACMonewireReset(bus)) {
    return -1;
  }
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_SKIP_ROM);
  vBSPACMonewireWriteByte(bus, BSPACM_ONEWIRE_CMD_READ_SCRATCHPAD);
  t = iBSPACMonewireReadByte(bus);
  t |= (iBSPACMonewireReadByte(bus) << 8);
  *temp_xCel = t;
  return 0;
}
