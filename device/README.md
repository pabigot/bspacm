BSPACM device directory
=======================

This directory contains (recursively) information on each device
supported in the baseline BSPACM infrastructure.

A *device* is a Cortex-M microcontroller.  The manufacturer's product
identifier, converted to lower case, should be used as the device
identifier.  For example, Texas Instruments produces the TM4C123GH6PM
and Silicon Labs produces the EFM32GG990F1024.

Each device belongs to a product *line*, which is a collection of
related microcontrollers.  Usually devices in a line share a Cortex-M
processor type, and may have other characteristics (like the set of
supported peripherals) that are worth encapsulating in a group.  For
example, the TM4C123GH6PM is a device in the TM4C123 line from Texas
Instruments, and the EFM32GG990F1024 is a device in the EFM32GG "Giant
Gecko" line from Silicon Labs.

Each device line falls within a product *series*, which is the top-level
aggregation of related devices.  Devices in a series share properties
that are not shared by the more refined line or device instances.  For
exampe, the T4MC123 and TM4C129 lines from Texas Instruments fall within
the TM4C Tiva&trade; C series, and the EFM32GG and EFM32TG lines from
Silicon Labs fall within the EFM32 series.

Each series is encapsulated in a subdirectory named by the series
identifier.  Within that subdirectory should be the `Makefile.series`
and subdirectories for each line in the series.

Each line should be encapsulated in a subdirectory named by the line
identifier, within the subdirectory of its containing series.  Within
that subdirectory should be the `Makefile.line` and subdirectories for
each device in the line.

Each device should be encapsulated in a subdirectory named by the device
identifier, within the subdirectory of its containing line.  Within that
subdirectory should be the `Makefile.device`.

The following files are expected to be found within this hierarchy:

Makefile.series
---------------

Set the series identifier in the `DEVICE_SERIES` make variable.  Add any
other information, such as the path to the manufacturer's driver
libraries, target-specific compiler flags, and the mechanism used to
program the device for manufacturer-provided evaluation boards.

This file must appear at the series level.

Makefile.line
-------------

Set the line identifier in the `DEVICE_LINE` make variable.  Include the
parent `Makefile.series`.

Where all devices in a line share a common configuration this should be
set in `Makefile.line` rather than repeated in each `Makefile.device`.
When all devices in the series share the configuration, it should be
placed in `Makefile.series` instead.

This file must appear at the line level.

Makefile.device
---------------

Set the device identifier in the `DEVICE` make variable.  Include the
parent `Makefile.line`.

This file must appear at the device level.

memory.ld
---------

A file for the GCC toolchain describing the memory regions, i.e. size of
flash and ram available on the device.  The file will be included into
the generic linker script for the processor.

This file generally appears at the device level.

include
-------

At each level in the hierarchy may appear an include directory, which
mirrors the top-level BSPACM include hierarchy but generally only
provides overriding files in the `bspacm/internal/device` portion of the
hierarchy.

Applications and libraries will be built using this hierarchy with the
precedence that board overrides device overrides line overrides series.

Somewhere within the include hierarchy overlays the following files
should be found:

### vector_table.inc

A file describing the entries in the interrupt vector table for the
device, in a form suitable for inclusion into linker scripts and/or
startup sources.  The file wraps the standard names, in order, with a
macro that is expanded in context to create a `_IRQHandler` definition
or reference.

This file may appear at the device, line, or series, or board levels
