prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

export LD_LIBRARY_PATH=$libpath:$LD_LIBRARY_PATH

all: util.o msg.o rtai_sysvipc.o testmsg

util.o: util.c linux2rtai.h util.h rtai_sysvmsg.h
	$(CC) $(MODULE_CFLAGS) -c $<

msg.o: msg.c linux2rtai.h util.h rtai_sysvmsg.h
	$(CC) $(MODULE_CFLAGS) -c $<

rtai_sysvipc.o: util.o msg.o
	ld -r -static -o rtai_sysvipc.o util.o msg.o

testmsg: testmsg.c linux2rtai.h rtai_sysvmsg.h
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

clean:
	rm -f *.o testmsg

.PHONY: clean
