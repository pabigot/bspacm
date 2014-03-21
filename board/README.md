BSPACM board directory
======================

This directory contains information on each board supported in the
baseline BSPACM infrastructure.

A *board* is a hardware solution comprising a microcontroller, leds,
external peripherals, headers, etc.  The manufacturer's preferred name,
in lower case, should be used as the board identifier.

Each board is encapsulated in a subdirectory named after the board.
Within that directory should be the following files:

Makefile.board
--------------

Set the board identifier in the `BOARD` make variable.  Then include the
appropriate MCU Makefile.device from the device hierarchy.

Any other board-specific make variables may also be set.  Generally
these should begin with the prefix `BOARD_`.  For example, `BOARD_STYLE`
is used in EFM32 boards to distinguish DK (development kit) from STK
(starter kit).

Readme.md
---------

Contain salient information about the board: a link to manufacturer's
home page, basic characteristics like flash/RAM sizes, pin assignments
for LEDs, buttons, and console UART, and whatever else might be useful.
Don't repeat information that can easily be found elsewhere; don't make
the reader go hunting for the most basic information.

include
-------

Each board should provide an include directory, which mirrors the
top-level BSPACM include hierarchy but generally only provides
overriding files in the `bspacm/internal/board` portion of the
hierarchy.

Applications and libraries will be built using this hierarchy with the
precedence that board overrides device overrides line overrides series.
