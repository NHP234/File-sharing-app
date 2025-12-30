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
    char group_name[MAX_GROUPNAME];
    
    /* Check if logged in */
    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "CREATE %s", group_name) != 1) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Validate group name */
    if (strlen(group_name) == 0 || strlen(group_name) >= MAX_GROUPNAME) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Check if user already in a group */
    if (state->user_group_id != -1) {
        tcp_send(state->sockfd, "407");
        return;
    }
    
    pthread_mutex_lock(&group_mutex);
    
    /* Check if group name already exists */
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i].group_name, group_name) == 0) {
            pthread_mutex_unlock(&group_mutex);
            tcp_send(state->sockfd, "501");
            return;
        }
    }
    
    /* Check if group limit reached */
    if (group_count >= MAX_GROUPS) {
        pthread_mutex_unlock(&group_mutex);
        tcp_send(state->sockfd, "504");
        return;
    }
    
    /* Get next group ID */
    int new_group_id = get_next_group_id();
    
    /* Create new group */
    groups[group_count].group_id = new_group_id;
    strcpy(groups[group_count].group_name, group_name);
    strcpy(groups[group_count].leader, state->logged_user);
    group_count++;
    
    /* Save groups to file */
    save_groups();
    
    pthread_mutex_unlock(&group_mutex);
    
    /* Update user's group_id in accounts */
    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, state->logged_user) == 0) {
            accounts[i].group_id = new_group_id;
            state->user_group_id = new_group_id;
            break;
        }
    }
    save_accounts();
    pthread_mutex_unlock(&account_mutex);
    
    /* Create group folder */
    char folder_path[MAX_PATH];
    snprintf(folder_path, sizeof(folder_path), "groups/%s", group_name);
    mkdir(folder_path, 0755);
    
    /* Log the group creation */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Group created: %s by %s", group_name, state->logged_user);
    write_log(log_msg);
    
    tcp_send(state->sockfd, "202");
    printf("Group created: %s by %s (ID: %d)\n", group_name, state->logged_user, new_group_id);
}

/**
 * @function handle_join_group: Handle JOIN command
 * @param state: Connection state
 * @param command: Command string "JOIN <group_name>"
 * Response codes:
 *   160: Join request sent successfully
 *   400: Not logged in
 *   407: User already in a group
 *   500: Group does not exist
 *   504: Request list full
 *   300: Syntax error
 **/
void handle_join_group(conn_state_t *state, char *command) {
    char group_name[MAX_GROUPNAME];
    
    /* Check if logged in */
    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }
    
    /* Parse command */
    if (sscanf(command, "JOIN %s", group_name) != 1) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Check if user already in a group */
    if (state->user_group_id != -1) {
        tcp_send(state->sockfd, "407");
        return;
    }
    
    pthread_mutex_lock(&group_mutex);
    
    /* Find group by name */
    int target_group_id = -1;
    for (int i = 0; i < group_count; i++) {
        if (strcmp(groups[i].group_name, group_name) == 0) {
            target_group_id = groups[i].group_id;
            break;
        }
    }
    
    /* Group does not exist */
    if (target_group_id == -1) {
        pthread_mutex_unlock(&group_mutex);
        tcp_send(state->sockfd, "500");
        return;
    }
    
    pthread_mutex_unlock(&group_mutex);
    
    pthread_mutex_lock(&request_mutex);
    
    /* Check if request already exists */
    for (int i = 0; i < request_count; i++) {
        if (strcmp(requests[i].username, state->logged_user) == 0 &&
            requests[i].group_id == target_group_id) {
            pthread_mutex_unlock(&request_mutex);
            tcp_send(state->sockfd, "160");  /* Already sent, but return success */
            return;
        }
    }
    
    /* Check if request limit reached */
    if (request_count >= MAX_REQUESTS) {
        pthread_mutex_unlock(&request_mutex);
        tcp_send(state->sockfd, "504");
        return;
    }
    
    /* Add join request */
    strcpy(requests[request_count].username, state->logged_user);
    requests[request_count].group_id = target_group_id;
    request_count++;
    
    /* Save requests to file */
    save_requests();
    
    pthread_mutex_unlock(&request_mutex);
    
    /* Log the join request */
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Join request: %s -> %s", state->logged_user, group_name);
    write_log(log_msg);
    
    tcp_send(state->sockfd, "160");
    printf("Join request: %s -> %s\n", state->logged_user, group_name);
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
 *   407: User already in a group
 *   300: Syntax error
 **/
