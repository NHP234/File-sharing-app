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
#define BACKLOG 20

/* Account structure */
typedef struct {
    char username[MAX_USERNAME];
    int status; /* 0: blocked, 1: active */
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
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Function prototypes */
void load_accounts();
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
    while (fscanf(f, "%s %d %d", accounts[account_count].username, 
                  &accounts[account_count].status, &accounts[account_count].group_id) == 3) {
        accounts[account_count].logged_in = 0;
        account_count++;
        if (account_count >= MAX_ACCOUNTS) break;
    }
    
    fclose(f);
    printf("Loaded %d accounts\n", account_count);
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
    
    /* Handle USER command */
    if (strcmp(cmd, "USER") == 0) {
        /* Check if already logged in */
        if (state->is_logged_in) {
            tcp_send(state->sockfd, "213");
            return;
        }
        
        /* Try to parse username */
        if (sscanf(command, "USER %s", arg) != 1) {
            /* USER command without username -> account not exist */
            tcp_send(state->sockfd, "212");
            return;
        }
        
        /* Find account */
        pthread_mutex_lock(&account_mutex);
        int found = -1;
        for (i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, arg) == 0) {
                found = i;
                break;
            }
        }
        
        if (found == -1) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "212");
            return;
        }
        
        if (accounts[found].status == 0) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "211");
            return;
        }
        
        if (accounts[found].logged_in == 1) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "214");
            return;
        }
        
        /* Login successful */
        accounts[found].logged_in = 1;
        strcpy(state->logged_user, arg);
        state->is_logged_in = 1;
        state->current_group_id = accounts[found].group_id;
        pthread_mutex_unlock(&account_mutex);
        
        tcp_send(state->sockfd, "110");
        write_log(state->client_addr, command, "+OK Login successful");
        printf("User %s logged in\n", arg);
    }
    /* Handle POST command */
    else if (strcmp(cmd, "POST") == 0) {
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "221");
            return;
        }
        
        /* Extract article content (after "POST ") */
        char *article = command + 5;
        if (strlen(article) == 0) {
            tcp_send(state->sockfd, "300");
            return;
        }
        
        printf("User %s posted: %s\n", state->logged_user, article);
        tcp_send(state->sockfd, "120");
        write_log(state->client_addr, command, "+OK Article posted");
    }
    /* Handle BYE command */
    else if (strcmp(cmd, "BYE") == 0) {
        if (!state->is_logged_in) {
            tcp_send(state->sockfd, "221");
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
