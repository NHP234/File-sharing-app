#ifndef CLIENT_COMMON_H
#define CLIENT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ==================== CONSTANTS ==================== */

#define BUFF_SIZE 8192
#define MAX_PATH 256
#define CHUNK_SIZE 4096

/* ==================== DATA STRUCTURES ==================== */

/* Connection state */
typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
} conn_state_t;

/* ==================== FUNCTION PROTOTYPES ==================== */

/* network.c - Network I/O functions */
int tcp_send(int sockfd, char *msg);
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len);
int send_file(int sockfd, const char *filepath);
int receive_file(int sockfd, const char *filepath, long file_size);

/* ui.c - UI functions */
void print_main_menu();
void print_response(char *response);

/* commands.c - Command functions */
void do_register(int sockfd, conn_state_t *state);
void do_login(int sockfd, conn_state_t *state, int *is_logged_in);
void do_logout(int sockfd, conn_state_t *state, int *is_logged_in);
void do_upload(int sockfd, conn_state_t *state);
void do_download(int sockfd, conn_state_t *state);
void do_create_group(int sockfd, conn_state_t *state);
void do_join_group(int sockfd, conn_state_t *state);
void do_approve(int sockfd, conn_state_t *state);
void do_invite(int sockfd, conn_state_t *state);
void do_accept(int sockfd, conn_state_t *state);
void do_leave(int sockfd, conn_state_t *state);
void do_kick(int sockfd, conn_state_t *state);
void do_list_groups(int sockfd, conn_state_t *state);
void do_list_members(int sockfd, conn_state_t *state);
void do_list_requests(int sockfd, conn_state_t *state);
void do_rename_file(int sockfd, conn_state_t *state);
void do_delete_file(int sockfd, conn_state_t *state);
void do_copy_file(int sockfd, conn_state_t *state);
void do_move_file(int sockfd, conn_state_t *state);
void do_mkdir(int sockfd, conn_state_t *state);
void do_rename_folder(int sockfd, conn_state_t *state);
void do_rmdir(int sockfd, conn_state_t *state);
void do_copy_folder(int sockfd, conn_state_t *state);
void do_move_folder(int sockfd, conn_state_t *state);
void do_list_content(int sockfd, conn_state_t *state);

#endif /* CLIENT_COMMON_H */

