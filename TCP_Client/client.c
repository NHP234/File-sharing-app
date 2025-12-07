#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 
#include <libgen.h>  

#define BUFF_SIZE 4096

/* Connection state */
typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
} conn_state_t;

/* Function prototypes */
int tcp_send(int sockfd, char *msg);
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len);
void print_menu();
void print_response(char *code);

long long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/**
 * @function tcp_send: Send message to server with \r\n delimiter
 * @param sockfd: Socket file descriptor of the server connection
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
 * @function tcp_receive: Receive complete message from server (delimited by \r\n)
 * @param sockfd: Socket file descriptor of the server connection
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

void handle_upload(int sockfd, conn_state_t *state) {
    char filepath[256];
    char buffer[BUFF_SIZE];
    long long filesize;
    
    // 1. Nhập đường dẫn file
    printf("Enter file path: ");
    if (fgets(filepath, sizeof(filepath), stdin) == NULL) return;
    filepath[strcspn(filepath, "\n")] = 0; // Xóa \n

    // 2. Kiểm tra file và lấy kích thước
    filesize = get_file_size(filepath);
    if (filesize < 0) {
        printf("Error: File not found or cannot access.\n");
        return;
    }

    // 3. Chuẩn bị lệnh UPLOAD
    // Dùng basename() để chỉ lấy tên file (vd: /home/user/test.txt -> test.txt)
    char *filename = basename(filepath);
    char command[BUFF_SIZE];
    snprintf(command, sizeof(command), "UPLOAD %s %lld", filename, filesize);

    // 4. Gửi lệnh và chờ phản hồi "141"
    if (tcp_send(sockfd, command) <= 0) return;
    
    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) <= 0) return;

    // Nếu Server trả về lỗi (không phải 141)
    if (strcmp(buffer, "141") != 0) {
        print_response(buffer); // In lỗi (400, 300...)
        return;
    }

    // 5. Bắt đầu gửi dữ liệu file (Binary Mode)
    printf("Server is ready. Uploading...\n");
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        perror("Cannot open file");
        return;
    }

    char file_buf[65536]; // Buffer 64KB như yêu cầu
    size_t n_read;
    long long total_sent = 0;

    while ((n_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        // Lưu ý: Dùng send() gốc, KHÔNG dùng tcp_send() vì tcp_send tự thêm \r\n
        int sent = send(sockfd, file_buf, n_read, 0);
        if (sent < 0) {
            perror("Send file failed");
            break;
        }
        total_sent += sent;
        
        // (Tùy chọn) In tiến độ upload
        // printf("\rSent %lld / %lld bytes", total_sent, filesize);
    }
    printf("\n");
    fclose(fp);

    // 6. Chờ phản hồi kết quả cuối cùng (140)
    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) > 0) {
        print_response(buffer);
    }
}

/**
 * @function receive_file_content_client: Receive binary data from server and save to file
 * Handles data already present in the buffer (leftover) before receiving more.
 */
int receive_file_content_client(int sockfd, conn_state_t *state, char *filepath, long long filesize) {
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        printf("Error: Cannot open file %s for writing.\n", filepath);
        return -1;
    }

    long long total_received = 0;

    // 1. QUAN TRỌNG: Xử lý dữ liệu thừa trong buffer (Leftover)
    // Sau khi tcp_receive đọc xong dòng "151 <size>\r\n", phần dư trong buffer
    // chính là phần đầu của dữ liệu file.
    if (state->buffer_pos > 0) {
        long long chunk_size = state->buffer_pos;
        
        // Nếu phần dư lớn hơn cả kích thước file (file quá nhỏ, hoặc dính cả lệnh 150 phía sau)
        if (chunk_size > filesize) {
            chunk_size = filesize;
        }

        fwrite(state->recv_buffer, 1, chunk_size, fp);
        total_received += chunk_size;

        // Dọn dẹp buffer: Xóa phần file đã ghi, dồn phần còn lại (nếu có - ví dụ lệnh 150) lên đầu
        int remaining = state->buffer_pos - chunk_size;
        if (remaining > 0) {
            memmove(state->recv_buffer, state->recv_buffer + chunk_size, remaining);
        }
        state->buffer_pos = remaining;
    }

    // 2. Nhận phần còn lại trực tiếp từ socket
    char file_buf[BUFF_SIZE];
    int n;

    while (total_received < filesize) {
        // Tính toán số byte cần nhận để không đọc lố sang lệnh tiếp theo (150)
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
        
        // (Tùy chọn) In tiến độ
        // printf("\rDownloading... %lld / %lld bytes", total_received, filesize);
    }

    printf("\n");
    fclose(fp);
    return 0;
}

