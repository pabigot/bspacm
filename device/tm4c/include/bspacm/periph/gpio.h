/* Copyright 2014, Peter A. Bigot
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
 * @brief Support for peripheral pin multiplexing configuration
 *
 * TM4C peripherals are mapped to specific GPIO pins; the specific
 * peripheral function supported by a GPIO pin is determined by the
 * value written to the corresponding GPIOPCTL port control register.
 *
 * The mapping between pin and peripheral function is constrained by
 * the specific device.  In some cases the same function can be mapped
 * to multiple pins; in that situation, the desired mapping may be
 * specific to the board or even to the application.
 *
 * This file declares arrays that provide pin mapping definitions for
 * all peripherals that are used by the application.  These mappings
 * are used by the generic TM4C peripheral configuration routines.
 * The application, board, or device should provide definitions for
 * the structure corresponding to each required peripheral in a source
 * file named @c periph_config.c with an absolute path set in the make
 * variable @c PERIPH_CONFIG_SRC.  By default the board-specific @c
 * periph_config.c is used.
 *
 * @section device_tm4c_gpio_howto Configuring GPIO Pins on TM4C Microcontrollers
 *
 * This material adapted from my <a
 * href="http://forum.stellarisiti.com/topic/1890-the-datasheet-book-club/?p=6412">Stellarisiti
 * forum post on TM4C123 10.3 Initialization and Configuration
 * steps</a>, repeated here as an easier-to-find reference:
 *
 * @li (1) Enable the module port, if not done already. Wait 3 cycles after enabling.
 * @code
 * BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(GPIO_PORT)) = 1;
 * __NOP(); __NOP(); __NOP(); // delay 3 cycles
 * @endcode
 * @li (1.5) Unlock any bits in CR if necessary (n1)
 * @li (2) Configure DIR if necessary (n2)
 * @code
 * BSPACM_CORE_BITBAND_PERIPH(GPIO_PORT->DIR, GPIO_PIN) = 1;
 * @endcode
 * @li (3) Configure AFSEL
 * @li (4) Set one of DR2R, DR4R, DR8R (n3)
 * @li (5a) Set PUR or PDR, or clear both (n4)
 * @li (5b) Configure ODR (n4)
 * @li (5c) Configure SLR if using 8mA drive strength
 * @li (6) Configure DEN and (if necessary) AMSEL
 * @li (7) Configure interrupts (see details in 10.3 step 7)
 * @li (8) Optionally restore the lock bit. Note that the description
 * in 10.3 step 8 is misleading: this is not a bit in GPIOLOCK, it is
 * a bit in GPIOCR which requires a set of the GPIOLOCK key before
 * being modified.
 *
 * Notes:
 * @li (n1) Bits that are JTAG/SWD pins or NMI pins; see 10.2.4 and
 * the GPIOLOCK and GPIOCR register descriptions. In library code you
 * can simply check the CR bit: if it's clear, modifications require
 * the unlock process.
 *
 * @li (n2) For some peripheral functions (e.g. UART) this bit is
 * don't-care. See Table 10-3 "GPIO Pad Configuration Examples".
 *
 * @li (n3) It's unclear to me whether it is meaningful to set a zero
 * mA drive strength by clearing a bit in all three registers,
 * although this can be done.
 *
 * @li (n4) Table 10-3 suggests that ODR and PUR/PDR should be
 * configured in certain cases regardless of direction, and should not
 * be considered a "pick one of the three options" situation as
 * implied by the description in section 10.3 step 5.
 *
 * TM4C129 devices have extended drive that supports 6mA, 10mA, and
 * 12mA drive strengths, but the DR2R DR4R and DR8R registers are
 * documented to behave the same as the TM4C123 ones.  The techniques
 * required to select these strengths are not addressed here; see a
 * TM4C129x datasheet for details.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_DEVICE_TM4C_PERIPH_GPIO_H
#define BSPACM_DEVICE_TM4C_PERIPH_GPIO_H

/* Standard port tag/shift map:
 *
 * PA 0    PB 1    PC 2    PD 3
 * PE 4    PF 5    PG 6    PH 7
 * *PJ 8   PK 9    PL 10   PM 11
 * PN 12   *PP 13  PQ 14   PR 15
 * PS 16   PT 17
 */

#include <bspacm/core.h>

/** Core structure associating a GPIO port, pin, and port control
 * function.
 *
 * @note An all-zero pinmux denotes an unconfigured GPIO due to #pctl
 * being zero. */
