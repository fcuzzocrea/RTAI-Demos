from ctypes import *

cdll.LoadLibrary("libc.so.6")
libc = CDLL("libc.so.6")

cdll.LoadLibrary("liblxrt.so")
rtai = CDLL("liblxrt.so")

# lxrt services

rtai.nam2num.argstype = [c_void_p]
rtai.nam2num.restype  = c_ulong
nam2num = rtai.nam2num

rtai.num2nam.argstype = [c_ulong, c_void_p]
num2nam = rtai.num2nam

rtai.rt_get_adr.argtypes = [c_ulong]
rtai.rt_get_adr.restype = c_void_p
rt_get_adr = rtai.rt_get_adr

rtai.rt_get_name.argtypes = [c_void_p]
rtai.rt_get_name.restype  = c_ulong
rt_get_name = rtai.rt_get_name.restype

rtai.rt_task_init_schmod.argtypes = [c_ulong, c_int, c_int, c_int, c_int, c_int]
rtai.rt_task_init_schmod.restype  = c_void_p
rt_task_init_schmod = rtai.rt_task_init_schmod

rtai.rt_task_init.argtypes = [c_ulong, c_int, c_int, c_int]
rtai.rt_task_init.restype  = c_void_p
rt_task_init = rtai.rt_task_init.restype

rtai.rt_thread_create.argtypes = [c_void_p, c_void_p, c_int]
rtai.rt_thread_create.restype  = c_ulong
rt_thread_create = rtai.rt_thread_create

rt_make_soft_real_time = rtai.rt_make_soft_real_time
rt_make_hard_real_time = rtai.rt_make_hard_real_time

rtai.rt_task_delete.argtypes = [c_void_p]
rt_task_delete = rtai.rt_task_delete
rt_thread_delete = rtai.rt_task_delete

rtai.rt_thread_join.argtypes = [c_ulong]
rt_thread_join = rtai.rt_thread_join

rtai.rt_set_sched_policy.argtypes = [c_void_p, c_int, c_int]
rt_set_sched_policy = rtai.rt_set_sched_policy

rtai.rt_change_prio.argtypes = [c_void_p, c_int]
rt_change_prio = rtai.rt_change_prio

rtai.rt_is_hard_real_time.argtypes = [c_void_p]
rt_is_hard_real_time = rtai.rt_is_hard_real_time

def rt_is_soft_real_time():
	if rtai.rt_is_hard_real_time():
		return 0
	else:
		return 1

rtai.rt_task_suspend.argtypes = [c_void_p]
rt_task_suspend = rtai.rt_task_suspend

rtai.rt_task_suspend_if.argtypes = [c_void_p]
rt_task_suspend_if = rtai.rt_task_suspend_if

rtai.rt_task_suspend_until.argtypes = [c_void_p, c_longlong]
rt_task_suspend_until = rtai.rt_task_suspend_until

rtai.rt_task_suspend_timed.argtypes = [c_void_p, c_longlong]
rt_task_suspend_timed = rtai.rt_task_suspend_timed

rtai.rt_task_resume.argtypes = [c_void_p]
rt_task_resume = rtai.rt_task_resume

rtai.rt_task_masked_unblock.argtypes = [c_void_p, c_ulong]
rt_task_masked_unblock = rtai.rt_task_masked_unblock

rt_task_yield = rtai.rt_task_yield

rtai.rt_sleep.argtypes = [c_longlong]
rt_sleep = rtai.rt_sleep

rtai.rt_sleep_until.argtypes = [c_longlong]
rt_sleep_until = rtai.rt_sleep_until

rt_sched_lock = rtai.rt_sched_lock

rt_sched_unlock = rtai.rt_sched_unlock

rtai.rt_task_make_periodic.argtypex = [c_void_p, c_longlong, c_longlong]
rt_task_make_periodic = rtai.rt_task_make_periodic

rtai.rt_task_make_periodic_relative_ns.argtypes = [c_void_p, c_longlong, c_longlong]
rt_task_make_periodic_relative_ns = rtai.rt_task_make_periodic_relative_ns

rt_task_wait_period = rtai.rt_task_wait_period

rt_is_hard_timer_running = rtai.rt_is_hard_timer_running

rt_set_periodic_mode = rtai.rt_set_periodic_mode

rt_set_oneshot_mode = rtai.rt_set_oneshot_mode

rtai.start_rt_timer.restype = c_longlong
start_rt_timer = rtai.start_rt_timer

stop_rt_timer = rtai.stop_rt_timer

rtai.rt_get_time.restype = c_longlong
rt_get_time = rtai.rt_get_time

rtai.rt_get_real_time.restype = c_longlong
rt_get_real_time = rtai.rt_get_real_time

rtai.rt_get_real_time_ns.restype = c_longlong
rt_get_real_time_ns = rtai.rt_get_real_time_ns

rtai.rt_get_time_ns.restype = c_longlong
rt_get_time_ns = rtai.rt_get_time_ns

rtai.rt_get_cpu_time_ns.restype = c_longlong
rt_get_cpu_time_ns = rtai.rt_get_cpu_time_ns

rtai.rt_get_exectime.argtypes = [c_void_p, c_void_p]
rt_get_exectime = rtai.rt_get_exectime

rtai.rt_gettimeorig.argtypes = [c_void_p]
rt_gettimeorig = rtai.rt_gettimeorig

rtai.count2nano.argtypes = [c_longlong]
rtai.count2nano.restype = c_longlong
count2nano = rtai.count2nano

