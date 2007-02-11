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

#define LOOPS 50000

#define STACK_SIZE 100000

SEM *sem1, *sem2;

static volatile int end;

void task1(void *cookie)
{
	rt_task_init_schmod(nam2num("TASK1"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_grow_and_lock_stack(STACK_SIZE - 10000);
	rt_make_hard_real_time();
	rt_printk("TASK1 TID = %d.\n", rt_gettid());

	while (!end) {
		rt_sem_wait(sem1);
		rt_sem_signal(sem2);
	}
        rt_task_delete(NULL);
	rt_printk("TASK1 EXITING.\n");
}

void task2(void *cookie)
{
	int i;

	rt_task_init_schmod(nam2num("TASK2"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_grow_and_lock_stack(STACK_SIZE - 10000);
	rt_make_hard_real_time();
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

	rt_printk("TESTING SUCCEEDING TRY DOWN ...");
	rt_sem_signal(sem2);
	for (i = 0; i < LOOPS; i++) {
		if (rt_sem_wait_if(sem2) > 0) {
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

	rt_printk("TESTING DOWN/UP ...");
	rt_sem_wait(sem2);
	for (i = 0; i < LOOPS; i++) {
		rt_sem_signal(sem1);
		if (rt_sem_wait(sem2) > 0) {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n");
	}
        rt_task_delete(NULL);
	rt_printk("TASK2 EXITING.\n");
}

static int thread1, thread2;

int main(void)
{
	rt_task_init_schmod(nam2num("MNTSK"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_printk("TESTING RTDM SEMs [LOOPs %d].\n", LOOPS);

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
	return 0;
}