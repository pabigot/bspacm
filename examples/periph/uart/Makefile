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

UART=default
RXBLEN=4
TXBLEN=16

# Provide additional flags for a non-default UART.  E.g. UART=LEUART0.
ifneq (default,$(UART))
AUX_CPPFLAGS+=-DAPP_ALTERNATIVE_UART
AUX_CPPFLAGS+=-DBSPACM_CONFIG_ENABLE_$(UART)=1 -DUART_HANDLE='&xBSPACMdevice$(DEVICE_SERIES_UC)periph$(UART)'
AUX_CPPFLAGS+=-DBSPACM_PERIPH_$(UART)_RX_BUFFER_SIZE=$(RXBLEN) -DBSPACM_PERIPH_$(UART)_TX_BUFFER_SIZE=$(TXBLEN)
endif

include $(BSPACM_ROOT)/make/Makefile.common
