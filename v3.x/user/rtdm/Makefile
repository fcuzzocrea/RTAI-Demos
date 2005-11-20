prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: test ftest

test: test.c
	$(CC) $(LXRT_CFLAGS) -g -o $@ $< $(LXRT_LDFLAGS) -llxrt

ftest: ftest.c
	$(CC) -I/usr/xenomai/include/ $(LXRT_CFLAGS) -g -o $@ $< $(LXRT_LDFLAGS) -llxrt -L/usr/xenomai/lib/ -lrtdm -lnative

clean:
	rm -f test ftest

.PHONY: clean
