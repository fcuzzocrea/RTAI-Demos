prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)

LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)

all:: screen keybrd

ifneq ($(findstring 2.6.,$(shell rtai-config --linux-version 2>/dev/null)),)

LINUX_DIR = $(shell rtai-config --linux-dir)

all::
	$(MAKE) -C $(LINUX_DIR) CC=$(CC) SUBDIRS=$$PWD V=$(V) modules

clean::
	$(RM) $(LINUX_DIR)/.tmp_versions/*_rt.mod *.o *.ko *.mod.c .*.cmd

else

MODULE_CFLAGS = $(shell rtai-config --module-cflags)

all:: clock_rt.o cmdclk_rt.o cmdcrn_rt.o dispclk_rt.o

clock%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

cmdclk%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

cmdcrn%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

dispclk%.o: %.c
	$(CC) $(MODULE_CFLAGS) -c $<

clean::
	$(RM) *.o

endif

screen: screen.c
	$(CC) $(LXRT_CFLAGS) -o $@ $<

keybrd: keybrd.c
	$(CC) $(LXRT_CFLAGS) -o $@ $<

clean::
	$(RM) screen keybrd

.PHONY: clean
