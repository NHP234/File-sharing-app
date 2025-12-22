# AGENT GUIDE - File Sharing Project

## 🚀 SETUP CHO AGENT (ĐỌC ĐẦU TIÊN)

Bạn là AI agent đang hỗ trợ phát triển **File Sharing Application** - một ứng dụng chia sẻ file giữa các nhóm người dùng, viết bằng C với pthread.

---

## 📖 Bước 1: Đọc các file context (THEO THỨ TỰ)

**BẮT BUỘC đọc theo đúng thứ tự:**

1. **`Docs/Description.md`** - Hiểu bài toán tổng quan, các chức năng cần có
2. **`Docs/Protocols.md`** - Nắm giao thức request/response, mã trạng thái
3. **`STRUCTURE.md`** - Hiểu cấu trúc code, phân chia công việc, quy tắc
4. **`PROGRESS.md`** - Xem tiến độ hiện tại của cả team, ai đang làm gì

---

## 👤 Bước 2: Xác định vai trò

Agent này đang hỗ trợ người nào và vai trò gì?

**Có 3 vai trò trong team:**

### 1️⃣ **Core Network** (Hạ tầng mạng & Truyền tải)
**Files chính:** `network.c`, `file_ops.c` (UPLOAD/DOWNLOAD)

**Nhiệm vụ:**
- Implement send_file/receive_file (chunked transfer)
- Implement UPLOAD/DOWNLOAD (server + client)
- Kiểm soát quyền truy cập
- Ghi log hoạt động

**Điểm:** 10 điểm

---

### 2️⃣ **Auth & Basic Group** (Người dùng & Nhóm cơ bản)
**Files chính:** `auth.c`, `group.c` (basic functions)

**Nhiệm vụ:**
- Implement REGISTER, LOGIN, LOGOUT
- Implement CREATE, JOIN, APPROVE, LEAVE
- Implement LIST_GROUPS, LIST_MEMBERS, LIST_REQUESTS

**Điểm:** 10 điểm

---

### 3️⃣ **File System & Admin** (Hệ thống File & Quản trị nâng cao)
**Files chính:** `folder_ops.c`, `file_ops.c` (rename/delete/copy/move), `group.c` (INVITE/KICK)

**Nhiệm vụ:**
- Implement MKDIR, RENAME_FOLDER, RMDIR, COPY_FOLDER, MOVE_FOLDER, LIST_CONTENT
- Implement RENAME_FILE, DELETE_FILE, COPY_FILE, MOVE_FILE
- Implement INVITE, KICK

**Điểm:** 10 điểm

---

## 🔍 Bước 3: Kiểm tra dependencies

Trước khi implement, **BẮT BUỘC** check `PROGRESS.md`:

### ❓ Tự hỏi:
- ✅ Các function phụ thuộc đã được implement chưa?
  - Ví dụ: CREATE cần LOGIN hoạt động trước
  - Ví dụ: UPLOAD cần user đã ở trong nhóm
- ✅ Có ai đang làm việc trên cùng file không?
- ✅ Có conflict với công việc của người khác không?

### 📋 Dependencies chính:
```
LOGIN/LOGOUT → CREATE → JOIN/APPROVE → UPLOAD/DOWNLOAD
                    ↓
                 INVITE/ACCEPT
                    ↓
                 KICK/LEAVE
```

---

## ✅ Bước 4: Sau khi hoàn thành task

### 1. **Test kỹ chức năng**
   - Compile thành công: `make clean && make`
   - Chạy được: `./server 8080` hoặc `./client 127.0.0.1 8080`
   - Test các trường hợp: success, error, edge cases
   - Test thread-safe (chạy multiple clients)

### 2. **Update PROGRESS.md**
   ```markdown
   | REGISTER (server) | ✅ Done | Tested with multiple users |
   ```

### 3. **Ghi log trong Communication Log**
   ```markdown
   **2024-12-13 14:30 - Admin:**
   - Completed REGISTER, LOGIN, LOGOUT (server + client)
   - Tested with 3 concurrent clients
   - Ready for CREATE implementation
   ```

