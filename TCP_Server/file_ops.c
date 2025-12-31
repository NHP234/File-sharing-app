#include "common.h"
#include <fcntl.h>
#include <sys/file.h>
#include <libgen.h>
#include <sys/stat.h>

#define STORAGE_ROOT "groups"

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
        // Prevent directory traversal
        if (strstr(user_path, "..")) {
            strcpy(clean_path, "");
        } else if (user_path[0] == '/') {
            strcpy(clean_path, user_path + 1);
        } else {
            strcpy(clean_path, user_path);
        }
    }

    // Find group name from group_id
    char group_name[MAX_GROUPNAME] = "";
    for (int i = 0; i < group_count; i++) {
        if (groups[i].group_id == group_id) {
            strcpy(group_name, groups[i].group_name);
            break;
        }
    }

    snprintf(full_path, MAX_PATH, "%s/%s/%s", STORAGE_ROOT, group_name, clean_path);

    // Remove trailing slash
    size_t len = strlen(full_path);
    if (len > 0 && full_path[len-1] == '/') {
        full_path[len-1] = '\0';
    }
}

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
 
 *   300: Syntax error
 **/
void handle_upload(conn_state_t *state, char *command) {
    char filename[MAX_PATH];
    long long filesize;
    
    
    char *access_error = role_based_access_control("UPLOAD", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        return;
    }
    
    
    if (sscanf(command, "UPLOAD %s %lld", filename, &filesize) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }
    
    
    if (filesize <= 0) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Invalid file size");
        return;
    }
    
    
    char group_folder[MAX_PATH];
    get_group_folder_path(state->user_group_id, group_folder, sizeof(group_folder));
    
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", group_folder, filename);
    

    
    /* Send ready signal */
    tcp_send(state->sockfd, "141");
    
    
    int ret = receive_file_content(state->sockfd, state, filepath, filesize);
    
    if (ret == 0) {
        tcp_send(state->sockfd, "140");
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
    
    
    char *access_error = role_based_access_control("DOWNLOAD", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        return;
    }
    
    /* Parse command: DOWNLOAD <filename> */
    if (sscanf(command, "DOWNLOAD %s", filename) != 1) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
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
        write_log_detailed(state->client_addr, command, "-ERR File does not exist");
        return;
    }
    
    if (filesize == -2) {
        tcp_send(state->sockfd, "504");
        write_log_detailed(state->client_addr, command, "-ERR Cannot download folder");
        return;
    }
    
    /* Check if file is locked (being uploaded) */
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR Cannot access file");
        return;
    }
    
    /* Try to get shared lock (non-blocking) - will fail if file has exclusive lock */
    if (flock(fd, LOCK_SH | LOCK_NB) == -1) {
        close(fd);
        tcp_send(state->sockfd, "505");
        write_log_detailed(state->client_addr, command, "-ERR File is being uploaded");
        return;
    }
    
    /* Release the test lock */
    flock(fd, LOCK_UN);
    close(fd);
    
    /* Send file size */
    char msg[100];
    snprintf(msg, sizeof(msg), "151 %lld", filesize);
    tcp_send(state->sockfd, msg);
    
    /* Send file content with file lock */
    int ret = send_file_content(state->sockfd, filepath);
    
    if (ret == 0) {
        tcp_send(state->sockfd, "150");
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
    char old_name[MAX_PATH], new_name[MAX_PATH];

    // Check access control
    char *access_error = role_based_access_control("RENAME_FILE", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    // Parse command
    if (sscanf(command, "RENAME_FILE %s %s", old_name, new_name) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
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

    // Check if file is locked (being uploaded/downloaded)
    int fd = open(old_phys_path, O_RDWR);
    if (fd == -1) {
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR File not found");
        return;
    }
    
    // Try to get exclusive lock (non-blocking)
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        close(fd);
        tcp_send(state->sockfd, "505");
        write_log_detailed(state->client_addr, command, "-ERR File is being used");
        return;
    }

    // Check if new name already exists
    struct stat st;
    if (stat(new_phys_path, &st) == 0) {
        flock(fd, LOCK_UN);
        close(fd);
        tcp_send(state->sockfd, "501"); /* Name already exists */
        write_log_detailed(state->client_addr, command, "-ERR New file name already exists");
        return;
    }

    if (rename(old_phys_path, new_phys_path) == 0) {
        tcp_send(state->sockfd, "210");
        write_log_detailed(state->client_addr, command, "+OK File renamed successfully");
    } else {
        tcp_send(state->sockfd, "500"); // file not found
        write_log_detailed(state->client_addr, command, "-ERR File not found");
    }

    // Release lock after operation completes
    flock(fd, LOCK_UN);
    close(fd);
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
    char path[MAX_PATH];

    // Check access control
    char *access_error = role_based_access_control("DELETE_FILE", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    // Parse command
    if (sscanf(command, "DELETE_FILE %s", path) != 1) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->user_group_id, path);

    // Check if file is locked (being uploaded/downloaded)
    int fd = open(phys_path, O_RDWR);
    if (fd == -1) {
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR File not found");
        return;
    }
    
    // Try to get exclusive lock (non-blocking)
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        close(fd);
        tcp_send(state->sockfd, "505");
        write_log_detailed(state->client_addr, command, "-ERR File is being used");
        return;
    }

    if (unlink(phys_path) == 0) {
        tcp_send(state->sockfd, "211");
        write_log_detailed(state->client_addr, command, "+OK File deleted successfully");
    } else {
        tcp_send(state->sockfd, "500"); /* File not found */
        write_log_detailed(state->client_addr, command, "-ERR File not found");
    }

    // Release lock after operation completes
    flock(fd, LOCK_UN);
    close(fd);
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
    char src_path[MAX_PATH], dest_path[MAX_PATH];

    // Check access control
    char *access_error = role_based_access_control("COPY_FILE", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    // Parse command
    if (sscanf(command, "COPY_FILE %s %s", src_path, dest_path) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }

    char src_phys[MAX_PATH];
    char dest_phys[MAX_PATH];

    resolve_path(src_phys, state->user_group_id, src_path);
    resolve_path(dest_phys, state->user_group_id, dest_path);

    FILE *in = fopen(src_phys, "rb");
    if (!in) {
        tcp_send(state->sockfd, "500"); // Source not found
        write_log_detailed(state->client_addr, command, "-ERR Source file not found");
        return;
    }

    // Lock source file
    if (file_lock(fileno(in), LOCK_SH) == -1) {
        fclose(in);
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR Cannot lock source file");
        return;
    }

    // Open destination file
    FILE *out = fopen(dest_phys, "wb");
    if (!out) {
        fclose(in);
        tcp_send(state->sockfd, "503"); // Invalid destination
        write_log_detailed(state->client_addr, command, "-ERR Invalid destination path");
        return;
    }

    // Lock destination file for writing
    if (file_lock(fileno(out), LOCK_EX) == -1) {
        fclose(out);
        fclose(in);
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR Cannot lock destination file");
        return;
    }

    //Copy file
    char buf[CHUNK_SIZE];
    size_t n;
    int success = 1;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            success = 0;
            break;
        }
    }

    fclose(in);
    fclose(out);

    if (success) {
        tcp_send(state->sockfd, "212");
        write_log_detailed(state->client_addr, command, "+OK File copied successfully");
    } else {
        tcp_send(state->sockfd, "500"); // Copy failed
        write_log_detailed(state->client_addr, command, "-ERR Copy operation failed");
    }
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
    char src_path[MAX_PATH], dest_dir[MAX_PATH];

    // Check access control
    char *access_error = role_based_access_control("MOVE_FILE", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    // Parse command
    if (sscanf(command, "MOVE_FILE %s %s", src_path, dest_dir) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }

    char src_phys[MAX_PATH];
    char dest_folder_phys[MAX_PATH];
    char final_dest_phys[MAX_PATH];

    resolve_path(src_phys, state->user_group_id, src_path);
    resolve_path(dest_folder_phys, state->user_group_id, dest_dir);

    // Check if file is locked (being uploaded/downloaded)
    int fd = open(src_phys, O_RDWR);
    if (fd == -1) {
        tcp_send(state->sockfd, "500");
        write_log_detailed(state->client_addr, command, "-ERR File not found");
        return;
    }
    
    // Try to get exclusive lock (non-blocking)
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        close(fd);
        tcp_send(state->sockfd, "505");
        write_log_detailed(state->client_addr, command, "-ERR File is being used");
        return;
    }

    // Check if destination folder exists
    struct stat st;
    if (stat(dest_folder_phys, &st) != 0 || !S_ISDIR(st.st_mode)) {
        flock(fd, LOCK_UN);
        close(fd);
        tcp_send(state->sockfd, "503");
        write_log_detailed(state->client_addr, command, "-ERR Invalid destination path");
        return;
    }

    // Get basename of source file
    char temp_src[MAX_PATH];
    strcpy(temp_src, src_path);
    char *filename = basename(temp_src);

    // Target path = dest_dir + / + basename(src_path)
    snprintf(final_dest_phys, sizeof(final_dest_phys), "%s/%s", dest_folder_phys, filename);

    // Move file
    if (rename(src_phys, final_dest_phys) == 0) {
        tcp_send(state->sockfd, "213");
        write_log_detailed(state->client_addr, command, "+OK File moved successfully");
    } else {
        tcp_send(state->sockfd, "500"); // Source not found
        write_log_detailed(state->client_addr, command, "-ERR Source file not found");
    }

    // Release lock after operation completes
    flock(fd, LOCK_UN);
    close(fd);
}

