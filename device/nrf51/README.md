The nRF51 series varies along several dimensions:

* Device part number.  The options are:

    + nRF51422: top of model line, incorporates ANT, BlueTooth, 2.4GHz
      proprietary radio

    + nRF51822: All capabilities of nRF51422 except ANT

  There are no configuration differences based on device part number., so
  this variation point is reduced to a single option nRF51422 as the
  BSPACM line.

* Memory footprints.  The options are:

    + xxaa: 256 KiBy flash, 16 KiBy RAM
    + xxab: 128 KiBy flash, 16 KiBy RAM
    + xxac: 256 KiBy flash, 32 KiBy RAM

  This variation is used for the BSPACM device layer.

* Selection of a "soft-device" providing capabilities such as BlueTooth
  Smart through a SVC (formerly SWI) supervisor call interface.  The set
  of options are: blank, s110, s120, s130, s210, s310

  Assuming memory requirements are met, the same device can accept any
  of the the soft devices; the impact is on the memory map used by the
  linker.
