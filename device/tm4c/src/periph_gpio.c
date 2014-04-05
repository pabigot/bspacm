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

/* Implementation for TM4C device series pinmux functions
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/gpio.h>
#include <bspacm/device/periphs.h>
#include <inc/hw_gpio.h>

void vBSPACMdeviceTM4CpinmuxConfigure (const sBSPACMdeviceTM4Cpinmux * cfgp,
                                       int enablep,
                                       int initial_high)
{
  GPIOCommon_Type * const gpio = (GPIOCommon_Type *)cfgp->port;
  const unsigned int bit = 1U << cfgp->pin;
  const unsigned int pctl_shift = 4 * cfgp->pin;
  int with_lock;

  if (! gpio) {
    return;
  }
  /* Verify the port index matches the port.  Lacking an assert
   * mechanism, spin here under the expectation that the bug will be
   * discovered during testing. */
  while (wBSPACMdeviceTM4CperiphGPIO[cfgp->port_shift] != (uint32_t)gpio) {
    /* port != port_shift */;
  }
  with_lock = !(bit & gpio->CR);
  gpio->PCTL &= ~(0x0F << pctl_shift);
  if (with_lock) {
    gpio->LOCK = GPIO_LOCK_KEY;
    *(volatile uint32_t*)&gpio->CR |= bit;
  }
  if (enablep && cfgp->pctl) {
    gpio->PCTL |= (cfgp->pctl << pctl_shift);
    gpio->AFSEL |= bit;
    gpio->ODR &= ~bit;
  } else {
    gpio->DIR &= ~bit;
    gpio->AFSEL &= ~bit;
    gpio->ODR &= ~bit;
  }
  gpio->DEN |= bit;
  if (with_lock) {
    gpio->LOCK = GPIO_LOCK_KEY;
    *(volatile uint32_t*)&gpio->CR &= bit;
  }
}