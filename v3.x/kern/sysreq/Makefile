prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)

all: sysreq.o display

%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

display: display.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< -lpthread

clean:
	rm -f *.o display

.PHONY: clean
