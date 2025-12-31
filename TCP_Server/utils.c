#include "common.h"

/* ==================== GLOBAL VARIABLES DEFINITION ==================== */

account_t accounts[MAX_ACCOUNTS];
int account_count = 0;
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;

group_t groups[MAX_GROUPS];
int group_count = 0;
pthread_mutex_t group_mutex = PTHREAD_MUTEX_INITIALIZER;

request_t requests[MAX_REQUESTS];
int request_count = 0;
pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;

invite_t invites[MAX_INVITES];
int invite_count = 0;
pthread_mutex_t invite_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ==================== DATA LOADING/SAVING FUNCTIONS ==================== */

/**
 * @function load_accounts: Load user accounts from file into memory
 * @return: None
 **/
void load_accounts() {
    FILE *f = fopen("data/accounts.txt", "r");
    if (f == NULL) {
        perror("Cannot open data/accounts.txt");
        exit(1);
    }
    
    account_count = 0;
    while (fscanf(f, "%s %s %d", 
                  accounts[account_count].username,
                  accounts[account_count].password,
                  &accounts[account_count].group_id) == 3) {
        accounts[account_count].is_logged_in = 0;
        account_count++;
        if (account_count >= MAX_ACCOUNTS) break;
    }
    
    fclose(f);
    printf("Loaded %d accounts\n", account_count);
}

/**
 * @function load_groups: Load groups from file into memory
 * @return: None
 **/
void load_groups() {
    FILE *f = fopen("data/groups.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("data/groups.txt", "w");
        if (f) fclose(f);
        group_count = 0;
        printf("Created new groups.txt\n");
        return;
    }
    
    group_count = 0;
    while (fscanf(f, "%d %s %s",
                  &groups[group_count].group_id,
                  groups[group_count].group_name,
                  groups[group_count].leader) == 3) {
        group_count++;
        if (group_count >= MAX_GROUPS) break;
    }
    
    fclose(f);
    printf("Loaded %d groups\n", group_count);
}

/**
 * @function load_requests: Load join requests from file into memory
 * @return: None
 **/
void load_requests() {
    FILE *f = fopen("data/requests.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("data/requests.txt", "w");
        if (f) fclose(f);
        request_count = 0;
        printf("Created new requests.txt\n");
        return;
    }
    
    request_count = 0;
    while (fscanf(f, "%s %d",
                  requests[request_count].username,
                  &requests[request_count].group_id) == 2) {
        request_count++;
        if (request_count >= MAX_REQUESTS) break;
    }
    
    fclose(f);
    printf("Loaded %d requests\n", request_count);
}

/**
 * @function load_invites: Load invites from file into memory
 * @return: None
 **/
void load_invites() {
    FILE *f = fopen("data/invites.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("data/invites.txt", "w");
        if (f) fclose(f);
        invite_count = 0;
        printf("Created new invites.txt\n");
        return;
    }
    
    invite_count = 0;
    while (fscanf(f, "%s %d",
                  invites[invite_count].username,
                  &invites[invite_count].group_id) == 2) {
        invite_count++;
        if (invite_count >= MAX_INVITES) break;
    }
    
    fclose(f);
    printf("Loaded %d invites\n", invite_count);
}

/**
 * @function save_accounts: Save accounts from memory to file
 * @return: None
 **/
void save_accounts() {
    FILE *f = fopen("data/accounts.txt", "w");
    if (f == NULL) {
        perror("Cannot write to accounts.txt");
        return;
    }
    
    for (int i = 0; i < account_count; i++) {
        fprintf(f, "%s %s %d\n",
                accounts[i].username,
                accounts[i].password,
                accounts[i].group_id);
    }
    
    fclose(f);
}

/**
 * @function save_groups: Save groups from memory to file
 * @return: None
 **/
void save_groups() {
    FILE *f = fopen("data/groups.txt", "w");
    if (f == NULL) {
        perror("Cannot write to groups.txt");
        return;
    }
    
    for (int i = 0; i < group_count; i++) {
        fprintf(f, "%d %s %s\n",
                groups[i].group_id,
                groups[i].group_name,
                groups[i].leader);
    }
    
    fclose(f);
}

/**
 * @function save_requests: Save requests from memory to file
 * @return: None
 **/
void save_requests() {
    FILE *f = fopen("data/requests.txt", "w");
    if (f == NULL) {
        perror("Cannot write to requests.txt");
        return;
    }
    
    for (int i = 0; i < request_count; i++) {
        fprintf(f, "%s %d\n",
                requests[i].username,
                requests[i].group_id);
    }
    
    fclose(f);
}

/**
 * @function save_invites: Save invites from memory to file
 * @return: None
 **/
void save_invites() {
    FILE *f = fopen("data/invites.txt", "w");
    if (f == NULL) {
        perror("Cannot write to invites.txt");
        return;
    }
    
    for (int i = 0; i < invite_count; i++) {
        fprintf(f, "%s %d\n",
                invites[i].username,
                invites[i].group_id);
    }
    
    fclose(f);
}

/**
 * @function get_log_filename: Generate log filename based on current date
 * @param filename: Buffer to store the filename
 * @param size: Size of the buffer
 **/
