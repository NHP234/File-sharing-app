#ifndef COMMON_H
#define COMMON_H

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
#include <dirent.h>
#include <time.h>

/* ==================== CONSTANTS ==================== */

#define BUFF_SIZE 8192
#define MAX_ACCOUNTS 100
#define MAX_GROUPS 50
#define MAX_REQUESTS 200
#define MAX_INVITES 200
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_GROUPNAME 50
#define MAX_PATH 256
#define BACKLOG 20
#define CHUNK_SIZE 4096

/* ==================== DATA STRUCTURES ==================== */

/* Account structure */
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int group_id;           /* -1 if not in any group */
    int is_logged_in;       /* 0: offline, 1: online */
} account_t;

/* Group structure */
typedef struct {
    int group_id;
    char group_name[MAX_GROUPNAME];
    char leader[MAX_USERNAME];
} group_t;

/* Join request structure (pending requests to join a group) */
typedef struct {
    char username[MAX_USERNAME];
    int group_id;
} request_t;

/* Invite structure (pending invites to join a group) */
typedef struct {
    char username[MAX_USERNAME];
    int group_id;
} invite_t;

/* Connection state for each client */
typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
    int sockfd;
    char logged_user[MAX_USERNAME];
    int is_logged_in;
    int user_group_id;      /* Cache of user's group_id */
    char client_addr[50];   /* Client IP:Port for logging */
} conn_state_t;

/* ==================== GLOBAL VARIABLES ==================== */

extern account_t accounts[MAX_ACCOUNTS];
extern int account_count;
extern pthread_mutex_t account_mutex;

extern group_t groups[MAX_GROUPS];
extern int group_count;
extern pthread_mutex_t group_mutex;

extern request_t requests[MAX_REQUESTS];
extern int request_count;
extern pthread_mutex_t request_mutex;

extern invite_t invites[MAX_INVITES];
extern int invite_count;
extern pthread_mutex_t invite_mutex;

extern pthread_mutex_t file_mutex;

/* ==================== FUNCTION PROTOTYPES ==================== */

/* utils.c - Data loading/saving functions */
void load_accounts();
void load_groups();
void load_requests();
void load_invites();
void save_accounts();
void save_groups();
void save_requests();
void save_invites();
void write_log(const char *message);
void write_log_detailed(const char *client_addr, const char *request, const char *result);
void get_log_filename(char *filename, size_t size);
int get_next_group_id();
char* get_group_folder_path(int group_id, char *buffer, int buf_size);
int is_group_leader(const char *username, int group_id);
int count_group_members(int group_id);
char* role_based_access_control(const char *command, conn_state_t *state);

/* network.c - Network I/O functions */
int tcp_send(int sockfd, char *msg);
int tcp_receive(int sockfd, conn_state_t *state, char *buffer, int max_len);
int send_all(int sockfd, const void *buffer, int length);
long long get_file_size(const char *filename);
int send_file_content(int sockfd, const char *filepath);
int receive_file_content(int sockfd, conn_state_t *state, const char *filepath, long long filesize);

/* auth.c - Authentication command handlers */
void handle_register(conn_state_t *state, char *command);
void handle_login(conn_state_t *state, char *command);
void handle_logout(conn_state_t *state, char *command);

/* group.c - Group management command handlers */
void handle_create_group(conn_state_t *state, char *command);
void handle_join_group(conn_state_t *state, char *command);
void handle_approve(conn_state_t *state, char *command);
void handle_invite(conn_state_t *state, char *command);
void handle_accept(conn_state_t *state, char *command);
void handle_leave(conn_state_t *state, char *command);
void handle_kick(conn_state_t *state, char *command);
void handle_list_groups(conn_state_t *state, char *command);
void handle_list_members(conn_state_t *state, char *command);
void handle_list_requests(conn_state_t *state, char *command);

/* file_ops.c - File operation command handlers */
void handle_upload(conn_state_t *state, char *command);
void handle_download(conn_state_t *state, char *command);
void handle_rename_file(conn_state_t *state, char *command);
void handle_delete_file(conn_state_t *state, char *command);
void handle_copy_file(conn_state_t *state, char *command);
void handle_move_file(conn_state_t *state, char *command);

/* folder_ops.c - Folder operation command handlers */
void handle_mkdir(conn_state_t *state, char *command);
void handle_rename_folder(conn_state_t *state, char *command);
void handle_rmdir(conn_state_t *state, char *command);
void handle_copy_folder(conn_state_t *state, char *command);
void handle_move_folder(conn_state_t *state, char *command);
void handle_list_content(conn_state_t *state, char *command);

#endif /* COMMON_H */

