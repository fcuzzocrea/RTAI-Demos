/*
COPYRIGHT (C) 2004  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#ifndef _LINUX2RTAI_H
#define _LINUX2RTAI_H

#include <rtai_schedcore.h>
#include <rtai_sem.h>
#include <rtai_sysvmsg.h>

// we want to have both ADEOS and RTHAL happy
#undef current 
// this is the simples but you must be sure to be LXRT native
//#define current          ((RT_CURRENT)->lnxtsk)
// this is the safest (in approaching 2.6 use rtai_get_current instead)
#define current          (arti_get_current(hard_cpu_id()))

#define semaphore        rt_semaphore
#define sema_init(a, b)  rt_typed_sem_init(a, b, RES_SEM)
#define down(a)          rt_sem_wait(a)
#define up(a)            rt_sem_signal(a)

#if 0
// the simpler way
#define schedule()          rt_task_suspend(0)
#define wake_up_process(t)  rt_task_resume(t->this_rt_task[0])
#else
// with this we can manage errors and add a timeout service
#define schedule()  do { int i; rt_receive(0, &i); } while (0)
extern inline signed long schedule_timeout(signed long jiffies_left)
{
	int i;
	return (signed long)rt_receive_until(0, &i, nano2count((1000000000/HZ)*jiffies_left));
}
#define wake_up_process(t)  rt_send(t->this_rt_task[0], 0)
#endif

#define kmalloc(a, b)  rt_malloc(a)
#define vmalloc(a)     rt_malloc(a)
#define kfree(a)       rt_free(a)
#define vfree(a)       rt_free(a)

//static inline void sem_init(void) { }
static inline void shm_init(void) { }

#ifdef CONFIG_SMP
#define spin_lock(a)    rt_spin_lock_irq(a)
#define spin_unlock(a)  rt_spin_unlock_irq(a)
#endif

#define MODULE_NAME "RTAI_SYSVMSG"

#endif /* !_LINUX2RTAI_H */
