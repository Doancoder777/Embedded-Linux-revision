
# Hello world kernel module

EXTRA_CFLAGS = -Wall                    # cờ gcc thêm (kbuild đọc ở lượt 2)
obj-m = hello.o                         # gán biến khai báo "muốn có hello.o"
                                        # -> kbuild tự tìm hello.c và build (implicit, không viết hello.o: hello.c)
                                        # -> sau đó link thành hello.ko

KDIR = /lib/modules/`uname -r`/build    # KDIR: thư mục build của kernel đang chạy

all:                                    # explicit rule: tự viết target + recipe
# -C: sang KDIR chạy make; M=: chỉ chỗ code module
	make -C $(KDIR) M=`pwd` modules 
# Ở pwd ở đây là thư mục chứa Makefile này, nơi có hello.c, hello.h, hello.mod.c, hello.ko, hello.o, v.v.
# -C (chữ hoa) là cờ của make: đổi sang thư mục khác (change directory) rồi mới chạy make ở đó.
# Output của make ở KDIR sẽ được đưa về thư mục hiện tại (pwd) nhờ M=`pwd`.

clean:                                  # explicit rule
# nhờ kernel dọn file build
# Dọn sạch các file build của module ở thư mục hiện tại (pwd) nhờ M=`pwd`.
	make -C $(KDIR) M=`pwd` clean