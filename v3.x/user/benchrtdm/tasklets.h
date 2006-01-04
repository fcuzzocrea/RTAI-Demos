/**
 *
 * Copyright: 2006 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_TASKLETS_H
#define _RTAI_TASKLETS_H

struct rt_task_struct;

struct rt_tasklet_struct {
	struct rt_tasklet_struct *next, *prev;
	int priority, uses_fpu;
	RTIME firing_time, period;
	void (*handler)(unsigned long);
	unsigned long data, id;
	int thread;
	struct rt_task_struct *task;
	struct rt_tasklet_struct *usptasklet;
	struct { void *rb_parent; int rb_color; void *rb_right, *rb_left; } rbn;
	struct { void *rb_node; } rbr;
};

int __rtai_tasklets_init(void);

void __rtai_tasklets_exit(void);

struct rt_tasklet_struct *rt_init_tasklet(void);

int rt_delete_tasklet(struct rt_tasklet_struct *tasklet);

int rt_insert_tasklet(struct rt_tasklet_struct *tasklet, int priority, void (*handler)(unsigned long), unsigned long data, unsigned long id, int pid);

void rt_remove_tasklet(struct rt_tasklet_struct *tasklet);

struct rt_tasklet_struct *rt_find_tasklet_by_id(unsigned long id);

int rt_exec_tasklet(struct rt_tasklet_struct *tasklet);

void rt_set_tasklet_priority(struct rt_tasklet_struct *tasklet, int priority);

int rt_set_tasklet_handler(struct rt_tasklet_struct *tasklet, void (*handler)(unsigned long));

#define rt_fast_set_tasklet_handler(t, h) do { (t)->handler = (h); } while (0)

void rt_set_tasklet_data(struct rt_tasklet_struct *tasklet, unsigned long data);

#define rt_fast_set_tasklet_data(t, d)  do { (t)->data = (d); } while (0)

struct rt_task_struct *rt_tasklet_use_fpu(struct rt_tasklet_struct *tasklet, int use_fpu);

#define rt_init_timer rt_init_tasklet 

#define rt_delete_timer rt_delete_tasklet

int rt_insert_timer(struct rt_tasklet_struct *timer, int priority, RTIME firing_time, RTIME period, void (*handler)(unsigned long), unsigned long data, int pid);

void rt_remove_timer(struct rt_tasklet_struct *timer);

void rt_set_timer_priority(struct rt_tasklet_struct *timer, int priority);

void rt_set_timer_firing_time(struct rt_tasklet_struct *timer, RTIME firing_time);

void rt_set_timer_period(struct rt_tasklet_struct *timer, RTIME period);

#define rt_fast_set_timer_period(t, p)  do { (t)->period = (p); } while (0)

#define rt_set_timer_handler rt_set_tasklet_handler

#define rt_fast_set_timer_handler(t, h)  do { (t)->handler = (h); } while (0)

#define rt_set_timer_data rt_set_tasklet_data

#define rt_fast_set_timer_data(t, d)  do { (t)->data = (d); } while (0)

#define rt_timer_use_fpu rt_tasklet_use_fpu

void rt_wait_tasklet_is_hard(struct rt_tasklet_struct *tasklet, int thread);

void rt_register_task(struct rt_tasklet_struct *tasklet, struct rt_tasklet_struct *usptasklet, struct rt_task_struct *task);
 
#endif /* !_RTAI_TASKLETS_H */
