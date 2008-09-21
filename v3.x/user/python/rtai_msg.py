from rtai_def import *


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

rtai.rt_rpc_until.argtypes = [c_void_p, c_ulong, c_void_p, c_longlong]
rtai.rt_rpc_until.restype = c_void_p
rt_rpc_until = rtai.rt_rpc_until

rtai.rt_rpc_timed.argtypes = [c_void_p, c_ulong, c_void_p, c_longlong]
rtai.rt_rpc_timed.restype = c_void_p
rt_rpc_timed = rtai.rt_rpc_timed

rtai.rt_isrpc.argtypes = [c_void_p]
rt_isrpc = rtai.rt_isrpc

rtai.rt_return.argtypes = [c_void_p, c_ulong]
rtai.rt_return.restype = c_void_p
rt_return = rtai.rt_return

rtai.rt_sendx.argtypes = [c_void_p, c_void_p, c_int]
rtai.rt_sendx.restype = c_void_p
rt_sendx = rtai.rt_sendx

rtai.rt_sendx_if.argtypes = [c_void_p, c_void_p, c_int]
rtai.rt_sendx_if.restype = c_void_p
rt_sendx_if = rtai.rt_sendx_if

rtai.rt_sendx_until.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rtai.rt_sendx_until.restype = c_void_p
rt_sendx_until = rtai.rt_sendx_until

rtai.rt_sendx_timed.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rtai.rt_sendx_timed.restype = c_void_p
rt_sendx_timed = rtai.rt_sendx_timed

rtai.rt_evdrpx.argtypes = [c_void_p, c_void_p, c_int, c_void_p]
rtai.rt_evdrpx.restype = c_void_p
rt_evdrpx = rtai.rt_evdrpx

rtai.rt_receivex.argtypes = [c_void_p, c_void_p, c_int, c_void_p]
rtai.rt_receivex.restype = c_void_p
rt_receivex = rtai.rt_receivex

rtai.rt_receivex_if.argtypes = [c_void_p, c_void_p, c_int, c_void_p]
rtai.rt_receivex_if.restype = c_void_p
rt_receivex_if = rtai.rt_receivex_if

rtai.rt_receivex_until.argtypes = [c_void_p, c_void_p, c_int, c_void_p, c_longlong]
rtai.rt_receivex_until.restype = c_void_p
rt_receivex_until = rtai.rt_receivex_until

rtai.rt_receivex_timed.argtypes = [c_void_p, c_void_p, c_int, c_void_p, c_longlong]
rtai.rt_receivex_timed.restype = c_void_p
rt_receivex_timed = rtai.rt_receivex_timed

rtai.rt_rpcx.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int]
rtai.rt_rpcx.restype = c_void_p
rt_rpcx = rtai.rt_rpcx

rtai.rt_rpcx_if.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int]
rtai.rt_rpcx_if.restype = c_void_p
rt_rpcx_if = rtai.rt_rpcx_if

rtai.rt_rpcx_until.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_longlong]
rtai.rt_rpcx_until.restype = c_void_p
rt_rpcx_until = rtai.rt_rpcx_until

rtai.rt_rpcx_timed.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_longlong]
rtai.rt_rpcx_timed.restype = c_void_p
rt_rpcx_timed = rtai.rt_rpcx_timed

def rt_isrpcx(task) :
	return rt_isrpc(task)

rtai.rt_returnx.argtypes = [c_void_p, c_void_p, c_int]
rtai.rt_returnx.restype = c_void_p

rtai.rt_proxy_attach.argtypes = [c_void_p, c_void_p, c_int, c_int]
rtai.rt_proxy_attach.restype = c_void_p
rt_proxy_attach = rtai.rt_proxy_attach

rtai.rt_proxy_detach.argtypes = [c_void_p]
rt_proxy_detach = rtai.rt_proxy_detach

rtai.rt_trigger.argtypes = [c_void_p]
rtai.rt_trigger.restype = c_void_p
rt_trigger = rtai.rt_trigger

rtai.rt_Send.argtypes = [c_long, c_void_p, c_void_p, c_long, c_long]
rt_Send = rtai.rt_Send

rtai.rt_Receive.argtypes = [c_long, c_void_p, c_long, c_long]
rt_Receive = rtai.rt_Receive

rtai.rt_Creceive.argtypes = [c_long, c_void_p, c_long, c_void_p, c_longlong]
rt_Creceive = rtai.rt_Creceive

rtai.rt_Reply.argtypes = [c_long, c_void_p, c_long]
rt_Reply = rtai.rt_Reply

rtai.rt_Proxy_attach.argtypes = [c_long, c_void_p, c_int, c_int]
rt_Proxy_attach = rtai.rt_Proxy_attach

rt_Proxy_detach = rtai.rt_Proxy_detach

rt_Alias_attach = rtai.rt_Alias_attach

rt_Name_locate = rtai.rt_Name_locate

rt_Name_detach = rtai.rt_Name_detach
