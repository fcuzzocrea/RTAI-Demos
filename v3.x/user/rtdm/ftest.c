/*
Original example:
	 COPYRIGHT (C) 2002  Giuseppe Renoldi (giuseppe@renoldi.org)
Adaption to RTAI_TRIOSS using RTDM:
	COPYRIGHT (C) 2005  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
 * rtai-16550A test
 * ================
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

#include "/usr/xenomai/include/rtdm/rtdm.h"
#include "/usr/xenomai/include/rtdm/rtserial.h"

#include "rtai_fusion.h"
#include <rtai_lxrt.h>
#include <rtai_sem.h>

#define PRINT rt_printk

#define LOOPS  1000
#define TRX_TIMEOUT  1000000
#define BAUD_RATE    115200 //RTSER_115200_BAUD
#define PAUSE_TIME   100000
#define FTICK_FREQ   8192

static int sfd, rfd;

int main(void)
{
	RT_TASK *testcomtsk;
	FTASK ftask;
	char hello[] = "Hello World\n\r";
	rtser_config_t serconf;
	struct rtser_status status;
	int mcr_status, i;
	
	ftask_shadow(&ftask, "TESTCOM", 1, 0);
 	if (!(testcomtsk = ftask_init(nam2num("TESTCOM"), 1))) {
		printf("CANNOT INIT MASTER TASK\n");
		exit(1);
	}
	ftimer_start((1000000000 + FTICK_FREQ/2)/FTICK_FREQ);
	start_ftimer(0, FTICK_FREQ);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	if ((sfd = rt_dev_open("rtser0", O_RDWR)) < 0 || (rfd = rt_dev_open("rtser1", O_RDWR)) < 0) {
		PRINT("hello_world_lxrt: error in rt_dev_open()\n");
		return 1;
	} else {	
// GET_CONFIGs not needed, as the duplicated initializations, just for testing.
		rt_dev_ioctl(sfd, RTSER_RTIOC_GET_CONFIG, &serconf);
		serconf.config_mask = RTSER_SET_BAUD | RTSER_SET_TIMEOUT_TX;
		serconf.rx_timeout  = TRX_TIMEOUT;
		serconf.baud_rate   = BAUD_RATE;
		rt_dev_ioctl(sfd, RTSER_RTIOC_SET_CONFIG, &serconf);
		rt_dev_ioctl(rfd, RTSER_RTIOC_GET_CONFIG, &serconf);
		serconf.config_mask = RTSER_SET_BAUD | RTSER_SET_TIMEOUT_RX;
		serconf.rx_timeout  = TRX_TIMEOUT;
		serconf.baud_rate   = BAUD_RATE;
		rt_dev_ioctl(rfd, RTSER_RTIOC_SET_CONFIG, &serconf);
		for (i = 1; i <= LOOPS; i++) {
			strcpy(hello, "Hello World\n\r");
			rt_dev_write(sfd, hello, sizeof(hello) - 1);
			rt_dev_ioctl(sfd, RTSER_RTIOC_GET_STATUS, &status);
			PRINT("hello_world_lxrt: line status = 0x%x, modem status = 0x%x.\n", status.line_status, status.modem_status);
			rt_dev_ioctl(sfd, RTSER_RTIOC_GET_CONTROL, &mcr_status);
			rt_dev_ioctl(sfd, RTSER_RTIOC_SET_CONTROL, mcr_status);
			PRINT("hello_world_lxrt: modem control status = 0x%x.\n", mcr_status);
			rt_sleep(nano2count(PAUSE_TIME));
			PRINT("\nhello_world_lxrt: %d - SENT ON <rtser0>: >>%s<<.\n\n", i, hello);
			hello[0] = 0;
			PRINT("hello_world_lxrt: waiting to receive with timeout %llu (ns).\n", serconf.rx_timeout);
			rt_dev_read(rfd, hello, sizeof(hello) - 1);
			PRINT("\nhello_world_lxrt: %d - RECEIVED ON <rtser1>: >>%s<<.\n\n", i, hello);
		}
		rt_sleep(nano2count(PAUSE_TIME));
		PRINT("hello_world_lxrt: let's help letting the booby become soft\n");
		ftask_make_soft_real_time();
		rt_dev_close(sfd);
		rt_dev_close(rfd);
		PRINT("hello_world_lxrt: wait event (forced to be wrong) - 0x%x\n",
		rt_dev_ioctl(1000000, RTSER_RTIOC_WAIT_EVENT, NULL)
		);
		PRINT("hello_world_lxrt: rtser0 and rtser1 test finished\n");
	}    

	stop_ftimer();
	return 0;
}
