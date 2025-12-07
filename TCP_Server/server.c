#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h> 
#include <libgen.h> 
#include <fcntl.h>
#include <time.h>

#define STORAGE_DIR "server_storage"
#define LOG_DIR "logs"

#define BUFF_SIZE 4096
#define MAX_ACCOUNTS 100
#define MAX_USERNAME 50
#define MAX_GROUPS 50
#define MAX_GROUP_NAME 50
#define MAX_JOIN_REQUESTS 100
#define BACKLOG 20

/* Group structure */
typedef struct {
    int group_id;
    char group_name[MAX_GROUP_NAME];
    char leader[MAX_USERNAME];
    int active; /* 1: active, 0: deleted */
} group_t;

/* Join Request structure */
typedef struct {
    char username[MAX_USERNAME];
    int group_id;
} join_request_t;

/* Account structure */
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_USERNAME];
    int logged_in; /* 0: not logged in, 1: logged in */
    int group_id;
} account_t;

/* Connection state for each client */
typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
    int sockfd;
    char logged_user[MAX_USERNAME];
    int is_logged_in;
    int current_group_id;
    char client_addr[50]; /* IP:Port of client */
} conn_state_t;

/* Global variables */
account_t accounts[MAX_ACCOUNTS];
int account_count = 0;
group_t groups[MAX_GROUPS];
int group_count = 0;
join_request_t join_requests[MAX_JOIN_REQUESTS];
int join_request_count = 0;
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t group_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Function prototypes */
void load_accounts();
void load_groups();
int save_account(const char *username, const char *password);
int save_groups();
int find_group_by_name(const char *group_name);
int find_group_by_id(int group_id);
int add_join_request(const char *username, int group_id);
int remove_join_request(const char *username, int group_id);
int check_join_request_exists(const char *username, int group_id);
int tcp_send(int sockfd, char *msg);
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len);
void *handle_client(void *arg);
void process_command(conn_state_t *state, char *command);
void write_log(const char *client_addr, const char *request, const char *result);

/**
 * @function get_log_filename: Generate log filename based on current date
 * @param filename: Buffer to store the filename
 * @param size: Size of the buffer
 **/
