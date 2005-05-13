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


#ifndef _RTAI_FUSION_TASK_H
#define _RTAI_FUSION_TASK_H

#include <lxrt.h>
#include <timer.h>

#define CPU_SHIFT  24

#define T_FPU   1
#define T_SUSP  2
#define T_CPU(cpu)  (1 << (CPU_SHIFT + (cpu & 0xff)))

#define T_LOCK  1
#define T_RRB   2

#define T_LOPRIO  1
#define T_HIPRIO  99

typedef struct rt_task_t {
	void *task;
	unsigned long id;
	void (*fun)(void *);
	void *arg;
	int prio, mode;
} RT_TASK_T;

typedef RT_TASK_T RT_TASK;

#ifdef __cplusplus
extern "C" {
#endif

#include <sched.h>

static inline void *rtai_tskext(void)
{
	struct { unsigned long dummy; } arg;
        return (void *)rtai_lxrt(BIDX, SIZARG, RT_BUDDY, &arg).v[LOW];
}

static inline void rt_set_task_self(void *task, void *self)
{
	struct { void *task, *self; } arg = { task, self };
	rtai_lxrt(BIDX, SIZARG, SET_USP_FLG_MSK, &arg);
}

static inline void rt_make_hard_real_time(void)
{
        struct { unsigned long dummy; } arg;
        rtai_lxrt(BIDX, SIZARG, MAKE_HARD_RT, &arg);
}

static inline void rt_make_soft_real_time(void)
{
        struct { unsigned long dummy; } arg;
        rtai_lxrt(BIDX, SIZARG, MAKE_SOFT_RT, &arg);
}

static inline void *rt_task_ext(int name, int prio, int cpus_allowed)
{
        struct sched_param mysched;
        struct { int name, prio, stack_size, max_msg_size, cpus_allowed; } arg = { name, prio, 0, 0, cpus_allowed };

        if (arg.prio < 0) {
        	arg.prio = 0;
        }
        if ((mysched.sched_priority = sched_get_priority_max(SCHED_FIFO) - arg.prio) < 1) {
                mysched.sched_priority = 1;
        }
        if (sched_setscheduler(0, SCHED_FIFO, &mysched) < 0) {
                return 0;
        }
        rtai_iopl();

        return (void *)rtai_lxrt(BIDX, SIZARG, LXRT_TASK_INIT, &arg).v[LOW];
}

static inline int rt_task_suspend(RT_TASK *task)
{
        struct { void *task; } arg = { task ? task->task : NULL };
        return rtai_lxrt(BIDX, SIZARG, SUSPEND, &arg).i[LOW];
}

static inline int rt_task_resume(RT_TASK *task)
{
	struct { void *task; } arg = { task->task };
	return rtai_lxrt(BIDX, SIZARG, RESUME, &arg).i[LOW];
}

static inline int rt_task_delete(RT_TASK *task)
{
	struct { void *task; } arg = { task ? task->task : NULL };
	rt_make_soft_real_time();
        return rtai_lxrt(BIDX, SIZARG, LXRT_TASK_DELETE, &arg).i[LOW];
}

static inline int rt_task_yield(void)
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, YIELD, &arg).i[LOW];
}

static inline int rt_task_set_periodic(RT_TASK *task, RTIME idate, RTIME period)
{
        struct { void *task; RTIME idate, period; } arg = { task ? task->task : rtai_tskext(), idate == TM_NOW ? rt_timer_tsc() : rt_timer_ns2ticks(idate), rt_timer_ns2ticks(period) };
       	return rtai_lxrt(BIDX, SIZARG, MAKE_PERIODIC, &arg).i[LOW];
}

static inline int rt_task_wait_period(void)
{
	struct { unsigned long retval; } arg;
	arg.retval = rtai_lxrt(BIDX, SIZARG, WAIT_PERIOD, &arg).i[LOW];
	return !arg.retval ? 0 : arg.retval > 0 ? -ETIMEDOUT : -EINVAL;
}

static inline int rt_task_set_priority(RT_TASK *task, int prio)
{
	int retval;
	struct { RT_TASK *task; int prio; } arg;
	arg.task = task ? task->task : 0;
	if ((arg.prio = sched_get_priority_max(SCHED_FIFO) - prio) < 0) {
		arg.prio = 0;
	}
	retval = rtai_lxrt(BIDX, SIZARG, CHANGE_TASK_PRIO, &arg).i[LOW];
	return retval < 0 ? retval : 0;
}


static inline int rt_task_sleep(RTIME delay)
{
	int retval;
	struct { RTIME delay; } arg = { delay };
	retval = rtai_lxrt(BIDX, SIZARG, SLEEP, &arg).i[LOW];
	return !retval ? 0 : retval > 0 ? -ETIMEDOUT : -EINVAL;
}

