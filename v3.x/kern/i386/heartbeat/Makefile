EXTRA_CFLAGS += $(shell rtai-config --module-cflags)

obj-m += heartbeat_rt.o semtest_rt.o mutextest_rt.o eventest_rt.o

heartbeat_rt-objs := heartbeat.o
semtest_rt-objs := semtest.o
mutextest_rt-objs := mutextest.o
eventest_rt-objs := eventest.o
