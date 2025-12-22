#include "common.h"

/* ==================== FOLDER OPERATION COMMAND HANDLERS ==================== */

/**
 * @function handle_mkdir: Handle MKDIR command
 * @param state: Connection state
 * @param command: Command string "MKDIR <path>"
 * Response codes:
 *   220: Folder created successfully
 *   400: Not logged in
 *   404: Not in any group
 *   501: Folder already exists
 *   300: Syntax error
 **/
void handle_mkdir(conn_state_t *state, char *command) {
    // TODO: Implement mkdir
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_rename_folder: Handle RENAME_FOLDER command
 * @param state: Connection state
 * @param command: Command string "RENAME_FOLDER <old> <new>"
 * Response codes:
 *   221: Rename successful
 *   500: Folder does not exist
 *   501: New name already exists
 *   400: Not logged in
 *   404: Not in any group
 *   406: Not group leader
 *   300: Syntax error
 **/
void handle_rename_folder(conn_state_t *state, char *command) {
    // TODO: Implement rename folder
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_rmdir: Handle RMDIR command
 * @param state: Connection state
 * @param command: Command string "RMDIR <path>"
 * Response codes:
 *   222: Delete successful
 *   500: Folder does not exist
 *   400: Not logged in
 *   404: Not in any group
 *   406: Not group leader
 *   300: Syntax error
 **/
void handle_rmdir(conn_state_t *state, char *command) {
    // TODO: Implement rmdir
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_copy_folder: Handle COPY_FOLDER command
 * @param state: Connection state
 * @param command: Command string "COPY_FOLDER <src> <dest>"
 * Response codes:
 *   223: Copy successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: Source folder does not exist
 *   503: Invalid destination path
 *   300: Syntax error
 **/
void handle_copy_folder(conn_state_t *state, char *command) {
    // TODO: Implement copy folder
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_move_folder: Handle MOVE_FOLDER command
 * @param state: Connection state
 * @param command: Command string "MOVE_FOLDER <src> <dest>"
 * Response codes:
 *   224: Move successful
 *   400: Not logged in
 *   404: Not in any group
 *   500: Source folder does not exist
 *   503: Invalid destination path
 *   300: Syntax error
 **/
void handle_move_folder(conn_state_t *state, char *command) {
    // TODO: Implement move folder
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_list_content: Handle LIST_CONTENT command
 * @param state: Connection state
 * @param command: Command string "LIST_CONTENT <path>"
 * Response codes:
 *   225: List returned successfully
 *   400: Not logged in
 *   404: Not in any group
 *   500: Path does not exist
 *   300: Syntax error
 **/
void handle_list_content(conn_state_t *state, char *command) {
    // TODO: Implement list content
    tcp_send(state->sockfd, "300");
}

