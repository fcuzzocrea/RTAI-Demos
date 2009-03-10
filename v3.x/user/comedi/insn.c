/*
COPYRIGHT (C) 2009  Edoardo Vigoni   (vigoni@aero.polimi.it)
COPYRIGHT (C) 2009  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include <rtai_comedi.h>

#define ONECALL    1
#define TIMEDCALL  1

#define TIMEOUT  100000000

#define NICHAN  5
#define NOCHAN  2
#define NCHAN   (NICHAN + NICHAN)

#define SAMP_FREQ  10000
#define RUN_TIME   5

#define AI_RANGE  0
#define SAMP_TIME  (1000000000/SAMP_FREQ)
static comedi_t *dev;
static int subdev;
static comedi_krange krange;
static lsampl_t maxdata;

static int init_board(void)
{
	dev = comedi_open("/dev/comedi1");		
	printf("Comedi device (6071) handle: %p.\n", dev);
	if (!dev){
		printf("Unable to open (6071) %s.\n", "/dev/comedi1");
		return 1;
	}
	subdev = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_AI, 0);
	if (subdev < 0) {
		comedi_close(dev);
		printf("Subdev (6071) %d not found.\n", COMEDI_SUBD_AI);
		return 1;
	}
	comedi_get_krange(dev, subdev, 0, AI_RANGE, &krange);
	maxdata = comedi_get_maxdata(dev, subdev, 0);
	return 0;
}

static volatile int end;
void endme(int sig) { end = 1; }

int main(void)
{
	RT_TASK *task;
	comedi_insn insn[NCHAN];
        unsigned int read_chan[NICHAN] = { 2, 3, 4, 5, 6 };
	comedi_insnlist ilist = { NCHAN, insn };
	lsampl_t *hist;
	lsampl_t data[NCHAN];
	unsigned long i, k, n, retval;
	FILE *fp;

	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	hist = malloc(SAMP_FREQ*RUN_TIME*NCHAN*sizeof(lsampl_t) + 1000);
	memset(hist, 0, SAMP_FREQ*RUN_TIME*NCHAN*sizeof(lsampl_t) + 1000);

	start_rt_timer(0);
	task = rt_task_init_schmod(nam2num("MYTASK"), 1, 0, 0, SCHED_FIFO, 0xF);
	printf("COMEDI INSNLIST TEST BEGINS: SAMPLING FREQ: %d, RUN TIME: %d.\n", SAMP_FREQ, RUN_TIME);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();

	if (init_board()) {;
		printf("Board initialization failed.\n");
		return 1;
	}

        for (i = 0; i < NCHAN; i++) {
		insn[i].insn     = INSN_READ;
		insn[i].n        = 1;
	        insn[i].data     = &data[i];
		insn[i].subdev   = subdev;
		insn[i].chanspec = CR_PACK(read_chan[i], AI_RANGE, AREF_GROUND);
        }
	insn[NICHAN].insn = insn[NICHAN + 1].insn = INSN_READ;

	for (n = k = 0; k < SAMP_FREQ*RUN_TIME && !end; k++) {
		if ((retval = rt_comedi_do_insnlist(dev, &ilist)) < NCHAN) {
			for (i = 0; i < NCHAN; i++) {
				 hist[n++] = data[i];
			}
		} else {
			printf("Comedi insnlist processed only %lu out of %d.\n", retval, NCHAN);
			break;
		}
	}

	comedi_cancel(dev, subdev);
	comedi_close(dev);
	printf("COMEDI INSNLIST ENDS.\n");

	fp = fopen("rec.dat", "w");
	for (n = k = 0; k < SAMP_FREQ*RUN_TIME; k++) {
		for (i = 0; i < NCHAN; i++) {
			fprintf(fp, "%d\t", hist[n++]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	free(hist);

	stop_rt_timer();
	rt_make_soft_real_time();
	rt_task_delete(task);

	return 0;
}
