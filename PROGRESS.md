# PROGRESS TRACKING

**Last updated:** 2024-12-13 (Initial setup)

---

## 🎯 Người 1: Core Network (Hạ tầng mạng & Truyền tải)

**Status:** Ready to Start  
**Files:** `network.c`, `file_ops.c`, `TCP_Client/network.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| tcp_send | ✅ Done | Already implemented in skeleton |
| tcp_receive | ✅ Done | Already implemented in skeleton |
| send_file | 🔜 Todo | Need chunked transfer for large files |
| receive_file | 🔜 Todo | Depends on send_file |
| UPLOAD (server) | 🔜 Todo | Need LOGIN from Person 2 |
| UPLOAD (client) | 🔜 Todo | |
| DOWNLOAD (server) | 🔜 Todo | |
| DOWNLOAD (client) | 🔜 Todo | |
| Access control | 🔜 Todo | Check group membership |
| Logging | ✅ Done | write_log() already available |

**Blockers:** 
- Need LOGIN/CREATE from Person 2 to test UPLOAD/DOWNLOAD

**Next Steps:**
1. Implement send_file() and receive_file() first (independent)
2. Wait for LOGIN to be ready
3. Implement UPLOAD/DOWNLOAD with proper testing

**Dependencies:**
- Needs: LOGIN (from Person 2)
- Blocks: None

---

## 🎯 Người 2: Auth & Basic Group (Người dùng & Nhóm cơ bản)

**Status:** Ready to Start  
**Files:** `auth.c`, `group.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| REGISTER (server) | 🔜 Todo | Start here! |
| REGISTER (client) | 🔜 Todo | |
| LOGIN (server) | 🔜 Todo | Critical - blocks others |
| LOGIN (client) | 🔜 Todo | |
| LOGOUT (server) | 🔜 Todo | |
| LOGOUT (client) | 🔜 Todo | |
| CREATE (server) | 🔜 Todo | Depends on LOGIN |
| CREATE (client) | 🔜 Todo | |
| JOIN (server) | 🔜 Todo | |
| JOIN (client) | 🔜 Todo | |
| APPROVE (server) | 🔜 Todo | |
| APPROVE (client) | 🔜 Todo | |
| LIST_GROUPS (server) | 🔜 Todo | |
| LIST_GROUPS (client) | 🔜 Todo | |
| LIST_MEMBERS (server) | 🔜 Todo | |
| LIST_MEMBERS (client) | 🔜 Todo | |
| LIST_REQUESTS (server) | 🔜 Todo | |
| LIST_REQUESTS (client) | 🔜 Todo | |
| LEAVE (server) | 🔜 Todo | |
| LEAVE (client) | 🔜 Todo | |

**Blockers:** None

**Next Steps:**
1. **Phase 1:** REGISTER, LOGIN, LOGOUT (most critical!)
2. **Phase 2:** CREATE, LIST_GROUPS
3. **Phase 3:** JOIN, APPROVE, LIST_MEMBERS, LIST_REQUESTS
4. **Phase 4:** LEAVE

**Dependencies:**
- Needs: Nothing (can start immediately)
- Blocks: Person 1 (UPLOAD/DOWNLOAD), Person 3 (all file operations)

---

## 🎯 Người 3: File System & Admin (Hệ thống File & Quản trị nâng cao)