rtai.nano2count.argtypes = [c_longlong]
rtai.nano2count.restype = c_longlong
nano2count = rtai.nano2count

rt_busy_sleep = rtai.rt_busy_sleep

rtai.rt_force_task_soft.restype = c_ulong

rtai.rt_agent.restype = c_ulong

rt_buddy = rtai.rt_agent

rtai.rt_get_priorities.argtypes = [c_void_p, c_void_p, c_void_p]
rt_get_priorities = rtai.rt_get_priorities

rt_gettid = rtai.rt_gettid


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

rtai.rt_poll.argtypes = [c_void_p, c_ulong, c_longlong]
rt_poll = rtai.rt_poll


# mail boxes


rtai.rt_typed_mbx_init.argtypes = [c_ulong, c_int, c_int]
rtai.rt_typed_mbx_init.restype = c_void_p
rt_typed_mbx_init = rtai.rt_typed_mbx_init

rtai.rt_mbx_delete.argtypes = [c_void_p]
rt_mbx_delete = rtai.rt_mbx_delete

rtai.rt_typed_named_mbx_init.argtypes = [c_void_p, c_int, c_int]
rtai.rt_typed_named_mbx_init.restype = c_void_p
rt_typed_named_mbx_init = rtai.rt_typed_named_mbx_init

rtai.rt_named_mbx_delete.argtypes = [c_void_p]
rt_named_mbx_delete = rtai.rt_named_mbx_delete

def rt_named_mbx_init(mbx_name, size) :
	return rtai.rt_typed_named_mbx_init(mbx_name, size, FIFO_Q)

rtai.rt_mbx_send.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send = rtai.rt_mbx_send

rtai.rt_mbx_send_if.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send_if = rtai.rt_mbx_send_if

rtai.rt_mbx_send_until.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_send_until = rtai.rt_mbx_send_until

rtai.rt_mbx_send_timed.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_send_timed = rtai.rt_mbx_send_timed

rtai.rt_mbx_send_wp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send_wp = rtai.rt_mbx_send_wp

rtai.rt_mbx_ovrwr_send.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_ovrwr_send = rtai.rt_mbx_ovrwr_send

rtai.rt_mbx_evdrp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_evdrp = rtai.rt_mbx_evdrp

rtai.rt_mbx_receive.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive = rtai.rt_mbx_receive

rtai.rt_mbx_receive_if.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive_if = rtai.rt_mbx_receive_if

rtai.rt_mbx_receive_until.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_receive_until = rtai.rt_mbx_receive_until

rtai.rt_mbx_receive_timed.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_receive_timed = rtai.rt_mbx_receive_timed

rtai.rt_mbx_receive_wp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive_wp = rtai.rt_mbx_receive_wp


# intertasks messages


rtai.rt_send.argtypes = [c_void_p, c_ulong]
rtai.rt_send.restype = c_void_p
rt_send = rtai.rt_send

rtai.rt_send_if.argtypes = [c_void_p, c_ulong]
rtai.rt_send_if.restype = c_void_p
rt_send_if = rtai.rt_send_if

rtai.rt_send_until.argtypes = [c_void_p, c_ulong, c_longlong]
rtai.rt_send_until.restype = c_void_p
rt_send_until = rtai.rt_send_until

rtai.rt_send_timed.argtypes = [c_void_p, c_ulong, c_longlong]
rtai.rt_send_timed.restype = c_void_p
rt_send_timed = rtai.rt_send_timed

rtai.rt_evdrp.argtypes = [c_void_p, c_void_p]
rtai.rt_evdrp.restype = c_void_p
rt_evdrp = rtai.rt_evdrp

rtai.rt_receive.argtypes = [c_void_p, c_void_p]
rtai.rt_receive.restype = c_void_p
rt_receive = rtai.rt_receive

rtai.rt_receive_if.argtypes = [c_void_p, c_void_p]
rtai.rt_receive_if.restype = c_void_p
rt_receive_if = rtai.rt_receive_if

rtai.rt_receive_until.argtypes = [c_void_p, c_void_p, c_longlong]
rtai.rt_receive_until.restype = c_void_p
rt_receive_until = rtai.rt_receive_until

rtai.rt_receive_timed.argtypes = [c_void_p, c_void_p, c_longlong]
rtai.rt_receive_timed.restype = c_void_p
rt_receive_timed = rtai.rt_receive_timed

rtai.rt_rpc.argtypes = [c_void_p, c_ulong, c_void_p]
rtai.rt_rpc.restype = c_void_p
rt_rpc = rtai.rt_rpc

rtai.rt_rpc_if.argtypes = [c_void_p, c_ulong, c_void_p]
rtai.rt_rpc_if.restype = c_void_p
rt_rpc_if = rtai.rt_rpc_if

rtai.rt_rpc_until.argtypes = [c_void_p, c_ulong, c_void_p]
rtai.rt_rpc_until.restype = c_void_p
rt_rpc_until = rtai.rt_rpc_until

rtai.rt_rpc_timed.argtypes = [c_void_p, c_ulong, c_void_p]
rtai.rt_rpc_timed.restype = c_void_p
rt_rpc_timed = rtai.rt_rpc_timed

rtai.rt_isrpc.argtypes = [c_void_p]
rt_isrpc = rtai.rt_isrpc

rtai.rt_return.argtypes = [c_void_p, c_ulong]
rtai.rt_return.restype = c_void_p
rt_return = rtai.rt_return
