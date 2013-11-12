/*
COPYRIGHT (C) 2013  Paolo Mantegazza (mantegazza@aero.polimi.it)

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
#include <linux/kernel.h>

#include <math.h>

#include "rtai_kerrno.h"

MODULE_LICENSE("GPL");

static void fun(void) 
{
	const int DGT = 9;
	char str[30];
	int i;

	for (i = 1; i <= 1000; i++) {
		kerrno = 0;
		switch(i) {
		case 1:
			d2str(sin(1.570796326794), DGT, str); 
			printk("- SIN Pi/2 %s KERRNO %d\n", str, kerrno); 
		break;
		case 2:
			d2str(cos(3.141592), DGT, str);
			printk("- COS Pi %s KERRNO %d\n", str, kerrno); 
		break;
		case 3:
			d2str(tan(0.785398163397), DGT, str); 
			printk("- TAN Pi/4 %s KERRNO %d\n", str, kerrno); 
		break;
		case 4:
			d2str(asin(1), DGT, str); 
			printk("- ASIN 1 %s KERRNO %d\n", str, kerrno); 
		break;
		case 5:
			d2str(asin(1000), DGT, str); 
			printk("- ASIN 1000 %s KERRNO %d\n", str, kerrno); 
		break;
		case 6:
			d2str(acos(-1), DGT, str); 
			printk("- ACOS -1 %s KERRNO %d\n", str, kerrno); 
		break;
		case 7:
			d2str(atan(1), DGT, str);
			printk("- ATAN 1 %s KERRNO %d\n", str, kerrno); 
		break;
		case 8:
			d2str(log(2.718281828), DGT, str); 
			printk("- LOG 2.718281828 %s KERRNO %d\n", str, kerrno); 
		break;
		case 9:
			d2str(log10(1000000), DGT, str);
			printk("- LOG10 1000000 %s KERRNO %d\n", str, kerrno); 
		break;
		case 10:
			d2str(log(-1000000), DGT, str); 
			printk("- LOG -1000000 %s KERRNO %d\n", str, kerrno); 
		break;
		case 11:
			d2str(pow(-10, 6), DGT, str);
			printk("- POW -10^6 %s KERRNO %d\n", str, kerrno); 
		break;
		case 12:
			d2str(pow(-10.000001, 6.0000001), DGT, str);
			printk("- POW -10.xxx^6.xxx %s KERRNO %d\n", str, kerrno); 
		break;
		case 13:
			d2str(sqrt(81), DGT, str);
			printk("- SQRT 81 %s KERRNO %d\n", str, kerrno); 
		break;
		case 14:
			d2str(sqrt(-1), DGT, str);
			printk("- SQRT -1 %s KERRNO %d\n", str, kerrno);
		break;
		case 15:
			d2str(exp(1), DGT, str);
			printk("- EXP 1 %s KERRNO %d\n", str, kerrno);
		break;
		case 16:
			d2str(exp(1000000), DGT, str);
			printk("- EXP 1000000 %s KERRNO %d\n", str, kerrno);
		break;
		default:
		break;
		}
	}
}

int init_module(void)
{
	fun();
	return 0;
}

void cleanup_module(void)
{
}
