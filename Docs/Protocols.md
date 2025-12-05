

| Chức năng | Thông điệp yêu cầu (Request) | Thông điệp trả lời (Response) |
| :---- | :---- | :---- |
| Kết nối server | (Client kết nối) | 100: Kết nối thành công |
| Đăng nhập | LOGIN \<user\> \<pass\> | 110: Đăng nhập thành công 401: Tài khoản hoặc mật khẩu sai 402: Tài khoản không tồn tại 403: Phiên đã được đăng nhập trước đó 300: Sai cú pháp |
| Đăng ký | REGISTER \<user\> \<pass\> | 120: Đăng ký thành công 501: Username đã tồn tại 403: Phiên đã được đăng nhập 300: Sai cú pháp |
| Đăng xuất | LOGOUT | 130: Đăng xuất thành công 400: Chưa đăng nhập 300: Sai cú pháp |
| Upload file | UPLOAD \<path\> \<size\> | 140: Upload thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 502: Lỗi ghi file trên server 300: Sai cú pháp |
| Download file | DOWNLOAD \<path\> | 150: Download thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: File không tồn tại 300: Sai cú pháp |
| Xin vào nhóm | JOIN \<group\_name\> | 160: Gửi yêu cầu thành công 400: Chưa đăng nhập 405: Đã ở trong nhóm này rồi 500: Nhóm không tồn tại 300: Sai cú pháp |
| Duyệt thành viên | APPROVE \<username\> | 170: Phê duyệt thành công 400: Chưa đăng nhập 406: Không phải trưởng nhóm 500: Không tìm thấy yêu cầu từ user này 300: Sai cú pháp |
| Mời vào nhóm | INVITE \<username\> | 180: Gửi lời mời thành công 400: Chưa đăng nhập 406: Không phải trưởng nhóm 405: User này đã ở trong nhóm 300: Sai cú pháp |
| Chấp nhận mời | ACCEPT \<group\_name\> | 190: Gia nhập thành công 400: Chưa đăng nhập 407: User hiện tại đang có nhóm khác 300: Sai cú pháp |
| Rời nhóm | LEAVE | 200: Rời nhóm thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 300: Sai cú pháp |
| Xóa thành viên | KICK \<username\> | 201: Xóa thành viên thành công 400: Chưa đăng nhập 406: Không phải trưởng nhóm 500: Thành viên không có trong nhóm 300: Sai cú pháp |
| Tạo nhóm | CREATE \<group\_name\> | 202: Tạo nhóm thành công 400: Chưa đăng nhập 407: User đang ở trong nhóm khác 501: Tên nhóm đã tồn tại 300: Sai cú pháp |
| Xem DS nhóm | LIST\_GROUPS | 203: Trả về danh sách thành công 400: Chưa đăng nhập 300: Sai cú pháp |
| Xem thành viên | LIST\_MEMBERS | 204: Trả về danh sách thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 300: Sai cú pháp |
| Xem yêu cầu xin vào | LIST\_REQUESTS | 205: Trả về danh sách yêu cầu 400: Chưa đăng nhập 406: Không phải trưởng nhóm 300: Sai cú pháp |
| Sửa tên file | RENAME\_FILE \<old\> \<new\> | 210: Đổi tên thành công 500: File không tồn tại 501: Tên mới bị trùng 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 406: Không phải trưởng nhóm 300: Sai cú pháp |
| Xóa file | DELETE\_FILE \<path\> | 211: Xóa thành công 500: File không tồn tại 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 406: Không phải trưởng nhóm 300: Sai cú pháp |
| Copy file | COPY\_FILE \<src\> \<dest\> | 212: Copy thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: File nguồn không tồn tại 503: Đường dẫn đích không hợp lệ 300: Sai cú pháp |
| Di chuyển file | MOVE\_FILE \<src\> \<dest\> | 213: Di chuyển thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: File nguồn không tồn tại 503: Đường dẫn đích không hợp lệ 300: Sai cú pháp |
| Tạo folder | MKDIR \<path\> | 220: Tạo folder thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 501: Folder đã tồn tại 300: Sai cú pháp |
| Sửa tên folder | RENAME\_FOLDER \<old\> \<new\> | 221: Đổi tên thành công 500: Folder không tồn tại 501: Tên mới bị trùng 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 406: Không phải trưởng nhóm 300: Sai cú pháp |
| Xóa folder | RMDIR \<path\> | 222: Xóa thành công 500: Folder không tồn tại 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 406: Không phải trưởng nhóm 300: Sai cú pháp |
| Copy folder | COPY\_FOLDER \<src\> \<dest\> | 223: Copy thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: Folder nguồn không tồn tại 503: Đường dẫn đích không hợp lệ 300: Sai cú pháp |
| Di chuyển folder | MOVE\_FOLDER \<src\> \<dest\> | 224: Di chuyển thành công 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: Folder nguồn không tồn tại 503: Đường dẫn đích không hợp lệ 300: Sai cú pháp |
| Xem nội dung folder | LIST\_CONTENT \<path\> | 225: Trả về danh sách file/folder 400: Chưa đăng nhập 404: Chưa tham gia nhóm nào 500: Đường dẫn không tồn tại 404: Chưa tham gia nhóm 300: Sai cú pháp |

