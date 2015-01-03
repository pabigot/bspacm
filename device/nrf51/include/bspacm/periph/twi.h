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
 * @brief nRF51-specific support for I2C
 *
 * Nordic calls this "TWI", but it's I2C so that's what the API uses.
 *
 * @note The implementation depends on <bspacm/utility/hires.h> and
 * <bspacm/utility/uptime.h>
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2015, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_TWI_H
#define BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_TWI_H

#include <bspacm/core.h>

/* Certain negative error codes convey information.  For those cases,
 * the absolute value is taken from the ERRORSRC register augmented by
 * several flags for other error conditions. */

/** Bit set in absolute value of an error return code when a bus
 * timeout was discovered.  If #BSPACM_NRF_APPLY_PAN_56 is enabled the
 * TWI peripheral will have been power cycled to ensure recovery. */
#define BSPACM_NRF_TWI_BUS_ERROR_TIMEOUT 0x100

/** Bit set in absolute value of an error return code when the I2C bus
 * is being held by an unresponsive follower device and the master
 * failed to clear the hold. */
#define BSPACM_NRF_TWI_BUS_ERROR_CLEAR_FAILED 0x200

/** Bit set in absolute value of an error return code when the I2C
 * peripheral indicated an error, but the error cause could not be
 * identified. */
#define BSPACM_NRF_TWI_BUS_ERROR_UNKNOWN 0x400

#ifndef BSPACM_NRF_APPLY_PAN_36
/** PAN #36: Shortcuts are not functional
 * Use PPI channel to enforce BB to SUSPEND and STOP tasks.
 *
 * Define to C preprocessor true value to enable workaround.  This
 * impacts the accepted parameters of hBSPACMi2cConfigureBus().
 *
 * @cppflag */
#define BSPACM_NRF_APPLY_PAN_36 1
#endif /* BSPACM_NRF_APPLY_PAN_36 */

#ifndef BSPACM_NRF_APPLY_PAN_56
/** PAN #56: module lock-up
 *
 * Various conditions require a full power-cycle of the TWI module to
 * restore functionality.  The likelihood of occurrence can be
 * lessened by certain behaviors, but in the end we need to alarm if
 * an expected RXDREADY or TXDREADY signal is not received in a timely
 * manner.
 *
 * If enabled by defining to a C preprocessor true value the
 * recovery from I2C bus timeouts will include power-cycling the TWI
 * peripheral.
 *
 * @cppflag
 */
#define BSPACM_NRF_APPLY_PAN_56 1
#endif /* BSPACM_NRF_APPLY_PAN_56 */

/** The minimum acceptable per-byte timeout for I2C bus transactions,
 * in microseconds. */
#define BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us 100

typedef struct sBSPACMi2cBus {
  /** The NRF TWI peripheral used for this configuration. */
  NRF_TWI_Type * twi;

  /** The maximum time a read or write operation will wait for
   * per-byte completion before determining that the bus has timed
   * out.
   *
   * In addition to supporting PAN #56 recovery, this also sets a
   * timeout for other conditions where a follower device might hold
   * the bus longer than expected.  The delay must be long enough to
   * accommodate any clock stretching done by the follower.
   *
   * The specification doesn't define a maximum delay.  Note that at
   * 100 kHz transfer of one byte will take 80 us; at 400 kHz it'll
   * take 20 us.  This does not account for peripheral transfer delays
   * or waiting for bus control or clock stretching.
   *
   * The selected delay is specified in microseconds throug
   * hBSPACMi2cConfigureBus() but the stored value is rounded up to
   * the smallest integer clock count supported by the
   * bBSPACMuptimeSleep() infrastructure that is at least as large as
   * the requested delay or #BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us,
   * whichever is larger.
   *
   * In any case where a timeout is detected the TWI peripheral will
   * be power-cycled in accordance with the PAN #56 workaround.
   */
  unsigned int timeout_utt;

  /** The frequency configuration of the bus, as a value suitable for
   * storage in NRF_TWI_Type::FREQUENCY. */
  uint32_t frequency;

  /** The pin used for the SDA function */
  uint8_t sda_pin;

  /** The pin used for the SCL function */
  uint8_t scl_pin;

#if (BSPACM_NRF_APPLY_PAN_36 - 0)
  /** This field identifies the PPI channel that's used for the
   * workaround. */
  int8_t ppi_chidx;
#endif /* BSPACM_NRF_APPLY_PAN_36 */
} sBSPACMi2cBus;

