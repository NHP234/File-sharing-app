#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>

#define BUFF_SIZE 4096
#define MAX_USERNAME 50
#define MAX_PATH 1024

/* Shared Connection State Structure */
typedef struct {
    char recv_buffer[BUFF_SIZE];
    int buffer_pos;
    int sockfd;
    char logged_user[MAX_USERNAME];
    int is_logged_in;
    int current_group_id;
    char client_addr[50];
} conn_state_t;

/**
 * @function load_groups_from_file: Load group info (id, name, admin) from group.txt
 * @param: None
 * @return: None
 **/
void load_groups_from_file();

/**
 * @function handle_mkdir: Create directory with permission checks
 * @param state: Client connection state
 * @param path: Relative path to create
 * @return: Protocol code (220, 400, 404, 501, 500)
 **/
int handle_mkdir(conn_state_t *state, char *path);

/**
 * @function handle_list_content: List directory content
 * @param state: Client connection state
 * @param path: Relative path (or NULL/empty for root)
 * @param result_buf: Buffer to store the listing string
 * @return: Protocol code (225, 400, 404, 500)
 **/
int handle_list_content(conn_state_t *state, char *path, char *result_buf);

/**
 * @function handle_rename: Rename file/folder (Admin only)
 * @param state: Client connection state
 * @param old_name: Relative path to item
 * @param new_name: New name (just name, not path)
 * @return: Protocol code (210, 400, 404, 406, 500, 501)
 **/
int handle_rename(conn_state_t *state, char *old_name, char *new_name);

/**
 * @function handle_remove: Remove file/folder (Admin only)
 * @param state: Client connection state
 * @param path: Relative path to remove
 * @return: Protocol code (211/222, 400, 404, 406, 500)
 **/
int handle_remove(conn_state_t *state, char *path);

/**
 * @function handle_move: Move file/folder (No rename allowed)
 * @param state: Client connection state
 * @param src_path: Source relative path
 * @param dest_dir: Destination relative folder
 * @return: Protocol code (213/224, 400, 404, 500, 503)
 **/
int handle_move(conn_state_t *state, char *src_path, char *dest_dir);

/**
 * @function handle_copy: Copy file
 * @param state: Client connection state
 * @param src_path: Source relative path
 * @param dest_path: Destination relative path
 * @return: Protocol code (212/223, 400, 404, 500, 503)
 **/
int handle_copy(conn_state_t *state, char *src_path, char *dest_path);

#endif

