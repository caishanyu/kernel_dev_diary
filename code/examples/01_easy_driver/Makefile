obj-m += helloworld.o	# 指定编译为内核模块，需要严格匹配.c文件名
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build  # 内核源码路径

# -C表示切换到内核源码路径下编译
# -M指定模块源码路径
# modules表示编译为内核模块
# clean表示内核模块clean

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

.PHONY: all clean