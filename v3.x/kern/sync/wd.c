/*
COPYRIGHT (C) 2008  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <linux/kernel.h>
#include <linux/module.h>

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_nam2num.h>
#include <rtai_shm.h>
#include <rtai_msg.h>

MODULE_LICENSE("GPL");

#define PERIOD        25000
#define WAIT_DELAY    15000
#define WORKING_TIME  5000

#define WAIT_AIM_TIME()  do { while (rt_get_time() < aim_time); } while (0)

long *worst_lat;

static void fun(long none)
{
	long msg;
	RTIME period, wait_delay, sync_time, aim_time; 
	*worst_lat = -2000000000;
	wait_delay = nano2count(WAIT_DELAY); 
	period     = nano2count(PERIOD); 
	rtai_cli();
	aim_time  = rt_get_time();
	sync_time = aim_time + wait_delay; 
	aim_time += period; 
	while (!rt_receive_if(NULL, &msg)) {
		WAIT_AIM_TIME();
		sync_time = rt_get_time(); 
		msg = abs((long)(sync_time - aim_time));
		sync_time = aim_time + wait_delay; 
		aim_time  += period;
		if (msg > *worst_lat) {
			*worst_lat = msg;
		}
		rt_busy_sleep(WORKING_TIME);
		rt_sleep_until(sync_time);
	}
	rtai_sti();
}

static RT_TASK *task;

int init_module(void)
{
	start_rt_timer(0);
	worst_lat = rt_shm_alloc(nam2num("WSTLAT"), sizeof(RTIME), USE_VMALLOC);
	task = rt_named_task_init_cpuid("LOOPER", fun, 0, 8000, 0, 0, 0, 1);
	rt_task_resume(task);
	return 0;
}

void cleanup_module(void)
{
	stop_rt_timer();
	rt_shm_free(nam2num("WSTLAT"));
	rt_named_task_delete(task);
}
