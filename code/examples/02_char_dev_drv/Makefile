obj-m += char_dev_drv.o
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

usr_program := ioctl_test
usr_src := ioctl_test.c
CC := gcc

all: modules $(usr_program)

modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

$(usr_program) : $(usr_src)
	$(CC) -o $@ $^

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	rm -rf $(usr_program)

.PHONY: all clean modules