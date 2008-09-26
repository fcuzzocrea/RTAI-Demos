from rtai_def import *


# read write locks



def rt_rwl_init(rwl) :
	return rt_typed_rwl_init(rwl, RESEM_RECURS)

rtai.rt_typed_rwl_init.argtypes = [c_ulong, c_int]
rtai.rt_typed_rwl_init.restype = c_void_p
rt_typed_rwl_init = rtai.rt_typed_rwl_init

rtai.rt_rwl_delete.argtypes = [c_void_p]
rt_rwl_delete = rtai.rt_rwl_delete

rtai.rt_named_rwl_init.argtypes = [c_void_p]
rtai.rt_named_rwl_init.restype = c_void_p
rt_named_rwl_init = rtai.rt_named_rwl_init

rtai.rt_named_rwl_delete.argtypes = [c_void_p]
rt_named_rwl_delete = rtai.rt_named_rwl_delete

rtai.rt_rwl_rdlock.argtypes = [c_void_p]
rt_rwl_rdlock = rtai.rt_rwl_rdlock

rtai.rt_rwl_rdlock_if.argtypes = [c_void_p]
rt_rwl_rdlock_if = rtai.rt_rwl_rdlock_if

rtai.rt_rwl_rdlock_until.argtypes = [c_void_p, c_longlong]
rt_rwl_rdlock_until = rtai.rt_rwl_rdlock_until

rtai.rt_rwl_rdlock_timed.argtypes = [c_void_p, c_longlong]
rt_rwl_rdlock_timed = rtai.rt_rwl_rdlock_timed

rtai.rt_rwl_wrlock.argtypes = [c_void_p]
rt_rwl_wrlock = rtai.rt_rwl_wrlock

rtai.rt_rwl_wrlock_if.argtypes = [c_void_p]
rt_rwl_wrlock_if = rtai.rt_rwl_wrlock_if

rtai.rt_rwl_wrlock_until.argtypes = [c_void_p, c_longlong]
rt_rwl_wrlock_until = rtai.rt_rwl_wrlock_until

rtai.rt_rwl_wrlock_timed.argtypes = [c_void_p, c_longlong]
rt_rwl_wrlock_timed = rtai.rt_rwl_wrlock_timed

rtai.rt_rwl_unlock.argtypes = [c_void_p]
rt_rwl_unlock = rtai.rt_rwl_unlock