void handle_download(int sockfd, conn_state_t *state) {
    char filename[256];
    char buffer[BUFF_SIZE];
    long long filesize;

    // 1. Nhập tên file cần tải
    printf("Enter filename to download: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) return;
    filename[strcspn(filename, "\n")] = 0; // Xóa \n

    if (strlen(filename) == 0) return;

    // 2. Gửi lệnh: DOWNLOAD <filename>
    char command[BUFF_SIZE];
    snprintf(command, sizeof(command), "DOWNLOAD %s", filename);
    
    if (tcp_send(sockfd, command) <= 0) return;

    // 3. Nhận phản hồi đầu tiên
    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) <= 0) return;

    // 4. Kiểm tra mã phản hồi
    int code;
    if (sscanf(buffer, "%d", &code) != 1) {
        print_response(buffer); return;
    }

    if (code == 400) {
        printf(">> Error: Please login first.\n");
        return;
    } else if (code == 500) {
        printf(">> Error: File not found on server.\n");
        return;
    } else if (code == 151) {
        // Mã 151 <filesize>: Bắt đầu nhận file
        sscanf(buffer, "151 %lld", &filesize);
        printf("File found. Size: %lld bytes. Downloading...\n", filesize);

        // Gọi hàm nhận file nhị phân
        // Lưu file vào thư mục hiện tại với tên giống server
        if (receive_file_content_client(sockfd, state, filename, filesize) == 0) {
            printf("File saved as: %s\n", filename);
            
            // 5. Sau khi nhận xong file, Server sẽ gửi tiếp mã 150
            // Dữ liệu mã 150 có thể đã nằm sẵn trong buffer do receive_file_content_client xử lý leftover
            if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) > 0) {
                if (strcmp(buffer, "150") == 0) {
                    printf(">> Download successfully completed.\n");
                } else {
                    printf(">> Warning: Unexpected response after download: %s\n", buffer);
                }
            }
        } else {
            printf(">> Error during download.\n");
        }
    } else {
        print_response(buffer); // Các lỗi khác (300...)
    }
}

