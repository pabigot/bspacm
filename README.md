Release: 20140408

BSPACM is a build environment and development framework for ARM
Cortex<sup>&reg;</sup>-M applications.

**NOTE** BSPACM is currently in alpha state.  The build infrastructure
  is stable enough to use; higher-level capabilities are mostly absent.

**NOTE** "BSP" in "BSPACM" could stand for *board support package*,
  *basic system for programming*, or a variety of other phrases.  It
  doesn't.  It's just three letters.

Its features include:

* A common Make-based infrastructure for cross-platform applications;

* Support for several experimenter boards across two vendor lines (two
  Tiva&trade; C Series launchpads; at least two SiLabs/Energy Micro
  Gecko STKs);

* Leverages:

  * [CMSIS](http://www.arm.com/products/processors/cortex-m/cortex-microcontroller-software-interface-standard.php)
    for the low-level API across platforms;

  * Access to vendor-specific libraries and CMSIS device headers for
    full control of specific peripherals;

  * A higher-level API to abstract the vendor-specific peripherals into
    common capabilities (such as LED control, UARTs, etc.)

Its requirements include:

* A POSIX command-line development environment including GNU make;

* The [GNU Tools for ARM Embedded
  Processors](https://launchpad.net/gcc-arm-embedded) 4_8-2013q4 or
  later release;

* Vendor-specific CMSIS files for the target device.  These are readily
  provided by most vendors; CMSIS for some Texas Instruments devices may
  be found.

The overall concept is similar to
[BSP430](http://pabigot.github.io/bsp430/), though the Cortex-M is much
more architecturally consistent and allows use of board-specific
libraries without relying on a complex application-specific compile-time
configuration infrastructure.

Please see the [documentation](http://pabigot.github.io/bspacm/), [issue
tracker](http://github.com/pabigot/bspacm/issues), and [home
page](http://github.com/pabigot/bspacm) on [github]().  Get a copy using
git:

   git clone git://github.com/pabigot/bspacm.git

or by downloading the master branch via:
https://github.com/pabigot/bspacm/tarball/master

Copyright 2014, Peter A. Bigot, and licensed under
[BSD-3-Clause](http://www.opensource.org/licenses/BSD-3-Clause)
