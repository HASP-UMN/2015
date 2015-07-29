TARGET = fifo_driver
#TARGET = hello
ifneq ($(KERNELRELEASE),)

obj-m := $(TARGET).o

else

KERNELDIR ?= /usr/src/linux
PWD := $(shell pwd)

default:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
	rm -rf *.o *.mod.c

endif

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf *.o *.ko *.mod.c

