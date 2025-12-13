#include "common.h"

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
        
        bytes_received = recv(sockfd, state->recv_buffer + state->buffer_pos, 
                            BUFF_SIZE - state->buffer_pos - 1, 0);
        if (bytes_received <= 0) {
            return -1;
        }
        
        state->buffer_pos += bytes_received;
    }
}

/**
 * @function send_file: Send a file to client in chunks
 * @param sockfd: Socket file descriptor
 * @param filepath: Path to the file to send
 * @return: 0 on success, -1 on error
 **/
int send_file(int sockfd, const char *filepath) {
    // TODO: Implement file sending
    return 0;
}

/**
 * @function receive_file: Receive a file from client in chunks
 * @param sockfd: Socket file descriptor
 * @param filepath: Path where to save the file
 * @param file_size: Size of the file to receive
 * @return: 0 on success, -1 on error
 **/
int receive_file(int sockfd, const char *filepath, long file_size) {
    // TODO: Implement file receiving
    return 0;
}

