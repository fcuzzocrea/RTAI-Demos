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

#include <rtai_mbx.h>

#define BFSZ 80
#define NMBX 3
static MBX *mbx[NMBX], *inpt;

static void poll_fun(void *arg)
{
	int i;
	struct rt_poll_s polld[NMBX];
	char buf[BFSZ];

	rt_task_init_schmod(nam2num("POLL"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_make_hard_real_time();
	printf("\n");

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
		rt_mbx_send(inpt, buf, sizeof(buf));
		if (!buf[0]) break;
	}

	rt_make_soft_real_time();
	rt_task_delete(NULL);

	return;
}

static unsigned long poll_thread;

int main(int argc, char* argv[])
{
	int i;
	struct rt_poll_s polld[1];
	char buf[BFSZ];

	rt_task_init_schmod(nam2num("MAIN"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_set_oneshot_mode();
	start_rt_timer(0);

	for (i = 0; i < NMBX; i++) {
		mbx[i] = rt_mbx_init(rt_get_name(NULL), BFSZ);
	}
	inpt = rt_mbx_init(rt_get_name(NULL), BFSZ);

	poll_thread = rt_thread_create(poll_fun, NULL, 0);
	rt_sleep(nano2count(100000000));

	while (1) {
		printf("0 <= mbx < %d: ", NMBX);
		scanf("%d", &i);
		if (i < 0 || i >= NMBX) break;
		printf("msg: ");
		scanf("%s", buf);
		rt_mbx_send(mbx[i], buf, sizeof(buf));
		polld[0] = (struct rt_poll_s){ inpt, RT_POLL_MBX_RECV };
		rt_poll(polld, 1, 0);
		rt_mbx_receive(inpt, buf, sizeof(buf));
	}

	buf[0] = 0;
	rt_mbx_send(mbx[0], buf, sizeof(buf));
	rt_thread_join(poll_thread);

	for (i = 0; i < NMBX; i++) {
		rt_mbx_delete(mbx[i]);
	}
	rt_mbx_delete(inpt);
	rt_task_delete(NULL);

	return 0;
}
