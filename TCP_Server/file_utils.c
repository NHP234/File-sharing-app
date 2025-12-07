#include "file_utils.h"

#define STORAGE_ROOT "server_storage"
#define GROUP_FILE "group.txt"
#define MAX_GROUPS 100

typedef struct {
    int id;
    char name[50];
    char admin_username[MAX_USERNAME];
} group_t;

group_t groups[MAX_GROUPS];
int group_count = 0;

void resolve_path(char *full_path, int group_id, const char *user_path) {
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
    
    // Ensure storage root exists
    struct stat st = {0};
    if (stat(STORAGE_ROOT, &st) == -1) {
        mkdir(STORAGE_ROOT, 0777);
    }

    sprintf(full_path, "%s/group_%d/%s", STORAGE_ROOT, group_id, clean_path);
    
    // Remove trailing slash 
    size_t len = strlen(full_path);
    if (full_path[len-1] == '/') {
        full_path[len-1] = '\0';
    }
}

/**
 * @function is_admin: Check if user is admin of the current group
 */
int is_admin(int group_id, const char *username) {
    for (int i = 0; i < group_count; i++) {
        if (groups[i].id == group_id) {
            if (strcmp(groups[i].admin_username, username) == 0) {
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

/**
 * @function ensure_group_dir: Make sure group directory exists
 */
void ensure_group_dir(int group_id) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/group_%d", STORAGE_ROOT, group_id);
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0777);
    }
}


void load_groups_from_file() {
    FILE *f = fopen(GROUP_FILE, "r");
    if (!f) {
        perror("Group file not found");
        return;
    }
    
    group_count = 0;
    while (fscanf(f, "%d %s %s", &groups[group_count].id, 
                  groups[group_count].name, 
                  groups[group_count].admin_username) == 3) {
        group_count++;
        if (group_count >= MAX_GROUPS) break;
    }
    fclose(f);
    printf(">> Loaded %d groups from %s\n", group_count, GROUP_FILE);
}

int handle_mkdir(conn_state_t *state, char *path) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    ensure_group_dir(state->current_group_id);
    
    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->current_group_id, path);

    struct stat st;
    if (stat(phys_path, &st) == 0) {
        return 501; // Folder already exists
    }

    if (mkdir(phys_path, 0777) == 0) {
        return 220; // Success
    }
    return 500; // System error
}

int handle_list_content(conn_state_t *state, char *path, char *result_buf) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    ensure_group_dir(state->current_group_id);

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->current_group_id, path);

    DIR *d = opendir(phys_path);
    if (!d) return 500; // Path not found or error

    result_buf[0] = '\0';
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        
        // buffer overflow
        if (strlen(result_buf) + strlen(dir->d_name) + 5 >= BUFF_SIZE) break;

        strcat(result_buf, dir->d_name);
        if (dir->d_type == DT_DIR) {
            strcat(result_buf, "/");
        }
        strcat(result_buf, "\n");
    }
    closedir(d);
    return 225;
}

int handle_rename(conn_state_t *state, char *old_name, char *new_name) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    // Only Admin can rename
    if (!is_admin(state->current_group_id, state->logged_user)) {
        return 406; // Not group admin
    }

    char old_phys_path[MAX_PATH];
    char new_phys_path[MAX_PATH+1];
    char parent_dir[MAX_PATH];

    resolve_path(old_phys_path, state->current_group_id, old_name);
    
    // Extract parent directory to keep the file in the same folder
    strcpy(parent_dir, old_phys_path);
    dirname(parent_dir); // Modifies string to be parent

    snprintf(new_phys_path, sizeof(new_phys_path), "%s/%s", parent_dir, new_name);

    // Check if new name already exists
    struct stat st;
    if (stat(new_phys_path, &st) == 0) {
        return 501; // New name conflict
    }

    if (rename(old_phys_path, new_phys_path) == 0) {
        // Detect if it was a folder or file for correct return code
        // (Protocol says 210 for file, 221 for folder - let's return 210 generic or check type before)
        return 210; 
    }
    return 500; // Old file not found or system error
}

int handle_remove(conn_state_t *state, char *path) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    // Only Admin can remove
    if (!is_admin(state->current_group_id, state->logged_user)) {
        return 406; 
    }

    char phys_path[MAX_PATH];
    resolve_path(phys_path, state->current_group_id, path);
    
    char cmd[MAX_PATH + 10];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", phys_path);

    if (system(cmd) == 0) {
        return 211; // Success
    }
    
    return 500; 
}

int handle_move(conn_state_t *state, char *src_path, char *dest_dir) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    char src_phys[MAX_PATH];
    char dest_folder_phys[MAX_PATH];
    char final_dest_phys[MAX_PATH+2];

    resolve_path(src_phys, state->current_group_id, src_path);
    resolve_path(dest_folder_phys, state->current_group_id, dest_dir);

    // Check destination folder exists
    struct stat st;
    if (stat(dest_folder_phys, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return 503; // Invalid destination
    }

    // Target path = dest_dir + / + basename(src_path)
    char temp_src[MAX_PATH];
    strcpy(temp_src, src_path);
    char *filename = basename(temp_src);

    snprintf(final_dest_phys, sizeof(final_dest_phys), "%s/%s", dest_folder_phys, filename);

    if (rename(src_phys, final_dest_phys) == 0) {
        return 213;
    }
    return 500; // Source not found
}

int handle_copy(conn_state_t *state, char *src_path, char *dest_path) {
    if (!state->is_logged_in) return 400;
    if (state->current_group_id == -1) return 404;

    char src_phys[MAX_PATH];
    char dest_phys[MAX_PATH];

    resolve_path(src_phys, state->current_group_id, src_path);
    resolve_path(dest_phys, state->current_group_id, dest_path);

    FILE *in = fopen(src_phys, "rb");
    if (!in) return 500; // Source error

    FILE *out = fopen(dest_phys, "wb");
    if (!out) {
        fclose(in);
        return 503; // Dest error
    }

    char buf[4096];
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

    return success ? 212 : 500;
}
 

