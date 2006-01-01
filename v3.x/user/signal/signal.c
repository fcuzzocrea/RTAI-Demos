/*
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#include <linux/kernel.h>
#include <linux/module.h>

#include <rtai_schedcore.h>
#include "rtai_signal.h"

MODULE_LICENSE("GPL");

#define MODULE_NAME "RTAI_SIGNALS"

#undef  SIGNAL
#define SIGNAL ((struct rt_signal_t *)task->rt_signals)
struct rt_signal_t { unsigned long flags; RT_TASK *sigtask; };

int _rt_request_signal(RT_TASK *sigtask, RT_TASK *task, int signal)
{
	int retval;
	if (signal >= 0 && sigtask && task) {
		if (!task->rt_signals) {
			task->rt_signals = rt_malloc(MAXSIGNALS*sizeof(struct rt_signal_t));
			task->pstate = 0;
		}
		SIGNAL[signal].flags = (1 << SIGNAL_ENBIT);
		SIGNAL[signal].sigtask = sigtask;
		retval = 0;
	} else {
		retval = -EINVAL;
	}
	task->retval = retval;
	rt_task_resume(task);
	return retval;
}
EXPORT_SYMBOL(_rt_request_signal);

static inline void rt_exec_signal(RT_TASK *sigtask, RT_TASK *task)
{
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	if (!(--sigtask->suspdepth)) {
		if (task) {
			sigtask->priority = task->priority; 
 			if (!task->pstate++) {
				rem_ready_task(task);
				task->state |= RT_SCHED_SIGSUSP;
			}
		}
		sigtask->state &= ~RT_SCHED_SIGSUSP;
		sigtask->retval = (int)task;
		enq_ready_task(sigtask);
		RT_SCHEDULE(sigtask, rtai_cpuid());
	}
	rt_global_restore_flags(flags);
}

int rt_release_signal(int signal, RT_TASK *task)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if (signal >= 0 && SIGNAL && SIGNAL[signal].sigtask) {
		SIGNAL[signal].sigtask->priority = task->priority; 
		rt_exec_signal(SIGNAL[signal].sigtask, 0);
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(rt_release_signal);

void rt_send_signal(int signal, RT_TASK *task)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if (signal >= 0 && SIGNAL && SIGNAL[signal].sigtask) {
		do {
			if (test_and_clear_bit(SIGNAL_ENBIT, &SIGNAL[signal].flags)) {
				rt_exec_signal(SIGNAL[signal].sigtask, task);
				test_and_set_bit(SIGNAL_ENBIT, &SIGNAL[signal].flags);
			} else {
				test_and_set_bit(SIGNAL_PNDBIT, &SIGNAL[signal].flags);
				break;
			}
		} while (test_and_clear_bit(SIGNAL_PNDBIT, &SIGNAL[signal].flags));
	}
}
EXPORT_SYMBOL(rt_send_signal);

void rt_enable_signal(int signal, RT_TASK *task)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if (signal >= 0 && SIGNAL) {
		set_bit(SIGNAL_ENBIT, &SIGNAL[signal].flags);
	}
}
EXPORT_SYMBOL(rt_enable_signal);

void rt_disable_signal(int signal, RT_TASK *task)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if (signal >= 0 && SIGNAL) {
		clear_bit(SIGNAL_ENBIT, &SIGNAL[signal].flags);
	}
}
EXPORT_SYMBOL(rt_disable_signal);

static int rt_sync_sigreq(RT_TASK *task)
{
	rt_task_suspend(task);
	return task->retval;
}

static unsigned long long rt_signal_info(void)
{
	union rtai_lxrt_t retval;
	retval.v[HIGH] = RT_CURRENT;
	retval.i[LOW] = ((RT_TASK *)retval.v[HIGH])->runnable_on_cpus;
	return retval.rt;
}

int rt_wait_signal(RT_TASK *sigtask, RT_TASK *task)
{
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	if (!sigtask->suspdepth++) {
		sigtask->state |= RT_SCHED_SIGSUSP;
		rem_ready_current(sigtask);
		if (task->pstate > 0 && !(--task->pstate) && (task->state &= ~RT_SCHED_SIGSUSP) == RT_SCHED_READY) {
                       	enq_ready_task(task);
       		}
		rt_schedule();
	}
	rt_global_restore_flags(flags);
	return sigtask->retval;
}
EXPORT_SYMBOL(rt_wait_signal);

static struct rt_fun_entry rtai_signals_fun[] = {
	[SIGNAL_SYNCREQ] = { 1, rt_sync_sigreq },   // internal, not for users
	[SIGNAL_INFO]    = { 1, rt_signal_info },   // internal, not for users
	[SIGNAL_WAITSIG] = { 1, rt_wait_signal },   // internal, not for users
	[SIGNAL_REQUEST] = { 1, _rt_request_signal },
	[SIGNAL_RELEASE] = { 1, rt_release_signal },
	[SIGNAL_ENABLE]  = { 1, rt_enable_signal },
	[SIGNAL_DISABLE] = { 1, rt_disable_signal },
	[SIGNAL_SEND]    = { 1, rt_send_signal }
};

int init_module(void)
{
	if (set_rt_fun_ext_index(rtai_signals_fun, RTAI_SIGNALS_IDX)) {
		printk("Wrong index module for lxrt: %d.\n", RTAI_SIGNALS_IDX);
		return -EACCES;
	}
	printk("%s: loaded.\n", MODULE_NAME);
	return 0;
}

void cleanup_module(void)
{
	reset_rt_fun_ext_index(rtai_signals_fun, RTAI_SIGNALS_IDX);
	printk("%s: unloaded.\n", MODULE_NAME);
}
