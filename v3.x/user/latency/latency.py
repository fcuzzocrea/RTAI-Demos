from rtai import *

AVRGTIME = 1
PERIOD = 100000
TIMER_MODE = 0

SMPLSXAVRG = ((1000000000*AVRGTIME)/PERIOD)/10

class SAMP(Structure) :
	_fields_ = [("min", c_longlong),
		    ("max", c_longlong),
		    ("index", c_int),
		    ("ovrn", c_int)]
samp = SAMP(0, 0, 0, 0)
msg = c_ulong(0)
min_diff = 1000000000
max_diff = -1000000000
diff = 0
average = 0
t = c_longlong(0)
svt = c_longlong(0)
expected = c_longlong(0)
latchk = c_void_p(NULL)
sample = 0

mbx = rt_mbx_init(nam2num("LATMBX"), 20*sizeof(samp))
if mbx == NULL :
	libc.libc.printf("CANNOT CREATE MAILBOX\n")
	exit(1)

task = rt_task_init_schmod(nam2num("LATCAL"), 0, 0, 0, 0, 0xF)
if task == 0 :
	libc.printf("CANNOT INIT MASTER LATENCY TASK\n")
	exit(1)

libc.printf("\n## RTAI latency calibration tool ##\n")
libc.printf("# period = %i (ns) \n", PERIOD)
libc.printf("# average time = %i (s)\n", AVRGTIME)
libc.printf("# use the FPU\n")
if TIMER_MODE == 0 :
	libc.printf("# timer_mode is oneshot\n")
else :
	libc.printf("# timer_mode is periodic\n")
libc.printf("\n")

hard_timer_running = rt_is_hard_timer_running()
if hard_timer_running == 0 :
	if TIMER_MODE != 0 :
		rt_set_periodic_mode()
	elif TIMER_MODE == 0 :
		rt_set_oneshot_mode()
	period = start_rt_timer(nano2count(PERIOD))
else :
	period = nano2count(PERIOD)

rt_make_hard_real_time()
expected = rt_get_time() + 200*period
rt_task_make_periodic(task, expected, period)
svt = rt_get_cpu_time_ns()

while True :
	min_diff = 1000000000
	max_diff = -1000000000
	average = 0
	sample = SMPLSXAVRG

	while sample > 0 :
		sample -= 1
		expected += period
		if rt_task_wait_period() == 0 :
			if TIMER_MODE != 0 :
				t = rt_get_cpu_time_ns()
				diff = t - svt - PERIOD
				svt = t
			else :
				diff = count2nano(rt_get_time() - expected)
		else :
			samp.ovrn += 1
			diff = 0
			if TIMER_MODE != 0 :
				svt = rt_get_cpu_time_ns()
		if diff < min_diff :
			min_diff = diff
		if diff > max_diff :
			max_diff = diff
		average += diff

	samp.min = min_diff
	samp.max = max_diff
	samp.index = average/SMPLSXAVRG
	rt_mbx_send_if(mbx, byref(samp), sizeof(samp))
	latchk = rt_receive_if(rt_get_adr(nam2num("LATCHK")), byref(msg))
	if latchk != NULL :
		rt_return(latchk, msg)
		break

rt_make_soft_real_time()
while rt_get_adr(nam2num("LATCHK")) != NULL :
	rt_sleep(nano2count(1000000))
if hard_timer_running == 0 :
	stop_rt_timer()	
rt_task_delete(task)
rt_mbx_delete(mbx)