typedef struct sBSPACMdeviceTM4Cpinmux {
  /** The register pointer for the port to be configured.  The
   * specific device is GPIOA_Type or GPIOA_AHB_Type, depending on
   * device line, though the two types are structurally equivalent. */
  GPIOCommon_Type * port;

  /** The pin number, ranging from 0 through 7. */
  uint8_t pin;

  /** The port mux control nybble, with 0 denoting the GPIO function
   * and higher values indicating a specific peripheral function.  A
   * valid value is in the range 1 through 15, and must be shifted
   * left by 4*#pin in order to place it in the correct location in
   * the corresponding GPIOPCTL register.
   *
   * @note A zero value for #pctl indicates that the pinmux is not
   * valid and the corresponding GPIO should not be configured for its
   * alternate function.  This is used in cases where a particular
   * peripheral supports multiple functions, and a specific function
   * is not needed/supported by the mapping (e.g. the RTS/CTS signals
   * on a UART). */
  uint8_t pctl;

  /** The port IRQ number (starting from 0 = Reset_Handler).  This
   * field is likely only set with #pctl is zero.  */
  uint8_t irqn;
} sBSPACMdeviceTM4Cpinmux;

/** Set the AFSEL bit and other bits required to configure a pin for a
 * peripheral function according to @p *cfgp, or when disabled to be
 * treated as a GPIO input.
 *
 * Where @link sBSPACMdeviceTM4Cpinmux::pctl cfgp->pctl@endlink is not
 * zero (pin provides a peripheral function): Disabling the device
 * disables the pin entirely (clears the DEN bit).
 *
 * Where @p cfg->pctl is zero (pin provides GPIO): Enabling configures
 * the pin for output; disabling configures the pin for input.  This
 * API is inadequate for full control as lacks the ability to set GPIO
 * output initial state, or GPIO input pullup/pulldown state.
 *
 * @note Pins that happen to be commit-control locked will be unlocked
 * to perform this reconfiguration then re-locked.
 *
 * @warning Pins with special considerations may, when disabled, lose
 * their special function: this routine does not return them to their
 * power-up configuration.
 *
 * @param cfgp pointer to the configuration structure.  If @link
 * sBSPACMdeviceTM4Cpinmux::port cfgp->port@endlink is null, no
 * configuration will be performed.
 *
 * @param enablep nonzero if the pin is to be configured based on
 * @link sBSPACMdeviceTM4Cpinmux::pctl cfgp->pctl@endlink (it will be
 * a GPIO output if @p cfgp->pctl is zero).  If @p enablep is zero the
 * pin will be disabled (or configured as a GPIO input if @p
 * cfgp->pctl is zero).
 */
void vBSPACMdeviceTM4CpinmuxConfigure (const sBSPACMdeviceTM4Cpinmux * cfgp,
                                       int enablep);

/** Convert a port instance index (shift) to the letter tag for the port.
 *
 * This adjustment accounts for GPIOI and GPIOO being skipped in the
 * letter identification, with the corresponding shifts being applied
 * to the next supported port.
 *
 * @param shift the bit index for the port
 *
 * @return an upper-case letter corresponding to the port with 'I' and
 * 'O' being skipped. */
static BSPACM_CORE_INLINE
int
iBSPACMdeviceTM4CgpioPortTagFromShift (unsigned int shift)
{
  int tag = 'A' + shift;
  if (('O' - 2) < tag) {
    tag += 2;
  } else if (('I' - 1) < tag) {
    tag += 1;
  }
  return tag;
}

/** Determine the bit position for a specific port within a register.
 *
 * This maps, for example, @c GPIOK to 9 because bit 9 of a register
 * like @ RCGCGPIO corresponds to @c GPIOK.
 *
 * @see iBSPACMdeviceTM4CgpioPortTagFromShift
 *
 * @param gpio a reference to the port instance; generically a pointer
 * with the bit value of the port base address.  You can pass @c GPIOA
 * or @GPIOA_AHB as appropriate.
 *
 * @return a non-negative shift value, or -1 if @p gpio does not
 * appear to be a valid port address. */
