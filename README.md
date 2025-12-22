# File Sharing Application

Ứng dụng chia sẻ file sử dụng C và pthread, hỗ trợ quản lý nhóm và chia sẻ file giữa các thành viên.

## 🚀 Quick Start

- **Người dùng:** Xem `QUICK_START.md` để bắt đầu làm việc với AI agent
- **AI Agent:** Đọc `AGENT_GUIDE.md` để hiểu quy tắc và workflow

## Cấu trúc dự án

```
File-sharing-app/
├── TCP_Server/
│   ├── common.h           # Shared header với structures và prototypes
│   ├── server.c           # Main server + command routing
│   ├── auth.c             # Authentication (REGISTER, LOGIN, LOGOUT)
│   ├── group.c            # Group management
│   ├── file_ops.c         # File operations
│   ├── folder_ops.c       # Folder operations
│   ├── utils.c            # Utilities (load/save data, logging)
│   ├── network.c          # Network I/O (tcp_send, tcp_receive)
│   ├── Makefile           # Build script cho server
│   ├── data/              # Database files
│   │   ├── accounts.txt
│   │   ├── groups.txt
│   │   ├── requests.txt
│   │   └── invites.txt
│   ├── groups/            # Thư mục chứa file của các nhóm
│   └── logs/              # Log files
│
├── TCP_Client/
│   ├── common.h           # Shared header
│   ├── client.c           # Main client + menu loop
│   ├── commands.c         # Command implementations
│   ├── ui.c               # UI functions (menu, response display)
│   ├── network.c          # Network I/O
│   └── Makefile           # Build script cho client
│
├── Docs/
│   ├── Description.md     # Mô tả bài toán
│   └── Protocols.md       # Giao thức truyền thông
│
└── References/
    ├── server_pthread.c   # Reference implementation
    └── client.c           # Reference implementation
```

## Build & Run

### Server

```bash
cd TCP_Server
make
./server <port>
```

Ví dụ:
```bash
./server 8080
```

### Client

```bash
cd TCP_Client
make
./client <server_ip> <port>
```

Ví dụ:
```bash
./client 127.0.0.1 8080
```

## Clean build files

```bash
# Server
cd TCP_Server
make clean

# Client
cd TCP_Client
make clean
```

## Phân chia công việc

### 1. Core Network (Hạ tầng mạng & Truyền tải)
**Files:** `network.c`, `file_ops.c` (upload/download)

- Cài đặt cơ chế vào/ra socket trên server
- Xử lý truyền dòng
- Upload/Download file
- Xử lý file có kích thước lớn
- Kiểm soát quyền truy cập
- Ghi log hoạt động

### 2. Auth & Basic Group (Người dùng & Nhóm cơ bản)
**Files:** `auth.c`, `group.c` (basic functions)

- Đăng ký và quản lý tài khoản
- Đăng nhập và quản lý phiên
- Yêu cầu tham gia nhóm và phê duyệt
- Tạo nhóm chia sẻ
- Liệt kê danh sách nhóm
- Liệt kê danh sách thành viên
- Rời nhóm

### 3. File System & Admin (Hệ thống File & Quản trị nâng cao)
**Files:** `folder_ops.c`, `group.c` (advanced functions)

- Thao tác với thư mục (tạo, sửa, xóa, copy, di chuyển)
- Thao tác với file (sửa tên, xóa, copy, di chuyển)
- Liệt kê nội dung thư mục
- Mời tham gia vào nhóm và phê duyệt
- Xóa thành viên khỏi nhóm

## Giao thức

Xem chi tiết trong `Docs/Protocols.md`

### Response Codes

- **1xx**: Success codes (100-130)
- **2xx**: Operation success (140-225)
- **3xx**: Syntax errors (300)
- **4xx**: Client errors (400-408)
- **5xx**: Server errors (500-504)

## Progress Tracking

Xem tiến độ chi tiết trong `PROGRESS.md`

## Notes

- Server sử dụng pthread để xử lý multiple clients đồng thời
- Tất cả operations đều thread-safe với mutex
- Protocol sử dụng `\r\n` làm delimiter
- File được truyền theo chunks để hỗ trợ file lớn

