prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags) -D_GNU_SOURCE
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)
RENAME = $(shell rtai-config --posix-wrap)

all: test

test: test.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(RENAME) -llxrt $(LXRT_LDFLAGS)

clean:
	rm -f *.o test

.PHONY: clean
