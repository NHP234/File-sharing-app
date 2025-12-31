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
        write_log_detailed(state->client_addr, command, "-ERR Already logged in");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "REGISTER %s %s", username, password) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }
    
    /* Check username and password length */
    if (strlen(username) == 0 || strlen(username) >= MAX_USERNAME ||
        strlen(password) == 0 || strlen(password) >= MAX_PASSWORD) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Invalid username or password length");
        return;
    }
    
    pthread_mutex_lock(&account_mutex);
    
    /* Check if username already exists */
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            pthread_mutex_unlock(&account_mutex);
            tcp_send(state->sockfd, "501");
            write_log_detailed(state->client_addr, command, "-ERR Username already exists");
            return;
        }
    }
    
    /* Check if account limit reached */
    if (account_count >= MAX_ACCOUNTS) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "504");
        write_log_detailed(state->client_addr, command, "-ERR Server full");
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
    
    tcp_send(state->sockfd, "120");
    
    /* Log the registration */
    write_log_detailed(state->client_addr, command, "+OK New user registered");
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
        write_log_detailed(state->client_addr, command, "-ERR Already logged in");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "LOGIN %s %s", username, password) != 2) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
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
        write_log_detailed(state->client_addr, command, "-ERR Account does not exist");
        return;
    }
    
    /* Check password */
    if (strcmp(accounts[found].password, password) != 0) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "401");
        write_log_detailed(state->client_addr, command, "-ERR Wrong password");
        return;
    }
    
    /* Check if already logged in on another client */
    if (accounts[found].is_logged_in) {
        pthread_mutex_unlock(&account_mutex);
        tcp_send(state->sockfd, "403");
        write_log_detailed(state->client_addr, command, "-ERR Already logged in on another client");
        return;
    }
    
    /* Login successful */
    accounts[found].is_logged_in = 1;
    strcpy(state->logged_user, username);
    state->is_logged_in = 1;
    state->user_group_id = accounts[found].group_id;
    
    pthread_mutex_unlock(&account_mutex);
    
    tcp_send(state->sockfd, "110");
    
    /* Log the login */
    write_log_detailed(state->client_addr, command, "+OK User logged in");
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
    /* Role-based access control */
    char *rbac_result = role_based_access_control("LOGOUT", state);
    if (rbac_result != NULL) {
        tcp_send(state->sockfd, rbac_result);
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
    
    printf("User logged out: %s\n", state->logged_user);
    
    tcp_send(state->sockfd, "130");
    
    /* Log the logout */
    write_log_detailed(state->client_addr, command, "+OK User logged out");
    
    /* Clear state */
    state->is_logged_in = 0;
    state->user_group_id = -1;
    state->logged_user[0] = '\0';
}

