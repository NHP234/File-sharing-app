# Cấu trúc Code - File Sharing Application

## 📁 Tổ chức Files

### Server (TCP_Server/)

| File | Dòng code | Chức năng | Phụ trách |
|------|-----------|-----------|-----------|
| `common.h` | ~150 | Structures, constants, prototypes | Shared |
| `server.c` | ~180 | Main + command routing + thread handler | Core Network |
| `auth.c` | ~50 | REGISTER, LOGIN, LOGOUT | Auth & Basic Group |
| `group.c` | ~200 | CREATE, JOIN, APPROVE, INVITE, ACCEPT, LEAVE, KICK, LIST | Auth & Basic Group + File System & Admin |
| `file_ops.c` | ~120 | UPLOAD, DOWNLOAD, RENAME_FILE, DELETE_FILE, COPY_FILE, MOVE_FILE | Core Network + File System & Admin |
| `folder_ops.c` | ~120 | MKDIR, RENAME_FOLDER, RMDIR, COPY_FOLDER, MOVE_FOLDER, LIST_CONTENT | File System & Admin |
| `utils.c` | ~280 | Load/save data, logging, utilities | Shared |
| `network.c` | ~100 | tcp_send, tcp_receive, send_file, receive_file | Core Network |

**Tổng: ~1200 dòng code** (chia đều cho 3 người, mỗi người ~400 dòng)

### Client (TCP_Client/)

| File | Dòng code | Chức năng |
|------|-----------|-----------|
| `common.h` | ~60 | Structures, prototypes |
| `client.c` | ~180 | Main + menu loop |
| `commands.c` | ~200 | Tất cả do_* functions |
| `ui.c` | ~150 | Menu display + response translation |
| `network.c` | ~100 | Network I/O |

**Tổng: ~690 dòng code**

---

## 🎯 Phân chia công việc theo 3 người

### 👤 Người 1: Core Network (Hạ tầng mạng & Truyền tải)

**Server files:**
- `network.c` - Hoàn chỉnh tcp_send, tcp_receive, **send_file**, **receive_file**
- `file_ops.c` - Implement **UPLOAD**, **DOWNLOAD**
- `server.c` - Kiểm tra và optimize thread handling, logging

**Client files:**
- `network.c` - Hoàn chỉnh send_file, receive_file
- `commands.c` - Implement **do_upload**, **do_download**

**Nhiệm vụ chính:**
1. Xử lý truyền dòng (tcp_send/tcp_receive)
2. Implement send_file/receive_file cho file lớn (chunked transfer)
3. Implement UPLOAD command (server + client)
4. Implement DOWNLOAD command (server + client)
5. Kiểm soát quyền truy cập (check group membership)
6. Ghi log hoạt động (sử dụng write_log)

**Điểm:** 10 điểm

---

### 👤 Người 2: Auth & Basic Group (Người dùng & Nhóm cơ bản)

**Server files:**
- `auth.c` - Implement **REGISTER**, **LOGIN**, **LOGOUT**
- `group.c` - Implement **CREATE**, **JOIN**, **APPROVE**, **LEAVE**, **LIST_GROUPS**, **LIST_MEMBERS**, **LIST_REQUESTS**

**Client files:**
- `commands.c` - Implement tất cả do_* functions tương ứng

**Nhiệm vụ chính:**
1. Đăng ký và quản lý tài khoản (REGISTER)
2. Đăng nhập và quản lý phiên (LOGIN, LOGOUT)
3. Tạo nhóm chia sẻ (CREATE)
4. Yêu cầu tham gia nhóm (JOIN)
5. Phê duyệt thành viên (APPROVE)
6. Liệt kê danh sách nhóm (LIST_GROUPS)
7. Liệt kê danh sách thành viên (LIST_MEMBERS)
8. Rời nhóm (LEAVE)

**Điểm:** 10 điểm

---

### 👤 Người 3: File System & Admin (Hệ thống File & Quản trị nâng cao)

