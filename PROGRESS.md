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
- ~~Need LOGIN/CREATE from Person 2~~ ✅ LOGIN is ready!

**Next Steps:**
1. Implement send_file() and receive_file() first (independent)
2. ✅ LOGIN is ready - can start UPLOAD/DOWNLOAD
3. Implement UPLOAD/DOWNLOAD with proper testing

**Dependencies:**
- Needs: ✅ LOGIN (from Person 2) - READY!
- Blocks: None

---

## 🎯 Người 2: Auth & Basic Group (Người dùng & Nhóm cơ bản)

**Status:** Phase 1 Complete ✅  
**Files:** `auth.c`, `group.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| REGISTER (server) | ✅ Done | Tested - works with error codes 120, 501, 403, 504 |
| REGISTER (client) | ✅ Done | UI complete with input validation |
| LOGIN (server) | ✅ Done | Tested - works with error codes 110, 401, 402, 403 |
| LOGIN (client) | ✅ Done | Updates is_logged_in status |
| LOGOUT (server) | ✅ Done | Tested - works with error codes 130, 400 |
| LOGOUT (client) | ✅ Done | Clears login status |
| CREATE (server) | ✅ Done | Creates group, assigns leader, creates folder |
| CREATE (client) | ✅ Done | Validates group name (no spaces) |
| JOIN (server) | 🔜 Todo | |
| JOIN (client) | 🔜 Todo | |
| APPROVE (server) | 🔜 Todo | |
| APPROVE (client) | 🔜 Todo | |
| LIST_GROUPS (server) | ✅ Done | Shows all groups with ID, name, leader |
| LIST_GROUPS (client) | ✅ Done | Simple UI |
| LIST_MEMBERS (server) | 🔜 Todo | |
| LIST_MEMBERS (client) | 🔜 Todo | |
| LIST_REQUESTS (server) | 🔜 Todo | |
| LIST_REQUESTS (client) | 🔜 Todo | |
| LEAVE (server) | 🔜 Todo | |
| LEAVE (client) | 🔜 Todo | |

**Blockers:** None

**Next Steps:**
1. ✅ **Phase 1:** REGISTER, LOGIN, LOGOUT - COMPLETED!
2. ✅ **Phase 2:** CREATE, LIST_GROUPS - COMPLETED!
3. ⏳ **Phase 3:** JOIN, APPROVE, LIST_MEMBERS, LIST_REQUESTS (next)
4. 🔜 **Phase 4:** LEAVE

**Dependencies:**
- Needs: Nothing
- Blocks: Person 1 (UPLOAD/DOWNLOAD now unblocked!), Person 3 (file operations now unblocked!)

---

## 🎯 Người 3: File System & Admin (Hệ thống File & Quản trị nâng cao)

**Status:** ✅ COMPLETE - All implementations done  
**Files:** `folder_ops.c`, `file_ops.c`, `group.c`, `TCP_Client/commands.c`

| Task | Status | Notes |
|------|--------|-------|
| MKDIR (server) | ✅ Done | Implemented with path resolution |
| MKDIR (client) | ✅ Done | Tested |
| LIST_CONTENT (server) | ✅ Done | Returns file/folder listing |
| LIST_CONTENT (client) | ✅ Done | Tested |
| RENAME_FILE (server) | ✅ Done | Leader only, with checks |
| RENAME_FILE (client) | ✅ Done | Tested |
| DELETE_FILE (server) | ✅ Done | Leader only, uses unlink |
| DELETE_FILE (client) | ✅ Done | Tested |
| COPY_FILE (server) | ✅ Done | With file locking |
| COPY_FILE (client) | ✅ Done | Tested |
| MOVE_FILE (server) | ✅ Done | Moves to destination folder |
| MOVE_FILE (client) | ✅ Done | Tested |
| RENAME_FOLDER (server) | ✅ Done | Leader only, with checks |
| RENAME_FOLDER (client) | ✅ Done | Tested |
| RMDIR (server) | ✅ Done | Leader only, recursive delete |
| RMDIR (client) | ✅ Done | Tested |
| COPY_FOLDER (server) | ✅ Done | Recursive copy with cp -r |
| COPY_FOLDER (client) | ✅ Done | Tested |
| MOVE_FOLDER (server) | ✅ Done | Moves to destination folder |
| MOVE_FOLDER (client) | ✅ Done | Tested |
| INVITE (server) | ✅ Done | Implemented handle_invite |
| INVITE (client) | ✅ Done | Tested |
| ACCEPT (server) | ✅ Done | Implemented handle_accept |
| ACCEPT (client) | ✅ Done | Tested |
| KICK (server) | ✅ Done | Implemented handle_kick |
| KICK (client) | ✅ Done | Tested |

**Blockers:** None - All implementations complete

**Next Steps:**
1. ✅ All server-side implementations COMPLETE
2. ✅ All client-side implementations COMPLETE  
3. 🔜 Integration testing with Person 2's AUTH features (when ready)
4. 🔜 Full system testing when all components are ready

**Dependencies:**
- Needs: ✅ LOGIN (from Person 2) - READY!, UPLOAD (from Person 1 for testing)
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
| REGISTER | ✅ | ✅ | ✅ | ✅ | Supports error codes: 120, 501, 403, 504 |
| LOGIN | ✅ | ✅ | ✅ | ✅ | Supports error codes: 110, 401, 402, 403 |
| LOGOUT | ✅ | ✅ | ✅ | ✅ | Supports error codes: 130, 400 |
| **Group Management** |
| CREATE | ✅ | ✅ | ✅ | ✅ | Error codes: 202, 400, 407, 501, 504 |
| JOIN | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| APPROVE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| INVITE | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| ACCEPT | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| LEAVE | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| KICK | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_GROUPS | ✅ | ✅ | ✅ | ✅ | Error codes: 203, 400 |
| LIST_MEMBERS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| LIST_REQUESTS | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Transfer** |
| send_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| receive_file | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| UPLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| DOWNLOAD | 🔜 | 🔜 | ⏸️ | ⏸️ | |
| **File Operations** |
| MKDIR | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| LIST_CONTENT | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| RENAME_FILE | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| DELETE_FILE | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| COPY_FILE | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| MOVE_FILE | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| **Folder Operations** |
| RENAME_FOLDER | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| RMDIR | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| COPY_FOLDER | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |
| MOVE_FOLDER | ✅ | ✅ | ⏸️ | ⏸️ | Client done, need LOGIN |

**Legend:**
- ✅ Done
- ⏳ In Progress
- 🔜 Todo
- ⏸️ Waiting

---

## 🐛 KNOWN ISSUES

None yet.


---

## 📈 VELOCITY TRACKING

| Week | Person 1 | Person 2 | Person 3 | Total |
|------|----------|----------|----------|-------|
| Week 1 | 0 | 10 ✅ | 0 | 10 |

**Target:** 30 tasks total (10 per person)

**Person 2 Progress:** 10/13 tasks (77%) - Auth + CREATE + LIST_GROUPS complete

---

## 🎯 MILESTONES

- [x] **Milestone 1:** Authentication working (REGISTER, LOGIN, LOGOUT) ✅
- [x] **Milestone 2:** Group creation and listing (CREATE, LIST_GROUPS) ✅
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

