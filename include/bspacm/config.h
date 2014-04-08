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
 *
 * @brief Application/board-specific header
 *
 * This file is a wrapper that provides fallback definitions for the
 * <bspacm/appconf.h> must exist somewhere in the application, board,
 * or device include hieararchies.  The purpose of these two files is
 * to control the features of the application-specific @c
 * periph_config.c file, which provides infrastructure for generic
 * access to resources such as UARTs and interrupt demultiplexers.
 *
 * Nothing in this file is permitted to affect the compilation of
 * material that is not application-specific.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_CONFIG_H
#define BSPACM_CONFIG_H

#ifndef BSPACM_CONFIG_ENABLE_UART
/** Define to a true value to create the data structures and functions
 * that support UART operations on the target board or for the current
 * application.
 *
 * @cppflag
 * @defaulted */
#define BSPACM_CONFIG_ENABLE_UART 0
#endif /* BSPACM_CONFIG_ENABLE_UART */

/** The size of the transmit buffer for the default UART device.
 *
 * @defaulted
 * @dependency #BSPACM_CONFIG_ENABLE_UART
 */
#ifndef BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE
#define BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE 32
#endif /* BSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE */

/** The size of the receive buffer for the default UART device.
 *
 * @defaulted
 * @dependency #BSPACM_CONFIG_ENABLE_UART
 */
#ifndef BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE
#define BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE 8
#endif /* BSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE */

/* Include the application-specific configuration data.  */
#include <bspacm/appconf.h>

#endif /* BSPACM_CONFIG_H */
