/* Linker script for Nordic Semiconductor nRF51 variant XXAC
 * 256 kiB flash at 0x0000.0000
 *  32 kiB SRAM  at 0x2000.0000
 *
 * S120 v2 (SDS 2.0) reserves 112 + 4 KiBy FLASH, 10 KiBy RAM
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x0001D000, LENGTH = 140k /* 256 - 112 - 4 */
  RAM (rwx)  : ORIGIN = 0x20002800, LENGTH = 22k  /* 32 - 10 */
}
