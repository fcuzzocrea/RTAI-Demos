prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)

all: speaker.o player

%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

player: player.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< -lpthread -lm

clean:
	rm -f *.o player

.PHONY: clean
