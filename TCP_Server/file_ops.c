#include "common.h"

/* ==================== FILE OPERATION COMMAND HANDLERS ==================== */

/**
 * @function handle_upload: Handle UPLOAD command
 * @param state: Connection state
 * @param command: Command string "UPLOAD <path> <size>"
 * Response codes:
 *   141: Ready to receive file
 *   140: Upload successful
 *   400: Not logged in
 *   404: Not in any group
 *   502: File write error
 *   503: Filename conflicts with folder
 *   300: Syntax error
 **/
void handle_upload(conn_state_t *state, char *command) {
    char filename[MAX_PATH];
    long long filesize;
    
    /* Check access control */
    char *access_error = role_based_access_control("UPLOAD", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        return;
    }
    
    /* Parse command: UPLOAD <filename> <filesize> */
    if (sscanf(command, "UPLOAD %s %lld", filename, &filesize) != 2) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Validate filesize */
    if (filesize <= 0) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Build filepath in group folder */
    char group_folder[MAX_PATH];
    get_group_folder_path(state->user_group_id, group_folder, sizeof(group_folder));
    
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", group_folder, filename);
    
    /* Check if name conflicts with existing directory */
    struct stat st;
    if (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode)) {
        tcp_send(state->sockfd, "503");
        write_log_detailed(state->client_addr, command, "-ERR Filename conflicts with folder");
        return;
    }
    
    /* Send ready signal */
    tcp_send(state->sockfd, "141");
    
    /* Receive file content */
    pthread_mutex_lock(&file_mutex);
    int ret = receive_file_content(state->sockfd, state, filepath, filesize);
    pthread_mutex_unlock(&file_mutex);
    
    if (ret == 0) {
        tcp_send(state->sockfd, "140");
        
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "User %s uploaded: %s (%lld bytes)", 
                 state->logged_user, filename, filesize);
        write_log(log_msg);
        write_log_detailed(state->client_addr, command, "+OK Successful upload");
        
        printf("Upload complete: %s by %s\n", filename, state->logged_user);
    } else if (ret == -1) {
        tcp_send(state->sockfd, "502");
        write_log_detailed(state->client_addr, command, "-ERR File write error");
    } else {
        write_log_detailed(state->client_addr, command, "-ERR Connection lost");
    }
}

/**
 * @function handle_download: Handle DOWNLOAD command
 * @param state: Connection state
 * @param command: Command string "DOWNLOAD <path>"
 * Response codes:
 *   151 <size>: Ready to send file
 *   150: Download successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: File does not exist
 *   504: Cannot download folder
 *   300: Syntax error
 **/
void handle_download(conn_state_t *state, char *command) {
    char filename[MAX_PATH];
    
    /* Check access control */
    char *access_error = role_based_access_control("DOWNLOAD", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        return;
    }
    
    /* Parse command: DOWNLOAD <filename> */
    if (sscanf(command, "DOWNLOAD %s", filename) != 1) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Build filepath in group folder */
    char group_folder[MAX_PATH];
    get_group_folder_path(state->user_group_id, group_folder, sizeof(group_folder));
    
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", group_folder, filename);
    
    /* Get file size */
    long long filesize = get_file_size(filepath);
    
    if (filesize == -1) {
        tcp_send(state->sockfd, "500");
        return;
    }
    
    if (filesize == -2) {
        tcp_send(state->sockfd, "504");
        return;
    }
    
    if (filesize == -3) {
        tcp_send(state->sockfd, "500");
        return;
    }
    
    /* Send file size */
    char msg[100];
    snprintf(msg, sizeof(msg), "151 %lld", filesize);
    tcp_send(state->sockfd, msg);
    
    /* Send file content */
    pthread_mutex_lock(&file_mutex);
    int ret = send_file_content(state->sockfd, filepath);
    pthread_mutex_unlock(&file_mutex);
    
    if (ret == 0) {
        tcp_send(state->sockfd, "150");
        
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "User %s downloaded: %s (%lld bytes)", 
                 state->logged_user, filename, filesize);
        write_log(log_msg);
        write_log_detailed(state->client_addr, command, "+OK Successful download");
        
        printf("Download complete: %s by %s\n", filename, state->logged_user);
    } else {
        write_log_detailed(state->client_addr, command, "-ERR Download failed");
    }
}

/**
 * @function handle_rename_file: Handle RENAME_FILE command
 * @param state: Connection state
 * @param command: Command string "RENAME_FILE <old> <new>"
 * Response codes:
 *   210: Rename successful
 *   500: File does not exist
 *   501: New name already exists
 *   400: Not logged in
 *   404: Not in any group
 *   406: Not group leader
 *   300: Syntax error
 **/
void handle_rename_file(conn_state_t *state, char *command) {
    // TODO: Implement rename file
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_delete_file: Handle DELETE_FILE command
 * @param state: Connection state
 * @param command: Command string "DELETE_FILE <path>"
 * Response codes:
 *   211: Delete successful
 *   500: File does not exist
 *   400: Not logged in
 *   404: Not in any group
 *   406: Not group leader
 *   300: Syntax error
 **/
void handle_delete_file(conn_state_t *state, char *command) {
    // TODO: Implement delete file
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_copy_file: Handle COPY_FILE command
 * @param state: Connection state
 * @param command: Command string "COPY_FILE <src> <dest>"
 * Response codes:
 *   212: Copy successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: Source file does not exist
 *   503: Invalid destination path
 *   300: Syntax error
 **/
void handle_copy_file(conn_state_t *state, char *command) {
    // TODO: Implement copy file
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_move_file: Handle MOVE_FILE command
 * @param state: Connection state
 * @param command: Command string "MOVE_FILE <src> <dest>"
 * Response codes:
 *   213: Move successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: Source file does not exist
 *   503: Invalid destination path
 *   300: Syntax error
 **/
void handle_move_file(conn_state_t *state, char *command) {
    // TODO: Implement move file
    tcp_send(state->sockfd, "300");
}

