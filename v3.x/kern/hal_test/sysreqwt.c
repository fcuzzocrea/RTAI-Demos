/*
COPYRIGHT (C) 2013-2017 Paolo Mantegazza (mantegazza@aero.polimi.it)

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

#define DIAGSRQ   0  // diagnose srq arg values
#define DIAGSLTMR 0  // diagnose if the soft LINUX timer is running
#define DIAGIPI   0  // diagnose if sched IPIs are received
#define DIAGHRTMR 0  // diagnose if the hard RTAI timer handler, intercepting the LINUX one, is running

#define LINUX_HZ_PERCENT 100 // !!! 1 to 100 !!!

#define LINUX_TIMER_FREQ ((HZ*LINUX_HZ_PERCENT)/100)

static int srq, ipi_count, tmr_count;

static DECLARE_MUTEX_LOCKED(sem);

static long long user_srq_handler(unsigned long req)
{
	int semret, cpyret;
	long long time;

#if DIAGSRQ
	printk("SRQ REQUEST %lu\n", req);
#endif
	switch (req) {
		case 1: {
			return (long long)(HZ/LINUX_TIMER_FREQ);
		}
		case 2: {
			return (long long)(HZ);
		}
		case 3: {
			return (long long)tmr_count;
		}
		case 4: {
#if DIAGIPI
			printk("SEND IPI FROM CPU: %d, AT TSCTIME: %lld\n", rtai_cpuid(), rtai_rdtsc());
#endif
			SEND_SCHED_IPI();
			return (long long)ipi_count;
		}
	}

	semret = down_interruptible(&sem);

// let's show how to communicate. Copy to and from user shall allow any kind of
// data interchange and service.
	time = rtai_llimd(rtai_rdtsc(), 1000000, RTAI_CLOCK_FREQ);
	cpyret = copy_to_user((long long *)req, &time, sizeof(long long));
	return time;
}

static void rtai_srq_handler(void)
{
	up(&sem);
}

static struct timer_list timer;

static void rt_soft_linux_timer_handler(unsigned long none)
{
#if DIAGSLTMR
	static int cnt[RTAI_NR_CPUS];
	int cpuid = rtai_cpuid();
	rt_printk("LINUX TIMER TICK: CPU %d, %d\n", cpuid, ++cnt[cpuid]);
#endif
	rt_pend_linux_srq(srq);
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
	return;
}

#if 0
extern void *rt_linux_hrt_next_shot;

int _rt_linux_hrt_next_shot(unsigned long deltat, void *hrt_dev)
{
	rtai_cli();
	rt_set_timer_delay(rtai_imuldiv(deltat, TIMER_FREQ, 1000000000));
	rtai_sti();
	return 0;
}
#endif

static void rt_rtai_timer_handler(int irq)
{
#if DIAGHRTMR
        int cpuid = rtai_cpuid();
        static int cnt[RTAI_NR_CPUS];
	printk("RECVD RTAI TIMER(s) IRQ %d AT CPU: %d, CNT: %d, AT TSCTIME: %lld\n", irq, cpuid, ++cnt[cpuid], rtai_rdtsc());
	if (irq == rtai_tunables.timer_irq) {
		printk("<<< CPU: %d, RECEIVED TIMER_IRQ: %d, COUNT: %d >>>\n", cpuid, irq, tmr_count);
	}
#endif
	update_linux_timer(cpuid);
	++tmr_count;
	return;
}

#ifdef CONFIG_SMP
static void sched_ipi_handler(void)
{
#if DIAGIPI
        static int cnt[RTAI_NR_CPUS];
        int cpuid = rtai_cpuid();
	printk("RECVD IPI AT CPU: %d, CNT: %d, AT TSCTIME: %lld\n", cpuid, ++cnt[cpuid], rtai_rdtsc());
#endif
	++ipi_count;
}
#endif

int init_module(void)
{
	srq = rt_request_srq(0xbeffa, rtai_srq_handler, user_srq_handler);
        init_timer(&timer);
        timer.function = rt_soft_linux_timer_handler;
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
#ifdef CONFIG_SMP
do {
	rt_request_irq(RTAI_RESCHED_IRQ, (void *)sched_ipi_handler, NULL, 0);
	rt_request_irq(rtai_tunables.timer_irq, (void *)rt_rtai_timer_handler, NULL, 0);
} while (0);
#else
	rt_request_timers((void *)rt_rtai_timer_handler);
#endif
	printk("TIMER_IRQ %d, LINUX TIMER IRQ %d, TIMER FREQ %lu.\n", rtai_tunables.timer_irq, rtai_tunables.linux_timer_irq, TIMER_FREQ);
        return 0;
}

void cleanup_module(void)
{
        del_timer(&timer);
	rt_free_srq(srq);
#ifdef CONFIG_SMP
	rt_release_irq(RTAI_RESCHED_IRQ);
	rt_release_irq(rtai_tunables.timer_irq);
#else
	rt_free_timers();
#endif
	printk("*** RESCHED IPIs: %d, RTAI-LINUX HARD IRQs %d ***\n", ipi_count, tmr_count);
}
