/*
COPYRIGHT (C) 2008  Bernhard Pfund (bernhard@chapter7.ch)

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


#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <rtai_mbx.h>

#include <rtnet.h>

#define USESEL    1
#define USEMBX    1
#define CPUMAP    0xF
#define WORKCYCLE 100000LL
#define TIMEOUT   (WORKCYCLE*10)/100

//#define ECHO rt_printk
#define ECHO printf

#define PORT 55555

struct sample { RTIME tx, rx; };

static RT_TASK *Receiver_Task;

static MBX *mbx;

static int sock;

static struct sockaddr_in transmit_addr;
static struct sockaddr_in receive_addr;

volatile int end;

static void endme(int unused) { end = 1; }

static void *sender(void *arg)
{
	RT_TASK *Sender_Task;
	RTIME stime;
	int msgcnt = 0, slen;

	if (!(Sender_Task = rt_thread_init(nam2num("TXTSK"), 0, 0, SCHED_FIFO, CPUMAP))) {
		printf("Cannot initialise the sender task\n");
		exit(1);
	}
	ECHO("Transmitter task initialised\n");

	rt_make_hard_real_time();

	while(!end) {
		stime = rt_get_time_ns();
		slen = rt_dev_sendto(sock, (void *)&stime, sizeof(RTIME), 0, (struct sockaddr*)&transmit_addr, sizeof(transmit_addr));
		ECHO("TRASMITTED %d %d %lld\n", ++msgcnt, slen, stime);
		rt_sleep(nano2count(WORKCYCLE));
	}

	rt_task_masked_unblock(Receiver_Task, ~RT_SCHED_READY);
	rt_dev_sendto(sock, (void *)&stime, sizeof(RTIME), 0, (struct sockaddr*)&transmit_addr, sizeof(transmit_addr)); // not needed, just to be safer
	rt_make_soft_real_time();
	printf("Transmitter exiting\n");
	rt_thread_delete(Sender_Task);
	return 0;
}

static void *receiver(void *arg)
{
	struct sample rx_samp;
	int msgcnt = 0, rlen, usetimeout = 0;
	socklen_t fromlen;
	fromlen = sizeof(receive_addr);
	fd_set rxfds;

	if (!(Receiver_Task = rt_thread_init(nam2num("RXTSK"), 0, 0, SCHED_FIFO, CPUMAP))) {
		printf("Cannot initialise the receiver task\n");
		exit(1);
	}

	ECHO("Receiver task initialised\n");

	rt_make_hard_real_time();

	while(!end) {
		int ready = 0;
#if USESEL
		RTIME timeout;
		usetimeout = 1 - usetimeout;
		timeout = usetimeout ? TIMEOUT : 0;
		FD_ZERO(&rxfds);
		FD_SET(sock, &rxfds);
		ready = rt_dev_select(sock + 1, &rxfds, NULL, NULL, timeout);
		ECHO("Receiver select returned %d (0 is TIMEDOUT)\n", ready);
		if (ready < 0) {
			end = 1;
			break;
		} else if (!ready) {
			continue;
		}
#endif
		if (!USESEL || (ready > 0 && FD_ISSET(sock, &rxfds))) {
			rlen = rt_dev_recvfrom(sock, (void *)&rx_samp.tx, sizeof(RTIME), 0, (struct sockaddr*) &receive_addr, &fromlen);
			if (rlen > 0) {
				rx_samp.rx = rt_get_time_ns();
				rt_mbx_send_if(mbx, &rx_samp, sizeof(rx_samp));
				ECHO("RECEIVED %d %d %lld %lld\n", ++msgcnt, rlen, rx_samp.tx, rx_samp.rx);
			} else {
				ECHO("RECEIVED REQUESTED %d GOT %d\n", sizeof(rx_samp), rlen);
			}
		}
	}

	rt_make_soft_real_time();
	ECHO("Receiver exiting\n");
	rt_thread_delete(Receiver_Task);
	return 0;
}

/** The main method */
int main(void)
{
	struct sockaddr_in local_addr;
	pthread_t sender_thread;
	pthread_t receiver_thread;
	RT_TASK *Main_Task;
	int msgcnt = 0;
	struct sample frombx;
	int broadcast = 1;

	signal(SIGHUP, endme);
	signal(SIGINT, endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	signal(SIGALRM, endme);

	rt_allow_nonroot_hrt();
	start_rt_timer(0);
	if (!(Main_Task = rt_thread_init(nam2num("MNTSK"), 0, 0, SCHED_FIFO, 0xF))) {
		printf("Cannot initialise the main task\n");
		exit(1);
	}
	if (!(mbx = rt_mbx_init(nam2num("MYMBX"), 20*sizeof(struct sample)))) {
		printf("Cannot create the mailbox\n");
		exit(1);
	}
	if (((sock = rt_dev_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) == -1) {
		printf("Error opening UDP/IP socket: %d\n", sock);
		exit(1);
	}
	if (rt_dev_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
		printf("Can't set broadcast options\n");
		rt_dev_close(sock);
		exit(1);
	}
	if (USESEL) {
		int64_t timeout = -1;
		rt_dev_ioctl(sock, RTNET_RTIOC_TIMEOUT, &timeout);
//		fcntl(sock, F_SETFL, O_NONBLOCK); do not use, it sets stdin
	}

	local_addr.sin_family      = AF_INET;
	local_addr.sin_port        = htons(PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	transmit_addr.sin_family      = AF_INET;
	transmit_addr.sin_port        = htons(PORT);
	transmit_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (rt_dev_bind(sock, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) == -1) {
		ECHO("Can't configure the network socket");
		rt_dev_close(sock);
		exit(1);
	}

	sender_thread   = rt_thread_create(sender,   NULL, 0);
	receiver_thread = rt_thread_create(receiver, NULL, 0);

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();

#if USEMBX
	while (!end) {
		rt_mbx_receive(mbx, (void *)&frombx, sizeof(frombx));
		ECHO("MAIN FROM MBX %d %lld %lld\n", ++msgcnt, frombx.tx, frombx.rx);
	}
#else
	pause();
#endif

	rt_thread_join(sender_thread);
	rt_thread_join(receiver_thread);
	rt_dev_close(sock);
	rt_dev_shutdown(sock, SHUT_RDWR);
	stop_rt_timer();
	rt_mbx_delete(mbx);
	rt_make_soft_real_time();
	rt_thread_delete(Main_Task);
	return 0;
}
