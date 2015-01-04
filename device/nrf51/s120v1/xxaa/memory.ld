/* Linker script for Nordic Semiconductor nRF51 variant XXAA
 * 256 kiB flash at 0x0000.0000
 *  16 kiB SRAM  at 0x2000.0000
 *
 * S120 v1 (SDS 1.1) reserves 96 KiBy FLASH, 10 KiBy RAM
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00018000, LENGTH = 160k /* 256 - 96 */
  RAM (rwx)  : ORIGIN = 0x20002800, LENGTH = 6k   /* 16 - 10 */
}
