from rtai_def import *


# non blocking shared memory circular buffers


HDRSIZ = (3*sizeof(c_int))

rtai.rt_scb_init.argtypes = [c_ulong, c_int, c_ulong]
rtai.rt_scb_init.restype = c_void_p
rt_scb_init = rtai.rt_scb_init

rtai.rt_scb_delete.argtypes = [c_ulong]
rt_scb_delete = rtai.rt_scb_delete

rtai.rt_scb_bytes.argtypes = [c_void_p]
rt_scb_bytes = rtai.rt_scb_bytes

rtai.rt_scb_get.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_get = rtai.rt_scb_get

rtai.rt_scb_evdrp.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_evdrp = rtai.rt_scb_evdrp

rtai.rt_scb_put.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_put = rtai.rt_scb_put

rtai.rt_scb_ovrwr.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_ovrwr = rtai.rt_scb_ovrwr
