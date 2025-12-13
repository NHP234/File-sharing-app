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
 *   300: Syntax error
 **/
void handle_register(conn_state_t *state, char *command) {
    // TODO: Implement registration
    tcp_send(state->sockfd, "300");
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
    // TODO: Implement login
    tcp_send(state->sockfd, "300");
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
    // TODO: Implement logout
    tcp_send(state->sockfd, "300");
}

