# BSPACM - Makefile for bootstrap/stdio application
#
# Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software. If not, see
# <http://creativecommons.org/publicdomain/zero/1.0/>.
#

SRC=main.c

# Transmit buffer length
TXBLEN=32
# Receive buffer length
RXBLEN=8

# Provide larger default UART buffer sizes
AUX_CPPFLAGS=-DBSPACM_CONFIG_DEFAULT_UART_TX_BUFFER_SIZE=$(TXBLEN)
AUX_CPPFLAGS+=-DBSPACM_CONFIG_DEFAULT_UART_RX_BUFFER_SIZE=$(RXBLEN)
AUX_CPPFLAGS+=-DBSPACM_CONFIG_ENABLE_UART=1

# Use BSPACM file descriptor support to provide stdio
WITH_FDOPS=1

include $(BSPACM_ROOT)/make/Makefile.common
