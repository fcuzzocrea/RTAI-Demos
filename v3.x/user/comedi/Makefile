prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all: cmd insn aocmd

cmd: cmd.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -lkcomedilxrt

insn: insn.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -lkcomedilxrt -lm

aocmd: aocmd.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -lkcomedilxrt -lm

clean:
	rm -f *.o cmd insn aocmd

.PHONY: clean