static BSPACM_CORE_INLINE
int
iBSPACMdeviceTM4CgpioPortShift (void * gpio)
{
#if defined(GPIOA_BASE)
  if (gpio == (void*)GPIOA_BASE) return 0;
#endif
#if defined(GPIOA_AHB_BASE)
  if (gpio == (void*)GPIOA_AHB_BASE) return 0;
#endif
#if defined(GPIOB_BASE)
  if (gpio == (void*)GPIOB_BASE) return 1;
#endif
#if defined(GPIOB_AHB_BASE)
  if (gpio == (void*)GPIOB_AHB_BASE) return 1;
#endif
#if defined(GPIOC_BASE)
  if (gpio == (void*)GPIOC_BASE) return 2;
#endif
#if defined(GPIOC_AHB_BASE)
  if (gpio == (void*)GPIOC_AHB_BASE) return 2;
#endif
#if defined(GPIOD_BASE)
  if (gpio == (void*)GPIOD_BASE) return 3;
#endif
#if defined(GPIOD_AHB_BASE)
  if (gpio == (void*)GPIOD_AHB_BASE) return 3;
#endif
#if defined(GPIOE_BASE)
  if (gpio == (void*)GPIOE_BASE) return 4;
#endif
#if defined(GPIOE_AHB_BASE)
  if (gpio == (void*)GPIOE_AHB_BASE) return 4;
#endif
#if defined(GPIOF_BASE)
  if (gpio == (void*)GPIOF_BASE) return 5;
#endif
#if defined(GPIOF_AHB_BASE)
  if (gpio == (void*)GPIOF_AHB_BASE) return 5;
#endif
#if defined(GPIOG_BASE)
  if (gpio == (void*)GPIOG_BASE) return 6;
#endif
#if defined(GPIOG_AHB_BASE)
  if (gpio == (void*)GPIOG_AHB_BASE) return 6;
#endif
#if defined(GPIOH_BASE)
  if (gpio == (void*)GPIOH_BASE) return 7;
#endif
#if defined(GPIOH_AHB_BASE)
  if (gpio == (void*)GPIOH_AHB_BASE) return 7;
#endif
  /* No GPIOI */
#if defined(GPIOJ_BASE)
  if (gpio == (void*)GPIOJ_BASE) return 8;
#endif
#if defined(GPIOJ_AHB_BASE)
  if (gpio == (void*)GPIOJ_AHB_BASE) return 8;
#endif
#if defined(GPIOK_BASE)
  if (gpio == (void*)GPIOK_BASE) return 9;
#endif
#if defined(GPIOK_AHB_BASE)
  if (gpio == (void*)GPIOK_AHB_BASE) return 9;
#endif
#if defined(GPIOL_BASE)
  if (gpio == (void*)GPIOL_BASE) return 10;
#endif
#if defined(GPIOL_AHB_BASE)
  if (gpio == (void*)GPIOL_AHB_BASE) return 10;
#endif
#if defined(GPIOM_BASE)
  if (gpio == (void*)GPIOM_BASE) return 11;
#endif
#if defined(GPIOM_AHB_BASE)
  if (gpio == (void*)GPIOM_AHB_BASE) return 11;
#endif
#if defined(GPION_BASE)
  if (gpio == (void*)GPION_BASE) return 12;
#endif
#if defined(GPION_AHB_BASE)
  if (gpio == (void*)GPION_AHB_BASE) return 12;
#endif
  /* No GPIOO */
#if defined(GPIOP_BASE)
  if (gpio == (void*)GPIOP_BASE) return 13;
#endif
#if defined(GPIOP_AHB_BASE)
  if (gpio == (void*)GPIOP_AHB_BASE) return 13;
#endif
#if defined(GPIOQ_BASE)
  if (gpio == (void*)GPIOQ_BASE) return 14;
#endif
#if defined(GPIOQ_AHB_BASE)
  if (gpio == (void*)GPIOQ_AHB_BASE) return 14;
#endif
#if defined(GPIOR_BASE)
  if (gpio == (void*)GPIOR_BASE) return 15;
#endif
#if defined(GPIOR_AHB_BASE)
  if (gpio == (void*)GPIOR_AHB_BASE) return 15;
#endif
#if defined(GPIOS_BASE)
  if (gpio == (void*)GPIOS_BASE) return 16;
#endif
#if defined(GPIOS_AHB_BASE)
  if (gpio == (void*)GPIOS_AHB_BASE) return 16;
#endif
#if defined(GPIOT_BASE)
  if (gpio == (void*)GPIOT_BASE) return 17;
#endif
#if defined(GPIOT_AHB_BASE)
  if (gpio == (void*)GPIOT_AHB_BASE) return 17;
#endif
  return -1;
}

#endif /* BSPACM_DEVICE_TM4C_PERIPH_GPIO_H */
