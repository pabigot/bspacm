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

/* Implementation for EFM32 device series pinmux functions
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/periph/gpio.h>

void vBSPACMdeviceEFM32pinmuxConfigure (const sBSPACMdeviceEFM32pinmux * cfgp,
                                        int enablep,
                                        int initial_high)
{
  GPIO_P_TypeDef * const port = cfgp->port;
  const uint32_t bit = (1U << cfgp->pin);
  unsigned int mode = cfgp->mode;
  volatile uint32_t * doutp = 0;

  if (! port) {
    return;
  }
  if (0 <= initial_high) {
    doutp = initial_high ? &port->DOUTSET : &port->DOUTCLR;
  }
  if (! enablep) {
    mode = 0;
  }

  if (doutp && mode) {
    *doutp = bit;
  }
  if (8 > cfgp->pin) {
    const unsigned int shift = 0x1F & (4 * cfgp->pin);
    port->MODEL = (port->MODEL & ~(0x0F << shift)) | (mode << shift);
  } else {
    const unsigned int shift = 0x1F & (4 * (cfgp->pin - 8));
    port->MODEH = (port->MODEH & ~(~0x0F << shift)) | (mode << shift);
  }
  if (doutp && ! mode) {
    *doutp = bit;
  }
}
