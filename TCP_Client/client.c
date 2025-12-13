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

typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
} conn_state_t;

int tcp_send(int sockfd, char *msg);
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len);
void print_menu();
void print_response(char *code);

long long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        // Kiểm tra xem có phải là file thường không (không phải thư mục, symlink, etc.)
        if (S_ISREG(st.st_mode)) {
            return st.st_size;
        }
        // Nếu là thư mục hoặc loại file khác, trả về -2 để phân biệt
        if (S_ISDIR(st.st_mode)) {
            return -2; // Là thư mục
        }
        return -3; // Là loại file khác (symlink, device, etc.)
    }
    return -1; // File không tồn tại hoặc không thể truy cập
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
 * @function send_all: Đảm bảo gửi toàn bộ dữ liệu trong buffer qua socket
 * @param sockfd: Socket file descriptor
 * @param buffer: Con trỏ tới dữ liệu cần gửi (có thể là chuỗi hoặc dữ liệu file)
 * @param length: Tổng số byte cần gửi
 * @return: 0 nếu thành công, -1 nếu có lỗi mạng
 */
int send_all(int sockfd, const void *buffer, int length) {
    const char *ptr = (const char *)buffer; // Ép kiểu để tính toán con trỏ
    int total_sent = 0;
    int bytes_left = length;
    int n;

    while (total_sent < length) {
        n = send(sockfd, ptr + total_sent, bytes_left, 0);
        
        if (n == -1) {
            perror("send() error"); 
            return -1;
        }
        
        total_sent += n;
        bytes_left -= n;
    }

    return 0; // Thành công
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
        for (i = 0; i < state->buffer_pos - 1; i++) {
            if (state->recv_buffer[i] == '\r' && state->recv_buffer[i + 1] == '\n') {
                int msg_len = i;
                if (msg_len >= max_len) {
                    msg_len = max_len - 1;
                }
                
                memcpy(buffer, state->recv_buffer, msg_len);
                buffer[msg_len] = '\0';
                
                state->buffer_pos -= (i + 2);
                memmove(state->recv_buffer, state->recv_buffer + i + 2, state->buffer_pos);
                
                return msg_len;
            }
        }
        
        if (state->buffer_pos >= BUFF_SIZE - 1) {
            return -1; 
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
    
    printf("Enter file path: ");
    if (fgets(filepath, sizeof(filepath), stdin) == NULL) return;
    filepath[strcspn(filepath, "\n")] = 0;

    printf("Hello\n");
    filesize = get_file_size(filepath);
    if (filesize == -1) {
        printf("Error: File not found or cannot access.\n");
        return;
    }
    if (filesize == -2) {
        printf("Error: '%s' is a directory, not a file.\n", filepath);
        return;
    }
    if (filesize == -3) {
        printf("Error: '%s' is not a regular file.\n", filepath);
        return;
    }
    printf("Hello2\n");

    char *filename = basename(filepath);
    char command[BUFF_SIZE];
    snprintf(command, sizeof(command), "UPLOAD %s %lld", filename, filesize);
    if (tcp_send(sockfd, command) <= 0) return;
    
    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) <= 0) return;
    if (strcmp(buffer, "141") != 0) {
        print_response(buffer); 
        return;
    }

    printf("Server is ready. Uploading...\n");
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        perror("Cannot open file");
        return;
    }

    char file_buf[65536]; 
    size_t n_read;
    long long total_sent = 0;

    while ((n_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        int sent = send_all(sockfd, file_buf, n_read);
        if (sent < 0) {
            perror("Send file failed");
            break;
        }
        total_sent += sent;
        
        printf("\rSent %lld / %lld bytes", total_sent, filesize);
    }
    printf("\n");
    fclose(fp);

    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) > 0) {
        print_response(buffer);
    }
}

/**
 * @function receive_file_content_client: Receive binary data from server and save to file
 * @param sockfd: Socket descriptor
 * @param state: Connection state
 * @param filepath: Full path to save the received file
 * @param filesize: Total size of the file to receive
 * @return: 0 on success, -1 on file error, -2 on connection error
 */
