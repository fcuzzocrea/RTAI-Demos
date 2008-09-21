from rtai_def import *


# semaphores


PRIO_Q = 0
FIFO_Q = 4
RES_Q  = 3

BIN_SEM = 1
CNT_SEM = 2
RES_SEM = 3

RESEM_RECURS = 1
RESEM_BINSEM = 0
RESEM_CHEKWT = -1

rtai.rt_typed_sem_init.argtypes = [c_ulong, c_int, c_int]
rtai.rt_typed_sem_init.restype = c_void_p
rt_typed_sem_init = rtai.rt_typed_sem_init

def rt_sem_init(name, value) :
	return rt_typed_sem_init(name, value, CNT_SEM);x

def rt_named_sem_init(sem_name, value) :
        return rt_typed_named_sem_init(sem_name, value, CNT_SEM)

rtai.rt_sem_delete.argtypes = [c_void_p]
rt_sem_delete = rtai.rt_sem_delete

rtai.rt_typed_named_sem_init.argtypes = [c_void_p, c_int, c_int]
rtai.rt_typed_named_sem_init.restype = c_void_p

rtai.rt_named_sem_delete.argtypes = [c_void_p]
rt_named_sem_delete = rtai.rt_named_sem_delete

rtai.rt_sem_signal.argtypes = [c_void_p]
rt_sem_signal = rtai.rt_sem_signal

rtai.rt_sem_broadcast.argtypes = [c_void_p]
rt_sem_broadcast = rtai.rt_sem_broadcast

rtai.rt_sem_wait.argtypes = [c_void_p]
rt_sem_wait = rtai.rt_sem_wait

rtai.rt_sem_wait_if.argtypes = [c_void_p]
rt_sem_wait_if = rtai.rt_sem_wait_if

rtai.rt_sem_wait_until.argtypes = [c_void_p, c_longlong]
rt_sem_wait_until = rtai.rt_sem_wait_until

rtai.rt_sem_wait_timed.argtypes = [c_void_p, c_longlong]
rt_sem_wait_timed = rtai.rt_sem_wait_timed

rtai.rt_sem_wait_barrier.argtypes = [c_void_p]
rt_sem_wait_barrier = rtai.rt_sem_wait_barrier

rtai.rt_sem_count.argtypes = [c_void_p]
rt_sem_count = rtai.rt_sem_count

def rt_cond_init(name) :
	return rt_typed_sem_init(name, 0, BIN_SEM)

def rt_cond_delete(cnd) :
		return rt_sem_delete(cnd)

def rt_cond_destroy(cnd) :
	return rt_sem_delete(cnd)

def rt_cond_broadcast(cnd) :
	return rt_sem_broadcast(cnd)

def rt_cond_timedwait(cnd, mtx, time) :
	return rt_cond_wait_until(cnd, mtx, time)

rtai.rt_cond_signal.argtypes = [c_void_p]
rt_cond_signal = rtai.rt_cond_signal

rtai.rt_cond_wait.argtypes = [c_void_p, c_void_p]
rt_cond_wait = rtai.rt_cond_wait

rtai.rt_cond_wait_until.argtypes = [c_void_p, c_void_p, c_longlong]
rt_cond_wait_until = rtai.rt_cond_wait_until

rtai.rt_cond_wait_timed.argtypes = [c_void_p, c_void_p, c_longlong]
rt_cond_wait_timed = rtai.rt_cond_wait_timed

#rtai.rt_poll.argtypes = [c_void_p, c_ulong, c_longlong]
#rt_poll = rtai.rt_poll
