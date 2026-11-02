.PHONY: stage1 stage2 stage3 stage4 all clean

# =========================================================================
# Thay vì build hết ra file object luôn 1 lần trong một target duy nhất, 
# ta sẽ build từng step (tách thành các rule riêng cho từng file .o).
# 
# Nhờ có "cơ chế kiểm tra dependency (sự phụ thuộc)" và mốc thời gian (timestamp) của Makefile, 
# nên ta có thể "chỉ biên dịch lại những file mã nguồn (.c) bị chỉnh sửa" (Incremental Build), 
# thay vì phải biên dịch lại toàn bộ project từ đầu, giúp tiết kiệm rất nhiều thời gian.
# =========================================================================

# Target hello phụ thuộc vào 2 file object: main.o và hello_A.o
hello: main.o hello_A.o
	gcc -o hello main.o hello_A.o 

# (Tuỳ chọn) Bạn có thể tạo thêm target để test link với bản B
hello_alt: main.o hello_B.o
	gcc -o hello main.o hello_B.o

# --- Build từng step cho các file object ---

# Nếu main.c thay đổi, chỉ đoạn này chạy lại để tạo lại main.o
main.o: main.c
	gcc -c -o main.o main.c

# Nếu hello_A.c thay đổi, chỉ đoạn này chạy lại để tạo lại hello_A.o
hello_A.o: hello_A.c
	gcc -c -o hello_A.o hello_A.c

# Nếu hello_B.c thay đổi, chỉ đoạn này chạy lại để tạo lại hello_B.o
hello_B.o: hello_B.c
	gcc -c -o hello_B.o hello_B.c


# =========================================================================
# Các giai đoạn biên dịch C tiêu chuẩn như bài trước
# =========================================================================
stage1:
	gcc -E hello.c -o hello.i

stage2:
	gcc -S hello.i -o hello.S

stage3:
	gcc -c hello.S -o hello.o

stage4:
	gcc -o hello hello.o

all:
	gcc -o hello hello.c

clean:
	rm -rf hello hello.i hello.S hello.o hello_A.o hello_B.o main.o