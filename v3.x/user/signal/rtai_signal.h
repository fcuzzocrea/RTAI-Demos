/*
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#ifndef _RTAI_SIGNAL_H_
#define _RTAI_SIGNAL_H_

#define RTAI_SIGNALS_IDX  2

#define SIGNAL_SYNCREQ  0
#define SIGNAL_INFO     1
#define SIGNAL_WAITSIG  2
#define SIGNAL_REQUEST  3
#define SIGNAL_RELEASE  4 
#define SIGNAL_ENABLE   5
#define SIGNAL_DISABLE  6
#define SIGNAL_SEND     7

#define SIGNAL_TASK_INIPRIO     0

struct sigsuprt_t { RT_TASK *sigtask; RT_TASK *task; int signal; void (*sighdl)(int, RT_TASK *); unsigned long runnable_on; };

#ifdef __KERNEL__

#define MAXSIGNALS  8

#define SIGNAL_ENBIT   0
#define SIGNAL_PNDBIT  1

#define SIGNAL_TASK_STACK_SIZE  4096
 
#ifndef __SIGNAL_SUPPORT_FUN__
#define __SIGNAL_SUPPORT_FUN__

int _rt_request_signal(RT_TASK *sigtask, RT_TASK *task, int signal);

int rt_wait_signal(RT_TASK *sigtask, RT_TASK *task);

static void signal_suprt_fun(struct sigsuprt_t *funarg)
{		
	struct sigsuprt_t arg = *funarg;

	arg.sigtask = rt_whoami();
	if (!_rt_request_signal(arg.sigtask, arg.task, arg.signal)) {
		while (rt_wait_signal(arg.sigtask, arg.task)) {
			arg.sighdl(arg.signal, arg.task);
		}
	} else {
		rt_task_resume(arg.task);
	}
}

#endif /* __SIGNAL_SUPPORT_FUN__ */

static inline int rt_request_signal(int signal, void (*sighdl)(int, RT_TASK *))
{
	if (signal >= 0 && sighdl) {
		RT_TASK *signal_suprt_task;
		struct sigsuprt_t arg;
		arg.task        = rt_whoami();
		arg.signal      = signal;
		arg.sighdl      = sighdl;
		arg.runnable_on = arg.task->runnable_on_cpus;
		if ((signal_suprt_task = rt_malloc(sizeof(RT_TASK)))) {
			rt_task_init_cpuid(signal_suprt_task, (void *)signal_suprt_fun, (long)&arg, SIGNAL_TASK_STACK_SIZE, arg.task->priority, 0, 0, arg.runnable_on);
			rt_task_resume(signal_suprt_task);
			rt_task_suspend(arg.task);
			return arg.task->retval;
		}
	}
	return -EINVAL;
}

int rt_release_signal(int signal, RT_TASK *task);

void rt_enable_signal(int signal, RT_TASK *task);

void rt_disable_signal(int signal, RT_TASK *task);

void rt_send_signal(int signal, RT_TASK *task);

#else /* !__KERNEL__ */

#include <rtai_lxrt.h>

#define SIGNAL_TASK_STACK_SIZE  0x4000

#ifndef __SIGNAL_SUPPORT_FUN__
#define __SIGNAL_SUPPORT_FUN__

static void signal_suprt_fun(struct sigsuprt_t *funarg)
{		
	struct sigtsk_t { RT_TASK *sigtask; RT_TASK *task; };
	struct sigreq_t { RT_TASK *sigtask; RT_TASK *task; int signal; void (*sighdl)(int, RT_TASK *); };
	struct sigsuprt_t arg = *funarg;

	if ((arg.sigtask = rt_thread_init(rt_get_name(0), SIGNAL_TASK_INIPRIO, 0, SCHED_FIFO, 1 << arg.runnable_on))) {
		if (!rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigreq_t), SIGNAL_REQUEST, &arg).i[LOW]) {
			rt_make_hard_real_time();
			while (rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigtsk_t), SIGNAL_WAITSIG, &arg).i[LOW]) {
				arg.sighdl(arg.signal, arg.task);
			}
			rt_make_soft_real_time();
		}
		rt_task_delete(arg.sigtask);
	}
}

#endif /* __SIGNAL_SUPPORT_FUN__ */

static inline int rt_request_signal(int signal, void (*sighdl)(int, RT_TASK *))
{
	if (signal >= 0 && sighdl) {
		struct sigsuprt_t arg;
		union rtai_lxrt_t tskarg;
		tskarg = rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(int), SIGNAL_INFO, &arg);
		arg.task        = tskarg.v[HIGH];
		arg.signal      = signal;
		arg.sighdl      = sighdl;
		arg.runnable_on = tskarg.i[LOW];
		rt_thread_create(signal_suprt_fun, &arg, SIGNAL_TASK_STACK_SIZE);
		return rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(RT_TASK *), SIGNAL_SYNCREQ, &arg.task).i[LOW];
	}
	return -EINVAL;
}

static inline int rt_release_signal(int signal, RT_TASK *task)
{
	struct { int signal; RT_TASK *task; } arg = { signal, task };
	return rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, SIGNAL_RELEASE, &arg).i[LOW];
}

static inline void rt_enable_signal(int signal, RT_TASK *task)
{
	struct { int signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, SIGNAL_ENABLE, &arg);
}

static inline void rt_disable_signal(int signal, RT_TASK *task)
{
	struct { int signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, SIGNAL_DISABLE, &arg);
}

static inline void rt_send_signal(int signal, RT_TASK *task)
{
	struct { int signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, SIGNAL_SEND, &arg);
}

#endif /* __KERNEL__ */

#endif /* !_RTAI_SIGNAL_H_ */
