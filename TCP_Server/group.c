#include "common.h"

/* ==================== GROUP MANAGEMENT COMMAND HANDLERS ==================== */

/**
 * @function handle_create_group: Handle CREATE command
 * @param state: Connection state
 * @param command: Command string "CREATE <group_name>"
 * Response codes:
 *   202: Group created successfully
 *   400: Not logged in
 *   407: User already in another group
 *   501: Group name already exists
 *   300: Syntax error
 **/
void handle_create_group(conn_state_t *state, char *command) {
    // TODO: Implement create group
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_join_group: Handle JOIN command
 * @param state: Connection state
 * @param command: Command string "JOIN <group_name>"
 * Response codes:
 *   160: Join request sent successfully
 *   400: Not logged in
 *   405: Already in this group
 *   500: Group does not exist
 *   300: Syntax error
 **/
void handle_join_group(conn_state_t *state, char *command) {
    // TODO: Implement join group
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_approve: Handle APPROVE command
 * @param state: Connection state
 * @param command: Command string "APPROVE <username>"
 * Response codes:
 *   170: Approval successful
 *   400: Not logged in
 *   406: Not group leader
 *   500: No request from this user
 *   300: Syntax error
 **/
void handle_approve(conn_state_t *state, char *command) {
    // TODO: Implement approve
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_invite: Handle INVITE command
 * @param state: Connection state
 * @param command: Command string "INVITE <username>"
 * Response codes:
 *   180: Invite sent successfully
 *   400: Not logged in
 *   406: Not group leader
 *   405: User already in this group
 *   300: Syntax error
 **/
void handle_invite(conn_state_t *state, char *command) {
    char username[MAX_USERNAME];
    sscanf(command, "INVITE %s", username);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "406");
        return;
    }

    pthread_mutex_lock(&group_mutex);
    int is_leader = is_group_leader(state->logged_user, state->user_group_id);
    pthread_mutex_unlock(&group_mutex);

    if (!is_leader) {
        tcp_send(state->sockfd, "406");
        return;
    }

    // Check if user exists and is not in group
    int user_found = 0;
    int already_in_group = 0;

    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            user_found = 1;
            if (accounts[i].group_id == state->user_group_id) {
                already_in_group = 1;
            }
            break;
        }
    }
    pthread_mutex_unlock(&account_mutex);

    if (!user_found) {
    	//TODO: Dinh nghia ma tra ve
        tcp_send(state->sockfd, "300");
        return;
    }

    if (already_in_group) {
        tcp_send(state->sockfd, "405");
        return;
    }

    // Add to invites
    pthread_mutex_lock(&invite_mutex);
    // Check if invite already exists
    int invite_exists = 0;
    for (int i = 0; i < invite_count; i++) {
        if (strcmp(invites[i].username, username) == 0 && invites[i].group_id == state->user_group_id) {
            invite_exists = 1;
            break;
        }
    }

    if (!invite_exists) {
        if (invite_count < MAX_INVITES) {
            strcpy(invites[invite_count].username, username);
            invites[invite_count].group_id = state->user_group_id;
            invite_count++;
            save_invites();
        } else {
             // TODO: Dinh nghia ma tra ve
             pthread_mutex_unlock(&invite_mutex);
             tcp_send(state->sockfd, "300"); // Server full
             return;
        }
    }
    pthread_mutex_unlock(&invite_mutex);

    tcp_send(state->sockfd, "180");
}

/**
 * @function handle_accept: Handle ACCEPT command
 * @param state: Connection state
 * @param command: Command string "ACCEPT <group_name>"
 * Response codes:
 *   190: Joined successfully
 *   400: Not logged in
 *   407: User already in another group
 *   300: Syntax error
 **/