/** A handle for a bus configuration */
typedef const sBSPACMi2cBus * hBSPACMi2cBus;

/** Configure the I2C bus structure.
 *
 * This validates and records all non-address bus configuration data.
 * It does not configure or enable the bus itself.
 *
 * @param tpp pointer to the bus structure
 *
 * @param twi the NRF TWI peripheral used by the bus; generally
 * #NRF_TWI0 or #NRF_TWI1.
 *
 * @param sda_pin the pin number to used for the I2C SDA
 * functionality.
 *
 * @param scl_pin the pin number to used for the I2C SCL
 * functionality.
 *
 * @param ppi_chidx The Programmable Peripheral Interconnect channel
 * used to support the workaround for PAN #36.  If
 * #BSPACM_NRF_APPLY_PAN_36 is enabled this must be a PPI channel
 * (0..15).  If #BSPACM_NRF_APPLY_PAN_36 is disabled this must be -1.
 *
 * @param frequency the bus frequency, expressed as the value of the
 * NRF_TWI_Type::FREQUENCY register.
 * E.g. #TWI_FREQUENCY_FREQUENCY_K400.
 *
 * @param timeout_us The maximum time the bus will wait for #TXDREADY
 * or #RXDREADY from a follower device.  This relates to
 * #BSPACM_NRF_APPLY_PAN_56 but has the specified effect even if that
 * PAN is not enabled.  The timeout is reset on each successful byte
 * transfer, and is implemented as a busy-wait.  Remember to account
 * for clock stretching.  Values less than
 * #BSPACM_I2C_MINIMUM_BUS_TIMEOUT_us are adjusted to meet that
 * minimum.
 *
 * @return @p tpp as handle if the configuration was acceptable, or
 * NULL if a parameter was rejected.
 *
 * @depends #BSPACM_NRF_APPLY_PAN_36 */
hBSPACMi2cBus
hBSPACMi2cConfigureBus (sBSPACMi2cBus * tpp,
                        NRF_TWI_Type * twi,
                        int sda_pin,
                        int scl_pin,
                        int ppi_chidx,
                        uint32_t frequency,
                        unsigned int timeout_us);

/** Enable or disable the TWI bus configuration.
 *
 * @return Zero if enable (or disable) was successful, otherwise a
 * negative error code.  Errors can occur when failing to gain control
 * of a newly enabled bus. */
int
iBSPACMi2cSetEnabled (hBSPACMi2cBus tpp,
                      bool enabled);

/** Read data from the I2C bus.
 *
 * @param tpp the bus to use
 *
 * @param addr the address of the device from which data will be read.
 * This is the 7-bit address, exclusive of the RW bit.
 *
 * @param dp where the requested data should be stored
 *
 * @param len the number of bytes to be read
 *
 * @return @p len on success, otherwise a negative error code.
 */
int
iBSPACMi2cRead (hBSPACMi2cBus tpp,
                unsigned int addr,
                uint8_t * dp,
                size_t len);

/** Write data to the I2C bus.
 *
 * @param tpp the bus to use
 *
 * @param addr the address of the device to which data will be written.
 * This is the 7-bit address, exclusive of the RW bit.
 *
 * @param sp pointer to the data to be written
 *
 * @param len the number of bytes to write
 *
 * @return @p len on success, otherwise a negative error code.
 */
int
iBSPACMi2cWrite (hBSPACMi2cBus tpp,
                 unsigned int addr,
                 const uint8_t * sp,
                 size_t len);

#endif /* BSPACM_DEVICE_NRF51_INTERNAL_PERIPH_TWI_H */
