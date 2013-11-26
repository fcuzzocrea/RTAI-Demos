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

#include <rtai_math.h>

MODULE_LICENSE("GPL");

#define TICK_PERIOD 100000 // nanoseconds

static RT_TASK mytask;

static void tskfun(long task) 
{
	const int DGT = 6;
	double r;
#ifdef CONFIG_RTAI_MATH_KCOMPLEX
	double complex cr;
#endif
	char str[40];
	int i;

	for (i = 1; i <= 1000; i++) {
		kerrno = 0;
		switch(i) {
		case 1:
			printk("***** REAL FUCTIONS *****\n"); 
			d2str(r = sin(1.570796326794), DGT, str); 
			printk("- SIN(Pi/2) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 2:
			d2str(r = cos(3.141592), DGT, str);
			printk("- COS(Pi) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 3:
			d2str(r = tan(0.785398163397), DGT, str); 
			printk("- TAN(Pi/4) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 4:
			d2str(r = asin(1), DGT, str); 
			printk("- ASIN(1) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 5:
			d2str(r = asin(1000), DGT, str); 
			printk("- ASIN(1000) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 6:
			d2str(r = cos(-1), DGT, str); 
			printk("- ACOS(-1) %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 7:
			d2str(r = atan(1), DGT, str);
			printk("- ATAN(1) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 8:
			d2str(r = log(2.718281828), DGT, str); 
			printk("- LOG(2.718281828) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 9:
			d2str(r = log10(1000000), DGT, str);
			printk("- LOG10(10^6) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 10:
			d2str(r = log(-1000000), DGT, str); 
			printk("- LOG(-10^6) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 11:
			d2str(r = pow(-10, 6), DGT, str);
			printk("- POW(-10, 6) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 12:
			d2str(r = pow(-10.000001, 6.0000001), DGT, str);
			printk("- POW(-10.xxx, 6.xxx) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 13:
			d2str(r = sqrt(81), DGT, str);
			printk("- SQRT(9^2) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno); 
		break;
		case 14:
			d2str(r = sqrt(-1), DGT, str);
			printk("- SQRT(-1) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno);
		break;
		case 15:
			d2str(r = exp(1), DGT, str);
			printk("- EXP(1) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno);
		break;
		case 16:
			d2str(r = exp(1000000), DGT, str);
			printk("- EXP(1000000) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno);
		break;
#ifdef CONFIG_RTAI_MATH_KCOMPLEX
		case 17:
			printk("***** COMPLEX FUCTIONS *****\n"); 
			d2str(r = cabs(3 + I*4), DGT, str);
			printk("- CABS(3 + 4j) = %s, FPKERR %d, KERRNO %d.\n", str, fpkerr(r), kerrno);
		break;
		case 18:
			cd2str(cr = cexp(1.570796326794*_Complex_I), DGT, str);
			printk("- CEXP(Pi/2)j = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 19:
			cd2str(cr = csqrt(cpow(5 + I*10, 2)), DGT, str);
			printk("- CSQRT(CPOW(5 + 10j, 2)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 20:
			cd2str(cr = cpow(1 + I*1, 2), DGT, str);
			printk("- CPOW(1 + 1j, 2) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 21:
			cd2str(cr = (1 + I*1)*(1 + I*1), DGT, str);
			printk("- (1 + 1j)*(1 + 1j) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 22:
			cd2str(cr = (-4 + I*16)/(3 + I*5), DGT, str);
			printk("- (-4 + 16j)/(3 + 5j) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 23:
			cd2str(cr = casin(csin(1 - I*2)), DGT, str);
			printk("- CASIN(CSIN(1 - 2j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 24:
			cd2str(cr = cacos(ccos(-2 + I*3)), DGT, str);
			printk("- CACOS(CCOS(-2 + 3j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 25:
			cd2str(cr = catan(ctan(-0.35 - I*0.7)), DGT, str);
			printk("- CATAN(CTAN(-0.35 - 0.7j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 26:
			cd2str(cr = casinh(csinh(1 - I*1.5)), DGT, str);
			printk("- CASINH(CSINH(1 - 1.5j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 27:
			cd2str(cr = cacosh(ccosh(-2 + I*1)), DGT, str);
			printk("- CACOSH(CCOSH(-2 + 1j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 28:
			cd2str(cr = catanh(ctanh(-0.35 - I*0.7)), DGT, str);
			printk("- CATANH(CTANH(-0.35 - 0.7j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 29:
			cd2str(cr = clog(cexp(1 - 2*_Complex_I)), DGT, str);
			printk("- CLOG(CEXP(1 - 2j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
		case 30:
			cd2str(cr = clog10(cpow(10, 1 - 0.5*_Complex_I)), DGT, str);
			printk("- CLOG10(CPOW(10, 1 - 0.5j)) = %s, CFPKERR %d, KERRNO %d.\n", str, cfpkerr(cr), kerrno);
		break;
#endif
		default:
			rt_task_suspend(&mytask);	
		}
		rt_task_wait_period();
	}
	rt_task_suspend(&mytask);	
}

int init_module(void)
{
	start_rt_timer(0);
	rt_task_init(&mytask, tskfun, 0, 4096, 0, 1, 0);
	rt_task_make_periodic(&mytask, rt_get_time(), nano2count(TICK_PERIOD));
	return 0;
}

void cleanup_module(void)
{
	stop_rt_timer();
        rt_task_delete(&mytask);
}