**Status:** Server Implementation Complete ✅  
**Files:** `folder_ops.c`, `file_ops.c`, `group.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| MKDIR (server) | ✅ Done | Implemented with path resolution |
| MKDIR (client) | 🔜 Todo | |
| LIST_CONTENT (server) | ✅ Done | Returns file/folder listing |
| LIST_CONTENT (client) | 🔜 Todo | |
| RENAME_FILE (server) | ✅ Done | Leader only, with checks |
| RENAME_FILE (client) | 🔜 Todo | |
| DELETE_FILE (server) | ✅ Done | Leader only, uses unlink |
| DELETE_FILE (client) | 🔜 Todo | |
| COPY_FILE (server) | ✅ Done | With file locking |
| COPY_FILE (client) | 🔜 Todo | |
| MOVE_FILE (server) | ✅ Done | Moves to destination folder |
| MOVE_FILE (client) | 🔜 Todo | |
| RENAME_FOLDER (server) | ✅ Done | Leader only, with checks |
| RENAME_FOLDER (client) | 🔜 Todo | |
| RMDIR (server) | ✅ Done | Leader only, recursive delete |
| RMDIR (client) | 🔜 Todo | |
| COPY_FOLDER (server) | ✅ Done | Recursive copy with cp -r |
| COPY_FOLDER (client) | 🔜 Todo | |
| MOVE_FOLDER (server) | ✅ Done | Moves to destination folder |
| MOVE_FOLDER (client) | 🔜 Todo | |
| RMDIR (server) | 🔜 Todo | |
| RMDIR (client) | 🔜 Todo | |
| COPY_FOLDER (server) | 🔜 Todo | |
| COPY_FOLDER (client) | 🔜 Todo | |
| MOVE_FOLDER (server) | 🔜 Todo | |
| MOVE_FOLDER (client) | 🔜 Todo | |
| INVITE (server) | ✅ Done | Implemented handle_invite |
| INVITE (client) | 🔜 Todo | |
| KICK (server) | ✅ Done | Implemented handle_kick |
| KICK (client) | 🔜 Todo | |

**Blockers:** 
- Need LOGIN/CREATE from Person 2 for full integration testing
- Need client implementation to complete user-facing features

**Next Steps:**
1. ✅ All server-side implementations COMPLETE
2. 🔜 Implement all client-side do_* functions in TCP_Client/commands.c
3. 🔜 Integration testing with Person 2's AUTH features
4. 🔜 Full system testing when all components are ready

**Dependencies:**
- Needs: LOGIN (from Person 2), UPLOAD (from Person 1 for testing)
- Blocks: None

---

## 📊 INTEGRATION STATUS

| Feature | Server | Client | Tested | Integrated | Notes |
|---------|--------|--------|--------|------------|-------|
| **Infrastructure** |
| Connection | ✅ | ✅ | ✅ | ✅ | Welcome message works |
| tcp_send/receive | ✅ | ✅ | ✅ | ✅ | Stream handling works |
| Data loading | ✅ | N/A | ✅ | ✅ | Loads accounts, groups, etc. |
| **Authentication** |
| REGISTER | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LOGIN | 🔜 | 🔜 | ⏸️ | ⏸️ | **CRITICAL** |
| LOGOUT | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **Group Management** |
| CREATE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| JOIN | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| APPROVE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| INVITE | ✅ | 🔜 | ⏸️ | ⏸️ | |
| ACCEPT | ✅ | 🔜 | ⏸️ | ⏸️ | |
| LEAVE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| KICK | ✅ | 🔜 | ⏸️ | ⏸️ | |
| LIST_GROUPS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_MEMBERS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_REQUESTS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Transfer** |
| send_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| receive_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| UPLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| DOWNLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Operations** |
| MKDIR | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| LIST_CONTENT | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| RENAME_FILE | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| DELETE_FILE | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| COPY_FILE | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| MOVE_FILE | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| **Folder Operations** |
| RENAME_FOLDER | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| RMDIR | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| COPY_FOLDER | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |
| MOVE_FOLDER | ✅ | 🔜 | ⏸️ | ⏸️ | Server done |

**Legend:**
- ✅ Done
- ⏳ In Progress
- 🔜 Todo
- ⏸️ Waiting

---

## 🐛 KNOWN ISSUES

None yet.

---

## 💬 COMMUNICATION LOG

### 2024-12-13 - Initial Setup

**Admin (Auth & Basic Group):**
- Created modular structure with 7 files per component
- Setup Makefile for easy compilation
- Created AGENT_GUIDE.md and PROGRESS.md for team coordination
- Ready to implement Auth functions

**Status:** Skeleton code complete, ready for implementation!

---

### [ADD YOUR UPDATES BELOW]

### 2024-12-14 10:00 - Person 3 (File System & Admin)

**Completed:**
- Implemented handle_invite (server)
- Implemented handle_accept (server)
- Implemented handle_kick (server)

**In Progress:**
- Waiting for other components to test

**Next:**
- Implement client side commands for INVITE, ACCEPT, KICK

---

### 2024-12-14 16:00 - Person 3 (File System & Admin)

**Completed:**
- ✅ Implemented ALL server-side file operations:
  - handle_rename_file (with leader check, file locking)
  - handle_delete_file (leader only, uses unlink)
  - handle_copy_file (with file locking for thread safety)
  - handle_move_file (validates destination folder)
  
- ✅ Implemented ALL server-side folder operations:
  - handle_mkdir (creates folders with permission checks)
  - handle_rename_folder (leader only, checks for existing names)
  - handle_rmdir (leader only, recursive delete)
  - handle_copy_folder (recursive copy using cp -r)
  - handle_move_folder (validates destination)
  - handle_list_content (returns file/folder listing with / suffix for dirs)

**Implementation Details:**
- Added helper functions: resolve_path(), ensure_group_dir(), file_lock()
- All functions check: logged_in (400), in_group (404), leader (406 where needed)
- Proper path resolution with directory traversal prevention (blocks "..")
- File operations use F_RDLCK/F_WRLCK for thread-safe read/write
- Comprehensive logging for all operations
- Response codes follow Protocols.md exactly

**In Progress:**
- All server-side implementations COMPLETE ✅

**Blockers:**
- Need LOGIN/CREATE from Person 2 for integration testing
- Cannot test without auth system

**Next:**
- Implement client-side do_* functions in TCP_Client/commands.c
- Integration testing once Person 2 completes AUTH

**Format:**
```markdown
### YYYY-MM-DD HH:MM - [Your Name] ([Your Role])

**Completed:**
- Task 1
- Task 2

**In Progress:**
- Task 3

**Blockers:**
- Issue 1

**Next:**
- Plan 1
```

---

## 📈 VELOCITY TRACKING

| Week | Person 1 | Person 2 | Person 3 | Total |
|------|----------|----------|----------|-------|
| Week 1 | 0 | 0 | 0 | 0 |

**Target:** 30 tasks total (10 per person)

---

## 🎯 MILESTONES

- [ ] **Milestone 1:** Authentication working (REGISTER, LOGIN, LOGOUT)
- [ ] **Milestone 2:** Group creation and listing (CREATE, LIST_GROUPS)
- [ ] **Milestone 3:** Group join flow (JOIN, APPROVE, LIST_MEMBERS)
- [ ] **Milestone 4:** File transfer (UPLOAD, DOWNLOAD)
- [ ] **Milestone 5:** Basic file operations (MKDIR, LIST_CONTENT)
- [ ] **Milestone 6:** Advanced file operations (RENAME, DELETE, COPY, MOVE)
- [ ] **Milestone 7:** Advanced group features (INVITE, ACCEPT, KICK, LEAVE)
- [ ] **Milestone 8:** Full integration testing
- [ ] **Milestone 9:** Performance optimization
- [ ] **Milestone 10:** Documentation and demo

---

**Remember:** Update this file after every significant progress! 🚀