void handle_invite(conn_state_t *state, char *command) {
    char username[MAX_USERNAME];

    /* Check access control */
    char *access_error = role_based_access_control("INVITE", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    /* Parse command */
    if (sscanf(command, "INVITE %s", username) != 1) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
        return;
    }

    // Check if user exists and is not in group
    int user_found = 0;
    int target_user_group_id = -1;

    pthread_mutex_lock(&account_mutex);
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            user_found = 1;
            target_user_group_id = accounts[i].group_id;
            break;
        }
    }
    pthread_mutex_unlock(&account_mutex);

    if (!user_found) {
        tcp_send(state->sockfd, "500"); // User not found
        write_log_detailed(state->client_addr, command, "-ERR User does not exist");
        return;
    }

    if (target_user_group_id != -1) {
        tcp_send(state->sockfd, "407"); // User already in a group
        write_log_detailed(state->client_addr, command, "-ERR User already in a group");
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
            pthread_mutex_unlock(&invite_mutex);
            tcp_send(state->sockfd, "504"); // Server full
            write_log_detailed(state->client_addr, command, "-ERR Invite list full");
            return;
        }
    }
    pthread_mutex_unlock(&invite_mutex);

    tcp_send(state->sockfd, "180");

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User %s invited %s to group %d",
             state->logged_user, username, state->user_group_id);
    write_log(log_msg);
    write_log_detailed(state->client_addr, command, "+OK Invite sent successfully");
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

    /* Check access control */
    char *access_error = role_based_access_control("ACCEPT", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    /* Parse command */
    if (sscanf(command, "ACCEPT %s", group_name) != 1) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
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
        tcp_send(state->sockfd, "500"); // Group not found
        write_log_detailed(state->client_addr, command, "-ERR Group does not exist");
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
        pthread_mutex_unlock(&invite_mutex);
        tcp_send(state->sockfd, "500"); // No invite found
        write_log_detailed(state->client_addr, command, "-ERR No invite found for this group");
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

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User %s accepted invite to group: %s (ID: %d)",
             state->logged_user, group_name, group_id);
    write_log(log_msg);
    write_log_detailed(state->client_addr, command, "+OK Joined group successfully");
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

    /* Check access control */
    char *access_error = role_based_access_control("KICK", state);
    if (access_error != NULL) {
        tcp_send(state->sockfd, access_error);
        write_log_detailed(state->client_addr, command, "-ERR Access denied");
        return;
    }

    /* Parse command */
    if (sscanf(command, "KICK %s", username) != 1) {
        tcp_send(state->sockfd, "300");
        write_log_detailed(state->client_addr, command, "-ERR Syntax error");
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
        write_log_detailed(state->client_addr, command, "-ERR Member not in group");
        return;
    }

    tcp_send(state->sockfd, "201");

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "User %s kicked %s from group %d",
             state->logged_user, username, state->user_group_id);
    write_log(log_msg);
    write_log_detailed(state->client_addr, command, "+OK Member removed successfully");
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
    char response[BUFF_SIZE];
    char temp[256];
    
    /* Check if logged in */
    if (!state->is_logged_in) {
        tcp_send(state->sockfd, "400");
        return;
    }
    
    pthread_mutex_lock(&group_mutex);
    
    /* Build response with list of groups */
    if (group_count == 0) {
        snprintf(response, sizeof(response), "203 No groups available");
    } else {
        snprintf(response, sizeof(response), "203 ");
        for (int i = 0; i < group_count; i++) {
            snprintf(temp, sizeof(temp), "[%d] %s (Leader: %s)", 
                    groups[i].group_id, 
                    groups[i].group_name, 
                    groups[i].leader);
            
            /* Add separator if not first item */
            if (i > 0) {
                strcat(response, " | ");
            }
            strcat(response, temp);
        }
    }
    
    pthread_mutex_unlock(&group_mutex);
    
    tcp_send(state->sockfd, response);
    printf("User %s listed groups\n", state->logged_user);
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
