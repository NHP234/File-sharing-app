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

**Status:** Waiting  
**Files:** `folder_ops.c`, `file_ops.c`, `group.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| MKDIR (server) | 🔜 Todo | Can start after LOGIN |
| MKDIR (client) | 🔜 Todo | |
| LIST_CONTENT (server) | 🔜 Todo | Can start after LOGIN |
| LIST_CONTENT (client) | 🔜 Todo | |
| RENAME_FILE (server) | 🔜 Todo | Need UPLOAD first for testing |
| RENAME_FILE (client) | 🔜 Todo | |
| DELETE_FILE (server) | 🔜 Todo | Need UPLOAD first for testing |
| DELETE_FILE (client) | 🔜 Todo | |
| COPY_FILE (server) | 🔜 Todo | |
| COPY_FILE (client) | 🔜 Todo | |
| MOVE_FILE (server) | 🔜 Todo | |
| MOVE_FILE (client) | 🔜 Todo | |
| RENAME_FOLDER (server) | 🔜 Todo | |
| RENAME_FOLDER (client) | 🔜 Todo | |
| RMDIR (server) | 🔜 Todo | |
| RMDIR (client) | 🔜 Todo | |
| COPY_FOLDER (server) | 🔜 Todo | |
| COPY_FOLDER (client) | 🔜 Todo | |
| MOVE_FOLDER (server) | 🔜 Todo | |
| MOVE_FOLDER (client) | 🔜 Todo | |
| INVITE (server) | 🔜 Todo | Need CREATE from Person 2 |
| INVITE (client) | 🔜 Todo | |
| KICK (server) | 🔜 Todo | Need CREATE from Person 2 |
| KICK (client) | 🔜 Todo | |

**Blockers:** 
- Waiting for LOGIN/CREATE from Person 2
- File operations need UPLOAD from Person 1 for testing

**Next Steps:**
1. Can start MKDIR and LIST_CONTENT (only need LOGIN)
2. Wait for UPLOAD to implement file operations
3. Implement INVITE/KICK after CREATE is ready

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
| INVITE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| ACCEPT | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LEAVE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| KICK | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_GROUPS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_MEMBERS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_REQUESTS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Transfer** |
| send_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| receive_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| UPLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| DOWNLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Operations** |
| MKDIR | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_CONTENT | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| RENAME_FILE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| DELETE_FILE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| COPY_FILE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| MOVE_FILE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **Folder Operations** |
| RENAME_FOLDER | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| RMDIR | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| COPY_FOLDER | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| MOVE_FOLDER | 🔜 | 🔜 | ⏸️ | ⏸️ | |

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

