# ============================================================
# STATIC LIBRARY (THƯ VIỆN TĨNH)
# ============================================================

.PHONY: all static link clean


# ============================================================
# 1. TẠO STATIC LIBRARY
# ============================================================

static:
	ar rcs lib/static/libhello.a obj/helloWorld.o obj/helloDoan.o

# Giải thích:
#   ar : archiver, công cụ tạo và quản lý thư viện tĩnh từ các file .o
#   r  : thêm file .o vào thư viện; nếu đã có thì thay bằng bản mới (replace)
#   c  : create, tạo thư viện nếu chưa có.
#        Không có c thì ar vẫn tạo, nhưng in cảnh báo "ar: creating ..."
#   s  : tạo bảng chỉ mục (index) tên các symbol trong thư viện,
#        giúp linker tìm hàm cần dùng nhanh hơn
#
# Ví dụ:
#   ar rcs libhello.a hello.o
#   => đưa hello.o vào thư viện libhello.a
#
#   lib/static/libhello.a : tên và đường dẫn thư viện sẽ tạo
#   obj/*.o               : các file object được đóng gói vào thư viện

# Sơ đồ:
#
#   helloWorld.c ----> helloWorld.o ----+
#                                       +----> libhello.a
#   helloDoan.c -----> helloDoan.o -----+
#
#   main.c ----> main.o      (nằm NGOÀI thư viện)

# LƯU Ý: KHÔNG đưa obj/main.o vào thư viện.
#   main.o chứa hàm main(), là điểm vào của riêng chương trình.
#   Chỉ đóng gói các module cung cấp chức năng dùng lại được.

# Kiểm tra nội dung thư viện:
#   ar t lib/static/libhello.a    liệt kê các file .o bên trong
#   nm lib/static/libhello.a      liệt kê symbol
#                                 T = được định nghĩa trong file này
#                                 U = còn thiếu, cần lấy từ nơi khác

# ============================================================
# 2. LINK CHƯƠNG TRÌNH VỚI STATIC LIBRARY
# ============================================================

link:
	gcc obj/main.o -Llib/static -lhello -o bin/exam
				
# 				  hoặc 
#	gcc obj/main.o -L lib/static -l hello -o bin/exam


# Giải thích:
#   obj/main.o : object chứa hàm main() của chương trình
#
#   -Llib/static : chỉ định thư mục để linker tìm thư viện.
#                  Không có -L thì linker không tự biết thư viện nằm đâu
#                  (lỗi: cannot find -lhello)
#
#   -lhello : yêu cầu linker tìm thư viện có tên "hello"
#
#   -o bin/exam : tạo file thực thi tại bin/exam

# Quy ước đặt tên thư viện:
#   libhello.a  ->  -lhello
#   libmath.a   ->  -lmath
#   libabc.so   ->  -labc
#
# Cách suy ra: libhello.a
#              ^^^          bỏ tiền tố "lib"
#                 ^^^^^     tên thư viện: hello
#                      ^^   bỏ phần mở rộng ".a"
#              Kết quả: -lhello

# Điều kiện để -lhello tìm được thư viện:
#   1. File thư viện đặt tên theo quy ước (ví dụ libhello.a)
#   2. Thư mục chứa thư viện được chỉ định bằng -L,
#      hoặc nằm trong thư mục linker tự tìm kiếm (ví dụ /usr/lib)
#   3. Thư viện tồn tại và đọc được
#   4. Thư viện cung cấp đủ symbol mà chương trình cần
#
# Nếu không đảm bảo:
#   - cannot find -lhello       : không tìm thấy thư viện
#   - undefined reference to .. : thiếu hàm cần thiết

# File KHÔNG có tiền tố lib (ví dụ hello.a):
#   Cách 1: ghi thẳng đường dẫn
#           gcc obj/main.o lib/static/hello.a -o bin/exam
#   Cách 2: dùng -l: (GNU ld)
#           gcc obj/main.o -Llib/static -l:hello.a -o bin/exam

# THỨ TỰ link quan trọng:
#   Đặt obj/main.o TRƯỚC -lhello.
#   Linker đọc từ trái sang phải: gặp main.o trước nên biết main.o đang cần
#   symbol nào, rồi mới tìm các symbol đó trong thư viện.
#   Đặt -lhello trước main.o thì lỗi undefined reference.

# Linker chỉ lấy module CẦN DÙNG trong thư viện.
#   Nếu chương trình chỉ gọi helloWorld() thì helloDoan.o
#   không được đưa vào file thực thi.


# ============================================================
# 3. BIÊN DỊCH CÁC FILE .c THÀNH FILE OBJECT .o
# ============================================================

all:
	gcc -c main.c -o obj/main.o -I./inc
	gcc -c src/helloWorld.c -o obj/helloWorld.o -I./inc
	gcc -c src/helloDoan.c -o obj/helloDoan.o -I./inc

# Giải thích:
#   -c       : chỉ biên dịch, chưa link thành chương trình thực thi
#   -o       : tên file đầu ra
#   -I./inc  : tìm các file header trong thư mục inc


# ============================================================
# 4. DỌN DẸP FILE ĐÃ TẠO
# ============================================================

clean:
	rm -rf obj/* bin/*
	rm -f lib/static/libhello.a

# Giải thích:
#   Xóa các file object, file thực thi và static library đã tạo.