/*
COPYRIGHT (C) 2005  Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#ifdef DIAG_FLAGS

#define CHECK_FLAGS() \
	do { \
		unsigned long flags, ibut; \
#ifdef CONFIG_X86
		ibit = 9;
		__asm__ __volatile__("pushfl; popl %0": "=g" (flags)); \
#endif
#ifdef CONFIG_ARM
		ibit = 7;
		__asm__ __volatile__("mrs %0, cpsr: "=r" (flags): : "memory"); \
#endif
		if (flags & (1 << ibit)) rt_printk("< BAD! ENABLED >\n"); \
	} while (0);
#else
#define CHECK_FLAGS()
#endif
