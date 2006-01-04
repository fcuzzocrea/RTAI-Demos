#include "tasklets.h"

#define XN_INFINITE           0
#define RTDM_CLASS_BENCHMARK  6

#define xnpod_ns2ticks nano2count

typedef struct rt_tasklet_struct xntimer_t;
typedef RTIME xnticks_t;

static inline void xntimer_init(xntimer_t *timer, void (*handler)(void *cookie), void *cookie)
{
	memset(timer, 0, sizeof(struct rt_tasklet_struct));
	timer->handler = (void *)handler;
	timer->data    = (unsigned long)cookie;
}

static inline void xntimer_start (xntimer_t *timer, xnticks_t value, xnticks_t interval)
{
//waltzing Matilda zaniness here, they subtract we add again!
	extern RTIME rt_get_time(void);
	rt_insert_timer(timer, 0, rt_get_time() + value, interval, timer->handler, timer->data, 0);
}

static inline void xntimer_destroy (xntimer_t *timer)
{
	rt_remove_timer(timer);
}

