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


#include <rtdm/rtdm_driver.h>

MODULE_LICENSE("GPL");

#define LOOPS 20000
#define DELAY 50000

rtdm_task_t stask1, stask2;
rtdm_sem_t sem;
rtdm_mutex_t mutex;

void task1(void *cookie)
{
	rtdm_mutex_lock(&mutex);
	rtdm_sem_down(&sem);
	rtdm_mutex_unlock(&mutex);
	rtdm_sem_up(&sem);
	rtdm_sem_down(&sem);
	while (1) {
		rtdm_mutex_lock(&mutex);
		if (rtdm_sem_down(&sem)) {
			break;
		}
		rtdm_mutex_unlock(&mutex);
	}
}

void task2(void *cookie)
{
	unsigned long i, max;
	nanosecs_abs_t t, dt;

	rt_printk("TESTING TIMING OUT TIMEDLOCK ...");
	for (max = i = 0; i < LOOPS; i++) {
		t = rtdm_clock_read();
		if (rtdm_mutex_timedlock(&mutex, DELAY, NULL) == -ETIMEDOUT) {
			dt = rtdm_clock_read() - t - DELAY;
			if (dt > max) {
				max = dt;
			}
		} else {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK [%lu (ns)].\n", max);
	} else {
		rt_printk(" NOT OK [MAXLAT %lu (ns)].\n", max);
	}

	rt_printk("TESTING FAILING TRY LOCK ...");
	for (i = 0; i < LOOPS; i++) {
		if (rtdm_mutex_timedlock(&mutex, RTDM_TIMEOUT_NONE, NULL) != -EWOULDBLOCK) {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n", max);
	}

	rt_printk("TESTING SUCCEEDING TRY LOCK ...");
	rtdm_sem_up(&sem);
	rtdm_sem_down(&sem);
	for (i = 0; i < LOOPS; i++) {
		if (!rtdm_mutex_timedlock(&mutex, RTDM_TIMEOUT_NONE, NULL)) {
			rtdm_mutex_unlock(&mutex);
		} else {
			break;
		}
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n", max);
	}

	rt_printk("TESTING LOCK/UNLOCK ...");
	rtdm_sem_up(&sem);
	for (i = 0; i < LOOPS; i++) {
		rtdm_sem_up(&sem);
		if (rtdm_mutex_lock(&mutex)) {
			break;
		}
		rtdm_mutex_unlock(&mutex);
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n", max);
	}

	rt_printk("TESTING NOT TIMING OUT TIMEDLOCK ...");
	for (i = 0; i < LOOPS; i++) {
		rtdm_sem_up(&sem);
		if (rtdm_mutex_timedlock(&mutex, DELAY, NULL)) {
			break;
		}
		rtdm_mutex_unlock(&mutex);
	}
	if (i == LOOPS) {
		rt_printk(" OK.\n");
	} else {
		rt_printk(" NOT OK.\n", max);
	}
}


int init_module(void)
{
	printk("TESTING RTDM MUTEXes [LOOPs %d, TIMEOUTs %d (ns)].\n", LOOPS, DELAY);
	start_rt_timer(0);
	rtdm_sem_init(&sem, 0);    
	rtdm_mutex_init(&mutex);    
	rtdm_task_init(&stask1, "task1", task1, NULL, 0, 0);
	rtdm_task_init(&stask2, "task2", task2, NULL, 1, 0);
	return 0;
}


void cleanup_module(void)
{
	rtdm_task_destroy(&stask1);
	rtdm_task_destroy(&stask2);
	rtdm_sem_destroy(&sem);    
	rtdm_mutex_destroy(&mutex);    
	stop_rt_timer();
}
