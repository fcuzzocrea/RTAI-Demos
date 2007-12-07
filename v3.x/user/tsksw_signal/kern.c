/*
 * Copyright (C) 2007 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

MODULE_LICENSE("GPL");

#include <rtai_sem.h>
#include <rtai_msg.h>
#include <rtai_malloc.h>
#include "rtai_signal.h"


#define PERIOD  10000000
#define STKSZ   4000
#define SWITCH_SIGNAL  1

static RT_TASK rtai_sig_task, sig_task;

static void rtai_sighdl(long signal, RT_TASK *rtai_sig_task)
{
	static int cnt;
	rt_printk("RTAI SIGNAL HANDLER # %d\n", ++cnt);
}

static void sighdl(void)
{
	static int cnt;
	rt_printk("SIGNAL HANDLER # %d\n", ++cnt);
}

static void rtai_sig_fun(long taskidx)
{
	rt_request_signal(SWITCH_SIGNAL, rtai_sighdl);
	rt_task_signal_handler(&rtai_sig_task, (void *)SWITCH_SIGNAL);
	while (1) {
		rt_sleep(nano2count(PERIOD));
	}
}

static void sig_fun(long arg)
{
	rt_task_signal_handler(&sig_task, sighdl);
	while (1) {
		rt_sleep(nano2count(PERIOD));
	}
}

int init_module(void)
{
	start_rt_timer(0);
	rt_task_init(&rtai_sig_task, rtai_sig_fun, 0, STKSZ, 0, 0, 0);
	rt_task_resume(&rtai_sig_task);
	rt_task_init(&sig_task, sig_fun, 0, STKSZ, 0, 1, 0);
	rt_task_resume(&sig_task);
	return 0;
}

void cleanup_module(void)
{
	stop_rt_timer();
	rt_task_delete(&rtai_sig_task);
	rt_task_delete(&sig_task);
}
