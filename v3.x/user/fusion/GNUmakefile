prefix := $(shell rtai-config --prefix)

ifeq ($(prefix),)
$(error Please add <rtai-install>/bin to your PATH variable)
endif

CC = $(shell rtai-config --cc)
LXRT_CFLAGS = $(shell rtai-config --lxrt-cflags)
LXRT_LDFLAGS = $(shell rtai-config --lxrt-ldflags)

all:: switch latency latwdisp display

ifneq ($(findstring 2.6.,$(shell rtai-config --linux-version 2>/dev/null)),)

LINUX_DIR = $(shell rtai-config --linux-dir)

all::
	$(MAKE) -C $(LINUX_DIR) CC=$(CC) SUBDIRS=$$PWD V=$(V) modules

clean::
	$(RM) $(LINUX_DIR)/.tmp_versions/*_rt.mod *.o *.ko *.mod.c .*.cmd

else

MODULE_CFLAGS = $(shell rtai-config --module-cflags)

all:: intrlat.o

intrlat.o: intrlat.c
	$(CC) $(MODULE_CFLAGS) -c $<

clean::
	$(RM) *.o

endif

switch: switch.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

latency: latency.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -lm -llxrt

latwdisp: latwdisp.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< -llxrt $(LXRT_LDFLAGS) -lm -llxrt

display: display.c
	$(CC) $(LXRT_CFLAGS) -o $@ $< $(LXRT_LDFLAGS) -llxrt

clean::
	$(RM) switch latency latwdisp display

.PHONY: clean