**Server files:**
- `folder_ops.c` - Implement tất cả folder operations
- `file_ops.c` - Implement **RENAME_FILE**, **DELETE_FILE**, **COPY_FILE**, **MOVE_FILE**
- `group.c` - Implement **INVITE**, **KICK**

**Client files:**
- `commands.c` - Implement tất cả do_* functions tương ứng

**Nhiệm vụ chính:**
1. Thao tác với thư mục (MKDIR, RENAME_FOLDER, RMDIR, COPY_FOLDER, MOVE_FOLDER)
2. Thao tác với file (RENAME_FILE, DELETE_FILE, COPY_FILE, MOVE_FILE)
3. Liệt kê nội dung thư mục (LIST_CONTENT)
4. Mời tham gia vào nhóm (INVITE)
5. Xóa thành viên khỏi nhóm (KICK)

**Điểm:** 10 điểm

---

## 🔧 Compile & Test

### Compile Server
```bash
cd TCP_Server
make clean
make
```

### Compile Client
```bash
cd TCP_Client
make clean
make
```

### Run
```bash
# Terminal 1 - Server
cd TCP_Server
./server 8080

# Terminal 2 - Client 1
cd TCP_Client
./client 127.0.0.1 8080

# Terminal 3 - Client 2
cd TCP_Client
./client 127.0.0.1 8080
```

---

## 📝 Quy tắc Code

1. **Thread-safe**: Luôn dùng mutex khi truy cập global data
   ```c
   pthread_mutex_lock(&account_mutex);
   // ... modify accounts ...
   pthread_mutex_unlock(&account_mutex);
   ```

2. **Error handling**: Kiểm tra điều kiện và trả về đúng response code
   ```c
   if (!state->is_logged_in) {
       tcp_send(state->sockfd, "400");
       return;
   }
   ```

3. **Logging**: Ghi log cho các hoạt động quan trọng
   ```c
   char log_msg[256];
   snprintf(log_msg, sizeof(log_msg), "User %s logged in", username);
   write_log(log_msg);
   ```

4. **Save data**: Nhớ save data sau khi modify
   ```c
   pthread_mutex_lock(&account_mutex);
   accounts[i].group_id = new_group_id;
   save_accounts();
   pthread_mutex_unlock(&account_mutex);
   ```

---

## 🚀 Thứ tự implement đề xuất

### Phase 1: Authentication (Người 2)
- REGISTER, LOGIN, LOGOUT
- Test: Đăng ký, đăng nhập, đăng xuất

### Phase 2: Group Basic (Người 2)
- CREATE, LIST_GROUPS
- Test: Tạo nhóm, xem danh sách nhóm

### Phase 3: Group Join Flow (Người 2)
- JOIN, APPROVE, LIST_MEMBERS, LIST_REQUESTS
- Test: User A tạo nhóm, User B xin vào, User A duyệt

### Phase 4: File Transfer (Người 1)
- send_file, receive_file, UPLOAD, DOWNLOAD
- Test: Upload file nhỏ, file lớn, download file

### Phase 5: Folder Operations (Người 3)
- MKDIR, LIST_CONTENT
- Test: Tạo folder, xem nội dung

### Phase 6: File Operations (Người 3)
- RENAME_FILE, DELETE_FILE
- Test: Đổi tên, xóa file (chỉ leader)

### Phase 7: Advanced Operations (Người 3)
- COPY_FILE, MOVE_FILE, COPY_FOLDER, MOVE_FOLDER, RENAME_FOLDER, RMDIR
- Test: Copy, move file/folder

### Phase 8: Group Advanced (Người 2 + 3)
- INVITE, ACCEPT, KICK, LEAVE
- Test: Leader mời user, user chấp nhận, kick member, leave group

---

## 📊 Progress Tracking

Mỗi người nên:
1. Tạo branch riêng: `git checkout -b feature/your-name`
2. Commit thường xuyên với message rõ ràng
3. Test kỹ trước khi merge
4. Update README với status của mình

**Ví dụ commit messages:**
- `[Core Network] Implement send_file and receive_file`
- `[Auth] Implement REGISTER command`
- `[File System] Implement MKDIR and LIST_CONTENT`

