prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
MODULE_CFLAGS = $(shell rtai-config --module-cflags)

all: namedstuff.o

%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

clean:
	rm -f *.o

.PHONY: clean
