prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: btest stest uctest utasklet utest kctest.o ktasklet.o

%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

btest: btest.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

stest: stest.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

uctest: uctest.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

utasklet: utasklet.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

utest: utest.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

clean:
	rm -f *.o btest stest uctest utasklet utest

.PHONY: clean
