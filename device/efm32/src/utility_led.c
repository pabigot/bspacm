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

/* Implementation for EFM32 device series LED interface.
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/utility/led.h>
#include <em_cmu.h>
#include <em_gpio.h>

xBSPACMled const xBSPACMleds[] = {
#define BSPACM_INC_EXPAND_LED_REFERENCE(port_,pin_) { GPIO->P + (port_), 1U << (pin_) },
#include <bspacm/internal/board/led.inc>
#undef BSPACM_INC_EXPAND_LED_REFERENCE
};
const uint8_t nBSPACMleds = sizeof(xBSPACMleds)/sizeof(*xBSPACMleds);

void
vBSPACMledConfigure ()
{
  /* Enable the GPIO peripherals required for LEDs. */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

#define BSPACM_INC_EXPAND_LED_REFERENCE(port_,pin_) do {           \
    GPIO_P_TypeDef * const port = &(GPIO->P[port_]);               \
    uint32_t const bit = (1U << (pin_));                           \
    port->DOUTCLR = bit;                                           \
    if ((pin_) < 8) {                                              \
      const unsigned int mode_shift = 0x1F & (4 * (pin_));         \
      port->MODEL =                                                \
        (port->MODEL & ~(_GPIO_P_MODEL_MODE0_MASK << mode_shift))  \
        | (GPIO_P_MODEL_MODE0_PUSHPULL << mode_shift);             \
    } else {                                                       \
      const unsigned int mode_shift = 0x1F & (4 * ((pin_) - 8U));  \
      port->MODEH =                                                \
        (port->MODEH & ~(_GPIO_P_MODEH_MODE8_MASK << mode_shift))  \
        | (GPIO_P_MODEH_MODE8_PUSHPULL << mode_shift);             \
    }                                                              \
  } while (0);
#include <bspacm/internal/board/led.inc>
#undef BSPACM_INC_EXPAND_LED_REFERENCE
}


