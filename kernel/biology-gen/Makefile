PWD = $(shell pwd)

KERNEL_RELEASE ?= $(shell uname -r)
KERNEL_DIR ?= /lib/modules/$(KERNEL_RELEASE)/build

BUILD_COMMAND = make -C $(KERNEL_DIR) CFLAGS_MODULE='-Werror -DC_GEN'

.PHONY: build install clean

build:
	$(BUILD_COMMAND) M=$(PWD) modules

install: build
	insmod biology-gen.ko

clean:
	rm -rf src/*.o *.ko *.o *.mod *.mod.c *.order Module.symvers
