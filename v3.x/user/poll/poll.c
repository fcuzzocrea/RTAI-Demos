/*
 * RTAI serial driver test
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <rtai_mbx.h>

#define NMBX 3
static MBX *mbx[NMBX];
static SEM *sem;

static void poll_fun(void *arg)
{
	int i;
	struct rt_poll_s polld[NMBX];
	char buf[50];

	rt_task_init_schmod(nam2num("POLL"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_make_hard_real_time();

	while (1) {
		for (i = 0; i < NMBX; i++) {
			polld[i] = (struct rt_poll_s){ mbx[i], RT_POLL_MBX_RECV };
		}
		rt_poll(polld, NMBX, 0);
		for (i = 0; i < NMBX; i++) {
			if (!polld[i].what) {
				rt_mbx_receive(mbx[i], buf, sizeof(buf));
				printf("mbx: %d, received: %s.\n", i, buf);
			}
		}
		rt_sem_signal(sem);
	}
	rt_make_soft_real_time();
	rt_task_delete(NULL);
}

int main(int argc, char* argv[])
{
	int i;
	char buf[50];

	rt_task_init_schmod(nam2num("MAIN"), 1, 0, 0, SCHED_FIFO, 0xF);
	rt_set_oneshot_mode();
	start_rt_timer(0);

	for (i = 0; i < NMBX; i++) {
		mbx[i] = rt_mbx_init(rt_get_name(NULL), 1000);
	}
	sem = rt_sem_init(rt_get_name(NULL), 0);

	rt_thread_create(poll_fun,  NULL, 0);

	while (1) {
		printf("0 <= mbx < %d: ", NMBX);
		scanf("%d", &i);
		if (i < 0 || i >= NMBX) break;
		printf("msg: ");
		scanf("%s", buf);
		rt_mbx_send(mbx[i], buf, sizeof(buf));
		rt_sem_wait(sem);
	}

	for (i = 0; i < NMBX; i++) {
		rt_mbx_delete(mbx[i]);
	}
	rt_sem_delete(sem);

	rt_task_delete(NULL);

	return 0;
}