void get_log_filename(char *filename, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(filename, size, "%s/log_%04d%02d%02d.txt", 
             LOG_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

/**
 * @function write_log: Write log entry to file
 * @param client_addr: Client address in format IP:Port
 * @param request: Request received from client
 * @param result: Result/response sent to client
 **/
void write_log(const char *client_addr, const char *request, const char *result) {
    // Ensure log directory exists
    struct stat st = {0};
    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0755);
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Get log filename
    char log_filename[256];
    get_log_filename(log_filename, sizeof(log_filename));
    
    // Format timestamp
    char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "[%02d/%02d/%04d %02d:%02d:%02d]",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    // Write to log file (thread-safe)
    pthread_mutex_lock(&log_mutex);
    FILE *f = fopen(log_filename, "a");
    if (f != NULL) {
        if (request == NULL || strlen(request) == 0) {
            // Connection log (welcome message)
            fprintf(f, "%s$%s$%s\n", timestamp, client_addr, result);
        } else {
            // Request/response log
            fprintf(f, "%s$%s$%s\\r\\n$%s\n", timestamp, client_addr, request, result);
        }
        fclose(f);
    }
    pthread_mutex_unlock(&log_mutex);
}

/**
 * @function load_accounts: Load user accounts from file into memory
 * @param: None
 * @return: None
 **/
void load_accounts() {
    FILE *f = fopen("TCP_Server/account.txt", "r");
    if (f == NULL) {
        perror("Cannot open TCP_Server/account.txt");
        exit(1);
    }
    
    account_count = 0;
    while (fscanf(f, "%s %s %d", accounts[account_count].username, 
                  accounts[account_count].password, &accounts[account_count].group_id) == 3) {
        accounts[account_count].logged_in = 0;
        account_count++;
        if (account_count >= MAX_ACCOUNTS) break;
    }
    
    fclose(f);
    printf("Loaded %d accounts\n", account_count);
}

/**
 * @function save_account: Save new account to file and add to memory
 * @param username: Username of the new account
 * @param password: Password of the new account
 * @return: 0 on success, -1 on error
 **/
int save_account(const char *username, const char *password) {
    FILE *f = fopen("TCP_Server/account.txt", "a");
    if (f == NULL) {
        perror("Cannot open account.txt for writing");
        return -1;
    }
    
    // Ghi vào file: <username> <password> <group_id>
    // Mặc định group_id = -1 (chưa tham gia nhóm nào)
    fprintf(f, "%s %s -1\n", username, password);
    fclose(f);
    
    // Thêm vào memory
    if (account_count < MAX_ACCOUNTS) {
        strcpy(accounts[account_count].username, username);
        strcpy(accounts[account_count].password, password);
        accounts[account_count].logged_in = 0;
        accounts[account_count].group_id = -1;
        account_count++;
    }
    
    return 0;
}

/**
 * @function load_groups: Load groups from file into memory
 **/
void load_groups() {
    FILE *f = fopen("TCP_Server/groups.txt", "r");
    if (f == NULL) {
        // File không tồn tại là bình thường, tạo mới
        printf("No groups file found, starting fresh\n");
        return;
    }
    
    group_count = 0;
    while (fscanf(f, "%d %s %s %d", &groups[group_count].group_id,
                  groups[group_count].group_name,
                  groups[group_count].leader,
                  &groups[group_count].active) == 4) {
        group_count++;
        if (group_count >= MAX_GROUPS) break;
    }
    
    fclose(f);
    printf("Loaded %d groups\n", group_count);
}

/**
 * @function find_group_by_name: Find group index by name
 * @return: group index if found, -1 otherwise
 **/
int find_group_by_name(const char *group_name) {
    for (int i = 0; i < group_count; i++) {
        if (groups[i].active && strcmp(groups[i].group_name, group_name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @function find_group_by_id: Find group index by id
 * @return: group index if found, -1 otherwise
 **/
int find_group_by_id(int group_id) {
    for (int i = 0; i < group_count; i++) {
        if (groups[i].active && groups[i].group_id == group_id) {
            return i;
        }
    }
    return -1;
}

/**
 * @function add_join_request: Add a join request
 * @return: 0 on success, -1 on error
 **/
int add_join_request(const char *username, int group_id) {
    if (join_request_count >= MAX_JOIN_REQUESTS) {
        return -1;
    }
    strcpy(join_requests[join_request_count].username, username);
    join_requests[join_request_count].group_id = group_id;
    join_request_count++;
    return 0;
}

/**
 * @function remove_join_request: Remove a join request
 * @return: 0 on success, -1 if not found
 **/
int remove_join_request(const char *username, int group_id) {
    for (int i = 0; i < join_request_count; i++) {
        if (strcmp(join_requests[i].username, username) == 0 &&
            join_requests[i].group_id == group_id) {
            // Shift remaining requests
            for (int j = i; j < join_request_count - 1; j++) {
                join_requests[j] = join_requests[j + 1];
            }
            join_request_count--;
            return 0;
        }
    }
    return -1;
}

/**
 * @function save_groups: Save groups to file
 * @return: 0 on success, -1 on error
 **/
int save_groups() {
    FILE *f = fopen("TCP_Server/groups.txt", "w");
    if (f == NULL) {
        return -1;
    }
    
    for (int i = 0; i < group_count; i++) {
        fprintf(f, "%d %s %s %d\n", 
                groups[i].group_id,
                groups[i].group_name,
                groups[i].leader,
                groups[i].active);
    }
    
    fclose(f);
    return 0;
}

/**
 * @function check_join_request_exists: Check if join request exists
 * @return: 1 if exists, 0 otherwise
 **/
int check_join_request_exists(const char *username, int group_id) {
    for (int i = 0; i < join_request_count; i++) {
        if (strcmp(join_requests[i].username, username) == 0 &&
            join_requests[i].group_id == group_id) {
            return 1;
        }
    }
    return 0;
}

/**
 * @function tcp_send: Send message to client with \r\n delimiter
 * @param sockfd: Socket file descriptor of the client
 * @param msg: Message string to send (without \r\n)
 * @return: Number of bytes sent on success, -1 on error
 **/
int tcp_send(int sockfd, char *msg) {
    char buffer[BUFF_SIZE + 2];
    int len, total = 0, bytes_sent;
    
    /* Add \r\n to message */
    snprintf(buffer, sizeof(buffer), "%s\r\n", msg);
    len = strlen(buffer);
    
    /* Send all data */
    while (total < len) {
        bytes_sent = send(sockfd, buffer + total, len - total, 0);
        if (bytes_sent <= 0) {
            return -1;
        }
        total += bytes_sent;
    }
    
    return total;
}

/**
 * @function tcp_receive: Receive complete message from client (delimited by \r\n)
 * @param sockfd: Socket file descriptor of the client
 * @param state: Connection state containing receive buffer
 * @param buffer: Buffer to store the received message
 * @param max_len: Maximum length of the buffer
 * @return: Length of received message on success, -1 on error
 **/
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len) {
    int bytes_received, i;
    
    while (1) {
        /* Check if we have \r\n in recv_buffer */
        for (i = 0; i < state->buffer_pos - 1; i++) {
            if (state->recv_buffer[i] == '\r' && state->recv_buffer[i + 1] == '\n') {
                /* Found complete message */
                int msg_len = i;
                if (msg_len >= max_len) {
                    msg_len = max_len - 1;
                }
                
                memcpy(buffer, state->recv_buffer, msg_len);
                buffer[msg_len] = '\0';
                
                /* Remove message from buffer */
                state->buffer_pos -= (i + 2);
                memmove(state->recv_buffer, state->recv_buffer + i + 2, state->buffer_pos);
                
                return msg_len;
            }
        }
        
        /* Receive more data */
        if (state->buffer_pos >= BUFF_SIZE - 1) {
            return -1; /* Buffer full */
        }
        
        bytes_received = recv(sockfd, state->recv_buffer + state->buffer_pos, BUFF_SIZE - state->buffer_pos - 1, 0);
        if (bytes_received <= 0) {
            return -1;
        }
        
        state->buffer_pos += bytes_received;
    }
}

/**
 * @function check_and_create_dir: Ensure storage directory exists
 */
void check_and_create_dir() {
    struct stat st = {0};
    if (stat(STORAGE_DIR, &st) == -1) {
        mkdir(STORAGE_DIR, 0700); // Tạo thư mục với quyền đọc/ghi cho owner
    }
}

// --- THÊM ĐOẠN NÀY VÀO TRƯỚC process_command ---

/**
 * @function get_file_size: Get size of a file
 * @param filename: Path to file
 * @return: File size in bytes, or -1 if error
 */
long long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/**
 * @function send_file_content: Read file from disk and send raw bytes to client
 * @param sockfd: Socket descriptor
 * @param filepath: Full path to file
 * @return: 0 on success, -1 on error
 */
int send_file_content(int sockfd, char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return -1;
    }

    char file_buf[BUFF_SIZE];
    size_t n_read;
    
    while ((n_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        // Gửi dữ liệu thô, không dùng tcp_send (vì tcp_send thêm \r\n)
        int n_sent = send(sockfd, file_buf, n_read, 0);
        if (n_sent < 0) {
            fclose(fp);
            return -1;
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * @function receive_file_content: Receive raw binary data from client
 * Handles buffer leftovers from previous recv operations.
 */
int receive_file_content(int sockfd, conn_state_t *state, char *filepath, long long filesize) {
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        perror("File open failed");
        return -1; // Lỗi 502
    }

    long long total_received = 0;
    
    // 1. Xử lý dữ liệu còn tồn đọng trong buffer (Leftover data)
    // Đây là dữ liệu client đã gửi ngay sau lệnh UPLOAD mà server đã recv nhưng chưa xử lý
    if (state->buffer_pos > 0) {
        long long to_write = state->buffer_pos;
        
        // Nếu dữ liệu tồn đọng lớn hơn cả kích thước file (trường hợp file rất nhỏ)
        if (to_write > filesize) {
            to_write = filesize;
        }

        fwrite(state->recv_buffer, 1, to_write, fp);
        total_received += to_write;

        // Dọn dẹp buffer: Xóa phần đã ghi vào file, dồn phần thừa (nếu có - lệnh tiếp theo) lên đầu
        int remaining = state->buffer_pos - to_write;
        if (remaining > 0) {
            memmove(state->recv_buffer, state->recv_buffer + to_write, remaining);
        }
        state->buffer_pos = remaining;
    }

    // 2. Nhận phần còn lại trực tiếp từ socket
    char file_buf[BUFF_SIZE];
    int n;
    
    while (total_received < filesize) {
        // Tính số byte cần nhận (không nhận quá filesize)
        long long bytes_to_recv = sizeof(file_buf);
        if (filesize - total_received < bytes_to_recv) {
            bytes_to_recv = filesize - total_received;
        }

        n = recv(sockfd, file_buf, bytes_to_recv, 0);
        if (n <= 0) {
            fclose(fp);
            return -2; // Lỗi kết nối
        }

        fwrite(file_buf, 1, n, fp);
        total_received += n;
    }

    fclose(fp);
    printf("File saved: %s (%lld bytes)\n", filepath, total_received);
    return 0; // Thành công
}

/**
 * @function process_command: Process and execute client commands (USER, POST, BYE)
 * @param state: Connection state of the client
 * @param command: Command string received from client
 * @return: None
 **/
void process_command(conn_state_t *state, char *command) {
    char cmd[20], arg[BUFF_SIZE];
    int i;
    long long filesize;
    char log_result[256]; /* Buffer for log result message */
    
    /* Parse command */
    if (sscanf(command, "%s", cmd) != 1) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Handle LOGIN command */
    if (strcmp(cmd, "LOGIN") == 0) {
        /* Check if already logged in */
        if (state->is_logged_in) {
            tcp_send(state->sockfd, "403");
            write_log(state->client_addr, command, "-ERR Already logged in");
            return;
        }
        
        /* Parse username and password */
        char username[MAX_USERNAME], password[MAX_USERNAME];
        if (sscanf(command, "LOGIN %s %s", username, password) != 2) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }
        
        /* Find account */
        pthread_mutex_lock(&account_mutex);
        int found = -1;
        for (i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, username) == 0) {
                found = i;
                break;
            }
        }
        
        if (found == -1) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "402"); // Tài khoản không tồn tại
            write_log(state->client_addr, command, "-ERR Account does not exist");
            return;
        }
        
        /* Check password */
        if (strcmp(accounts[found].password, password) != 0) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "401"); // Sai mật khẩu
            write_log(state->client_addr, command, "-ERR Wrong password");
            return;
        }
        
        if (accounts[found].logged_in == 1) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "403"); // Đã đăng nhập trước đó
            write_log(state->client_addr, command, "-ERR Already logged in elsewhere");
            return;
        }
        
        /* Login successful */
        accounts[found].logged_in = 1;
        strcpy(state->logged_user, username);
        state->is_logged_in = 1;
        state->current_group_id = accounts[found].group_id;
        pthread_mutex_unlock(&account_mutex);
        
        tcp_send(state->sockfd, "110");
        write_log(state->client_addr, command, "+OK Login successful");
        printf("User %s logged in\n", username);
    }
    /* Handle REGISTER command */
    else if (strcmp(cmd, "REGISTER") == 0) {
        /* Check if already logged in */
        if (state->is_logged_in) {
            tcp_send(state->sockfd, "403"); // Phiên đã được đăng nhập
            write_log(state->client_addr, command, "-ERR Already logged in");
            return;
        }
        
        /* Parse username and password */
        char username[MAX_USERNAME], password[MAX_USERNAME];
        if (sscanf(command, "REGISTER %s %s", username, password) != 2) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }
        
        /* Check if username already exists */
        pthread_mutex_lock(&account_mutex);
        int exists = 0;
        for (i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, username) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (exists) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "501"); // Username đã tồn tại
            write_log(state->client_addr, command, "-ERR Username already exists");
            return;
        }
        
        /* Save new account */
        if (save_account(username, password) == 0) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "120"); // Đăng ký thành công
            write_log(state->client_addr, command, "+OK Registration successful");
            printf("New account registered: %s\n", username);
        } else {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "502"); // Lỗi ghi file
            write_log(state->client_addr, command, "-ERR Failed to save account");
        }
    }
    /* Handle LEAVE command */
    else if (strcmp(cmd, "LEAVE") == 0) {
        // 1. Kiểm tra đã đăng nhập
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Chưa đăng nhập
            return;
        }
        
        // 2. Kiểm tra có ở trong nhóm nào không
        if (state->current_group_id == -1) {
            tcp_send(state->sockfd, "404"); // Chưa tham gia nhóm nào
            write_log(state->client_addr, command, "-ERR Not in any group");
            return;
        }
        
        int old_group_id = state->current_group_id;
        
        pthread_mutex_lock(&group_mutex);
        
        // 3. Kiểm tra xem user có phải là leader không
        int group_idx = find_group_by_id(old_group_id);
        int is_leader = 0;
        if (group_idx != -1 && strcmp(groups[group_idx].leader, state->logged_user) == 0) {
            is_leader = 1;
        }
        
        pthread_mutex_unlock(&group_mutex);
        
        // 4. Nếu là leader → Soft delete nhóm và đá tất cả thành viên
        if (is_leader) {
            pthread_mutex_lock(&group_mutex);
            
            // Set active = 0 (soft delete)
            if (group_idx != -1) {
                groups[group_idx].active = 0;
                save_groups(); // Ghi lại groups.txt
            }
            
            pthread_mutex_unlock(&group_mutex);
            
            // Đá tất cả thành viên ra khỏi nhóm (set group_id = -1)
            pthread_mutex_lock(&account_mutex);
            for (i = 0; i < account_count; i++) {
                if (accounts[i].group_id == old_group_id) {
                    accounts[i].group_id = -1;
                }
            }
            
            // Ghi lại account.txt
            FILE *f = fopen("TCP_Server/account.txt", "w");
            if (f != NULL) {
                for (int j = 0; j < account_count; j++) {
                    fprintf(f, "%s %s %d\n", accounts[j].username, 
                            accounts[j].password, accounts[j].group_id);
                }
                fclose(f);
            }
            pthread_mutex_unlock(&account_mutex);
            
            state->current_group_id = -1;
            tcp_send(state->sockfd, "200"); // Rời nhóm thành công
            write_log(state->client_addr, command, "+OK Group deleted (leader left)");
            printf("Leader %s left and group %d was deleted\n", state->logged_user, old_group_id);
        }
        // 5. Nếu là thành viên thường → Chỉ rời nhóm
        else {
            pthread_mutex_lock(&account_mutex);
            for (i = 0; i < account_count; i++) {
                if (strcmp(accounts[i].username, state->logged_user) == 0) {
                    accounts[i].group_id = -1;
                    
                    // Ghi lại vào file account.txt
                    FILE *f = fopen("TCP_Server/account.txt", "w");
                    if (f != NULL) {
                        for (int j = 0; j < account_count; j++) {
                            fprintf(f, "%s %s %d\n", accounts[j].username, 
                                    accounts[j].password, accounts[j].group_id);
                        }
                        fclose(f);
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&account_mutex);
            
            state->current_group_id = -1;
            tcp_send(state->sockfd, "200"); // Rời nhóm thành công
            write_log(state->client_addr, command, "+OK Left group successfully");
            printf("User %s left group (group_id: %d)\n", state->logged_user, old_group_id);
        }
    }
    /* Handle LOGOUT command */
    else if (strcmp(cmd, "JOIN") == 0) {
        // 1. Kiểm tra đã đăng nhập
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Chưa đăng nhập
            return;
        }
        
        // 2. Parse group name
        char group_name[MAX_GROUP_NAME];
        if (sscanf(command, "JOIN %s", group_name) != 1) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }
        
        pthread_mutex_lock(&group_mutex);
        
        // 3. Tìm group
        int group_idx = find_group_by_name(group_name);
        if (group_idx == -1) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "500"); // Nhóm không tồn tại
            write_log(state->client_addr, command, "-ERR Group does not exist");
            return;
        }
        
        int group_id = groups[group_idx].group_id;
        
        // 4. Kiểm tra đã ở trong nhóm này chưa
        if (state->current_group_id == group_id) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "405"); // Đã ở trong nhóm này rồi
            write_log(state->client_addr, command, "-ERR Already in this group");
            return;
        }
        
        // 5. Kiểm tra đã gửi request chưa
        if (check_join_request_exists(state->logged_user, group_id)) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "160"); // Đã gửi yêu cầu rồi (coi như thành công)
            write_log(state->client_addr, command, "+OK Request already sent");
            return;
        }
        
        // 6. Thêm join request
        if (add_join_request(state->logged_user, group_id) == 0) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "160"); // Gửi yêu cầu thành công
            write_log(state->client_addr, command, "+OK Join request sent");
            printf("User %s requested to join group %s\n", state->logged_user, group_name);
        } else {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "502"); // Lỗi hệ thống
            write_log(state->client_addr, command, "-ERR System error");
        }
    }
    /* Handle APPROVE command */
    else if (strcmp(cmd, "APPROVE") == 0) {
        // 1. Kiểm tra đã đăng nhập
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Chưa đăng nhập
            return;
        }
        
        // 2. Parse username
        char target_username[MAX_USERNAME];
        if (sscanf(command, "APPROVE %s", target_username) != 1) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }
        
        pthread_mutex_lock(&group_mutex);
        
        // 3. Kiểm tra user hiện tại có phải trưởng nhóm không
        if (state->current_group_id == -1) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "406"); // Không phải trưởng nhóm (chưa có nhóm)
            return;
        }
        
        int group_idx = find_group_by_id(state->current_group_id);
        if (group_idx == -1 || strcmp(groups[group_idx].leader, state->logged_user) != 0) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "406"); // Không phải trưởng nhóm
            write_log(state->client_addr, command, "-ERR Not group leader");
            return;
        }
        
        // 4. Kiểm tra có yêu cầu từ user này không
        if (!check_join_request_exists(target_username, state->current_group_id)) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "500"); // Không tìm thấy yêu cầu
            write_log(state->client_addr, command, "-ERR No request from this user");
            return;
        }
        
        // 5. Phê duyệt: Xóa request và cập nhật group_id cho user
        remove_join_request(target_username, state->current_group_id);
        
        pthread_mutex_lock(&account_mutex);
        for (i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, target_username) == 0) {
                accounts[i].group_id = state->current_group_id;
                
                // Cập nhật file account.txt
                FILE *f = fopen("TCP_Server/account.txt", "w");
                if (f != NULL) {
                    for (int j = 0; j < account_count; j++) {
                        fprintf(f, "%s %s %d\n", accounts[j].username, 
                                accounts[j].password, accounts[j].group_id);
                    }
                    fclose(f);
                }
                break;
            }
        }
        pthread_mutex_unlock(&account_mutex);
        pthread_mutex_unlock(&group_mutex);
        
        tcp_send(state->sockfd, "170"); // Phê duyệt thành công
        write_log(state->client_addr, command, "+OK Approval successful");
        printf("User %s approved %s to join group\n", state->logged_user, target_username);
    }
    /* Handle LOGOUT command */
    else if (strcmp(cmd, "LOGOUT") == 0) {
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Chưa đăng nhập
            return;
        }
        
        /* Logout */
        pthread_mutex_lock(&account_mutex);
        for (i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, state->logged_user) == 0) {
                accounts[i].logged_in = 0;
                break;
            }
        }
        pthread_mutex_unlock(&account_mutex);
        
        printf("User %s logged out\n", state->logged_user);
        state->is_logged_in = 0;
        tcp_send(state->sockfd, "130");
        write_log(state->client_addr, command, "+OK Logout successful");
    }
    /* Handle UPLOAD command */
    else if (strcmp(cmd, "UPLOAD") == 0) {
        // 1. Kiểm tra đăng nhập
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Mã lỗi: Chưa đăng nhập
            return;
        }

        if (state->current_group_id == -1) {
            tcp_send(state->sockfd, "404"); // Lỗi: Chưa tham gia nhóm
            return;
        }

        // 2. Phân tích cú pháp: UPLOAD <filename> <filesize>
        // Lưu ý: filename có thể chứa đường dẫn, ta chỉ lấy tên file
        if (sscanf(command, "UPLOAD %s %lld", arg, &filesize) != 2) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }

        // 3. Chuẩn bị đường dẫn file
        // Hiện tại chưa có Group, ta lưu tạm vào thư mục server_storage/
        check_and_create_dir();
        
        char filepath[BUFF_SIZE + 100];
        // Đơn giản hóa tên file để tránh tấn công Directory Traversal (../)
        // Trong thực tế nên dùng basename(arg) nhưng cần include libgen.h
        snprintf(filepath, sizeof(filepath), "%s/%s", STORAGE_DIR, arg);

        // 4. Gửi mã 141 - Sẵn sàng nhận file
        tcp_send(state->sockfd, "141");

        // 5. Chuyển sang chế độ nhận file binary
        int ret = receive_file_content(state->sockfd, state, filepath, filesize);

        if (ret == 0) {
            tcp_send(state->sockfd, "140"); // Upload thành công
            snprintf(log_result, sizeof(log_result), "+OK Successful upload");
            write_log(state->client_addr, command, log_result);
        } else if (ret == -1) {
            tcp_send(state->sockfd, "502"); // Lỗi ghi file
            write_log(state->client_addr, command, "-ERR File write error");
        } else {
            // Lỗi kết nối (-2), handle_client sẽ tự đóng socket sau khi hàm này return
            // Không cần gửi gì cả vì kết nối có thể đã đứt
            write_log(state->client_addr, command, "-ERR Connection lost");
        }
    } 
    else if (strcmp(cmd, "DOWNLOAD") == 0) {
        // 1. Kiểm tra đăng nhập
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "400"); // Mã lỗi: Chưa đăng nhập
            return;
        }

        if (state->current_group_id == -1) {
            tcp_send(state->sockfd, "404"); // Lỗi: Chưa tham gia nhóm
            return;
        }

        // 2. Phân tích cú pháp: DOWNLOAD <filename>
        if (sscanf(command, "DOWNLOAD %s", arg) != 1) {
            tcp_send(state->sockfd, "300"); // Sai cú pháp
            return;
        }

        // 3. Kiểm tra file tồn tại
        char filepath[512];
        // Giả sử file nằm trong thư mục server_storage
        snprintf(filepath, sizeof(filepath), "%s/%.495s", STORAGE_DIR, arg);
        
        long long filesize = get_file_size(filepath);
        if (filesize < 0) {
            tcp_send(state->sockfd, "500"); // File không tồn tại
            return;
        }

        // 4. Gửi thông báo bắt đầu truyền: 151 <filesize>
        // Mã 151 báo hiệu cho Client biết file có tồn tại và kích thước bao nhiêu
        char msg[100];
        snprintf(msg, sizeof(msg), "151 %lld", filesize);
        tcp_send(state->sockfd, msg);

        // --- ĐỒNG BỘ HÓA ---
        // Để an toàn, tránh việc Server gửi dữ liệu quá nhanh làm tràn buffer Client
        // khi Client chưa kịp chuyển sang chế độ nhận file binary.
        // Server sẽ chờ một tín hiệu "READY" từ Client.
        // Tuy nhiên, trong mô hình poll đơn luồng, việc gọi tcp_receive ở đây
        // có thể làm block luồng chính nếu Client chậm.
        // ĐỂ ĐƠN GIẢN CHO BÀI TẬP: Ta sẽ gửi luôn. 
        // Client PHẢI xử lý được dữ liệu dồn toa (Buffer leftover).
        
        // 5. Gửi nội dung file
        if (send_file_content(state->sockfd, filepath) == 0) {
            // Gửi file xong mới gửi thông báo thành công theo giao thức
            // Client sẽ nhận được mã này sau khi đã đọc đủ số byte của file
            tcp_send(state->sockfd, "150");
            snprintf(log_result, sizeof(log_result), "+OK Successful download %lld bytes", filesize);
            write_log(state->client_addr, command, log_result);
        } else {
            // Lỗi trong quá trình đọc file server hoặc gửi mạng
            // (Thường thì connection sẽ đứt sau đó)
            write_log(state->client_addr, command, "-ERR Download failed");
        }
    }
    /* ------------------------- */
    else {
        tcp_send(state->sockfd, "300");
    }
}

