/*
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#ifndef _RTAI_FUSION_TIMER_H
#define _RTAI_FUSION_TIMER_H

#include <lxrt.h>

#define TM_UNSET   0
#define TM_ONESHOT 0

#ifdef __cplusplus
extern "C" {
#endif

static inline RTIME rt_timer_ns2ticks(RTIME ns)
{
        struct { RTIME ns; } arg = { ns };
        return rtai_lxrt(BIDX, SIZARG, NANO2COUNT, &arg).rt;
}

static inline RTIME rt_timer_ticks2ns(RTIME ticks)
{
        struct { RTIME ticks; } arg = { ticks };
        return rtai_lxrt(BIDX, SIZARG, COUNT2NANO, &arg).rt;
}

static inline RTIME rt_timer_read(void)
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_CPU_TIME_NS, &arg).rt;
}

static inline RTIME rt_timer_tsc(void)
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_TIME, &arg).rt;
}

static inline int rt_timer_start(RTIME nstick)
{
        struct { int dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, SET_ONESHOT_MODE, &arg);
	rtai_lxrt(BIDX, SIZARG, START_TIMER, &arg);
	return 0;
}

static inline void rt_timer_stop(void)
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, STOP_TIMER, &arg);
}

static inline int rt_timer_inquire(void *info)
{
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* !_RTAI_FUSION_TIMER_H */
