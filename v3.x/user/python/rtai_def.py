from ctypes import *

cdll.LoadLibrary("libc.so.6")
libc = CDLL("libc.so.6")

cdll.LoadLibrary("liblxrt.so")
rtai = CDLL("liblxrt.so")

NULL = None

PRIO_Q = 0
FIFO_Q = 4
RES_Q  = 3

BIN_SEM = 1
CNT_SEM = 2
RES_SEM = 3

RESEM_RECURS = 1
RESEM_BINSEM = 0
RESEM_CHEKWT = -1

GLOBAL_HEAP_ID = 0x9ac6d9e7

USE_VMALLOC = 0
USE_GFP_KERNEL = 1
USE_GFP_ATOMIC = 2
USE_GFP_DMA = 3

