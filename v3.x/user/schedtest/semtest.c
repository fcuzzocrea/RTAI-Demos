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


#include <stdio.h>
#include <sys/mman.h>
#include <sys/poll.h>

#include <rtai_sem.h>

#define LOOPS 5000

#define STACK_SIZE 100000

SEM *sem1, *sem2;

static volatile int end;

#define MAKE_HARD()  rt_make_hard_real_time()

void task1(void *nothing)
{
	rt_thread_init(nam2num("TASK1"), 1, 0, SCHED_FIFO, 0x1);
	rt_grow_and_lock_stack(STACK_SIZE - 10000);
#ifdef MAKE_HARD
	MAKE_HARD();
#endif	
	rt_make_hard_real_time();
	rt_printk("TASK1 TID = %d : ", rt_gettid());

	while (!end) {
		rt_sem_wait(sem1);
		rt_sem_signal(sem2);
	}

        rt_task_delete(NULL);
	rt_printk("TASK1 EXITING.\n");
	return;
}

void task2(void *nothing)
{
	int i;

	rt_thread_init(nam2num("TASK2"), 2, 0, SCHED_FIFO, 0x1);
	rt_grow_and_lock_stack(STACK_SIZE - 10000);
#ifdef MAKE_HARD
	MAKE_HARD();
#endif	
	rt_printk("TASK2 TID = %d.\n", rt_gettid());

	rt_printk("TESTING FAILING WAIT IF ...");
	for (i = 0; i < LOOPS; i++) {
		if (rt_sem_wait_if(sem2) > 0) {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n");
	}

	rt_printk("TESTING SUCCEEDING WAIT IF ...");
	rt_sem_signal(sem2);
	for (i = 0; i < LOOPS; i++) {
		if (rt_sem_wait_if(sem2) == 1) {
			rt_sem_signal(sem2);
		} else {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n");
	}

	rt_printk("TESTING WAIT/SIGNAL ...");
	rt_sem_wait(sem2);
	for (i = 0; i < LOOPS; i++) {
		rt_sem_signal(sem1);
		if (rt_sem_wait(sem2) == 0) {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n");
	}

        rt_task_delete(NULL);
	rt_printk("TASK2 EXITING : ");
	return;
}

static int thread1, thread2;

int main(void)
{
	rt_thread_init(nam2num("MNTSK"), 100, 0, SCHED_FIFO, 0x1);
	rt_printk("\nTESTING THE SCHEDULER WITH SEMs [%d LOOPs].\n", LOOPS);

	sem1 = rt_sem_init(0xcacca1, 0);    
	sem2 = rt_sem_init(0xcacca2, 0);    
	thread1 = rt_thread_create(task1, NULL, STACK_SIZE);
	poll(NULL, 0, 100);
	thread2 = rt_thread_create(task2, NULL, STACK_SIZE);
	rt_thread_join(thread2);
	end = 1;
	rt_sem_signal(sem1);
	rt_thread_join(thread1);
	rt_sem_delete(sem1);    
	rt_sem_delete(sem2);    

        rt_task_delete(NULL);
	rt_printk("END SCHEDULER TEST WITH SEMs.\n\n");
	return 0;
}
