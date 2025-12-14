#include "common.h"

/* ==================== AUTHENTICATION COMMAND HANDLERS ==================== */

/**
 * @function handle_register: Handle REGISTER command
 * @param state: Connection state
 * @param command: Command string "REGISTER <username> <password>"
 * Response codes:
 *   120: Registration successful
 *   501: Username already exists
 *   403: Already logged in
 *   504: Internal server error
 *   300: Syntax error
 **/
void handle_register(conn_state_t *state, char *command) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    
    /* Check if already logged in */
    if (state->is_logged_in) {
        tcp_send(state->sockfd, "403");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "REGISTER %s %s", username, password) != 2) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Check username and password length */
    if (strlen(username) == 0 || strlen(username) >= MAX_USERNAME ||
        strlen(password) == 0 || strlen(password) >= MAX_PASSWORD) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    pthread_mutex_lock(&account_mutex);
    
    /* Check if username already exists */
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "501");
            return;
        }
    }
    
    /* Check if account limit reached */
    if (account_count >= MAX_ACCOUNTS) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "504");
        return;
    }
    
    /* Create new account */
    strcpy(accounts[account_count].username, username);
    strcpy(accounts[account_count].password, password);
    accounts[account_count].group_id = -1;  /* Not in any group */
    accounts[account_count].is_logged_in = 0;
    account_count++;
    
    /* Save to file */
    save_accounts();
    
    pthread_mutex_unlock(&account_mutex);
    
    /* Log the registration */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "New user registered: %s", username);
    write_log(log_msg);
    
    tcp_send(state->sockfd, "120");
    printf("New user registered: %s\n", username);
}

/**
 * @function handle_login: Handle LOGIN command
 * @param state: Connection state
 * @param command: Command string "LOGIN <username> <password>"
 * Response codes:
 *   110: Login successful
 *   401: Wrong username or password
 *   402: Account does not exist
 *   403: Already logged in
 *   300: Syntax error
 **/
void handle_login(conn_state_t *state, char *command) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    
    /* Check if already logged in in this session */
    if (state->is_logged_in) {
        tcp_send(state->sockfd, "403");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "LOGIN %s %s", username, password) != 2) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    pthread_mutex_lock(&account_mutex);
    
    /* Find account */
    int found = -1;
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            found = i;
            break;
        }
    }
    
    /* Account does not exist */
    if (found == -1) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "402");
        return;
    }
    
    /* Check password */
    if (strcmp(accounts[found].password, password) != 0) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "401");
        return;
    }
    
    /* Check if already logged in on another client */
    if (accounts[found].is_logged_in) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "403");
        return;
    }
    
    /* Login successful */
    accounts[found].is_logged_in = 1;
    strcpy(state->logged_user, username);
    state->is_logged_in = 1;
    state->user_group_id = accounts[found].group_id;
    
    pthread_mutex_unlock(&account_mutex);
    
    /* Log the login */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User logged in: %s", username);
    write_log(log_msg);
    
    tcp_send(state->sockfd, "110");
    printf("User logged in: %s\n", username);
}

/**
 * @function handle_logout: Handle LOGOUT command
 * @param state: Connection state
 * @param command: Command string "LOGOUT"
 * Response codes:
 *   130: Logout successful
 *   400: Not logged in
 *   300: Syntax error
 **/
void handle_logout(conn_state_t *state, char *command) {
    /* Parse command */
    if (sscanf(command, "LOGOUT") != 0) {
        tcp_send(state->sockfd, "300");
        return;
    }

    /* Check if logged in */
    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }
    
    pthread_mutex_lock(&account_mutex);
    
    /* Find account and mark as logged out */
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, state->logged_user) == 0) {
            accounts[i].is_logged_in = 0;
            break;
        }
    }
    
    pthread_mutex_unlock(&account_mutex);
    
    /* Log the logout */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User logged out: %s", state->logged_user);
    write_log(log_msg);
    
    printf("User logged out: %s\n", state->logged_user);
    
    /* Clear state */
    state->is_logged_in = 0;
    state->user_group_id = -1;
    state->logged_user[0] = '\0';
    
    tcp_send(state->sockfd, "130");
}

