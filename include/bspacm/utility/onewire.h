/* Copyright 2012-2015, Peter A. Bigot
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
 * @brief Basic support for 1-Wire(R) communications.
 *
 * This currently supports enough to use DS18B20 one-wire temperature
 * sensors with a dedicated bus and external or parasitic power.
 * Multiple devices on the bus have not been tested.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2012-2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_UTILITY_ONEWIRE_H
#define BSPACM_UTILITY_ONEWIRE_H

#include <bspacm/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Forward declaration.  Definition found in device-specific header
 * included below. */
struct sBSPACMonewireBus;

/** A generic handle to a bus configuration.
 *
 * The underlying structure is device-specific, as is the mechanism
 * used to initialize it.  Look for a routine named
 * hBSPACMonewireConfigureBus() in your device-specific headers. */
typedef const struct sBSPACMonewireBus * hBSPACMonewireBus;

/** Structure holding a 1-wire serial number. */
typedef struct sBSPACMonewireSerialNumber {
  /** The serial number in order MSB to LSB */
  uint8_t id[6];
} sBSPACMonewireSerialNumber;

/** Define protocol state times in microseconds.  User-level code
 * doesn't need this; device-specific implementation does. */
enum {
  /** Minimum time to hold bus low to ensure reset */
  BSPACM_ONEWIRE_T_RSTL_us = 480,

  /** Time to wait for presence detection after reset to quiesce */
  BSPACM_ONEWIRE_T_RSTH_us = 480,

  /** Delay before presence pulse is known to be valid (15us..60us) */
  BSPACM_ONEWIRE_T_PDHIGH_us = 60,

  /** Minimum time to hold bus low when writing a zero */
  BSPACM_ONEWIRE_T_LOW0_us = 60,

  /** Minimum time to hold bus low when writing a one */
  BSPACM_ONEWIRE_T_LOW1_us = 1,

  /** Recovery delay between write/read transaction cycles */
  BSPACM_ONEWIRE_T_REC_us = 1,

  /** Time to hold bus low when initiating a read slot */
  BSPACM_ONEWIRE_T_INT_us = 1,

  /** Point at which read value should be sampled */
  BSPACM_ONEWIRE_T_RDV_us = 15 - BSPACM_ONEWIRE_T_INT_us,

  /** Minimum duration of a read or write slot */
  BSPACM_ONEWIRE_T_SLOT_us = 60,
};

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

/** Reset the bus and check for device presence.
 *
 * This executes the 1-wire bus reset protocol, then detects whether a
 * 1-wire device is present.  The protocol requires that the bus be
 * held low for a given period, then returns it to pullup input.  A
 * device present on the bus will respond by pulling the bus low.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @return 0 if no device was detected, 1 if a device responded to the
 * reset.
 */
int iBSPACMonewireReset (hBSPACMonewireBus bus);

/** Configure the bus pin for low power mode.
 *
 * This reconfigures the port as output low.  Note that this affects
 * the MCU power, not the device power.  It does remove power from
 * parasitic devices, but externally powered devices will still draw
 * standby current.
 *
 * @param bus device-specific information identifying the 1-wire bus
 */
void vBSPACMonewireShutdown (hBSPACMonewireBus bus);

/** Control parasite power to the device.
 *
 * If parasitic power is being used (see
 * iBSPACMonewireReadPowerSupply()) user code must turn on parasite
 * within 10 us after initiating an operation that requires
 * significant power, hold it for the maximum time required by the
 * operation (750 ms for 12-bit temperature calculation), then turn it
 * off before reading the results of the operation.
 * #BSPACM_ONEWIRE_CMD_CONVERT_T used by
 * iBSPACMonewireRequestTemperature() is one such operation.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @param powered true to turn on parasite power; false to turn it off. */
void
vBSPACMonewireParasitePower (hBSPACMonewireBus bus,
                             bool powered);

/** Write a byte onto the 1-wire bus.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @param byte The value to be written.  The low 8 bits are
 * transmitted LSB-first. */
void vBSPACMonewireWriteByte (hBSPACMonewireBus bus,
                              uint8_t byte);

/** Read a bit from the 1-wire bus.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @return The value of the bit read (0 or 1). */
int iBSPACMonewireReadBit (hBSPACMonewireBus bus);

/** Read a byte from the 1-wire bus.
 *
 * Invokes iBSPACMonewireReadBit() eight times to read the data, least
 * significant bit first.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @return The value of the byte read (0 through 255). */
