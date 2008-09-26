from rtai_def import *


# spinlocks


rtai.rt_spl_init.argtypes = [c_ulong]
rtai.rt_spl_init.restype = c_void_p
rt_spl_init = rtai.rt_spl_init

rtai.rt_spl_delete.argtypes = [c_void_p]
rt_spl_delete = rtai.rt_spl_delete

rtai.rt_named_spl_init.argtypes = [c_void_p]
rtai.rt_named_spl_init.restype = c_void_p
rt_named_spl_init = rtai.rt_named_spl_init

rtai.rt_named_spl_delete.argtypes = [c_void_p]
rt_named_spl_delete = rtai.rt_named_spl_delete

rtai.rt_spl_lock.argtypes = [c_void_p]
rt_spl_lock = rtai.rt_spl_lock

rtai.rt_spl_lock_if.argtypes = [c_void_p]
rt_spl_lock_if = rtai.rt_spl_lock_if

rtai.rt_spl_lock_timed.argtypes = [c_void_p, c_longlong]
rt_spl_lock_timed = rtai.rt_spl_lock_timed

rtai.rt_spl_unlock.argtypes = [c_void_p]
rt_spl_unlock = rtai.rt_spl_unlock
