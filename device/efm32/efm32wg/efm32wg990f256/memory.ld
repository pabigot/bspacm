/* Linker script for SiLabs EFM32WG990F256
 * 256 kiB flash at 0x0000.0000
 * 32 kiB SRAM at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x40000   /* 256k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x8000    /* 32k */
}
