ifndef KERNEL_DIR
$(error KERNEL_DIR must be set in the command line)
endif
PWD := $(shell pwd)
CROSS_COMPILE = arm-linux-gnueabihf-

obj-m = pulse.o
obj-m += pulse-gpio.o

all: modules

modules clean:
	$(MAKE) -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) \
		SUBDIRS=$(PWD) $@
