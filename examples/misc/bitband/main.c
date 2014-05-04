/* BSPACM - misc/bitband demonstration application
 *
 * Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

/* Evaluate performance of a read-modify-write sequence to set a
 * single bit in an event mask versus a bitband assignment.
 *
 * See also: http://forum.stellarisiti.com/topic/1935-on-bit-banding/?p=6834
 */

#include <bspacm/core.h>
#include <stdio.h>

#define EVENT_S 4
#define EVENT (1U << EVENT_S)
volatile unsigned int events;

unsigned int rmw_set ()
{
  unsigned int t0;
  unsigned int t1;

  t0 = BSPACM_CORE_CYCCNT();
  events |= EVENT;
  t1 = BSPACM_CORE_CYCCNT();
  return t1-t0;
}

unsigned int bitband_set ()
{
  unsigned int t0;
  unsigned int t1;

  t0 = BSPACM_CORE_CYCCNT();
  BSPACM_CORE_BITBAND_SRAM32(events, EVENT_S) = 1;
  t1 = BSPACM_CORE_CYCCNT();
  return t1-t0;
}

void main ()
{
  unsigned int t0;
  unsigned int t1;
  unsigned int cycles;

  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  BSPACM_CORE_ENABLE_CYCCNT();

  events = 0;
  t0 = BSPACM_CORE_CYCCNT();
  events |= EVENT;
  t1 = BSPACM_CORE_CYCCNT();
  printf("Inline RMW %x took %u cycles including overhead\n", events, t1-t0);

  events = 0;
  t0 = BSPACM_CORE_CYCCNT();
  BSPACM_CORE_BITBAND_SRAM32(events, EVENT_S) = 1;
  t1 = BSPACM_CORE_CYCCNT();
  printf("Inline BITBAND %x took %u cycles including overhead\n", events, t1-t0);

  events = 0;
  cycles = rmw_set();
  printf("Outline RMW %x took %u cycles including overhead\n", events, cycles);

  events = 0;
  cycles = bitband_set();
  printf("Outline BITBAND %x took %u cycles including overhead\n", events, cycles);

  t0 = BSPACM_CORE_CYCCNT();
  t1 = BSPACM_CORE_CYCCNT();
  printf("Timing overhead %u cycles\n", t1-t0);
}
