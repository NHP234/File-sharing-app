#include "common.h"

/* ==================== COMMAND FUNCTIONS ==================== */

void do_register(int sockfd, conn_state_t *state) {
    char username[100];
    char password[100];
    char command[256];
    char response[BUFF_SIZE];
    
    printf("\n=== REGISTER NEW ACCOUNT ===\n");
    printf("Enter username (less than 50 chars): ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return;
    }
    username[strcspn(username, "\n")] = 0;  /* Remove newline */
    
    printf("Enter password (less than 50 chars): ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        return;
    }
    password[strcspn(password, "\n")] = 0;  /* Remove newline */
    
    /* Validate input */
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf(">> Username and password cannot be empty\n");
        return;
    }
    
    /* Send REGISTER command */
    snprintf(command, sizeof(command), "REGISTER %s %s", username, password);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }
    
    /* Receive response */
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_login(int sockfd, conn_state_t *state, int *is_logged_in) {
    char username[100];
    char password[100];
    char command[256];
    char response[BUFF_SIZE];
    
    /* Check if already logged in */
    if (*is_logged_in) {
        printf(">> You are already logged in\n");
        return;
    }
    
    printf("\n=== LOGIN ===\n");
    printf("Enter username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return;
    }
    username[strcspn(username, "\n")] = 0;  /* Remove newline */
    
    printf("Enter password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        return;
    }
    password[strcspn(password, "\n")] = 0;  /* Remove newline */
    
    /* Validate input */
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf(">> Username and password cannot be empty\n");
        return;
    }
    
    /* Send LOGIN command */
    snprintf(command, sizeof(command), "LOGIN %s %s", username, password);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }
    
    /* Receive response */
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
        
        /* Update login status if successful */
        if (strncmp(response, "110", 3) == 0) {
            *is_logged_in = 1;
        }
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_logout(int sockfd, conn_state_t *state, int *is_logged_in) {
    char response[BUFF_SIZE];
    
    /* Check if logged in */
    if (!*is_logged_in) {
        printf(">> You are not logged in\n");
        return;
    }
    
    /* Send LOGOUT command */
    if (tcp_send(sockfd, "LOGOUT") <= 0) {
        printf(">> Failed to send command\n");
        return;
    }
    
    /* Receive response */
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
        
        /* Update login status if successful */
        if (strncmp(response, "130", 3) == 0) {
            *is_logged_in = 0;
        }
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_upload(int sockfd, conn_state_t *state) {
    // TODO: Implement upload
    printf("Function not implemented yet\n");
}

void do_download(int sockfd, conn_state_t *state) {
    // TODO: Implement download
    printf("Function not implemented yet\n");
}

void do_create_group(int sockfd, conn_state_t *state) {
    char group_name[100];
    char command[256];
    char response[BUFF_SIZE];
    
    printf("\n=== CREATE NEW GROUP ===\n");
    printf("Enter group name (less than 50 chars): ");
    if (fgets(group_name, sizeof(group_name), stdin) == NULL) {
        return;
    }
    group_name[strcspn(group_name, "\n")] = 0;  /* Remove newline */
    
    /* Validate input */
    if (strlen(group_name) == 0) {
        printf(">> Group name cannot be empty\n");
        return;
    }
    
    /* Check for spaces in group name */
    if (strchr(group_name, ' ') != NULL) {
        printf(">> Group name cannot contain spaces\n");
        return;
    }
    
    /* Send CREATE command */
    snprintf(command, sizeof(command), "CREATE %s", group_name);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }
    
    /* Receive response */
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_join_group(int sockfd, conn_state_t *state) {
    // TODO: Implement join group
    printf("Function not implemented yet\n");
}

void do_approve(int sockfd, conn_state_t *state) {
    // TODO: Implement approve
    printf("Function not implemented yet\n");
}

void do_invite(int sockfd, conn_state_t *state) {
    // TODO: Implement invite
    printf("Function not implemented yet\n");
}

void do_accept(int sockfd, conn_state_t *state) {
    // TODO: Implement accept
    printf("Function not implemented yet\n");
}

void do_leave(int sockfd, conn_state_t *state) {
    // TODO: Implement leave
    printf("Function not implemented yet\n");
}

void do_kick(int sockfd, conn_state_t *state) {
    // TODO: Implement kick
    printf("Function not implemented yet\n");
}

void do_list_groups(int sockfd, conn_state_t *state) {
    char response[BUFF_SIZE];
    
    printf("\n=== LIST ALL GROUPS ===\n");
    
    /* Send LIST_GROUPS command */
    if (tcp_send(sockfd, "LIST_GROUPS") <= 0) {
        printf(">> Failed to send command\n");
        return;
    }
    
    /* Receive response */
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_list_members(int sockfd, conn_state_t *state) {
    // TODO: Implement list members
    printf("Function not implemented yet\n");
}

void do_list_requests(int sockfd, conn_state_t *state) {
    // TODO: Implement list requests
    printf("Function not implemented yet\n");
}

void do_rename_file(int sockfd, conn_state_t *state) {
    // TODO: Implement rename file
    printf("Function not implemented yet\n");
}

void do_delete_file(int sockfd, conn_state_t *state) {
    // TODO: Implement delete file
    printf("Function not implemented yet\n");
}

void do_copy_file(int sockfd, conn_state_t *state) {
    // TODO: Implement copy file
    printf("Function not implemented yet\n");
}

void do_move_file(int sockfd, conn_state_t *state) {
    // TODO: Implement move file
    printf("Function not implemented yet\n");
}

void do_mkdir(int sockfd, conn_state_t *state) {
    // TODO: Implement mkdir
    printf("Function not implemented yet\n");
}

void do_rename_folder(int sockfd, conn_state_t *state) {
    // TODO: Implement rename folder
    printf("Function not implemented yet\n");
}

void do_rmdir(int sockfd, conn_state_t *state) {
    // TODO: Implement rmdir
    printf("Function not implemented yet\n");
}

void do_copy_folder(int sockfd, conn_state_t *state) {
    // TODO: Implement copy folder
    printf("Function not implemented yet\n");
}

void do_move_folder(int sockfd, conn_state_t *state) {
    // TODO: Implement move folder
    printf("Function not implemented yet\n");
}

void do_list_content(int sockfd, conn_state_t *state) {
    // TODO: Implement list content
    printf("Function not implemented yet\n");
}

