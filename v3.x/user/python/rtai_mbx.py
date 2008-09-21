from rtai_def import *


# mail boxes


rtai.rt_typed_mbx_init.argtypes = [c_ulong, c_int, c_int]
rtai.rt_typed_mbx_init.restype = c_void_p
rt_typed_mbx_init = rtai.rt_typed_mbx_init

def rt_mbx_init(name, size) :
	return rt_typed_mbx_init(name, size, FIFO_Q)

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
