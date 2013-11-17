EXTRA_CFLAGS += $(shell rtai-config --module-cflags) -ffast-math -msse -mno-sse2 -I/usr/include -D_IEEE_LIBM -D_ISOC99_SOURCE -D_SVID_SOURCE

obj-m += libm_rt.o fp_rt.o

libm_rt-objs := libm.o
fp_rt-objs    := fp.o

