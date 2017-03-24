ifndef KERNEL_DIR
$(error KERNEL_DIR must be set in the command line)
endif
PWD := $(shell pwd)

# This specifies the kernel module to be compiled
obj-m += dummy.o

# The default action
all: modules

# The main tasks
modules clean:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
		SUBDIRS=$(PWD) $@
