/* Linker script for SiLabs EFM32GG990F1024
 * 1 MiB flash at 0x0000.0000
 * 128 kiB SRAM at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x100000   /* 1M */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x20000    /* 128k */
}
