/*
COPYRIGHT (C) 2000  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <linux/module.h>

#include <rtai_posix.h>

MODULE_LICENSE("GPL");

#define TICK 1000000

#define RT_STACK 1000

static pthread_t task1, task2, task3, task4, jtask;

static pthread_cond_t cond;

static pthread_mutex_t mtx;

static pthread_barrier_t barrier;

static int cond_data;

static void *task_func1(void *dummy)
{
	rt_printk("Starting task1, waiting on the conditional variable to be 1.\n");
	pthread_barrier_wait_rt(&barrier);
	pthread_mutex_lock_rt(&mtx);
	while(cond_data < 1) {
		pthread_cond_wait_rt(&cond, &mtx);
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data == 1) {
		rt_printk("task1, conditional variable signalled, value: %d.\n", cond_data);
	}
	rt_printk("task1 signals after setting data to 2.\n");
	rt_printk("task1 waits for a broadcast.\n");
	pthread_mutex_lock_rt(&mtx);
	cond_data = 2;
	pthread_cond_signal_rt(&cond);
	while(cond_data < 3) {
		pthread_cond_wait_rt(&cond, &mtx);
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data == 3) {
		rt_printk("task1, conditional variable broadcasted, value: %d.\n", cond_data);
	}
	rt_printk("Ending task1.\n");
	pthread_exit_rt(0);
	return 0;
}


static void *task_func2(void *dummy)
{
	rt_printk("Starting task2, waiting on the conditional variable to be 2.\n");
	pthread_barrier_wait_rt(&barrier);
	pthread_mutex_lock_rt(&mtx);
	while(cond_data < 2) {
		pthread_cond_wait_rt(&cond, &mtx);
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data == 2) {
		rt_printk("task2, conditional variable signalled, value: %d.\n", cond_data);
	}
	rt_printk("task2 waits for a broadcast.\n");
	pthread_mutex_lock_rt(&mtx);
	while(cond_data < 3) {
		pthread_cond_wait_rt(&cond, &mtx);
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data == 3) {
		rt_printk("task2, conditional variable broadcasted, value: %d.\n", cond_data);
	}
	rt_printk("Ending task2.\n");
	pthread_exit_rt(0);
	return 0;
}

static void *task_func3(void *dummy)
{
	rt_printk("Starting task3, waiting on the conditional variable to be 3 with a 2 s timeout.\n");
	pthread_barrier_wait_rt(&barrier);
	pthread_mutex_lock_rt(&mtx);
	while(cond_data < 3) {
		struct timespec abstime;
		nanos2timespec(rt_get_time_ns() + 2000000000LL, &abstime);
		if (pthread_cond_timedwait_rt(&cond, &mtx, &abstime) < 0) {
			break;
		}
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data < 3) {
		rt_printk("task3, timed out, conditional variable value: %d.\n", cond_data);
	}
	pthread_mutex_lock_rt(&mtx);
	cond_data = 3;
	pthread_mutex_unlock_rt(&mtx);
	rt_printk("task3 broadcasts after setting data to 3.\n");
	pthread_cond_broadcast_rt(&cond);
	rt_printk("Ending task3.\n");
	pthread_exit_rt(0);
	return 0;
}

static void *task_func4(void *dummy)
{
	rt_printk("Starting task4, signalling after setting data to 1, then waits for a broadcast.\n");
	pthread_barrier_wait_rt(&barrier);
	pthread_mutex_lock_rt(&mtx);
	cond_data = 1;
  	pthread_mutex_unlock_rt(&mtx);
	pthread_cond_signal_rt(&cond);
	pthread_mutex_lock_rt(&mtx);
	while(cond_data < 3) {
		pthread_cond_wait_rt(&cond, &mtx);
	}
	pthread_mutex_unlock_rt(&mtx);
	if(cond_data == 3) {
		rt_printk("task4, conditional variable broadcasted, value: %d.\n", cond_data);
	}
	rt_printk("Ending task4.\n");
	pthread_exit_rt(0);
	return 0;
}

static int cleanup;

static void *join_task(void *dummy)
{
	pthread_barrier_wait_rt(&barrier);
	pthread_join_rt(task1, 0);
	pthread_join_rt(task2, 0);
	pthread_join_rt(task3, 0);
	pthread_join_rt(task4, 0);
	cleanup = 1;
	return 0;
}

int init_module(void)
{
	pthread_attr_t attr = { RT_STACK, 0, 0, 0 };
	start_rt_timer(nano2count(TICK));
	printk("Conditional semaphore test program.\n");
	printk("Wait for all tasks to end, then type: ./rem.\n\n");
	pthread_cond_init_rt(&cond, 0);
	pthread_mutex_init_rt(&mtx, 0);
	pthread_barrier_init_rt(&barrier, NULL, 5);
	pthread_create_rt(&task1, &attr, task_func1, 0);
	attr.priority = 1;
	pthread_create_rt(&task2, &attr, task_func2, 0);
	attr.priority = 2;
	pthread_create_rt(&task3, &attr, task_func3, 0);
	attr.priority = 3;
	pthread_create_rt(&task4, &attr, task_func4, 0);
	attr.priority = 4;
	pthread_create_rt(&jtask, &attr, join_task, 0);
	printk("Do not panic, wait 2 s, till task3 times out.\n\n");
	return 0;
}

void cleanup_module(void)
{
	while (!cleanup) {
		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(HZ/10);
	}
	pthread_cond_destroy_rt(&cond);
	pthread_mutex_destroy_rt(&mtx);
	pthread_barrier_destroy_rt(&barrier);
	stop_rt_timer();
	pthread_cancel_rt(task1);
	pthread_cancel_rt(task2);
	pthread_cancel_rt(task3);
	pthread_cancel_rt(task4);
	pthread_cancel_rt(jtask);
	printk("Conditional semaphore test program removed.\n");

}
