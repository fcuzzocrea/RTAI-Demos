/*
COPYRIGHT (C) 2007  Paolo Mantegazza (mantegazza@aero.polimi.it)

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

MODULE_LICENSE("GPL");

static RT_TASK task;

static RTIME wd_period_ns = 100000000; // period of WD = 1/10 sec. 

static void fun(long none)
{
	while (1);
}

int init_module(void)
{
	rt_task_init(&task, fun, 0, 8000, 1, 0, 0);
	rt_task_make_periodic(&task, rt_get_time(), nano2count(30*wd_period_ns));
	return 0;
}

void cleanup_module(void)
{
	rt_task_delete(&task);
}