static inline int rt_task_sleep_until(RTIME date)
{
	int retval;
	struct { RTIME date; } arg = { date };
	retval = rtai_lxrt(BIDX, SIZARG, SLEEP_UNTIL, &arg).i[LOW];
	return !retval ? 0 : retval > 0 ? -ETIMEDOUT : -EINVAL;
}

static inline RT_TASK *rt_task_self(void)
{
        struct { void *task; } arg = { rtai_tskext() };
        return (RT_TASK *)rtai_lxrt(BIDX, SIZARG, GET_USP_FLG_MSK, &arg).v[LOW];
}

static inline int rt_task_slice(RT_TASK *task, RTIME quantum)
{
	struct { RT_TASK *task; int policy; int rr_quantum_ns; } arg = { task ? task->task : rtai_tskext(), 1, quantum };
	rtai_lxrt(BIDX, SIZARG, SET_SCHED_POLICY, &arg);
	return 0;
}

#ifndef __SUPPORT_TASK__
#define __SUPPORT_TASK__

static inline void support_task(RT_TASK *task)
{
	if (!(task->task = rt_get_handle(task->id))) {
		task->task = rt_task_ext(task->id, sched_get_priority_max(SCHED_FIFO) - task->prio, task->mode >> CPU_SHIFT);
	}
	rt_set_task_self(task->task, task);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	rt_task_resume(task->arg);
	rt_task_suspend(task);
	rt_release_waiters(~task->id);
	task->fun(task->arg);
	rt_make_soft_real_time();
	rt_task_delete(task);
}
#endif /* __SUPPORT_TASK__ */

#include <pthread.h>

#define RT_THREAD_STACK_MIN 64*1024

static inline int rt_thread_create(void *fun, void *args, int stack_size)
{
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (pthread_attr_setstacksize(&attr, stack_size > RT_THREAD_STACK_MIN ?
stack_size : RT_THREAD_STACK_MIN)) {
                return -1;
        }
        if (pthread_create(&thread, &attr, (void *(*)(void *))fun, args)) {
                return -1;
        }
        return thread;
}

static inline int rt_task_create(RT_TASK *task, const char *name, int stksize, int prio, int mode)
{
	RT_TASK me;
	if (!(me.task = rtai_tskext())) {
		me.task = rt_task_ext((unsigned long)name, 0, 0xF);
	}
	task->id = nam2id(name);
	task->prio = prio;
	task->mode = mode;
	task->arg  = &me;
	rt_thread_create(support_task, (void *)task, stksize);
	rt_task_suspend(&me);
	return 0;
}

static inline int rt_task_start(RT_TASK *task, void (*fun)(void *cookie), void *cookie)
{
	task->fun = fun;
	task->arg = cookie;
	if (!(task->mode & T_SUSP)) {
		rt_task_resume(task);
	}
	return 0;
}

#define RT_SCHED_DELAYED      4
#define RT_SCHED_SEMAPHORE    8
#define RT_SCHED_MBXSUSP    256

static inline int rt_task_unblock(RT_TASK *task)
{
	struct { RT_TASK *task; unsigned long mask; } arg = { task->task, (RT_SCHED_DELAYED | RT_SCHED_SEMAPHORE | RT_SCHED_MBXSUSP) };
	return rtai_lxrt(BIDX, SIZARG, WAKEUP_SLEEPING, &arg).i[LOW];
}

static inline int rt_task_inquire(RT_TASK *task, void *info)
{
	return 0;
}

static inline int rt_task_catch(void (*handler)(int))
{
	return 0;
}

static inline int rt_task_notify(RT_TASK *task, int signals)
{
	return 0;
}

static inline int rt_task_set_mode(int clrmask, int setmask, int *mode_r)
{
	if (setmask & T_LOCK) {
		struct { int dummy; } arg;
		rtai_lxrt(BIDX, SIZARG, SCHED_LOCK, &arg);
	} else {
		struct { int dummy; } arg;
		rtai_lxrt(BIDX, SIZARG, SCHED_UNLOCK, &arg);
	}
       	struct { void *task; int policy; int rr_quantum; } arg = { rtai_tskext(), setmask & T_RRB, 0 };
       	rtai_lxrt(BIDX, SIZARG, SET_SCHED_POLICY, &arg);
	return 0;
}

static inline int rt_task_bind(RT_TASK *task, const char *name)
{
	return rt_obj_bind(task, name);
}

static inline int rt_task_unbind (RT_TASK *task)
{
	return rt_obj_unbind(task);
}

#include <sys/poll.h>
static inline void rt_task_join(const char *name)
{
	while (rt_get_handle(nam2id(name))) {
		poll(0, 0, 200);
	}
}

#ifdef __cplusplus
}
#endif

#endif /* !_RTAI_FUSION_TASK_H */
