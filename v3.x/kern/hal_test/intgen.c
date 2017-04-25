/*
COPYRIGHT (C) 2017 Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/semaphore.h>
#include <asm/uaccess.h>

#include <asm/rtai.h>
#include <rtai_schedcore.h>
#include "sched_ipi.h"

MODULE_LICENSE("GPL");

#define DIAGTMR 0  // diagnose if the LINUX timer is running
#define DIAGIPI 0  // diagnose if sched IPIs are received

#define LINUX_HZ_PERCENT 100 // !!! 1 to 100 !!!

#define LINUX_TIMER_FREQ ((HZ*LINUX_HZ_PERCENT)/100)

static int ipi_count, ipi_echo, ipi_sample = 5*HZ;

RTIME ts;

long min = 1000000, max = -1000000, avrg;

static struct timer_list timer;

static void timer_handler(unsigned long none)
{
#if DIAGTMR
	int cpuid = rtai_cpuid();
	static int cnt[RTAI_NR_CPUS];
	rt_printk("TIMER TICK: CPU %d, %d\n", cpuid, ++cnt[cpuid]);
#endif
	rtai_cli();
	ts = rtai_rdtsc();
	SEND_SCHED_IPI();
	rtai_sti();
	if (ipi_count > (ipi_echo + ipi_sample)) {
		long lmin, lmax, lavrg;
		ipi_echo = ipi_count;
		lmin = rtai_llimd(min, 1000000000, rtai_tunables.clock_freq);
		lmax = rtai_llimd(max, 1000000000, rtai_tunables.clock_freq);
		lavrg = rtai_llimd(avrg/ipi_count, 1000000000, rtai_tunables.clock_freq);
		printk("*** RESCHED IPIs: %d, MIN %ld, MAX %ld, AVRG %ld ***\n", ipi_count, lmin, lmax, lavrg);
	}
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
	return;
}

static void sched_ipi_handler(void)
{
	long diff;
#if DIAGIPI
        static int cnt[RTAI_NR_CPUS];
        int cpuid = rtai_cpuid();
	printk("RECVD IPI AT CPU: %d, CNT: %d, AT TSCTIME: %lld\n", cpuid, ++cnt[cpuid], rtai_rdtsc());
#endif
	++ipi_count;
	diff = rtai_rdtsc() - ts;
	if (diff > max) {
		max = diff;
	} else if (diff < min) {
		min = diff;
	}
	avrg += diff;	
}

int init_module(void)
{
        init_timer(&timer);
        timer.function = timer_handler;
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
	rt_request_irq(RTAI_RESCHED_IRQ, (void *)sched_ipi_handler, NULL, 0);
	printk("*** RESCHED IRQ %d ***\n", RTAI_RESCHED_IRQ);
        return 0;
}

void cleanup_module(void)
{
        del_timer(&timer);
	rt_release_irq(RTAI_RESCHED_IRQ);
	min = rtai_llimd(min, 1000000000, rtai_tunables.clock_freq);
	max = rtai_llimd(max, 1000000000, rtai_tunables.clock_freq);
	avrg = rtai_llimd(avrg/ipi_count, 1000000000, rtai_tunables.clock_freq);
	printk("*** RESCHED IPIs: %d, MIN %ld, MAX %ld, AVRG %ld ***\n", ipi_count, min, max, avrg);
}
