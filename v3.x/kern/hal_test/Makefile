EXTRA_CFLAGS += $(shell rtai-config --module-cflags)

obj-m += sysreq_rt.o sysreqwt_rt.o lnxshintr_rt.o

sysreq_rt-objs := sysreq.o

sysreqwt_rt-objs := sysreqwt.o

lnxshintr_rt-objs := lnxshintr.o