int receive_file_content_client(int sockfd, conn_state_t *state, char *filepath, long long filesize) {
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        printf("Error: Cannot open file %s for writing.\n", filepath);
        return -1;
    }

    long long total_received = 0;
    
    if (state->buffer_pos > 0) {
        long long to_write = state->buffer_pos;
        
        if (to_write > filesize) {
            to_write = filesize;
        }

        fwrite(state->recv_buffer, 1, to_write, fp);
        total_received += to_write;

        int remaining = state->buffer_pos - to_write;
        if (remaining > 0) {
            memmove(state->recv_buffer, state->recv_buffer + to_write, remaining);
        }
        state->buffer_pos = remaining;
    }


    char file_buf[BUFF_SIZE];
    int n;

    while (total_received < filesize) {
        long long bytes_to_recv = sizeof(file_buf);
        if (filesize - total_received < bytes_to_recv) {
            bytes_to_recv = filesize - total_received;
        }

        n = recv(sockfd, file_buf, bytes_to_recv, 0);
        if (n <= 0) {
            fclose(fp);
            return -2; 
        }

        fwrite(file_buf, 1, n, fp);
        total_received += n;
        
        printf("\rDownloading... %lld / %lld bytes", total_received, filesize);
    }

    printf("\n");
    fclose(fp);
    return 0;
}

void handle_download(int sockfd, conn_state_t *state) {
    char filename[256];
    char buffer[BUFF_SIZE];
    long long filesize;

    printf("Enter filename to download: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) return;
    filename[strcspn(filename, "\n")] = 0;

    if (strlen(filename) == 0) return;

    char command[BUFF_SIZE];
    snprintf(command, sizeof(command), "DOWNLOAD %s", filename);
    if (tcp_send(sockfd, command) <= 0) return;

    if (tcp_receive(sockfd, state, buffer, BUFF_SIZE) <= 0) return;

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
        sscanf(buffer, "151 %lld", &filesize);
        printf("File found. Size: %lld bytes. Downloading...\n", filesize);

        if (receive_file_content_client(sockfd, state, filename, filesize) == 0) {
            printf("File saved as: %s\n", filename);
            
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
        print_response(buffer);
    }
}

void print_menu() {
    printf("\n=== ARTICLE POSTING SYSTEM ===\n");
    printf("1. Login\n");
    printf("2. Post article\n");
    printf("3. Logout\n");
    printf("4. Exit\n");
    printf("5. Upload File\n"); 
    printf("6. Download File\n");
    printf("==============================\n");
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
        printf(">> Article posted successfully\n");
    } else if (strcmp(code, "130") == 0) {
        printf(">> Logout successful\n");
    } else if (strcmp(code, "211") == 0) {
        printf(">> Account is blocked\n");
    } else if (strcmp(code, "212") == 0) {
        printf(">> Account does not exist\n");
    } else if (strcmp(code, "213") == 0) {
        printf(">> Already logged in\n");
    } else if (strcmp(code, "214") == 0) {
        printf(">> Account already logged in on another client\n");
    } else if (strcmp(code, "221") == 0) {
        printf(">> Not logged in yet\n");
    } else if (strcmp(code, "300") == 0) {
        printf(">> Unknown command\n");
    } else if (strcmp(code, "140") == 0) {
        printf(">> Upload successfully!\n");
    } else if (strcmp(code, "400") == 0) {
        printf(">> Error: Please login first.\n");
    } else if (strcmp(code, "404") == 0) {
        printf(">> Error: You are not in any group. Please join or create a group first.\n");
    } else if (strcmp(code, "502") == 0) {
        printf(">> Error: Server could not save file.\n");
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
    char article[BUFF_SIZE];
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
                printf("Enter username: ");
                if (fgets(username, sizeof(username), stdin) == NULL) {
                    break;
                }
                username[strcspn(username, "\n")] = 0; 
                
                snprintf(command, sizeof(command), "USER %s", username);
                if (tcp_send(sockfd, command) > 0) {
                    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                        print_response(buffer);
                        if (strcmp(buffer, "110") == 0) {
                            is_logged_in = 1;
                        }
                    }
                }
                break;
                
            case 2: /* Post article */
                printf("Enter article content: ");
                if (fgets(article, sizeof(article) - 6, stdin) == NULL) {
                    break;
                }
                article[strcspn(article, "\n")] = 0; 
                
                snprintf(command, sizeof(command), "POST %.4089s", article);
                if (tcp_send(sockfd, command) > 0) {
                    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
                        print_response(buffer);
                    }
                }
                break;
                
            case 3: /* Logout */
                if (tcp_send(sockfd, "BYE") > 0) {
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
                    tcp_send(sockfd, "BYE");
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
                
            default:
                printf("Invalid choice\n");
        }
    }
    
    close(sockfd);
    return 0;
}

