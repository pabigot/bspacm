/* Linker script for Nordic Semiconductor nRF51 variant XXAC
 * 256 kiB flash at 0x0000.0000
 *  32 kiB SRAM  at 0x2000.0000
 *
 * S120 v1 (SDS 1.1) reserves 96 KiBy FLASH, 10 KiBy RAM
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00018000, LENGTH = 160k /* 256 - 96 */
  /* NB: S120v1 aborts if RAM goes beyond 16 KiBy.
   * https://devzone.nordicsemi.com/question/24295/s120-v-101-and-xxac-devices/ */
  RAM (rwx)  : ORIGIN = 0x20002800, LENGTH = 6k  /* 32 - 10 */
}
