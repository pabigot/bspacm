/* Linker script for SiLabs EFM32TG840F32
 * 32 kiB flash at 0x0000.0000
 * 4 kiB SRAM at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x8000   /* 32k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x1000   /* 4k */
}
