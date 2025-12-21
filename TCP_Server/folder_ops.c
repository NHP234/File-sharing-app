#include "common.h"
#include <libgen.h>
#define STORAGE_ROOT "data/groups"

/**
 * @function resolve_path: Resolve user path to physical path
 * @param full_path: Buffer to store resolved path
 * @param group_id: Group ID
 * @param user_path: User-provided path
 **/
static void resolve_path(char *full_path, int group_id, const char *user_path) {
    char clean_path[MAX_PATH];

    if (user_path == NULL || strlen(user_path) == 0 || strcmp(user_path, "/") == 0) {
        strcpy(clean_path, "");
    } else {
        /* Prevent directory traversal */
        if (strstr(user_path, "..")) {
            strcpy(clean_path, "");
        } else if (user_path[0] == '/') {
            strcpy(clean_path, user_path + 1);
        } else {
            strcpy(clean_path, user_path);
        }
    }

    snprintf(full_path, MAX_PATH, "%s/group_%d/%s", STORAGE_ROOT, group_id, clean_path);

    // Remove trailing slash
    size_t len = strlen(full_path);
    if (len > 0 && full_path[len-1] == '/') {
        full_path[len-1] = '\0';
    }
}

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
    char path[MAX_PATH];
    sscanf(command, "MKDIR %s", path);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->user_group_id, path);

    struct stat st;
    if (stat(phys_path, &st) == 0) {
        tcp_send(state->sockfd, "501"); // Already exists
        return;
    }

    // Create directory
    if (mkdir(phys_path, 0777) == 0) {
        tcp_send(state->sockfd, "220"); // Success

        char log_msg[BUFF_SIZE];
        snprintf(log_msg, sizeof(log_msg), "",
                 state->logged_user, path, state->user_group_id);
        write_log(log_msg);
    } else {
        tcp_send(state->sockfd, "500"); // System error
    }
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
    char old_name[MAX_PATH], new_name[MAX_PATH];
    sscanf(command, "RENAME_FOLDER %s %s", old_name, new_name);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    if (!is_group_leader(state->logged_user, state->user_group_id)) {
        tcp_send(state->sockfd, "406");
        return;
    }

    char old_phys_path[MAX_PATH];
    char new_phys_path[MAX_PATH];
    char parent_dir[MAX_PATH];

    resolve_path(old_phys_path, state->user_group_id, old_name);

    // Extract parent directory
    strcpy(parent_dir, old_phys_path);
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
    }

    snprintf(new_phys_path, sizeof(new_phys_path), "%s/%s", parent_dir, new_name);

    // Check if new name already exists
    struct stat st;
    if (stat(new_phys_path, &st) == 0) {
        tcp_send(state->sockfd, "501"); // Name already exists
        return;
    }

    // Rename folder
    if (rename(old_phys_path, new_phys_path) == 0) {
        tcp_send(state->sockfd, "221"); // Success

        char log_msg[BUFF_SIZE];
        snprintf(log_msg, sizeof(log_msg), "",
                 state->logged_user, old_name, new_name);
        write_log(log_msg);
    } else {
        tcp_send(state->sockfd, "500"); // Old folder not found
    }
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
    char path[MAX_PATH];
    sscanf(command, "RMDIR %s", path);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    if (!is_group_leader(state->logged_user, state->user_group_id)) {
        tcp_send(state->sockfd, "406");
        return;
    }

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->user_group_id, path);

    // Remove directory (recursive)
    char cmd[MAX_PATH + 20];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", phys_path);

    if (system(cmd) == 0) {
        tcp_send(state->sockfd, "222"); // Success

        char log_msg[BUFF_SIZE];
        snprintf(log_msg, sizeof(log_msg), "",
                 state->logged_user, path);
        write_log(log_msg);
    } else {
        tcp_send(state->sockfd, "500"); // Folder not found
    }
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
    char src_path[MAX_PATH], dest_path[MAX_PATH];
    sscanf(command, "COPY_FOLDER %s %s", src_path, dest_path);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    char src_phys[MAX_PATH];
    char dest_phys[MAX_PATH];

    resolve_path(src_phys, state->user_group_id, src_path);
    resolve_path(dest_phys, state->user_group_id, dest_path);

    // Check if source exists
    struct stat st;
    if (stat(src_phys, &st) != 0 || !S_ISDIR(st.st_mode)) {
        tcp_send(state->sockfd, "500"); // Source not found
        return;
    }

    // Check if destination parent exists
    char dest_parent[MAX_PATH];
    strcpy(dest_parent, dest_phys);
    char *last_slash = strrchr(dest_parent, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (stat(dest_parent, &st) != 0) {
            tcp_send(state->sockfd, "503"); // Invalid destination
            return;
        }
    }

    // Copy folder recursively using cp command
    char cmd[MAX_PATH * 2 + 20];
    snprintf(cmd, sizeof(cmd), "cp -r \"%s\" \"%s\"", src_phys, dest_phys);

    if (system(cmd) == 0) {
        tcp_send(state->sockfd, "223"); // Success

        char log_msg[BUFF_SIZE];
        snprintf(log_msg, sizeof(log_msg), "",
                 state->logged_user, src_path, dest_path);
        write_log(log_msg);
    } else {
        tcp_send(state->sockfd, "500"); // Copy failed
    }
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
    char src_path[MAX_PATH], dest_dir[MAX_PATH];
    sscanf(command, "MOVE_FOLDER %s %s", src_path, dest_dir);

    // Check logged in
    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    // Check in group
    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    char src_phys[MAX_PATH];
    char dest_folder_phys[MAX_PATH];
    char final_dest_phys[MAX_PATH];

    resolve_path(src_phys, state->user_group_id, src_path);
    resolve_path(dest_folder_phys, state->user_group_id, dest_dir);

    // Check if destination folder exists
    struct stat st;
    if (stat(dest_folder_phys, &st) != 0 || !S_ISDIR(st.st_mode)) {
        tcp_send(state->sockfd, "503"); // Invalid destination
        return;
    }

    // Get basename of source
    char temp_src[MAX_PATH];
    strcpy(temp_src, src_path);
    char *foldername = strrchr(temp_src, '/');
    if (foldername) {
        foldername++;
    } else {
        foldername = temp_src;
    }

    // Target path = dest_dir + / + basename(src_path)
    snprintf(final_dest_phys, sizeof(final_dest_phys), "%s/%s", dest_folder_phys, foldername);

    // Move folder
    if (rename(src_phys, final_dest_phys) == 0) {
        tcp_send(state->sockfd, "224"); // Success

        char log_msg[BUFF_SIZE];
        snprintf(log_msg, sizeof(log_msg), "",
                 state->logged_user, src_path, dest_dir);
        write_log(log_msg);
    } else {
        tcp_send(state->sockfd, "500"); // Source not found
    }
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
    char path[MAX_PATH];

    // Parse command
    char *space = strchr(command, ' ');
    if (space) {
        sscanf(command, "LIST_CONTENT %s", path);
    } else {
        strcpy(path, "/");
    }

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "404");
        return;
    }

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->user_group_id, path);

    // Open directory
    DIR *d = opendir(phys_path);
    if (!d) {
        tcp_send(state->sockfd, "500"); // Path not found
        return;
    }

    // Build result list
    char result_buf[BUFF_SIZE];
    result_buf[0] = '\0';

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        // Prevent buffer overflow
        if (strlen(result_buf) + strlen(dir->d_name) + 10 >= BUFF_SIZE) {
            break;
        }

        strcat(result_buf, dir->d_name);

        // Add / for directories
        if (dir->d_type == DT_DIR) {
            strcat(result_buf, "/");
        }

        strcat(result_buf, "\n");
    }
    closedir(d);

    // Send response
    char response[BUFF_SIZE];
    if (strlen(result_buf) > 0) {
        snprintf(response, sizeof(response), "225\n%s", result_buf);
    } else {
        snprintf(response, sizeof(response), "225\n(empty)");
    }

    tcp_send(state->sockfd, response);

    char log_msg[BUFF_SIZE];
    snprintf(log_msg, sizeof(log_msg), "",
             state->logged_user, path);
    write_log(log_msg);
}

