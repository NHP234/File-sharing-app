#include "common.h"

/* ==================== FILE OPERATION COMMAND HANDLERS ==================== */

/**
 * @function handle_upload: Handle UPLOAD command
 * @param state: Connection state
 * @param command: Command string "UPLOAD <path> <size>"
 * Response codes:
 *   140: Upload successful
 *   400: Not logged in
 *   404: Not in any group
 *   502: File write error
 *   300: Syntax error
 **/
void handle_upload(conn_state_t *state, char *command) {
    // TODO: Implement upload
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_download: Handle DOWNLOAD command
 * @param state: Connection state
 * @param command: Command string "DOWNLOAD <path>"
 * Response codes:
 *   150: Download successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: File does not exist
 *   300: Syntax error
 **/
void handle_download(conn_state_t *state, char *command) {
    // TODO: Implement download
    tcp_send(state->sockfd, "300");
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

