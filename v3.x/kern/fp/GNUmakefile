prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)

LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)

all:: display

ifneq ($(findstring 2.6.,$(shell rtai-config --linux-version 2>/dev/null)),)

LINUX_DIR = $(shell rtai-config --linux-dir)

all:: fp_rt.ko

fp_rt.ko: _fp_rt.ko
	$(LD) -r -static -o $@ $< -L/usr/lib -lm

_fp_rt.ko:
	$(MAKE) -C $(LINUX_DIR) CC=$(CC) SUBDIRS=$$PWD V=$(V) modules

clean::
	$(RM) $(LINUX_DIR)/.tmp_versions/*_rt.mod *.o *.ko *.mod.c .*.cmd

else

MODULE_CFLAGS = $(shell rtai-config --module-cflags)

all:: fp_rt.o

fp_rt.o: fp.c
	$(CC) $(MODULE_CFLAGS) -c $< -o _$@
	$(LD) -r -static -o $@ _$@ -L/usr/lib -lm

clean::
	$(RM) *.o

endif

display: display.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< -lpthread -lm

clean::
	$(RM) display

.PHONY: clean
