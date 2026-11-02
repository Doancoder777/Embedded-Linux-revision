Vùng nhớ trong Linux là các phân vùng do người dùng (hoặc trình cài đặt) chia ra từ ổ cứng, thông tin chia được ghi trong bảng phân vùng trên ổ. Block device file là cái khe dẫn tới các vùng đó, chỉ chứa major và minor; kernel dùng hai số này để tra ra vùng đó bắt đầu từ đâu, kích thước bao nhiêu.