/**
 * @function handle_client: Thread function to handle a client connection
 * @param arg: Pointer to conn_state_t structure for this client
 * @return: NULL after client disconnects
 **/
void *handle_client(void *arg) {
    conn_state_t *state = (conn_state_t *)arg;
    char buffer[BUFF_SIZE];
    int ret;
    
    /* Send welcome message */
    tcp_send(state->sockfd, "100");
    write_log(state->client_addr, NULL, "+OK Welcome to file server");
    
    /* Process commands */
    while (1) {
        ret = tcp_receive(state->sockfd, state, buffer, BUFF_SIZE);
        if (ret <= 0) {
            break; /* Connection closed or error */
        }
        
        printf("Received: %s\n", buffer);
        process_command(state, buffer);
    }
    
    /* Logout if logged in */
    if (state->is_logged_in) {
        pthread_mutex_lock(&account_mutex);
        for (int i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, state->logged_user) == 0) {
                accounts[i].logged_in = 0;
                printf("User %s disconnected (auto logout)\n", state->logged_user);
                break;
            }
        }
        pthread_mutex_unlock(&account_mutex);
    }
    
    close(state->sockfd);
    free(state);
    pthread_detach(pthread_self());
    return NULL;
}

/**
 * @function main: Main server function to initialize and accept connections
 * @param argc: Number of command line arguments
 * @param argv: Array of command line arguments (argv[1] is port number)
 * @return: 0 on normal exit, 1 on error
 **/
int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    pthread_t tid;
    int port;
    
    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        return 1;
    }
    
    port = atoi(argv[1]);
    
    load_accounts();
    load_groups();
    
    /* Create socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        return 1;
    }
    
    
    /* Bind */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(listenfd);
        return 1;
    }
    
    /* Listen */
    if (listen(listenfd, BACKLOG) == -1) {
        perror("listen() error");
        close(listenfd);
        return 1;
    }
    
    printf("Server started at port %d\n", port);
    
    /* Accept connections */
    while (1) {
        sin_size = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &sin_size);
        if (connfd == -1) {
            perror("accept() error");
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        /* Create state for this connection */
        conn_state_t *state = malloc(sizeof(conn_state_t));
        memset(state, 0, sizeof(conn_state_t));
        state->sockfd = connfd;
        
        /* Store client address for logging */
        snprintf(state->client_addr, sizeof(state->client_addr), "%s:%d",
                 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        /* Create thread to handle client */
        pthread_create(&tid, NULL, handle_client, state);
    }
    
    close(listenfd);
    return 0;
}