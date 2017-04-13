/*
 * Functional testing of RTDM services.
 *
 * Copyright (C) 2010 Jan Kiszka <jan.kiszka@web.de>.
 *
 * Released under the terms of GPLv2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "rttesting.h"

#define rt_timer_tsc    rt_get_time
#define rt_timer_tsc2ns count2nano

#define NS_PER_MS (1000000)

static void check_inner(const char *fn, int line, const char *msg,
			int status, int expected)
{
	if (status == expected)
		return;

	fprintf(stderr, "FAILED %s:%d: %s returned %d instead of %d - %s\n",
		fn, line, msg, status, expected, strerror(-status));
	exit(EXIT_FAILURE);
}

#define check(msg, status, expected) ({					\
	int __status = status;						\
	if(__status != expected)					\
		check_inner(__FUNCTION__, __LINE__, msg,		\
			    __status < 0 ? -errno : __status, expected);\
	__status;							\
})

#define check_no_error(msg, status) ({					\
	int __status = status;						\
	if(__status != 0)						\
		check_inner(__FUNCTION__, __LINE__, msg,		\
			    __status < 0 ? -errno : 0, 0);		\
	__status;							\
})

static void check_sleep_inner(const char *fn, int line,
			      const char *msg, unsigned long long start)
{
	unsigned long long diff = rt_timer_tsc2ns(rt_timer_tsc() - start);

	if (diff < 300 * NS_PER_MS) {
		fprintf(stderr, "FAILED %s:%d: %s waited only %Ld.%06u ms\n",
			fn, line, msg, diff / 1000000,
			(unsigned)(diff % 1000000));
		exit(EXIT_FAILURE);
	}
}
#define check_sleep(msg, start) \
	check_sleep_inner(__FUNCTION__, __LINE__, msg, start)

static const char *devname = "rttest-rtdm0";
static const char *devname2 = "rttest-rtdm1";

int main(int argc, const char *argv[])
{
	unsigned long long start;
	int dev, dev2;

	printf("Setup\n");
	dev = check_no_error("open", rt_dev_open(devname, O_RDWR));

	printf("Exclusive open\n");
	check("open", rt_dev_open(devname, O_RDWR), -EBUSY);

	printf("Successive open\n");
	dev2 = check("open", rt_dev_open(devname2, O_RDWR), dev + 1);
	check("close", rt_dev_close(dev2), 0);

	printf("Defer close by driver handler\n");
	check("ioctl", rt_dev_ioctl(dev, RTTST_RTIOC_RTDM_DEFER_CLOSE,
			     RTTST_RTDM_DEFER_CLOSE_HANDLER), 0);
	start = rt_timer_tsc();
	check("close", rt_dev_close(dev), 0);
	check("open", rt_dev_open(devname, O_RDWR), -EBUSY);
	dev2 = check("open", rt_dev_open(devname2, O_RDWR), dev);
	check("close", rt_dev_close(dev2), 0);
	usleep(300000);
	dev = check("open", rt_dev_open(devname, O_RDWR), dev);

	printf("Defer close by pending reference\n");
	check("ioctl", rt_dev_ioctl(dev, RTTST_RTIOC_RTDM_DEFER_CLOSE,
			     RTTST_RTDM_DEFER_CLOSE_CONTEXT), 0);
	start = rt_timer_tsc();
	check("close", rt_dev_close(dev), 0);
	check("open", rt_dev_open(devname, O_RDWR), -EBUSY);
	dev2 = check("open", rt_dev_open(devname2, O_RDWR), dev2);
	check("close", rt_dev_close(dev2), 0);
	usleep(300000);
	dev = check("open", rt_dev_open(devname, O_RDWR), dev);

	printf("Normal close\n");
	check("ioctl", rt_dev_ioctl(dev, RTTST_RTIOC_RTDM_DEFER_CLOSE,
			     RTTST_RTDM_NORMAL_CLOSE), 0);
	check("close", rt_dev_close(dev), 0);
	dev = check("open", rt_dev_open(devname, O_RDWR), dev);

	printf("Deferred module unload\n");
	check("ioctl", rt_dev_ioctl(dev, RTTST_RTIOC_RTDM_DEFER_CLOSE,
			     RTTST_RTDM_DEFER_CLOSE_CONTEXT), 0);
	start = rt_timer_tsc();
	check("close", rt_dev_close(dev), 0);
	check("rmmod", system("rmmod rtdmtest"), 0);
	check_sleep("rmmod", start);

	return 0;
}
