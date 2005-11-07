prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: test

test: test.c
	$(CC) -I/usr/xenomai/include/ $(LXRT_CFLAGS) -g -o $@ $< $(LXRT_LDFLAGS) -llxrt -L/usr/xenomai/lib/ -lrtdm -lnative

clean:
	rm -f *.o test

.PHONY: clean
