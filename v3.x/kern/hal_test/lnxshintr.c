/*
COPYRIGHT (C) 2013-2017 Paolo Mantegazza (mantegazza@aero.polimi.it)

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


#include <linux/module.h>

#include <asm/rtai.h>

MODULE_LICENSE("GPL");

#define MAX_NIRQ 10
static int nirq;
static int irq[] = {0, 1, 12, 26};
static int level[] = {0, 0, 0, 0};
static int cnt[] = {0, 0, 0, 0};
static char post_handler[MAX_NIRQ][20];
static void *dev_id[MAX_NIRQ];

static void rtai_handler(int irqa, void *idx)
{
	long i;
	i = (long)idx;
	cnt[i]++;	
	rt_pend_linux_irq(irq[i]);
}

static int linux_post_handler(int irqa, void *dev_id, struct pt_regs *regs)
{
	long i = (long)dev_id;
	if (level[i-1]) rt_enable_irq(irqa);
	rt_printk("LINUX IRQ %d: %d %d %d\n", i, cnt[i-1], irqa, irq[i-1]);
	return 1;
}

int init_module(void)
{
	long i;
	nirq = sizeof(irq)/sizeof(int);
	if (nirq >= MAX_NIRQ) {
		printk("SET MAX_NIRQ MACRO AT LEAST TO %d.\n", nirq);
		return 1;
	}
	for (i = 0; i < nirq; i++) {
		snprintf(post_handler[i], sizeof(post_handler[i]), "POST_HANDLER%ld", i + 1);
		dev_id[i] = (void *)(i+1);
		rt_request_linux_irq(irq[i], linux_post_handler, post_handler[i], dev_id[i]);
		if (level[i]) rt_set_irq_ack(irq[i], (void *)rt_disable_irq);
		rt_request_irq(irq[i], (void *)rtai_handler, (void *)i, 0);
	}
	return 0;
}

void cleanup_module(void)
{
	int i;
	for (i = 0; i < nirq; i++) {
		rt_release_irq(irq[i]);
		rt_free_linux_irq(irq[i], dev_id[i]);
	}
}

