/*
COPYRIGHT (C) 2013  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <sys/poll.h>

#include <asm/rtai_srq.h>

#define PRINT_PERCENT_OF_HZ 20 // !!! 1 to 100 !!!

static volatile int end;

static void endme(int dummy) { end = 1; }

#define INT 7

#define RTAI_DO_INT(i)  do { __asm__ __volatile__ ( __rtai_do_trap(i)); } while (0)

int main(void)
{
	struct pollfd kbrd = { 0, POLLIN };
	int srq, jtick;
	int tcount = 0, tnextcount, trepeat, ipi_count = 0, tmr_count = 0;
	struct sched_param mysched;
	long long time0, time, dt;

//	RTAI_DO_INT(INT);
//	__asm__ __volatile__ ("int $7"); 

	signal (SIGINT, endme);
	signal (SIGTERM, endme);

	mysched.sched_priority = 99;

	if( sched_setscheduler( 0, SCHED_FIFO, &mysched ) == -1 ) {
		puts(" ERROR IN SETTING THE SCHEDULER UP");
		perror( "errno" );
		exit( 0 );
 	}       

	srq = rtai_open_srq(0xbeffa);
	rtai_srq(srq, (unsigned long)&time0);
	jtick    = rtai_srq(srq, 1);
	tnextcount = trepeat  = (rtai_srq(srq, 2)*PRINT_PERCENT_OF_HZ)/100;
	dt = time0;

	while (!end) {
		tmr_count = rtai_srq(srq, 3);
		ipi_count = rtai_srq(srq, 4);
		rtai_srq(srq, (unsigned long)&time);
		tcount += jtick;
		if (tcount > tnextcount) {
			tnextcount += trepeat;
			printf("# %d > JIFFIES TICK: %d, TIME: %lld (us), TIME DIFF: %lld (us);\n", tcount - 1, jtick, time - time0, time - dt);
			dt = time;
			if (ipi_count) {
				printf("SCHED IPIs %d.\n", ipi_count);
			}
			if (tmr_count) {
				printf("RTAI-LINUX TIMERs IRQs %d.\n", tmr_count + trepeat);
			}
		}
		if (poll(&kbrd, 1, 0)) {
			break;
		}
	}
	return 0;
}
