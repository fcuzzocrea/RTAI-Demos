prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)

ifneq ($(findstring 2.6.,$(shell rtai-config --linux-version 2>/dev/null)) || $(findstring 3.,$(shell rtai-config --linux-version 3>/dev/null)),)

LINUX_DIR = $(shell rtai-config --linux-dir)

all:
	$(MAKE) -C $(LINUX_DIR) CC=$(CC) SUBDIRS=$$PWD V=$(V) modules

clean:
	$(RM) $(LINUX_DIR)/.tmp_versions/*_rt.mod *.o *.ko *.mod.c .*.cmd *.order *.symvers

else

MODULE_CFLAGS = $(shell rtai-config --module-cflags)

all: testscsync.o

testscsync.o: testscsync.c
	$(CC) $(MODULE_CFLAGS) -c $<

clean:
	rm -f *.o

endif

.PHONY: clean