void handle_accept(conn_state_t *state, char *command) {
    char group_name[MAX_GROUPNAME];
    sscanf(command, "ACCEPT %s", group_name);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id != -1) {
        tcp_send(state->sockfd, "407");
        return;
    }
    
    // Retrieve group id
    int group_id = -1;
    pthread_mutex_lock(&group_mutex);
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i].group_name, group_name) == 0) {
            group_id = groups[i].group_id;
            break;
        }
    }
    pthread_mutex_unlock(&group_mutex);

    // Group not exist
    if (group_id == -1) {
    	//TODO: Dinh nghia ma tra ve
        tcp_send(state->sockfd, "300");
        return;
    }

    int invite_index = -1;
    pthread_mutex_lock(&invite_mutex);
    for (int i = 0; i < invite_count; i++) {
        if (strcmp(invites[i].username, state->logged_user) == 0 && invites[i].group_id == group_id) {
            invite_index = i;
            break;
        }
    }

    // Invite not exist
    if (invite_index == -1) {
    	//TODO: Dinh nghia ma tra ve
        pthread_mutex_unlock(&invite_mutex);
        tcp_send(state->sockfd, "300");
        return;
    }

    // Remove invite
    for (int i = invite_index; i < invite_count - 1; i++) {
        invites[i] = invites[i+1];
    }
    invite_count--;
    save_invites();
    pthread_mutex_unlock(&invite_mutex);

    // Update user group
    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, state->logged_user) == 0) {
            accounts[i].group_id = group_id;
            state->user_group_id = group_id;
            break;
        }
    }
    save_accounts();
    pthread_mutex_unlock(&account_mutex);

    tcp_send(state->sockfd, "190");
}

/**
 * @function handle_leave: Handle LEAVE command
 * @param state: Connection state
 * @param command: Command string "LEAVE"
 * Response codes:
 *   200: Left group successfully
 *   400: Not logged in
 *   404: Not in any group
 *   408: Leader must remove all members first
 *   300: Syntax error
 **/
void handle_leave(conn_state_t *state, char *command) {
    // TODO: Implement leave
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_kick: Handle KICK command
 * @param state: Connection state
 * @param command: Command string "KICK <username>"
 * Response codes:
 *   201: Member removed successfully
 *   400: Not logged in
 *   406: Not group leader
 *   500: Member not in group
 *   300: Syntax error
 **/
void handle_kick(conn_state_t *state, char *command) {
    char username[MAX_USERNAME];
    sscanf(command, "KICK %s", username);

    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }

    if (state->user_group_id == -1) {
        tcp_send(state->sockfd, "406");
        return;
    }

    pthread_mutex_lock(&group_mutex);
    int is_leader = is_group_leader(state->logged_user, state->user_group_id);
    pthread_mutex_unlock(&group_mutex);

    if (!is_leader) {
        tcp_send(state->sockfd, "406");
        return;
    }

    int user_found_in_group = 0;
    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            if (accounts[i].group_id == state->user_group_id) {
                accounts[i].group_id = -1; // Remove user from group
                user_found_in_group = 1;
                save_accounts();
            }
            break;
        }
    }
    pthread_mutex_unlock(&account_mutex);

    if (!user_found_in_group) {
        tcp_send(state->sockfd, "500");
        return;
    }

    tcp_send(state->sockfd, "201");
}

/**
 * @function handle_list_groups: Handle LIST_GROUPS command
 * @param state: Connection state
 * @param command: Command string "LIST_GROUPS"
 * Response codes:
 *   203: List returned successfully
 *   400: Not logged in
 *   300: Syntax error
 **/
void handle_list_groups(conn_state_t *state, char *command) {
    // TODO: Implement list groups
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_list_members: Handle LIST_MEMBERS command
 * @param state: Connection state
 * @param command: Command string "LIST_MEMBERS"
 * Response codes:
 *   204: List returned successfully
 *   400: Not logged in
 *   404: Not in any group
 *   300: Syntax error
 **/
void handle_list_members(conn_state_t *state, char *command) {
    // TODO: Implement list members
    tcp_send(state->sockfd, "300");
}

/**
 * @function handle_list_requests: Handle LIST_REQUESTS command
 * @param state: Connection state
 * @param command: Command string "LIST_REQUESTS"
 * Response codes:
 *   205: List returned successfully
 *   400: Not logged in
 *   406: Not group leader
 *   300: Syntax error
 **/
void handle_list_requests(conn_state_t *state, char *command) {
    // TODO: Implement list requests
    tcp_send(state->sockfd, "300");
}
