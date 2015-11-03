COMEDI_DIR = $(shell rtai-config --comedi-dir)
EXTRA_CFLAGS += $(shell rtai-config --module-cflags)
EXTRA_CFLAGS += -I/usr/include -I$(COMEDI_DIR)/include

obj-m += kcomedi_rt.o

kcomedi_rt-objs := kcomedi.o
