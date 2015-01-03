/* BSPACM - nRF51 uptime example
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

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/utility/uptime.h>
#include <stdio.h>
#include <fcntl.h>

struct tick_type {
  sBSPACMuptimeAlarm alarm;
  volatile unsigned int uptime_s;
};

void
tick_flih (int ccidx,
           hBSPACMuptimeAlarm ap)
{
  struct tick_type * cp = (struct tick_type *)ap;
  (void)ccidx;
  vBSPACMledSet(0, -1);
  cp->uptime_s += 1;
}

struct tick_type tick_state = {
  .alarm = {
    .callback_flih = tick_flih,
    .interval_utt = BSPACM_UPTIME_Hz
  }
};

void main ()
{
  vBSPACMledConfigure();
  vBSPACMuptimeStart();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  unsigned int uptime = tick_state.uptime_s;
  int rc = iBSPACMuptimeAlarmSet(1, BSPACM_UPTIME_Hz, &tick_state.alarm);
  printf("Alarm config: %d\n", rc);
  while (1) {
    unsigned int new_uptime = tick_state.uptime_s;

    while (new_uptime == uptime) {
      __WFE();
      new_uptime = tick_state.uptime_s;
      vBSPACMledSet(3, -1);
    }
    vBSPACMledSet(1, -1);
    unsigned int now = uiBSPACMuptime();
    uptime = new_uptime;
    printf("Uptime %u counter %x counts %u\n", uptime, now, now / BSPACM_UPTIME_Hz);
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
