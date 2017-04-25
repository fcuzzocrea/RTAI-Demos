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

MODULE_LICENSE("GPL");

#define DIAGSRQ   0  // diagnose srq arg values
#define DIAGSLTMR 0  // diagnose if the soft LINUX timer is running
#define DIAGIPI   0  // diagnose if sched IPIs are received
#define DIAGHLTMR 0  // diagnose if the hard RTAI timer handler, intercepting the LINUX one, is running

#define LINUX_HZ_PERCENT 100 // !!! 1 to 100 !!!

#define LINUX_TIMER_FREQ ((HZ*LINUX_HZ_PERCENT)/100)

static int srq, ipi_count, ltmr_count;

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
			return (long long)ltmr_count;
		}
		case 4: {
#ifdef CONFIG_SMP
			int cpu, thiscpu = rtai_cpuid();
#if DIAGIPI
			printk("SEND IPI FROM CPU: %d, TO CPU: %d, AT TSCTIME: %lld\n", cpuid, cpuid ? 0 : 1, rtai_rdtsc());
#endif
			for (cpu = 0; cpu < RTAI_NR_CPUS; cpu++) {
				if (cpu != thiscpu) {
		                	rtai_cli();
			                send_sched_ipi(1 << cpu);
        			        rtai_sti();
				}
			}
#endif
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
	static int cnt[NR_RT_CPUS];
	int cpuid = rtai_cpuid();
	rt_printk("LINUX TIMER TICK: CPU %d, %d\n", cpuid, ++cnt[cpuid]);
#endif
	rt_pend_linux_srq(srq);
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
	return;
}

static void rt_hard_linux_timer_handler(int irq)
{
#if DIAGHLTMR
        int cpuid = rtai_cpuid();
        static int cnt[NR_RT_CPUS];
	printk("RECVD LINUX IRQ AT CPU: %d, CNT: %d, AT TSCTIME: %lld\n", cpuid, ++cnt[cpuid], rtai_rdtsc());
#endif
#if 0
	hal_pend_uncond(irq, cpuid);
#else
	update_linux_timer(cpuid);
#endif
	++ltmr_count;
}

static void sched_ipi_handler(void)
{
#if DIAGIPI
        static int cnt[NR_RT_CPUS];
        int cpuid = rtai_cpuid();
	printk("RECVD IPI AT CPU: %d, CNT: %d, AT TSCTIME: %lld\n", cpuid, ++cnt[cpuid], rtai_rdtsc());
#endif
	++ipi_count;
}

int init_module(void)
{
	srq = rt_request_srq(0xbeffa, rtai_srq_handler, user_srq_handler);
        init_timer(&timer);
        timer.function = rt_soft_linux_timer_handler;
	mod_timer(&timer, jiffies + (HZ/LINUX_TIMER_FREQ));
	rt_request_irq(rtai_tunables.linux_timer_irq, (void *)rt_hard_linux_timer_handler, NULL, 0);
#ifdef CONFIG_SMP
	rt_request_irq(RTAI_RESCHED_IRQ, (void *)sched_ipi_handler, NULL, 0);
#endif
	printk("TIMER_IRQ %d, LINUX TIMER IRQ %d, TIMER FREQ %lu.\n", rtai_tunables.timer_irq, rtai_tunables.linux_timer_irq, TIMER_FREQ);
        return 0;
}

void cleanup_module(void)
{
	rt_release_irq(rtai_tunables.linux_timer_irq);
        del_timer(&timer);
	rt_free_srq(srq);
#ifdef CONFIG_SMP
	rt_release_irq(RTAI_RESCHED_IRQ);
#endif
	printk("*** RESCHED IPIs: %d, RTAI-LINUX HARD IRQs %d ***\n", ipi_count, ltmr_count);
}