void get_log_filename(char *filename, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(filename, size, "logs/log_%04d%02d%02d.txt", 
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

/**
 * @function write_log_detailed: Write detailed log entry to daily log file
 * @param client_addr: Client address in format IP:Port
 * @param request: Request received from client
 * @param result: Result/response sent to client
 **/
void write_log_detailed(const char *client_addr, const char *request, const char *result) {
    /* Ensure log directory exists */
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0755);
    }
    
    /* Get current time */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    /* Get log filename */
    char log_filename[256];
    get_log_filename(log_filename, sizeof(log_filename));
    
    /* Format timestamp */
    char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "[%02d/%02d/%04d %02d:%02d:%02d]",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE *f = fopen(log_filename, "a");
    if (f != NULL) {
        if (request == NULL || strlen(request) == 0) {
            fprintf(f, "%s$%s$%s\n", timestamp, client_addr, result);
        } else {
            fprintf(f, "%s$%s$%s\\r\\n$%s\n", timestamp, client_addr, request, result);
        }
        fclose(f);
    }
}

/**
 * @function get_next_group_id: Get the next available group ID
 * @return: Next group ID
 **/
int get_next_group_id() {
    int max_id = 0;
    for (int i = 0; i < group_count; i++) {
        if (groups[i].group_id > max_id) {
            max_id = groups[i].group_id;
        }
    }
    return max_id + 1;
}

/**
 * @function get_group_folder_path: Get the folder path for a group
 * @param group_id: Group ID
 * @param buffer: Buffer to store the path
 * @param buf_size: Size of the buffer
 * @return: Pointer to buffer
 **/
char* get_group_folder_path(int group_id, char *buffer, int buf_size) {
    /* Find group name */
    for (int i = 0; i < group_count; i++) {
        if (groups[i].group_id == group_id) {
            snprintf(buffer, buf_size, "groups/%s", groups[i].group_name);
            return buffer;
        }
    }
    buffer[0] = '\0';
    return buffer;
}

/**
 * @function is_group_leader: Check if a user is the leader of a group
 * @param username: Username to check
 * @param group_id: Group ID
 * @return: 1 if leader, 0 otherwise
 **/
int is_group_leader(const char *username, int group_id) {
    for (int i = 0; i < group_count; i++) {
        if (groups[i].group_id == group_id) {
            return strcmp(groups[i].leader, username) == 0;
        }
    }
    return 0;
}

/**
 * @function count_group_members: Count number of members in a group
 * @param group_id: Group ID
 * @return: Number of members
 **/
int count_group_members(int group_id) {
    int count = 0;
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].group_id == group_id) {
            count++;
        }
    }
    return count;
}

/**
 * @function sync_user_group_id: Sync user's group_id from accounts to state
 * @param state: Connection state
 * @note: Called automatically by RBAC to refresh group membership
 **/
void sync_user_group_id(conn_state_t *state) {
    if (!state->is_logged_in) {
        return;
    }
    
    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, state->logged_user) == 0) {
            state->user_group_id = accounts[i].group_id;
            break;
        }
    }
    pthread_mutex_unlock(&account_mutex);
}

/**
 * @function role_based_access_control: Check if user has permission to execute command
 * @param command: Command string (first word only, e.g., "UPLOAD", "DOWNLOAD")
 * @param state: Connection state of the client
 * @return: NULL if allowed, error code string ("400", "404", "406") if not allowed
 **/
char* role_based_access_control(const char *command, conn_state_t *state) {
    /* Don't require login */
    if (strcmp(command, "LOGIN") == 0 || strcmp(command, "REGISTER") == 0) {
        return NULL;
    }
    
    /* Require login */
    if (!state->is_logged_in) {
        return "400";
    }
    
    /* Sync group_id from accounts to prevent stale state */
    sync_user_group_id(state);
    
    /* Only require login */
    if (strcmp(command, "JOIN") == 0 ||
        strcmp(command, "ACCEPT") == 0 ||
        strcmp(command, "CREATE") == 0 ||
        strcmp(command, "LIST_GROUPS") == 0 ||
        strcmp(command, "LOGOUT") == 0) {
        return NULL;
    }
    
    /* Require login + being in a group */
    if (strcmp(command, "UPLOAD") == 0 ||
        strcmp(command, "DOWNLOAD") == 0 ||
        strcmp(command, "LEAVE") == 0 ||
        strcmp(command, "LIST_MEMBERS") == 0 ||
        strcmp(command, "COPY_FILE") == 0 ||
        strcmp(command, "MOVE_FILE") == 0 ||
        strcmp(command, "MKDIR") == 0 ||
        strcmp(command, "COPY_FOLDER") == 0 ||
        strcmp(command, "MOVE_FOLDER") == 0 ||
        strcmp(command, "LIST_CONTENT") == 0) {
        
        if (state->user_group_id == -1) {
            return "404";
        }
        return NULL;
    }
    
    /* Require login + being in a group + being group leader */
    if (strcmp(command, "APPROVE") == 0 ||
        strcmp(command, "INVITE") == 0 ||
        strcmp(command, "KICK") == 0 ||
        strcmp(command, "LIST_REQUESTS") == 0 ||
        strcmp(command, "RENAME_FILE") == 0 ||
        strcmp(command, "DELETE_FILE") == 0 ||
        strcmp(command, "RENAME_FOLDER") == 0 ||
        strcmp(command, "RMDIR") == 0) {
        
        if (state->user_group_id == -1) {
            return "404";
        }
        if (!is_group_leader(state->logged_user, state->user_group_id)) {
            return "406";
        }
        return NULL;
    }
    
    /* Unknown command - allow (will be handled by command processor) */
    return NULL;
}

