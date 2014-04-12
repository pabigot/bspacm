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

AUX_CPPFLAGS+=-DBSPACM_CONFIG_ENABLE_UART=1

# Example for testing non-default UARTs
#AUX_CPPFLAGS+=-DBSPACM_CONFIG_ENABLE_LEUART0=1 -DUART_HANDLE='&xBSPACMdeviceEFM32periphLEUART0'
#AUX_CPPFLAGS+=-DBSPACM_PERIPH_LEUART0_RX_BUFFER_SIZE=2 -DBSPACM_PERIPH_LEUART0_TX_BUFFER_SIZE=5

include $(BSPACM_ROOT)/make/Makefile.common
