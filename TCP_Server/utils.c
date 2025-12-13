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
    FILE *f = fopen("TCP_Server/data/accounts.txt", "r");
    if (f == NULL) {
        perror("Cannot open TCP_Server/data/accounts.txt");
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
    FILE *f = fopen("TCP_Server/data/groups.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("TCP_Server/data/groups.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/requests.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("TCP_Server/data/requests.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/invites.txt", "r");
    if (f == NULL) {
        /* Create file if not exists */
        f = fopen("TCP_Server/data/invites.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/accounts.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/groups.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/requests.txt", "w");
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
    FILE *f = fopen("TCP_Server/data/invites.txt", "w");
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

/* ==================== UTILITY FUNCTIONS ==================== */

/**
 * @function write_log: Write a log message to server_log.txt
 * @param message: Log message to write
 * @return: None
 **/
void write_log(const char *message) {
    FILE *f = fopen("TCP_Server/logs/server_log.txt", "a");
    if (f == NULL) return;
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; /* Remove newline */
    
    fprintf(f, "[%s] %s\n", time_str, message);
    fclose(f);
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
            snprintf(buffer, buf_size, "TCP_Server/groups/%s", groups[i].group_name);
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

