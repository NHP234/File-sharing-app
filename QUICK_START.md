# QUICK START - Hướng dẫn cho Người dùng

## 🚀 Bắt đầu làm việc với Agent

### 📋 Prompt mẫu cho mỗi vai trò

Khi bắt đầu session mới với AI agent, **copy và paste** một trong các prompt sau:

---

## 👤 Người 1: Core Network

```
Tôi đang làm việc trong team 3 người phát triển File Sharing Application bằng C.

Vai trò của tôi: Core Network (Hạ tầng mạng & Truyền tải)

Hãy đọc các file sau THEO THỨ TỰ:
1. @AGENT_GUIDE.md
2. @Docs/Description.md
3. @Docs/Protocols.md
4. @STRUCTURE.md
5. @PROGRESS.md

Sau khi đọc xong, xác nhận bạn đã hiểu:
- Vai trò và nhiệm vụ của tôi (Core Network)
- Các file tôi cần làm việc (network.c, file_ops.c)
- Tiến độ hiện tại của team
- Dependencies với công việc của người khác

Sau đó đề xuất task tiếp theo tôi nên làm.
```

---

## 👤 Người 2: Auth & Basic Group

```
Tôi đang làm việc trong team 3 người phát triển File Sharing Application bằng C.

Vai trò của tôi: Auth & Basic Group (Người dùng & Nhóm cơ bản)

Hãy đọc các file sau THEO THỨ TỰ:
1. @AGENT_GUIDE.md
2. @Docs/Description.md
3. @Docs/Protocols.md
4. @STRUCTURE.md
5. @PROGRESS.md

Sau khi đọc xong, xác nhận bạn đã hiểu:
- Vai trò và nhiệm vụ của tôi (Auth & Basic Group)
- Các file tôi cần làm việc (auth.c, group.c)
- Tiến độ hiện tại của team
- Dependencies với công việc của người khác

Sau đó đề xuất task tiếp theo tôi nên làm.
```

---

## 👤 Người 3: File System & Admin

```
Tôi đang làm việc trong team 3 người phát triển File Sharing Application bằng C.

Vai trò của tôi: File System & Admin (Hệ thống File & Quản trị nâng cao)

Hãy đọc các file sau THEO THỨ TỰ:
1. @AGENT_GUIDE.md
2. @Docs/Description.md
3. @Docs/Protocols.md
4. @STRUCTURE.md
5. @PROGRESS.md

Sau khi đọc xong, xác nhận bạn đã hiểu:
- Vai trò và nhiệm vụ của tôi (File System & Admin)
- Các file tôi cần làm việc (folder_ops.c, file_ops.c, group.c)
- Tiến độ hiện tại của team
- Dependencies với công việc của người khác

Sau đó đề xuất task tiếp theo tôi nên làm.
```

---

## 💡 Tips

### Khi tiếp tục session đang làm dở:
```
Tôi đang tiếp tục làm việc trên [tên task].

Hãy đọc @PROGRESS.md để xem tiến độ hiện tại.

Tôi đã làm xong:
- [Task 1]
- [Task 2]

Bây giờ tôi muốn làm tiếp: [Task tiếp theo]
```

### Khi cần debug:
```
Tôi đang gặp lỗi [mô tả lỗi].

File: [tên file]
Function: [tên function]
Lỗi: [error message]

Hãy giúp tôi debug.
```

### Khi cần review code:
```
Tôi vừa implement xong [tên chức năng].

Hãy review code của tôi trong file @[tên file] và kiểm tra:
- Logic có đúng không?
- Thread-safe chưa?
- Response codes có đúng theo Protocols.md không?
- Có cần cải thiện gì không?
```

---

## 📊 Workflow chuẩn

```
1. Bắt đầu session → Dùng prompt mẫu ở trên
2. Agent đọc context → Hiểu nhiệm vụ
3. Agent đề xuất task → Bạn đồng ý
4. Agent implement → Code
5. Agent test → Verify
6. Agent update PROGRESS.md → Track
7. Bạn review → OK
8. Commit code → Done
9. Lặp lại từ bước 3
```

---

## 🎯 Checklist trước khi kết thúc session

- [ ] Code đã compile thành công
- [ ] Đã test các trường hợp success và error
- [ ] Đã update PROGRESS.md
- [ ] Đã commit code với message rõ ràng
- [ ] Đã thông báo với team nếu có blocker

---

## 📞 Liên hệ Team

Nếu cần hỏi team member khác, ghi vào `PROGRESS.md` → Communication Log:

```markdown
**2024-12-13 15:00 - [Tên bạn]:**
❓ Question for [Tên người khác]:
- [Câu hỏi]
```

---

**Happy coding! 🚀**

