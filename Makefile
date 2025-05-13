obj-m += asic_diag.o

asic_diag-objs := src/asic_diag.o src/pcie_events.o src/sysfs_interface.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.c modules.order Module.symvers

test:
	$(MAKE) -C tests

.PHONY: all clean test 