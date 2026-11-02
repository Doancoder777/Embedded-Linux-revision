.PHONY: stage1 stage2 stage3 stage4 all clean

# 1. Giai đoạn tiền xử lý (Pre-processing)
# - Loại bỏ comments, mở rộng macros, mở rộng include file, biên dịch câu lệnh điều kiện.
# - Kết quả thu được là một file ".i".
stage1:
	gcc -E hello.c -o hello.i

# 2. Giai đoạn dịch ngôn ngữ bậc cao sang asm (Compilation)
# - Mã nguồn tiếp tục thực hiện biên dịch từ file ".i" thu được ở bước trước thành một file ".s" (assembly).
stage2:
	gcc -S hello.i -o hello.S

# 3. Giai đoạn dịch asm sang mã máy (Assembly)
# - File ".s" ở giai đoạn trước tiếp tục được sử dụng. Thông qua assembler, output thu được là một file ".o" (chứa các lệnh cấp độ ngôn ngữ máy).
stage3:
	gcc -c hello.S -o hello.o

# 4. Giai đoạn Linker (Linking)
# - Liên kết các file ".o" (hoặc từ object) để thu được một file thực thi hoàn chỉnh.
stage4:
	gcc -o hello hello.o

# Build nhanh trực tiếp từ .c sang file thực thi
all:
	gcc -o hello hello.c

# Xóa các file được sinh ra sau quá trình biên dịch
clean:
	rm -rf hello hello.i hello.S hello.o