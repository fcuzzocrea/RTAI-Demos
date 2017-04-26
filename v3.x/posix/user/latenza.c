/*
COPYRIGHT (C) 2016  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <math.h>
#include <pthread.h>
#include <limits.h>

#define USA_RTAI 1

#if USA_RTAI
#include <rtai_posix.h>
#define IOPL(a)
#else
#define start_rt_timer(a)
#define stop_rt_timer()
#define pthread_setschedparam_np(a, b, c, d, e)
#define IOPL(a) do { iopl(3); } while(0)
#endif

#define AVRGTIME    1      // s                      
#define PERIOD      100000 // ns
#define GIGA        (1000000000LL)
#define SMPLSXAVRG ((GIGA*AVRGTIME)/PERIOD)

#define MAXDIM 1000
static double a[MAXDIM], b[MAXDIM];
static double dot(double *a, double *b, int n)
{
	int k = n - 1;
	double s = 0.0;
	for(; k >= 0; k--) {
		s = s + a[k]*b[k];
	}
	return s;
}

#define MAXIO 0
static void do_some_io(int val)
{
	int k = MAXIO;
	for(; k >= 0; k--) outb_p(val, 0x378);
}

static int end = 0;
static void endme(int sig)
{
	end = 1;
}


static long long clock_gettime_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return  (long long)ts.tv_sec*GIGA + (long long)ts.tv_nsec;
}
 
static void ns2ts(long long ns, struct timespec *ts)
{
	ts->tv_sec = ns/GIGA;
	ts->tv_nsec = ns - ts->tv_sec*GIGA;
}

static struct sample { long min, max, avrg; } samp;

void *display(void *arg)
{
	struct sched_param param = {.sched_priority = 98 };
	long max = -GIGA, min = GIGA;
	struct timespec tpausa;

	signal(SIGINT,  endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);

	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
	pthread_setschedparam_np(1, SCHED_FIFO, 0, 0xF, PTHREAD_SOFT_REAL_TIME_NP);

	while (!end) {
		if (max < samp.max) max = samp.max;
		if (min > samp.min) min = samp.min;
		printf("* min/minall: %ld/%ld (ns), max/maxall: %ld/%ld (ns), average: %ld (ns). <CTRL-C per finire> *\n", samp.min, min, samp.max, max, samp.avrg);
		ns2ts(AVRGTIME*GIGA, &tpausa);
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tpausa, NULL);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int i,  sample;
	long diff, min_diff, max_diff;
	long average;
	long long until_ns;
	struct timespec until_ts;
	double s;
	pthread_t dispthr;
	pthread_attr_t dispattr;
	struct sched_param param = {.sched_priority = 99 };

	signal(SIGINT,  endme);
	signal(SIGKILL, endme);
	signal(SIGTERM, endme);
	IOPL(3);

	start_rt_timer(0);

	pthread_attr_init(&dispattr);
	pthread_attr_setschedpolicy(&dispattr, SCHED_FIFO);
	pthread_attr_setstacksize(&dispattr, 10*PTHREAD_STACK_MIN);
	pthread_create(&dispthr, &dispattr, display, NULL);

	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

	printf("\n*** latency calibration tool ***\n");
	printf("# period = %i (ns) \n", PERIOD);
	printf("# average time = %i (s)\n\n", (int)AVRGTIME);

        for(i = 0; i < MAXDIM; i++) {
                a[i] = b[i] = 3.141592;
        }
	mlockall(MCL_CURRENT | MCL_FUTURE);

	pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME_NP);
	until_ns = clock_gettime_ns();

	while (!end) {
		min_diff =  GIGA;
		max_diff = -GIGA;
		average = 0;
		for (sample = 0; sample < SMPLSXAVRG && !end; sample++) {
			until_ns += PERIOD;
			ns2ts(until_ns, &until_ts);
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &until_ts, NULL);
			diff = clock_gettime_ns() - until_ns;
			if (diff < min_diff) {
				min_diff = diff;
			}
			if (diff > max_diff) {
				max_diff = diff;
			}
			average += diff;
			s = dot(a, b, MAXDIM);
			do_some_io(s > 0.0 ? 1 : 0);
		}
		samp.min  = min_diff;
		samp.max  = max_diff;
		samp.avrg = average/SMPLSXAVRG;
	}

	stop_rt_timer();	

	return 0;
}