### 4. **Commit với format chuẩn**
   ```bash
   git add .
   git commit -m "[Auth] Implement REGISTER, LOGIN, LOGOUT"
   ```

---

## 📋 QUY TẮC QUAN TRỌNG

### ❌ TUYỆT ĐỐI KHÔNG ĐƯỢC:

1. **Sửa code của người khác** (trừ khi thảo luận trước trong team)
2. **Thay đổi `common.h`** mà không thông báo (vì 3 người đều dùng)
3. **Implement chức năng không thuộc phần của mình**
4. **Update STRUCTURE.md** (file này là reference, không thay đổi)
5. **Thay đổi protocol** trong Protocols.md tự ý
6. **Skip testing** trước khi commit

### ✅ NÊN LÀM:

1. **Đọc PROGRESS.md** trước mỗi lần bắt đầu làm việc
2. **Update PROGRESS.md** ngay sau khi hoàn thành
3. **Test với các chức năng đã có** của người khác
4. **Ghi chú rõ ràng** nếu cần người khác làm gì (blockers)
5. **Follow coding style** đã có trong reference code
6. **Sử dụng mutex** cho mọi global data access
7. **Ghi log** cho các hoạt động quan trọng
8. **Save data** sau khi modify (save_accounts, save_groups, etc.)

---

## 🔧 TESTING CHECKLIST

Sau khi implement một chức năng, check:

- [ ] **Compile:** `make clean && make` không có error/warning
- [ ] **Run:** Server và client khởi động được
- [ ] **Success case:** Chức năng hoạt động đúng khi input hợp lệ
- [ ] **Error cases:** Response code đúng khi có lỗi
  - Not logged in (400)
  - Not in group (404)
  - Not leader (406)
  - Already exists (501)
  - etc.
- [ ] **Thread-safe:** Chạy 2-3 clients đồng thời không crash
- [ ] **Data persistence:** Restart server, data vẫn còn
- [ ] **Logging:** Log được ghi vào `TCP_Server/logs/server_log.txt`
- [ ] **Protocol:** Response format đúng theo Protocols.md

---

## 🐛 KHI GẶP LỖI

### 1. **Compile error:**
   - Check syntax C
   - Check có include đủ headers không
   - Check Makefile có đúng dependencies không

### 2. **Segmentation fault:**
   - Check NULL pointer
   - Check buffer overflow
   - Check mutex lock/unlock đúng

### 3. **Race condition:**
   - Check mọi global data access đều có mutex
   - Check mutex lock trước khi read/write
   - Check unlock sau khi xong

### 4. **Logic error:**
   - Đọc lại Protocols.md
   - Check response code có đúng không
   - Check điều kiện if/else

---

## 💬 COMMUNICATION

### Khi cần hỏi team:
Ghi vào PROGRESS.md → Communication Log:

```markdown
**2024-12-13 15:00 - Admin:**
❓ Question for Core Network team:
- Does UPLOAD check if user is logged in?
- Should I implement this check in my code or yours?
```

### Khi có blocker:
Update PROGRESS.md → Blockers section:

```markdown
**Blockers:** 
- Waiting for LOGIN implementation from Auth team
- Cannot test CREATE without authentication
```

---

## 📚 REFERENCE CODE

Khi không biết cách implement, tham khảo:

1. **`References/server_pthread.c`** - Example pthread server
2. **`References/client.c`** - Example client
3. **`TCP_Server/utils.c`** - Example data loading/saving
4. **`TCP_Server/network.c`** - Example tcp_send/tcp_receive

---

## 🎓 LEARNING RESOURCES

### C Programming:
- Mutex: `pthread_mutex_lock()`, `pthread_mutex_unlock()`
- Threads: `pthread_create()`, `pthread_detach()`
- File I/O: `fopen()`, `fscanf()`, `fprintf()`, `fclose()`
- Socket: `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`

### Debugging:
```bash
# Compile with debug symbols
gcc -g -pthread ...

# Run with gdb
gdb ./server
(gdb) run 8080
(gdb) bt  # backtrace when crash
```

---

**Good luck! 🚀**