void print_menu() {
    printf("\n=== FILE SHARING SYSTEM ===\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("3. Logout\n");
    printf("4. Exit\n");
    printf("5. Upload File\n"); 
    printf("6. Download File\n");
    printf("7. Join Group\n");
    printf("8. Approve Member\n");
    printf("9. Leave Group\n");
    printf("============================\n");
    printf("Your choice: ");
}

/**
 * @function print_response: Translate and display server response code
 * @param code: Response code string from server
 * @return: None
 **/
void print_response(char *code) {
    if (strcmp(code, "100") == 0) {
        printf(">> Connected to server successfully\n");
    } else if (strcmp(code, "110") == 0) {
        printf(">> Login successful\n");
    } else if (strcmp(code, "120") == 0) {
        printf(">> Registration successful\n");
    } else if (strcmp(code, "130") == 0) {
        printf(">> Logout successful\n");
    } else if (strcmp(code, "300") == 0) {
        printf(">> Error: Invalid command syntax\n");
    } else if (strcmp(code, "400") == 0) {
        printf(">> Error: Please login first\n");
    } else if (strcmp(code, "401") == 0) {
        printf(">> Error: Wrong username or password\n");
    } else if (strcmp(code, "402") == 0) {
        printf(">> Error: Account does not exist\n");
    } else if (strcmp(code, "403") == 0) {
        printf(">> Error: Already logged in\n");
    } else if (strcmp(code, "404") == 0) {
        printf(">> Error: You are not in any group\n");
    } else if (strcmp(code, "405") == 0) {
        printf(">> Error: Already in this group\n");
    } else if (strcmp(code, "406") == 0) {
        printf(">> Error: You are not the group leader\n");
    } else if (strcmp(code, "407") == 0) {
        printf(">> Error: You are already in another group\n");
    } else if (strcmp(code, "140") == 0) {
        printf(">> Upload successfully!\n");
    } else if (strcmp(code, "160") == 0) {
        printf(">> Join request sent successfully\n");
    } else if (strcmp(code, "170") == 0) {
        printf(">> Member approved successfully\n");
    } else if (strcmp(code, "200") == 0) {
        printf(">> Left group successfully\n");
    } else if (strcmp(code, "500") == 0) {
        printf(">> Error: Resource not found\n");
    } else if (strcmp(code, "502") == 0) {
        printf(">> Error: Username already exists\n");
    } else if (strcmp(code, "502") == 0) {
        printf(">> Error: Server could not save file\n");
    } else {
        printf(">> Response: %s\n", code);
    }
}

/**
 * @function main: Main client function to connect to server and handle user interaction
 * @param argc: Number of command line arguments
 * @param argv: Array of command line arguments (argv[1] is IP, argv[2] is port)
 * @return: 0 on normal exit, 1 on error
 **/
int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char ip_addr[50];
    int port;
    conn_state_t state;
    char buffer[BUFF_SIZE];
    char username[100];
    char command[BUFF_SIZE];
    int choice;
    int is_logged_in = 0;
    
    if (argc != 3) {
        printf("Usage: %s IP_Addr Port_Number\n", argv[0]);
        return 1;
    }
    
    strcpy(ip_addr, argv[1]);
    port = atoi(argv[2]);
    
    /* Create socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        return 1;
    }
    
    /* Connect to server */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        close(sockfd);
        return 1;
    }
    
    /* Initialize state */
    memset(&state, 0, sizeof(conn_state_t));
    
    /* Receive welcome message */
    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
        print_response(buffer);
    }
    
    /* Main loop */
    while (1) {
        print_menu();
        
        if (scanf("%d", &choice) != 1) {
            /* Clear input buffer */
            while (getchar() != '\n');
            printf("Invalid choice\n");
            continue;
        }
        getchar(); 
        
        switch (choice) {
            case 1: /* Login */
                {
                    char password[100];
                    printf("Enter username: ");
                    if (fgets(username, sizeof(username), stdin) == NULL) {
                        break;
                    }
                    username[strcspn(username, "\n")] = 0;
                    
                    printf("Enter password: ");
                    if (fgets(password, sizeof(password), stdin) == NULL) {
                        break;
                    }
                    password[strcspn(password, "\n")] = 0;
                    
                    snprintf(command, sizeof(command), "LOGIN %s %s", username, password);
                    if (tcp_send(sockfd, command) > 0) {
                        if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                            print_response(buffer);
                            if (strcmp(buffer, "110") == 0) {
                                is_logged_in = 1;
                            }
                        }
                    }
                }
                break;
                
            case 2: /* Register */
                {
                    char password[100];
                    printf("Enter username: ");
                    if (fgets(username, sizeof(username), stdin) == NULL) {
                        break;
                    }
                    username[strcspn(username, "\n")] = 0;
                    
                    printf("Enter password: ");
                    if (fgets(password, sizeof(password), stdin) == NULL) {
                        break;
                    }
                    password[strcspn(password, "\n")] = 0;
                    
                    snprintf(command, sizeof(command), "REGISTER %s %s", username, password);
                    if (tcp_send(sockfd, command) > 0) {
                        if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                            print_response(buffer);
                        }
                    }
                }
                break;
                
            case 3: /* Logout */
                if (tcp_send(sockfd, "LOGOUT") > 0) {
                    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                        print_response(buffer);
                        if (strcmp(buffer, "130") == 0) {
                            is_logged_in = 0;
                        }
                    }
                }
                break;
                
            case 4: /* Exit */
                if (is_logged_in) {
                    tcp_send(sockfd, "LOGOUT");
                    tcp_receive(sockfd, &state, buffer, BUFF_SIZE);
                }
                printf("Goodbye!\n");
                close(sockfd);
                return 0;

            case 5: /* Upload */
                handle_upload(sockfd, &state);
                break;

            case 6: /* Download */
                handle_download(sockfd, &state);
                break;
                
            case 7: /* Join Group */
                {
                    char group_name[100];
                    printf("Enter group name to join: ");
                    if (fgets(group_name, sizeof(group_name), stdin) == NULL) {
                        break;
                    }
                    group_name[strcspn(group_name, "\n")] = 0;
                    
                    snprintf(command, sizeof(command), "JOIN %s", group_name);
                    if (tcp_send(sockfd, command) > 0) {
                        if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                            print_response(buffer);
                        }
                    }
                }
                break;
                
            case 8: /* Approve Member */
                {
                    char target_user[100];
                    printf("Enter username to approve: ");
                    if (fgets(target_user, sizeof(target_user), stdin) == NULL) {
                        break;
                    }
                    target_user[strcspn(target_user, "\n")] = 0;
                    
                    snprintf(command, sizeof(command), "APPROVE %s", target_user);
                    if (tcp_send(sockfd, command) > 0) {
                        if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                            print_response(buffer);
                        }
                    }
                }
                break;
                
            case 9: /* Leave Group */
                if (tcp_send(sockfd, "LEAVE") > 0) {
                    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                        print_response(buffer);
                    }
                }
                break;
                
            default:
                printf("Invalid choice\n");
        }
    }
    
    close(sockfd);
    return 0;
}
