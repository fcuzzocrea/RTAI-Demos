/*
COPYRIGHT (C) 2002  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_tasklets.h>

#define TIMER_IRQ 0

static struct rt_tasklet_struct *tasklet;
static SEM *dspsem;
static volatile int ovr, intcnt;

#define DIAG_FLAGS
#include "check_flags.h"

static void timer_handler(unsigned long data)
{
	static int first;
	if (!first) {
		first = 1;
		rtai_cli();
	}
	CHECK_FLAGS();
	while ((ovr = rt_irq_wait_if(TIMER_IRQ)) > 0) {
		/* overrun processing, if any, goes here */
		rt_sem_signal(dspsem);
		return;
	}
	/* normal processing goes here */
	intcnt++;
	rt_sem_signal(dspsem);
	rt_pend_linux_irq(TIMER_IRQ);
}

int main(void)
{
	RT_TASK *maint;
	int maxcnt;

	printf("GIVE THE NUMBER OF INTERRUPTS YOU WANT TO COUNT: ");
	scanf("%d", &maxcnt);
	if (!(maint = rt_task_init(nam2num("MAIN"), 1, 0, 0))) {
		printf("CANNOT INIT MAIN TASK > MAIN <\n");
		exit(1);
	}
	if (!(dspsem = rt_sem_init(nam2num("DSPSEM"), 0))) {
		printf("CANNOT INIT SEMAPHORE > DSPSEM <\n");
		exit(1);
	}
	tasklet = rt_init_tasklet();
	rt_insert_tasklet(tasklet, 0, timer_handler, 111, nam2num("TSKLET"), 1);
	rt_request_irq_task(TIMER_IRQ, tasklet, RT_IRQ_TASKLET, 1);
        mlockall(MCL_CURRENT | MCL_FUTURE);

	while (intcnt < maxcnt) {
		rt_sem_wait(dspsem);
		printf("OVERRUNS %d, INTERRUPT COUNT %d\n", ovr, intcnt);
	}
	printf("TEST ENDS\n");
        rt_release_irq_task(TIMER_IRQ);
	rt_remove_tasklet(tasklet);
	rt_delete_tasklet(tasklet);
	rt_task_delete(maint);
	rt_sem_delete(dspsem);
	return 0;
}
