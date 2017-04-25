/*
COPYRIGHT (C) 2017 Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#ifndef _SCHED_IPI_
#define _SCHED_IPI_

#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/rtai.h>
#include <rtai_schedcore.h>

static inline void SEND_SCHED_IPI(void) 
{
#ifdef CONFIG_SMP
	int cpu, thiscpu = rtai_cpuid();
	for (cpu = 0; cpu < RTAI_NR_CPUS; cpu++) {
		if (cpu != thiscpu) {
                	rtai_cli();
	                send_sched_ipi(1 << cpu);
       		        rtai_sti();
		}
	}
#else 
	apic->send_IPI_self(ipipe_apic_irq_vector(RTAI_RESCHED_IRQ));
#endif
	return;
}

#endif /* _SCHED_IPI_ */
