/*
COPYRIGHT (C) 2003  Paolo Mantegazza (mantegazza@aero.polimi.it)

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

#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <rtai_lxrt.h>

static RTIME wd_period_ns = 100000000; // period of WD = 1/10 sec. 

int main (void)
{ 
	RT_TASK *rt_task;
	RTIME task_period; 

	mlockall(MCL_CURRENT|MCL_FUTURE); 
	rt_allow_nonroot_hrt();
	rt_task = rt_task_init_schmod(nam2num("RTRecv"), 1, 0, 0, SCHED_FIFO, 0xF);
	task_period = nano2count(30*wd_period_ns); 

	rt_make_hard_real_time();
	rt_task_make_periodic(rt_task, rt_get_time(), task_period);
	while (1);
	rt_make_soft_real_time();

	rt_task_delete(rt_task);
	return 0; 
}
