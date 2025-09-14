obj-m += netlink_kernel.o
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
CC := gcc
NL_USR_SRC := netlink_user.c
NL_USR := netlink_user

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	$(CC) $(NL_USR_SRC) -o $(NL_USR)

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
	rm -rf $(NL_USR)

.PHONY: all clean