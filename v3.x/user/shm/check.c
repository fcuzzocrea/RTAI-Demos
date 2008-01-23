/*
COPYRIGHT (C) 2008  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sched.h>

#include <rtai_shm.h>

int main(void)
{
	int *vm, *km, *ka, *kd, i;	
	vm = rt_shm_alloc(nam2num("VM"), 0, USE_VMALLOC);
	km = rt_shm_alloc(nam2num("KM"), 0, USE_GFP_KERNEL);
	ka = rt_shm_alloc(nam2num("KA"), 0, USE_GFP_ATOMIC);
	kd = rt_shm_alloc(nam2num("KD"), 0, USE_GFP_DMA);
	printf("SIZEs in USER: %d %d %d %d\n", vm[0], km[0], ka[0], kd[0]);
	for (i = 1; i < vm[0]; i++) {
		if ( vm[i] != km[i] || km[i] != ka[i] || ka[i] != kd[i]) {
			printf("wrong at index %i\n", i);
		}
	}
	rt_shm_free(nam2num("VM"));
	rt_shm_free(nam2num("KM"));
	rt_shm_free(nam2num("KA"));
	rt_shm_free(nam2num("KD"));
	return 0;
}
