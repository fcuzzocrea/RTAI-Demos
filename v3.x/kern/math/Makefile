EXTRA_CFLAGS += $(shell rtai-config --module-cflags) -ffast-math -msse -mno-sse2 -mpreferred-stack-boundary=4 -I/usr/include -D_IEEE_LIBM -D_ISOC99_SOURCE -D_SVID_SOURCE

obj-m += fp.o

