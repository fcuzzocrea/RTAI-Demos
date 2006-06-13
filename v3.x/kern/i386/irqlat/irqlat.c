/*
 * Copyright (C) 2006 Paolo Mantegazza <mantegazza@aero.polimi.it>.
 *
 * RTAI/fusion is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * RTAI/fusion is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTAI/fusion; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/module.h>

#include <rtai.h>
#include <asm/rtai_lxrt.h>

/*+++++++++++++++++++++ OUR LITTLE STAND ALONE LIBRARY +++++++++++++++++++++++*/

#ifndef STR
#define __STR(x) #x
#define STR(x) __STR(x)
#endif

#ifndef SYMBOL_NAME_STR
#define SYMBOL_NAME_STR(X) #X
#endif

static void asm_handler (void)
{
    __asm__ __volatile__ ( \
	"cld\n\t" \
        "pushl %es\n\t" \
        "pushl %ds\n\t" \
        "pushl %eax\n\t" \
        "pushl %ebp\n\t" \
	"pushl %edi\n\t" \
        "pushl %esi\n\t" \
        "pushl %edx\n\t" \
        "pushl %ecx\n\t" \
	"pushl %ebx\n\t" \
	__LXRT_GET_DATASEG(ebx) \
        "movl %ebx, %ds\n\t" \
        "movl %ebx, %es\n\t" \
        "call "SYMBOL_NAME_STR(c_handler)"\n\t" \
        "popl %ebx\n\t" \
        "popl %ecx\n\t" \
        "popl %edx\n\t" \
        "popl %esi\n\t" \
	"popl %edi\n\t" \
        "popl %ebp\n\t" \
        "popl %eax\n\t" \
        "popl %ds\n\t" \
        "popl %es\n\t" \
        "iret");
}

struct desc_struct rtai_set_gate_vector (unsigned vector, int type, int dpl, void *handler)
{
	struct desc_struct e = idt_table[vector];
	idt_table[vector].a = (__KERNEL_CS << 16) | ((unsigned)handler & 0x0000FFFF);
	idt_table[vector].b = ((unsigned)handler & 0xFFFF0000) | (0x8000 + (dpl << 13) + (type << 8));
	return e;
}

void rtai_reset_gate_vector (unsigned vector, struct desc_struct e)
{
	idt_table[vector] = e;
}

/*++++++++++++++++++ END OF OUR LITTLE STAND ALONE LIBRARY +++++++++++++++++++*/

#define IRQ 0

int PERIOD = 100000; // nanos
MODULE_PARM(PERIOD,"i");

#ifdef CONFIG_SMP
static int vector = 0x31; // SMPwise it is likely 0x31
#else
static int vector = 0x20; //  UPwise it is likely 0x20
#endif

static RTIME t0;
static int bus_period, tick, maxj, echo;
static volatile int cnt;

void c_handler (void)
{
	RTIME t;
	int jit;

	hal_root_domain->irqs[IRQ].acknowledge(IRQ);
	if (cnt) {
		t = rdtsc();
		if ((jit = abs((int)(t - t0) - bus_period)) > maxj) {
			maxj = jit;
		}
		t0 = t;
		rt_times.tick_time = rt_times.intr_time;
		rt_times.intr_time = rt_times.tick_time + rt_times.periodic_tick;
		if (rt_times.tick_time >= rt_times.linux_time) {
			rt_times.linux_time += rt_times.linux_tick;
			rt_pend_linux_irq(IRQ);
		} 
	} else {
		cnt = 1;
		t0 = rdtsc();
	}
}

static struct desc_struct desc;

int ECHO_PERIOD = 1000; // ms
MODULE_PARM(ECHO_PERIOD,"i");
static struct timer_list timer;

static void timer_fun(unsigned long none)
{
	int t;
	if (echo < maxj) {
		echo = maxj;
	}
	t = imuldiv(echo, 1000000000, rtai_tunables.cpu_freq);
	printk("INCREASED TO: %d.%-3d (us)\n", t/1000, t%1000);
	mod_timer(&timer, jiffies + ECHO_PERIOD*HZ/1000);
}

int _init_module(void)
{
	unsigned long flags;
	init_timer(&timer);
	timer.function = timer_fun;
	mod_timer(&timer, jiffies + ECHO_PERIOD*HZ/1000);
	printk("\nCHECKING WITH PERIOD: %d (us)\n\n", PERIOD/1000);
	bus_period = imuldiv(PERIOD, rtai_tunables.cpu_freq, 1000000000);
	tick = imuldiv(PERIOD, FREQ_8254, 1000000000);
	rt_times.linux_tick = LATCH;
	rt_times.tick_time = ((RTIME)rt_times.linux_tick)*(jiffies + 1);
	rt_times.intr_time = rt_times.tick_time + tick;
	rt_times.linux_time = rt_times.tick_time + rt_times.linux_tick;
	rt_times.periodic_tick = tick;
	flags = hal_critical_enter(NULL);
	outb(0x34, 0x43);
	outb(tick & 0xff, 0x40);
	outb(tick >> 8, 0x40);
	desc = rtai_set_gate_vector(vector, 14, 0, asm_handler);
	hal_critical_exit(flags);
	return 0;
}

void _cleanup_module(void)
{
	unsigned long flags;
	int t;

	del_timer(&timer);
	flags = hal_critical_enter(NULL);
	outb(0x34, 0x43);
	outb(LATCH & 0xff, 0x40);
	outb(LATCH >> 8,0x40);
	rtai_reset_gate_vector(vector, desc);
	hal_critical_exit(flags);
	t = imuldiv(echo, 1000000000, rtai_tunables.cpu_freq);
	printk("\nCHECKED WITH PERIOD: %d (us), MAXJ: %d.%-3d (us)\n", PERIOD/1000, t/1000, t%1000);
	return;
}

module_init(_init_module);
module_exit(_cleanup_module);
