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

static RT_TASK *Slow_Task;
static RT_TASK *Fast_Task;

static void *slow_fun(void *arg)
{
        if (!(Slow_Task = rt_task_init_schmod(nam2num("SLWTSK"), 3, 0, 0, SCHED_FIFO, 0x1))) {
                printf("CANNOT INIT SLOW TASK\n");
                exit(1);
        }

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
        rt_printk("SLOW\n");
        while (1);
	rt_make_soft_real_time();
	rt_task_delete(Slow_Task);
	return 0;
}                                        

static void *fast_fun(void *arg) 
{                             
        if (!(Fast_Task = rt_task_init_schmod(nam2num("FSTSK"), 2, 0, 0, SCHED_FIFO, 0x1))) {
                printf("CANNOT INIT FAST TASK\n");
                exit(1);
        }

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
        rt_printk("FAST\n");
	rt_sleep(nano2count(300000000));
	rt_task_suspend(Slow_Task);
	rt_task_delete(Slow_Task);
	rt_make_soft_real_time();
	rt_task_delete(Fast_Task);
	return 0;
}

static pthread_t fast_thread, slow_thread;

int main(void)
{
	RT_TASK *Main_Task;

        if (!(Main_Task = rt_task_init_schmod(nam2num("MNTSK"), 1, 0, 0, SCHED_FIFO, 0x1))) {
                printf("CANNOT INIT MAIN TASK\n");
                exit(1);
        }
	rt_set_oneshot_mode();
	start_rt_timer(0);

	pthread_create(&fast_thread, NULL, fast_fun, NULL);
	rt_sleep(nano2count(100000000));
	pthread_create(&slow_thread, NULL, slow_fun, NULL);
	pause();
	stop_rt_timer();	
	rt_task_delete(Main_Task);
	return 0;
}
