/*
 * Copyright (C) 2000-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *               2002  Robert Schwebel  <robert@schwebel.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <asm/io.h>

#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Latency measurement tool for RTAI");
MODULE_AUTHOR("Paolo Mantegazza <mantegazza@aero.polimi.it>, Robert Schwebel <robert@schwebel.de>");

/*
 *	command line parameters
 */

int overall = 1;
RTAI_MODULE_PARM(overall, int);
MODULE_PARM_DESC(overall,
		 "Calculate overall (1) or per-loop (0) statistics (default: 1)");

int period = 50000;
RTAI_MODULE_PARM(period, int);
MODULE_PARM_DESC(period, "period in ns (default: 100000)");

static int loops;
int avrgtime = 1;
RTAI_MODULE_PARM(avrgtime, int);
MODULE_PARM_DESC(avrgtime, "Averages are calculated for <avrgtime (s)> runs (default: 1)");

#define DEBUG_FIFO 3
#define RUNNABLE_ON_CPUS 1	// 1: on cpu 0 only, 2: on cpu 1 only, 3: on any;
#define RUN_ON_CPUS (num_online_cpus() > 1 ? RUNNABLE_ON_CPUS : 1)

/*
 *	Global Variables
 */

RT_TASK thread;
RTIME expected;
int period_counts;
struct sample {
	long long min;
	long long max;
	int index;
} samp;

static int cpu_used[RTAI_NR_CPUS];

#define MAXDIM 10
static double a[MAXDIM], b[MAXDIM];

static double dot(double *a, double *b, int n)
{
	int k; double s;
	for(k = n - 1, s = 0.0; k >= 0; k--) {
		s = s + a[k]*b[k];
	}
	return s;
}

/* 
 *	/proc/rtai/latency_calibrate entry
 */

#ifdef CONFIG_PROC_FS

extern struct proc_dir_entry *rtai_proc_root;

static int PROC_READ_FUN(rtai_proc_latency_read)
{
	PROC_PRINT_VARS;
	PROC_PRINT("\n## RTAI kernel latency (possible calibration tool) ##\n");
	PROC_PRINT("# period = %i (ns).\n", period);
	PROC_PRINT("# avrgtime = %i (s).\n", avrgtime);
	PROC_PRINT("# check %s worst case.\n", overall ? "overall" : "each average");
	PROC_PRINT("# timer_mode is oneshot.\n");
	PROC_PRINT("\n");
	PROC_PRINT_DONE;
}

PROC_READ_OPEN_OPS(rtai_latency_proc_fops, rtai_proc_latency_read);

static int rtai_proc_register(void)
{
	struct proc_dir_entry *rtai_latency_calibrate;

	rtai_latency_calibrate = CREATE_PROC_ENTRY("kern_latency", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root, &rtai_latency_proc_fops);
        if (!rtai_latency_calibrate) {
                printk(KERN_ERR "Unable to initialize /proc/kern_latency.\n");
                return -1;
        }
	SET_PROC_READ_ENTRY(rtai_latency_calibrate, rtai_proc_latency_read);

        return 0;
}

static void rtai_proc_unregister(void)
{
	remove_proc_entry("kern_latency", rtai_proc_root);
//        remove_proc_entry("rtai_kern_latency", NULL);
}

#endif


/*
 *	Periodic realtime thread 
 */
 
void fun(long dummy)
{
	int i;
	int min_diff = 0;
	int max_diff = 0;
	double refres;

	for(i = 0; i < MAXDIM; i++) {
		a[i] = b[i] = 3.141592;
	}
	/* If we want to make overall statistics */
	/* we have to reset min/max here         */
	if (overall) {
		min_diff =  1000000000;
		max_diff = -1000000000;
	}
#ifdef CONFIG_RTAI_FPU_SUPPORT
	if (thread.uses_fpu) {
		for(i = 0; i < MAXDIM; i++) {
			a[i] = b[i] = 3.141592;
		}
	}
#endif
	refres = dot(a, b, MAXDIM);
	while (1) {
		int i, diff, average;
		double dotres, tdif;
		/* Not overall statistics: reset min/max */
		if (!overall) {
			min_diff =  1000000000;
			max_diff = -1000000000;
		}

		average = 0;
		for (i = 0; i < loops; i++) {
			cpu_used[rtai_cpuid()]++;
			expected += period_counts;
			rt_task_wait_period();

			diff = (int) count2nano(rt_get_time() - expected);

			if (diff < min_diff) { min_diff = diff; }
			if (diff > max_diff) { max_diff = diff; }
			average += diff;
#ifdef CONFIG_RTAI_FPU_SUPPORT
			if (thread.uses_fpu) {
				dotres = dot(a, b, MAXDIM);
        	               	if ((tdif = dotres/refres - 1.0) < 0.0) {
                	       		tdif = -tdif;
				}
	                        if (tdif > 1.0e-16) {
					rt_printk("\nDOT PRODUCT ERROR.\n");
                		        return;
	                       	}
			}
#endif
		}
		samp.min = min_diff;
		samp.max = max_diff;
		samp.index = average / loops;
		rtf_put(DEBUG_FIFO, &samp, sizeof (samp));
	}
}


/*
 *	Initialisation. We have to select the scheduling mode and start 
 *      our periodical measurement task.  
 */

static int __latency_init(void)
{
	/* register a proc entry */
#ifdef CONFIG_PROC_FS
	 rtai_proc_register();
#endif

	rtf_create(DEBUG_FIFO, 16000);	/* create a fifo length: 16000 bytes */

	rt_task_init(			/* create our measuring task         */
			    &thread,	/* poiter to our RT_TASK             */
			    fun,	/* implementation of the task        */
			    0,		/* we could transfer data -> task    */
			    3000,	/* stack size                        */
			    0,		/* priority                          */
			    1,		/* do we use the FPU?                */
			    0		/* signal? XXX                       */
	);

	rt_set_runnable_on_cpus(	/* select on which CPUs the task is  */
		&thread,		/* allowed to run                    */
		RUN_ON_CPUS
	);

	period_counts = nano2count(period);

	loops = (1000000000*avrgtime)/period;

	/* Calculate the start time for the task. */
	/* We set this to "now plus 10 periods"   */
	expected = rt_get_time() + 10*period_counts;
	rt_task_make_periodic(&thread, expected, period_counts);
	return 0;
}


/*
 *	Cleanup 
 */

static void
__latency_exit(void)
{
	int cpuid;

	/* Now delete our task and remove the FIFO. */
	rt_task_delete(&thread);
	rtf_destroy(DEBUG_FIFO);

	/* Remove proc dir entry */
#ifdef CONFIG_PROC_FS
	rtai_proc_unregister();
#endif

	/* Output some statistics about CPU usage */
	printk("\n\nCPU USE SUMMARY\n");
	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		printk("# %d -> %d\n", cpuid, cpu_used[cpuid]);
	}
	printk("END OF CPU USE SUMMARY\n\n");

}

module_init(__latency_init);
module_exit(__latency_exit);
