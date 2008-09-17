from ctypes import *

cdll.LoadLibrary("libc.so.6")
libc = CDLL("libc.so.6")

cdll.LoadLibrary("liblxrt.so")
rtai = CDLL("liblxrt.so")

rtai.rt_task_init_schmod.restype = c_void_p
rtai.rt_task_delete.argtypes = [c_void_p]
rtai.rt_get_adr.restype = c_void_p
rtai.rt_mbx_receive.argtypes = [c_void_p, c_void_p, c_long]
rtai.rt_rpc.argtypes = [c_void_p, c_ulong, c_void_p]

class SAMP(Structure) :
	_fields_ = [("max", c_longlong), 
	            ("min", c_longlong),
		    ("index", c_int),
		    ("ovrn", c_int)]
samp = SAMP(0, 0, 0, 0)

class POLL(Structure) :
	_fields_ = [("max", c_int), 
	            ("min", c_int)]
ufds = POLL(0, 1)

max = -1000000000
min = 1000000000
msg = c_ulong(0)

task = rtai.rt_task_init_schmod(rtai.nam2num("LATCHK"), 20, 0, 0, 0, 0xf)
mbx = rtai.rt_get_adr(rtai.nam2num("LATMBX"))
rtai.rt_make_hard_real_time()

while 1 :
	rtai.rt_mbx_receive(mbx, byref(samp), sizeof(samp))
	if max < samp.max :
		max = samp.max
	if min > samp.min :
		min = samp.min
	print "* min:", samp.min, "/", min, "max:", samp.max, "/", max, "average: ", samp.index, "(", samp.ovrn, ") <Hit [RETURN] to stop> *\n"
	if libc.poll(byref(ufds), 1, 1) :
		ch = libc.getchar()
		break

print "* SENDING END MESSAGE TO LATENCY *"
rtai.rt_rpc(rtai.rt_get_adr(rtai.nam2num("LATCAL")), msg, byref(msg))
rtai.rt_make_soft_real_time()
rtai.rt_task_delete(task)
print "* EXITING DISPLAY *"
