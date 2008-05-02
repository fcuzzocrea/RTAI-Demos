#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/mman.h>

#include <rtai_nam2num.h>
#include <rtai_lxrt.h>
#include <rtai_scb.h>
#include "params.h"

static int end;

static void endme(int dummy) { end = 1; }

int main(void)
{
	unsigned int i, cnt;
	unsigned int n = 0;
	unsigned int data[BUFSIZE];
	void *scb;

	signal(SIGINT,  endme);
	signal(SIGTERM, endme);

	rt_thread_init(nam2num("MAIN"), 1, 0, SCHED_FIFO, 0xF);
	scb = rt_scb_init(nam2num("SCB"), 0, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();

	while (!end) {
		if ((cnt = randu()*BUFSIZE) > 0) {
			printf("UCNT %d\n", cnt);
			for (i = 0; i < cnt; i++) {
				data[i] = ++n;
			}
			while (rt_scb_put(scb, data, cnt*sizeof(int))) {
				rt_sleep(nano2count(SLEEP_TIME));
			}
		}
	}

	rt_make_soft_real_time();
	rt_task_delete(NULL);
	return 0;
}
