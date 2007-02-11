/*
COPYRIGHT (C) 1999-2007 Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#define USE_APIC 0
#define TICK 100000 //ns (!!! CAREFULL NEVER BELOW HZ IF USE_APIC == 0 !!!)

#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <asm/rtai.h>
#include <rtai_sched.h>

MODULE_LICENSE("GPL");

static int srq;

static DECLARE_MUTEX_LOCKED(sem);

static long long user_srq_handler(unsigned long whatever)
{
	long long time;

	if (whatever == 1) {
		return (long long)(TICK/1000);
	}
	down_interruptible(&sem);
	time = llimd(rtai_rdtsc(), 1000000, CPU_FREQ);
// let's show how to communicate. Copy to and from user shall allow any kind of
// data interchange and service.
	copy_to_user((long long *)whatever, &time, 8);
	return time;
}

static void rtai_srq_handler(void)
{
	up(&sem);
}

static void rt_timer_tick(void)
{
	int cpuid = rtai_cpuid();

#if 0 // diagnose to see if interrupts are coming in
	static int cnt[NR_RT_CPUS];
	printk("TIMER TICK: CPU %d, %d\n", cpuid, ++cnt[cpuid]);
#endif

#ifdef CONFIG_SMP
	if (!cpuid)
#endif
	rt_pend_linux_srq(srq);

#if !USE_APIC
	rt_times.tick_time = rt_times.intr_time;
	rt_times.intr_time = rt_times.tick_time + rt_times.periodic_tick;
	rt_set_timer_delay(0);
	if (rt_times.tick_time >= rt_times.linux_time) {
		rt_times.linux_time += rt_times.linux_tick;
		rt_pend_linux_irq(TIMER_8254_IRQ);
	} 
#endif
	return;
}

int init_module(void)
{
	srq = rt_request_srq(0xcacca, rtai_srq_handler, user_srq_handler);
#if USE_APIC && defined(CONFIG_SMP)
	do {
		int cpuid;
		struct apic_timer_setup_data setup_data[NR_RT_CPUS];
		for (cpuid = 0; cpuid < NR_RT_CPUS; cpuid++) {
			setup_data[cpuid].mode  = 1;
			setup_data[cpuid].count = TICK;
		}
		rt_request_apic_timers(rt_timer_tick, setup_data);
	} while (0);
#else
	rt_request_timer(rt_timer_tick, imuldiv(TICK, USE_APIC ? FREQ_APIC : FREQ_8254, 1000000000), USE_APIC); 
#endif
	return 0;
}

void cleanup_module(void)
{
#if USE_APIC && defined(CONFIG_SMP)
	rt_free_apic_timers();
#else
	rt_free_timer();
#endif
	rt_free_srq(srq);
}
