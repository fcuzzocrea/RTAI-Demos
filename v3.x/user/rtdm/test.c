/*
COPYRIGHT (C) 2002  Giuseppe Renoldi (giuseppe@renoldi.org)

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


/*
 * rtai-16550A-LXRT test
 * =====================
 *
 * Adaptation of rtai_spdrv test provided in RTAI.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/io.h>
#include <signal.h>

#define SHOWROOM
#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtdm/rtserial.h>

#define LOOPS  10
#define RX_TIMEOUT  1000000000
#define PAUSE_TIME  1000000000

static int sfd, rfd;

static void endme(int dummy) 
{ 
	rt_dev_close(sfd);
	rt_dev_close(rfd);
}

int main(void)
{
	RT_TASK *testcomtsk;
	char hello[20];
	rtser_config_t serconf = { 0xFFFF, RTSER_DEF_BAUD, RTSER_DEF_PARITY, RTSER_DEF_BITS, RTSER_DEF_STOPB, RTSER_DEF_HAND, RTSER_DEF_FIFO_DEPTH, RTSER_DEF_TIMEOUT, RTSER_DEF_TIMEOUT, RTSER_DEF_TIMEOUT, RTSER_DEF_TIMESTAMP_HISTORY };
	struct rtser_status status;
	int mcr_status, i;
	
        signal(SIGINT, endme);
 	if (!(testcomtsk = rt_task_init(nam2num("TESTCOM"), 1, 0, 0))) {
		printf("CANNOT INIT MASTER TASK\n");
		exit(1);
	}
	rt_set_oneshot_mode();
	start_rt_timer(0);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();

	if ((sfd = rt_dev_open("rtser0", O_RDWR)) < 0 || (rfd = rt_dev_open("rtser1", O_RDWR)) < 0) {
		printf("hello_world_lxrt: error in rt_dev_open()\n");
		return 1;
	} else {	
		rt_dev_ioctl(sfd, RTSER_RTIOC_SET_CONFIG, &serconf);
		rt_dev_ioctl(rfd, RTSER_RTIOC_GET_CONFIG, &serconf);
		serconf.config_mask = RTSER_SET_TIMEOUT_RX;
		serconf.rx_timeout  = RX_TIMEOUT;
		rt_dev_ioctl(rfd, RTSER_RTIOC_SET_CONFIG, &serconf);
		printf("\nhello_world_lxrt: rtser0 test started (fd_count = %d)\n", rt_dev_fdcount());
		for (i = 0; i < LOOPS; i++) {
			strcpy(hello, "Hello World\n\r");
			rt_dev_write(sfd, hello, sizeof(hello) - 1);
			rt_dev_ioctl(sfd, RTSER_RTIOC_GET_STATUS, &status);
			printf("hello_world_lxrt: line status = 0x%x, modem status = 0x%x.\n", status.line_status, status.modem_status);
			rt_dev_ioctl(sfd, RTSER_RTIOC_GET_CONTROL, &mcr_status);
			rt_dev_ioctl(sfd, RTSER_RTIOC_SET_CONTROL, mcr_status);
			printf("hello_world_lxrt: modem control status = 0x%x.\n", mcr_status);
			rt_sleep(nano2count(PAUSE_TIME));
			printf("\nhello_world_lxrt: %d - SENT ON <rtser0>: >>%s<<.\n\n", i, hello);
			hello[0] = 0;
			printf("hello_world_lxrt: waiting to receive with timeout %llu (ns).\n", serconf.rx_timeout);
			rt_dev_read(rfd, hello, sizeof(hello) - 1);
			printf("\nhello_world_lxrt: %d - RECEIVED ON <rtser1>: >>%s<<.\n\n", i, hello);
		}
		rt_dev_close(sfd);
		rt_dev_close(rfd);
		printf("hello_world_lxrt: wait event (forced to be wrong) - 0x%x\n",
		rt_dev_ioctl(1000000, RTSER_RTIOC_WAIT_EVENT, NULL)
		);
		printf("hello_world_lxrt: rtser0 and rtser1 test finished\n");
	}    

	rt_make_soft_real_time();
	stop_rt_timer();
	rt_task_delete(testcomtsk);
	return 0;
}
