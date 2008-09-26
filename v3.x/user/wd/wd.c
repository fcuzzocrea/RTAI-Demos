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
#include <rtai_msg.h>

#define TASK_PERIOD 1000000 // ns

int main (void)
{ 
	RT_TASK *rt_task;
	unsigned long msg;

	rt_allow_nonroot_hrt();
	rt_task = rt_task_init_schmod(nam2num("LOOPER"), 1, 0, 0, SCHED_FIFO, 0x2);
	mlockall(MCL_CURRENT | MCL_FUTURE); 
	rt_make_hard_real_time();
	while (!rt_receive_if(NULL, &msg));
	rt_make_soft_real_time();

	rt_task_delete(rt_task);
	return 0; 
}
