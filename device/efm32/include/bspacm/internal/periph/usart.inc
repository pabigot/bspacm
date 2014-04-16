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
 * @brief Include file to define USART instances for EFM32 devices
 *
 * In your @c periph_config.c file you should define parameter macros
 * described below, and include this file, once for each USART device
 * you need in your application.  Make sure the @c periph_config.c
 * file also defines a pin mux assignment for the peripherals you will
 * used.
 *
 * Define:
 *
 * @li @c BSPACM_INC_PERIPHNUM to the instance number that should be
 * adjoined to the peripheral name (e.g. 0 for USART0, 1 for USART1).
 *
 * @li @c BSPACM_INC_TX_BUFFER_SIZE (optional) to the desired size for
 * the internal transmit fifo
 *
 * @li @c BSPACM_INC_RX_BUFFER_SIZE (optional) to the desired size for
 * the internal receive fifo
 *
 * This will add the following symbols (where # denotes the peripheral number):
 *
 * @li @c USART#_RX_IRQHandler and @c USART#_TX_IRQHandler which get
 * installed in the interrupt vector
 *
 * @li @c xBSPACMdeviceEFM32periphUSART# which may be used as a handle
 * to operations described in <bspacm/periph/uart.h>.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/internal/utility/cppinc.h>

/* @cond DOXYGEN_EXCLUDE */

#define BSPACM_INC_TX_ALLOCATION BSPACM_INC_CATENATE(tx_allocation_USART,BSPACM_INC_PERIPHNUM)
#define BSPACM_INC_RX_ALLOCATION BSPACM_INC_CATENATE(rx_allocation_USART,BSPACM_INC_PERIPHNUM)

#if (BSPACM_INC_TX_BUFFER_SIZE - 0)
FIFO_DEFINE_ALLOCATION(BSPACM_INC_TX_ALLOCATION, BSPACM_INC_TX_BUFFER_SIZE);
#endif /* BSPACM_INC_TX_BUFFER_SIZE */

#if (BSPACM_INC_RX_BUFFER_SIZE - 0)
FIFO_DEFINE_ALLOCATION(BSPACM_INC_RX_ALLOCATION, BSPACM_INC_RX_BUFFER_SIZE);
#endif /* BSPACM_INC_RX_BUFFER_SIZE */

sBSPACMperiphUARTstate BSPACM_INC_CATENATE(xBSPACMdeviceEFM32periphUSART,BSPACM_INC_PERIPHNUM) = {
  .uart = BSPACM_INC_CATENATE(USART,BSPACM_INC_PERIPHNUM),
  .devcfg = { .ptr = &BSPACM_INC_CATENATE3(xBSPACMdeviceEFM32periphUSART,BSPACM_INC_PERIPHNUM,devcfg) },
  .ops = &xBSPACMdeviceEFM32periphUSARToperations,
#if (BSPACM_INC_TX_BUFFER_SIZE - 0)
  .tx_fifo_ni_ = FIFO_FROM_ALLOCATION(BSPACM_INC_TX_ALLOCATION),
#endif /* BSPACM_INC_TX_BUFFER_SIZE */
#if (BSPACM_INC_RX_BUFFER_SIZE - 0)
  .rx_fifo_ni_ = FIFO_FROM_ALLOCATION(BSPACM_INC_RX_ALLOCATION),
#endif /* BSPACM_INC_RX_BUFFER_SIZE */
};

void BSPACM_INC_CATENATE3(USART,BSPACM_INC_PERIPHNUM,_RX_IRQHandler)(void)
{
  vBSPACMdeviceEFM32periphUSARTrxirqhandler(&BSPACM_INC_CATENATE(xBSPACMdeviceEFM32periphUSART,BSPACM_INC_PERIPHNUM));
}

void BSPACM_INC_CATENATE3(USART,BSPACM_INC_PERIPHNUM,_TX_IRQHandler)(void)
{
  vBSPACMdeviceEFM32periphUSARTtxirqhandler(&BSPACM_INC_CATENATE(xBSPACMdeviceEFM32periphUSART,BSPACM_INC_PERIPHNUM));
}

#undef BSPACM_INC_RX_ALLOCATION
#undef BSPACM_INC_TX_ALLOCATION

/* @endcond */

/*
 * Local Variables:
 * mode:c
 * End:
 */
