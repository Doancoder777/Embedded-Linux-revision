# ===== GNU MAKE: PHONY TARGET + VARIABLE ASSIGNMENT =====


# ===== .PHONY =====

.PHONY: rule1 rule2 rule3
# Phony target: target giả, không đại diện cho file thật.
# -> Recipe luôn được chạy, bất kể trong thư mục có file trùng tên hay không.

# Nếu KHÔNG khai báo .PHONY, và trong thư mục chứa Makefile
# có file trùng tên với target, ví dụ file "rule1":
#   make: 'rule1' is up to date.
# "up to date" = target đã mới nhất, Make bỏ qua recipe.

# Make mặc định coi target là TÊN FILE.
# Nó so sánh thời gian sửa file (timestamp) giữa target và prerequisite
# để quyết định có cần build lại hay không.
# -> Chỉ bỏ qua khi: file target đã tồn tại
#    VÀ không có prerequisite nào mới hơn nó.
# -> Nếu prerequisite mới hơn target, Make vẫn chạy recipe.


# ===== VARIABLE ASSIGNMENT (Phép gán biến) =====

var  := "var"
var3 := "var3"

-include abcd.mk
# Include directive: đọc thêm makefile khác.
# Dấu "-" phía trước = bỏ qua lỗi nếu file không tồn tại.

var1 = $(var)
# Recursively expanded variable (lazy evaluation)
# -> Chỉ mở rộng khi var1 được DÙNG.
# -> var đổi thì var1 đổi theo.

var2 := $(var)
# Simply expanded variable (immediate evaluation)
# -> Tính ngay tại dòng này.
# -> var đổi sau đó thì var2 KHÔNG đổi.

var3 ?= $(var)
# Conditional variable assignment
# -> Chỉ gán nếu var3 CHƯA có giá trị.
# -> Ở đây var3 đã có giá trị ("var3") nên dòng này bị bỏ qua.

var := "changed"
# Gán lại var SAU khi var1, var2 đã được định nghĩa.


# ===== RULES =====

rule3:
	@echo "$(var1)"
	@echo "$(var2)"
	@echo "$(var3)"
# Recipe: các lệnh của rule, phải bắt đầu bằng Tab.
# "@" = silent mode (không in lại chính câu lệnh).
# Kết quả dự đoán: "changed", "var", "var3"

rule1:
	echo "Hi im rule 1"

rule2:
	@echo "Hi im rule 2"
