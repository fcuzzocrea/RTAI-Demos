/*
COPYRIGHT (C) 2004  Paolo Mantegazza (mantegazza@aero.polimi.it)

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

#include <rtai_lxrt.h>
#include <rtai_sysvmsg.h>

#define SLEEP_TIME  3000000

#define NTASKS  10

#define MAXSIZ  2000

static volatile int cnt[NTASKS], end = -1;

static double randu(void)
{
	static int i = 783637;
	i = 125*i;
	i = i - (i/2796203)*2796203;
	return (-1.0 + (double)(i + i)/2796203.0);
}

static void *mfun(int t)
{
	char tname[6] = "MFUN", mname[6] = "RMBX";
	RT_TASK *mytask;
	int smbx, rmbx[NTASKS], msg[MAXSIZ + 1], mtype, i;

	randu();
	tname[4] = mname[4] = t + '0';
	tname[5] = mname[5] = 0;
	mytask = rt_task_init_schmod(nam2num(tname), t + 1, 0, 0, SCHED_FIFO, 1 << (t%2));
	rt_make_hard_real_time();
	smbx    = rt_msgget(nam2num("SMSG"), 0);
	rmbx[t] = rt_msgget(nam2num(mname), 0);

	msg[0] = t;
	while (end < t) {
		msg[MAXSIZ] = 0;
		for (i = 1; i < MAXSIZ; i++) {
			msg[MAXSIZ] += (msg[i] = MAXSIZ*randu()); 
		}
		if (rt_msgsnd_(smbx, 1, msg, sizeof(msg), 0)) {
			rt_printk("SEND FAILED, TASK: %d\n", t);
			goto prem;
		}
		msg[0] = msg[1] = 0;
		if (rt_msgrcv_(rmbx[t], &mtype, &msg, sizeof(msg), 1, 0) < 0) {
			rt_printk("RECEIVE FAILED, TASK: %d\n", t);
			goto prem;
		}
		if (msg[0] != t || msg[1] != 0xFFFFFFFF) {
			rt_printk("WRONG REPLY TO TASK: %d.\n", t);
                        goto prem;
		}
		cnt[t]++;
//		rt_printk("TASK: %d, OK (%d).\n", t, cnt[t]);
		rt_sleep(nano2count(SLEEP_TIME));
	}
prem: 
	rt_make_soft_real_time();
	rt_task_delete(mytask);
	printf("TASK %d ENDS.\n", t);
	return 0;
}

static void *bfun(int t)
{
	RT_TASK *mytask;
	int smbx, rmbx[NTASKS], msg[MAXSIZ + 1], mtype, i, n;

	mytask = rt_task_init_schmod(nam2num("BFUN"), 0, 0, 0, SCHED_FIFO, 0xF);
	rt_make_hard_real_time();
	smbx = rt_msgget(nam2num("SMSG"), 0);
	for (i = 0; i < NTASKS; i ++) {
		char mname[6] = "RMBX";
		mname[4] = i + '0';
		mname[5] = 0;
		rmbx[i] = rt_msgget(nam2num(mname), 0);
	}

	while (end < NTASKS) {
		rt_msgrcv_(smbx, &mtype, msg, sizeof(msg), 1, 0);
		n = 0;
		for (i = 1; i < MAXSIZ; i++) {
			n += msg[i];
		}
		if (msg[MAXSIZ] != n) {
			rt_printk("SERVER RECEIVED AN UNKNOWN MSG.\n");
			goto prem;
		}
		msg[1] = 0xFFFFFFFF;
		rt_msgsnd_(rmbx[msg[0]], 1, msg, 2*sizeof(int), 0);
	}
prem:
	rt_make_soft_real_time();
	rt_task_delete(mytask);
	printf("SERVER TASK ENDS.\n");
	return 0;
}

static int smbx, rmbx[NTASKS];
static pthread_t bthread, mthread[NTASKS];

int main(void)
{
	pthread_attr_t attr;
	RT_TASK *mytask;
	char msg[] = "let's end the game";
	int i;

	mytask = rt_task_init_schmod(nam2num("MAIN"), 2, 0, 0, SCHED_FIFO, 0xF);

	smbx    = rt_msgget(nam2num("SMSG"), 0x666 | IPC_CREAT);
	for (i = 0; i < NTASKS; i ++) {
		char mname[6] = "RMBX";
		mname[4] = i + '0';
		mname[5] = 0;
		rmbx[i] = rt_msgget(nam2num(mname), 0x666 | IPC_CREAT);
	}

	rt_set_oneshot_mode();
	start_rt_timer(0);

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x8000);
	pthread_create(&bthread, &attr, (void *)bfun, 0);
	for (i = 0; i < NTASKS; i ++) {
		pthread_create(mthread + i, &attr, (void *)mfun, (void *)i);
	}

	printf("IF NOTHING HAPPENS IS OK, TYPE ENTER TO FINISH.\n");
	getchar();

	for (i = 0; i < NTASKS; i ++) {
		end = i;
		rt_msgsnd_(rmbx[i], 1, &msg, sizeof(msg), 0);
		pthread_join(mthread[i], NULL);
	}

	end = NTASKS;
	rt_msgsnd_(smbx,    1, &msg, sizeof(msg), 0);
	pthread_join(bthread, NULL);

	for (i = 0; i < NTASKS; i ++) {
		rt_msgctl(rmbx[i], IPC_RMID, NULL);
		rt_printk("TASK %d, LOOPS: %d.\n", i, cnt[i]); 
	}
	rt_msgctl(smbx, IPC_RMID, NULL);

	stop_rt_timer();
	rt_task_delete(mytask);
	printf("MAIN TASK ENDS.\n");
	return 0;
}
