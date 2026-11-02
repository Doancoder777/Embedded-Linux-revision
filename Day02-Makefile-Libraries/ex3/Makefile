# ===== MAKEFILE: IMPLICIT RULE TỰ VIẾT VÀ HEADER DEPENDENCY =====


.PHONY: all test clean

# Chỉ khai báo target KHÔNG tạo file (all, test, clean).
# Target tạo file thật (hellomake, hello) thì KHÔNG đưa vào .PHONY,
# để Make so timestamp và chỉ build lại khi cần (incremental build).


CC        := gcc
CFLAGS    := -I.

# -I. : thêm thư mục hiện tại vào đường dẫn tìm header (include path).
# "-I" đứng một mình (không có thư mục) là LỖI.

INC_FILES := hello.h


all: hellomake

# Đặt "all" ĐẦU TIÊN để "make" trơn chạy hellomake (default goal).


# ----- IMPLICIT RULE TỰ VIẾT: .c -> .o -----

# Implicit rule có sẵn của Make chỉ biết: file.o cần file.c.
# -> Nó KHÔNG biết file.c dùng header nào (header dependency).
# -> Sửa hello.h thì Make không build lại và KHÔNG báo lỗi (silent bug).

# Viết lại implicit rule dựa trên 2 dòng lệnh Make tự sinh ra khi chạy:
#
#   gcc -I.   -c -o main.o main.c
#   gcc -I.   -c -o hello.o hello.c
#
# Hai dòng chỉ khác nhau ở tên file, nên thay bằng % và biến tự động:
#   main.o, hello.o  ->  $@   (tên target)
#   main.c, hello.c  ->  $<   (dependency đầu tiên)

# Implicit rule tự viết bên dưới được ưu tiên hơn rule có sẵn
# (user-defined rule overrides built-in rule),
# và thêm $(INC_FILES) làm dependency để sửa header là build lại.

%.o: %.c $(INC_FILES)
	$(CC) $(CFLAGS) -c -o $@ $<

# Lưu ý: nó là pattern rule (có %), một dạng implicit rule, chưa phải explicit rule.
# Explicit rule (tường minh) ghi rõ tên file, ví dụ:
#
#   main.o: main.c hello.h


# ----- TEST: xem biến tự động (automatic variables) -----

test: hello.h hello.c
	@echo $@
	@echo $<
	@echo $^

# $@ = tên target, $< = dependency đầu tiên, $^ = tất cả dependency


# ----- BUILD -----

hellomake: main.o hello.o
	$(CC) -o $@ main.o hello.o

# Bước link: .o -> file chạy. Cờ -I chỉ dùng lúc compile, không cần ở đây.


hello: main.c hello.c $(INC_FILES)
	$(CC) $(CFLAGS) -o $@ main.c hello.c

# Compile + link một lệnh. Có dependency nên sửa file thì tự build lại.
# Nếu không có dependency, lần sau Make sẽ báo "up to date" dù đã sửa code.


clean:
	rm -f hello hellomake *.o