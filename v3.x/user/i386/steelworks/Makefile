prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: handler.o press

%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

press: press.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< -lncurses $(LXRT_LDFLAGS)

clean:
	rm -f *.o press

.PHONY: clean
