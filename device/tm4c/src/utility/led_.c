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

/* Implementation for TM4C device series LED interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/utility/led.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom.h>

xBSPACMled const xBSPACMleds[] = {
#define BSPACM_INC_EXPAND_LED_CONFIGURE(periph_,port_,bits_)
#define BSPACM_INC_EXPAND_LED_REFERENCE(port_,pin_) &(BSPACM_CORE_BITBAND_PERIPH(port_->DATA, pin_)),
#include <bspacm/internal/board/led.inc>
#undef BSPACM_INC_EXPAND_LED_REFERENCE
#undef BSPACM_INC_EXPAND_LED_CONFIGURE
};
const uint8_t nBSPACMleds = sizeof(xBSPACMleds)/sizeof(*xBSPACMleds);

void
vBSPACMledConfigure ()
{
  /* Enable the GPIO peripherals required for LEDs.  NOTE: We assume
   * that the GPIO is in its power-up configuration, that you didn't
   * pick a commit-controlled register as an LED, that 2mA drive is
   * sufficient, and that LEDs are active-high. */
#define BSPACM_INC_EXPAND_LED_CONFIGURE(periph_,port_,bits_) do { \
    GPIOCommon_Type * const port = port_;                               \
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_##periph_);          \
    port->DIR |= (bits_);                                         \
    port->PDR |= (bits_);                                         \
    port->DEN |= (bits_);                                         \
  } while (0);
#define BSPACM_INC_EXPAND_LED_REFERENCE(port_,pin_)
#include <bspacm/internal/board/led.inc>
#undef BSPACM_INC_EXPAND_LED_REFERENCE
#undef BSPACM_INC_EXPAND_LED_CONFIGURE
}
