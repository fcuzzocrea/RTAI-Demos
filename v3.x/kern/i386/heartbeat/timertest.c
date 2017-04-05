/*
COPYRIGHT (C) 2017  Paolo Mantegazza (mantegazza@aero.polimi.it)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
*/


#include <linux/delay.h>

#include <rtdm/rtdm_driver.h>

MODULE_LICENSE("GPL");

#define LOOPS                     10
#define RELATIVE_FIRST_SHOT_DELAY 1000000000
#define PERIOD                    1000000

struct rtdm_timer_struct timer;
static int loops;

void timer_handler(rtdm_timer_t *timer)
{
	static int first = 1;
	static nanosecs_rel_t tp;
	nanosecs_rel_t t;

	if (first) {
		tp = rtdm_clock_read();
		first = 0;
	}
	t = rtdm_clock_read();
	if (loops++ < LOOPS) {
		printk("LOOP: %d, PERIOD: %lld (us).\n", loops, (t - tp + 499)/1000);
	}
	tp = t;
}

int init_module(void)
{
	printk("TESTING RTDM TIMERs [LOOPs %d, RELATIVE FIRST SHOT DELAY %d (ns), PERIOD %d (ns)].\n", LOOPS, RELATIVE_FIRST_SHOT_DELAY, PERIOD);
	rtdm_timer_init(&timer, timer_handler, "RTDMTM");
	rtdm_timer_start(&timer, RELATIVE_FIRST_SHOT_DELAY, PERIOD, RTDM_TIMERMODE_RELATIVE);
	while(loops < LOOPS) {
		msleep(100);
	}
	rtdm_timer_stop(&timer);
	return 0;
}


void cleanup_module(void)
{
	rtdm_timer_destroy(&timer);
}