int iBSPACMonewireReadByte (hBSPACMonewireBus bus);

/** Calculate the CRC over the data.
 *
 * When the last byte of the data is the CRC of the preceding bytes,
 * the return value of this function should be zero.
 *
 * @param data data for which CRC is desired
 *
 * @param len the number of bytes that contribute to CRC calculation
 *
 * @return the calculated CRC value */
int iBSPACMonewireComputeCRC (const unsigned char * data, int len);

/** Read the serial number from a 1-wire device.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @param snp Pointer to where the serial number should be stored
 *
 * @return 0 if the serial number was successfully read; -1 if an
 * error occurred. */
int iBSPACMonewireReadSerialNumber (hBSPACMonewireBus bus,
                                    sBSPACMonewireSerialNumber * snp);

/** Determine whether device is externally powered or uses parasite power.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @return 1 if the bus is externally powered; 0 if the bus is
 * parasitically powered; a negative value if an error is encountered.
 */
int iBSPACMonewireReadPowerSupply (hBSPACMonewireBus bus);

/** Send the command sequence to initiate a temperature measurement
 *
 * Note that this simply requests that the device start to measure the
 * temperature.  The measurement process may take up to 750 msec at
 * the default 12-bit resolution.
 *
 * If the device is externally powered,
 * iBSPACMonewireTemperatureReady() can be invoked to determine
 * whether the requested measurement has completed.  If using
 * parasitic power, the application must use
 * iBSPACMonewireParasitePower() and should wait for at least the
 * maximum sample time before invoking
 * iBSPACMonewireReadTemperature().
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @return 0 if the request was successfully submitted, -1 if an error
 * occured. */
int iBSPACMonewireRequestTemperature (hBSPACMonewireBus bus);

/** Test whether the device has completed measuring the temperature
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @note Do not invoke this for parasitically-powered devices.  See
 * iBSPACMonewireRequestTemperature().
 *
 * @return 0 if the device is still busy; 1 if the sample is ready. */
static BSPACM_CORE_INLINE
int
iBSPACMonewireTemperatureReady (hBSPACMonewireBus bus)
{
  return iBSPACMonewireReadBit(bus);
}

/** Read the most recent temperature measurement
 *
 * The temperature is a signed 16-bit value representing 1/16th
 * degrees Celcius.  I.e., a value of 1 is 0.0625 Cel.
 *
 * @note The accuracy and precision of the measurement are much more
 * coarse than its resolution.
 *
 * @param bus device-specific information identifying the 1-wire bus
 *
 * @param temp_xCel Pointer to a location into which the temperature
 * will be written.  This must not be null.
 *
 * @return 0 if the read was successful; -1 if an error occurred.  If
 * there was an error, the value pointed to by temp_xCel remains
 * unchanged. */
int iBSPACMonewireReadTemperature (hBSPACMonewireBus bus,
                                   int16_t * temp_xCel);

/** Convert temperature from 1/16th Cel to tenths Celcius (dCel)
 *
 * For those of us who operate in lands under the domain of the Comité
 * Consultatif d'Unités.
 *
 * 9 * t / 8 == (9/5) * 10 * (t / 16) without incidental overflow
 */
#define BSPACM_ONEWIRE_xCel_TO_dCel(xcel_) ((10 * (xcel_)) / 16)

/** Convert temperature from 1/16th Cel to tenths Fahrenheit (d[degF])
 *
 * For those of us who live in the US and just can't get used to SI.
 *
 * 9 * t / 8 == (9/5) * 10 * (t / 16) without incidental overflow
 */
#define BSPACM_ONEWIRE_xCel_TO_ddegF(xcel_) (320 + ((9 * (xcel_)) / 8))

/** Convert temperature from 1/16th Cel to tenths Kelvin (dK)
 *
 * For those of us who follow the way of the <a
 * href="http://unitsofmeasure.org/ucum.html">Unified Code for Units
 * of Measure</a>
 *
 * 10 * (273.15 + x / 16)
 *  = (16*27315 + 100*x) / 160
 *  = (437040 + 100 * x) / 160
 *  = (21852 + 5 * x) / 8
 */
#define BSPACM_ONEWIRE_xCel_TO_dK(xcel_) ((unsigned int)(21852U + 5U * (xcel_)) / 8U)

/* Include the device-specific header which defines struct
 * sBSPACMonewireBus and provides hBSPACMonewireConfigureBus(). */
#include <bspacm/utility/onewire_.h>

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSPACM_UTILITY_ONEWIRE_H */
