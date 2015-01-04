The nRF51 series varies along several dimensions that do not map well to
the BSPACM series/line/device taxonomy.

* Device part number (DEVICE_PART).  The options are:

    + 51422 : nRF51422, top of model line incorporates ANT, BlueTooth,
      2.4GHz proprietary radio

    + 51822 : nRF51822, All capabilities of nRF51422 except ANT

  There are no configuration differences based on device part number.
  For completeness, boards define DEVICE_PART to 51422 or 51822, but
  this is not currently reflected in the series/line/device hierarchy.
  All nRF51 device parts are reduced to nRF51 as the BSPACM line.

  DEVICE_PART is defined in Makefile.board and cannot be overridden.

* Device variant (DEVICE_VARIANT).  This is denoted by the second pair
  of characters in the package/variant identifier and indicates memory
  layout.  The options are:

    + xxaa : 256 KiBy flash, 16 KiBy RAM
    + xxab : 128 KiBy flash, 16 KiBy RAM
    + xxac : 256 KiBy flash, 32 KiBy RAM

  DEVICE_VARIANT is defined in Makefile.board and cannot be overridden.

* Application soft device (DEVICE_SD).  Selection of a "soft-device"
  providing capabilities such as BlueTooth Smart through a SVC (formerly
  SWI) supervisor call interface.  The set of options are:

    + blank : no soft device installed
    + s110 : Bluetooth 4.1, peripheral+broadcaster role
      - s110v6 : Version 5.x and 6.x use 80 KiBy flash
      - s110v7 : Version 7.x uses 88 KiBy flash
    + s120 : Bluetooth 4.1, 8x central+observer, peripheral+broadcaster roles
      - s120v1 : Version 1.x uses 96 KiBy flash
      - s120v2 : Version 2.x uses 116 KiBy flash
    + s130 : Bluetooth 4.1, 3x central, 1 ea peripheral+observer+broadcaster
    + s210 : ANT soft device (51422 only)
    + s310 : ANT soft device (51422 only)

  Assuming memory requirements are met, the same device can accept any
  of the soft devices; the impact is on the memory map used by the
  linker.

  DEVICE_SD is defaulted in Makefile.series but values in application
  Makefile supersede this default.

* Printed Circuit Assembly (DEVICE_PCA).  This is the identifier used to
  distinguish evaluation boards.  "nrf-$(DEVICE_PCA)" is the BSPACM
  board identifier.

  DEVICE_PCA is defined in Makefile.board and cannot be overridden.
