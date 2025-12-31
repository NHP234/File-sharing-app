#include "common.h"
#include <fcntl.h>

/**
 * @function file_lock: Lock a file for reading or writing
 * @param fd: File descriptor
 * @param type: Lock type (F_RDLCK, F_WRLCK, F_UNLCK)
 * @return: 0 on success, -1 on failure
 **/
int file_lock(int fd, int type) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock entire file

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl lock failed");
        return -1;
    }
    return 0;
}

/* ==================== NETWORK I/O FUNCTIONS ==================== */

/**
 * @function tcp_send: Send message to client with \r\n delimiter
 * @param sockfd: Socket file descriptor of the client
 * @param msg: Message string to send (without \r\n)
 * @return: Number of bytes sent on success, -1 on error
 **/
int tcp_send(int sockfd, char *msg) {
    char buffer[BUFF_SIZE + 2];
    int len, total = 0, bytes_sent;
    

    snprintf(buffer, sizeof(buffer), "%s\r\n", msg);
    len = strlen(buffer);
    

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
        
        bytes_received = recv(sockfd, state->recv_buffer + state->buffer_pos, 
                            BUFF_SIZE - state->buffer_pos - 1, 0);
        if (bytes_received <= 0) {
            return -1;
        }
        
        state->buffer_pos += bytes_received;
    }
}

/**
 * @function send_all: Ensure all data in buffer is sent through socket
 * @param sockfd: Socket file descriptor
 * @param buffer: Pointer to data to send (can be string or file data)
 * @param length: Total bytes to send
 * @return: 0 on success, -1 on network error
 **/
int send_all(int sockfd, const void *buffer, int length) {
    const char *ptr = (const char *)buffer;
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

    return 0;
}

/**
 * @function get_file_size: Get size of a file
 * @param filename: Path to file
 * @return: File size in bytes, -1 if not exist, -2 if directory
 **/
long long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        /* Check if it's a regular file */
        if (S_ISREG(st.st_mode)) {
            return st.st_size;
        }
        /* If it's a directory */
        if (S_ISDIR(st.st_mode)) {
            return -2;
        }

    }
    return -1; /* File does not exist or cannot access */
}

/**
 * @function send_file_content: Read file from disk and send raw bytes to client
 * @param sockfd: Socket descriptor
 * @param filepath: Full path to file
 * @return: 0 on success, -1 on error
 **/
int send_file_content(int sockfd, const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return -1;
    }

    int fd = fileno(fp);
    
    /* Lock file for reading */
    if (file_lock(fd, F_RDLCK) == -1) {
        fclose(fp);
        return -1;
    }

    char file_buf[BUFF_SIZE];
    size_t n_read;
    
    while ((n_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
        int n_sent = send_all(sockfd, file_buf, n_read);
        if (n_sent < 0) {
            file_lock(fd, F_UNLCK);
            fclose(fp);
            return -1;
        }
    }
    

    file_lock(fd, F_UNLCK);
    fclose(fp);
    return 0;
}

/**
 * @function receive_file_content: Receive raw binary data from client
 * @param sockfd: Socket descriptor
 * @param state: Connection state
 * @param filepath: Full path to save the received file
 * @param filesize: Total size of the file to receive
 * @return: 0 on success, -1 on file error, -2 on connection error
 **/
int receive_file_content(int sockfd, conn_state_t *state, const char *filepath, long long filesize) {
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        perror("File open failed");
        return -1;
    }

    int fd = fileno(fp);
    
    
    if (file_lock(fd, F_WRLCK) == -1) {
        fclose(fp);
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
            file_lock(fd, F_UNLCK);
            fclose(fp);
            return -2;
        }

        fwrite(file_buf, 1, n, fp);
        total_received += n;
    }

    
    file_lock(fd, F_UNLCK);
    fclose(fp);
    printf("File saved: %s (%lld bytes)\n", filepath, total_received);
    return 0;
}

