#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include "file_utils.h" // file này đã có extern group_t

// Không cần khai báo lại group_count/groups ở đây nếu đã include header và link với file_utils.c
// Tuy nhiên, để chắc chắn không lỗi duplicate symbol, ta chỉ gán giá trị trong setup_env

void setup_env() {
    // 1. Dọn dẹp môi trường file
    system("rm -rf server_storage");
    system("mkdir -p server_storage/group_1");
    
    // 2. Tạo file mẫu
    FILE *f = fopen("server_storage/group_1/testfile.txt", "w");
    if (f) {
        fprintf(f, "Hello World Content");
        fclose(f);
    }

    // 3. QUAN TRỌNG: Thiết lập dữ liệu giả lập Admin trong RAM
    // Để hàm is_admin() trả về 1
    group_count = 1;
    groups[0].id = 1;
    strcpy(groups[0].name, "Test Group");
    strcpy(groups[0].admin_username, "admin"); // <<-- Gán user "admin" làm chủ group 1
}

void teardown_env() {
    system("rm -rf server_storage");
    // Reset lại biến toàn cục để không ảnh hưởng test khác
    group_count = 0;
    memset(groups, 0, sizeof(groups));
}

conn_state_t create_mock_state(const char *user, int group_id) {
    conn_state_t state;
    memset(&state, 0, sizeof(state));
    state.is_logged_in = 1;
    strcpy(state.logged_user, user);
    state.current_group_id = group_id;
    return state;
}

/* ... Các hàm test case giữ nguyên như cũ ... */

void test_resolve_path() {
    /* ... code cũ ... */
    printf("[TEST] resolve_path... -> PASS\n");
}

void test_mkdir_and_list() {
    /* ... code cũ ... */
    printf("[TEST] mkdir and list_content... -> PASS\n");
}

void test_file_ops_basic() {
    printf("[TEST] Basic Ops (Rename, Move, Remove)...\n");
    conn_state_t state = create_mock_state("admin", 1); // User "admin" khớp với setup_env

    // 1. Rename
    // Bây giờ is_admin sẽ trả về 1, nên code lỗi 406 sẽ biến mất
    int ret = handle_rename(&state, "testfile.txt", "renamed.txt");
    
    if (ret != 210) {
        printf("   [FAIL] Expected 210 but got %d\n", ret);
        // Debug nếu lỗi
        if (ret == 406) printf("   -> Reason: Not Admin\n");
        if (ret == 500) printf("   -> Reason: File not found or system error\n");
    }
    assert(ret == 210);
    
    // Kiểm tra file cũ mất, file mới có
    struct stat st;
    assert(stat("server_storage/group_1/testfile.txt", &st) == -1);
    assert(stat("server_storage/group_1/renamed.txt", &st) == 0);

    // 2. Move
    ret = handle_move(&state, "renamed.txt", "new_folder"); // Cần đảm bảo new_folder tồn tại
    // Lưu ý: new_folder được tạo ở test trước, nhưng setup_env() đã xóa sạch.
    // Nên ta cần tạo lại folder ở đây hoặc trong setup_env
    handle_mkdir(&state, "new_folder"); // Tạo folder đích trước khi move
    
    ret = handle_move(&state, "renamed.txt", "new_folder");
    assert(ret == 213);
    assert(stat("server_storage/group_1/new_folder/renamed.txt", &st) == 0);

    // 3. Remove
    ret = handle_remove(&state, "new_folder/renamed.txt");
    assert(ret == 211);

    printf("   -> PASS\n");
}


void test_locking_concurrency() {
    printf("[TEST] File Locking (Concurrent Copy)...\n");
    
    // Tạo file nguồn lớn một chút (hoặc file thường)
    FILE *f = fopen("server_storage/group_1/shared.db", "w");
    fprintf(f, "Simulated Database Data");
    fclose(f);

    conn_state_t state = create_mock_state("admin", 1);

    pid_t pid = fork();

    if (pid == 0) {
        // --- CHILD PROCESS (Giả lập Client A đang ghi file) ---
        // Child sẽ mở file và giữ Write Lock trong 2 giây
        // Điều này mô phỏng việc file đang bận.
        int fd = open("server_storage/group_1/shared.db", O_WRONLY);
        if (fd == -1) exit(1);

        struct flock lock;
        memset(&lock, 0, sizeof(lock));
        lock.l_type = F_WRLCK; // Khóa ghi độc quyền
        lock.l_whence = SEEK_SET;
        
        // Lock file
        fcntl(fd, F_SETLKW, &lock);
        // printf("   [Child] Locked file, sleeping 2s...\n");
        sleep(2); 
        // printf("   [Child] Unlocking and exiting...\n");
        
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &lock);
        close(fd);
        exit(0);
    } else {
        // --- PARENT PROCESS (Giả lập Client B muốn Copy file) ---
        
        // Ngủ 0.5s để đảm bảo Child đã kịp chạy và Lock file
        usleep(500000); 

        time_t start = time(NULL);
        
        // Gọi hàm copy. Hàm này sẽ cố lấy Read Lock.
        // Vì Child đang giữ Write Lock, Parent phải đợi (block).
        int ret = handle_copy(&state, "shared.db", "shared_backup.db");
        
        time_t end = time(NULL);
        double elapsed = difftime(end, start);

        wait(NULL); // Chờ child kết thúc hẳn

        assert(ret == 212); // Copy phải thành công sau khi chờ

        // Kiểm tra thời gian chờ
        // Nếu Locking hoạt động, handle_copy phải mất ít nhất khoảng 1.5s - 2s (do child sleep 2s)
        // Nếu Locking KHÔNG hoạt động, nó sẽ chạy xong ngay lập tức (< 1s)
        printf("   -> Time elapsed: %.1f seconds (Expected ~2.0s)\n", elapsed);
        
        if (elapsed >= 1.5) {
            printf("   -> PASS: Locking works (Process waited).\n");
        } else {
            printf("   -> FAIL: Process did not wait (Race condition possible).\n");
            exit(1);
        }
    }
}

int main() {
    printf("=== STARTING FILE UTILS TESTS ===\n");
    
    setup_env();
    test_resolve_path();
    teardown_env();

    setup_env();
    test_mkdir_and_list();
    teardown_env();

    setup_env();
    test_file_ops_basic();
    teardown_env();
    
    setup_env();
    test_locking_concurrency();
    teardown_env();
    
    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}



