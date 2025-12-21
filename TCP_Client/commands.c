#include "common.h"

/* ==================== COMMAND FUNCTIONS ==================== */

void do_register(int sockfd, conn_state_t *state) {
    char username[100];
    char password[100];
    char command[256];
    char response[BUFF_SIZE];
    
    printf("\n=== REGISTER NEW ACCOUNT ===\n");
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
    // TODO: Implement create group
    printf("Function not implemented yet\n");
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
    char username[100];
    char command[256];
    char response[BUFF_SIZE];

    printf("\n=== INVITE USER TO GROUP ===\n");
    printf("Enter username to invite: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return;
    }
    username[strcspn(username, "\n")] = 0;

    // Validate input
    if (strlen(username) == 0) {
        printf(">> Username cannot be empty\n");
        return;
    }

    // Send INVITE command
    snprintf(command, sizeof(command), "INVITE %s", username);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_accept(int sockfd, conn_state_t *state) {
    char group_name[100];
    char command[256];
    char response[BUFF_SIZE];

    printf("\n=== ACCEPT GROUP INVITATION ===\n");
    printf("Enter group name: ");
    if (fgets(group_name, sizeof(group_name), stdin) == NULL) {
        return;
    }
    group_name[strcspn(group_name, "\n")] = 0;

    // Validate input
    if (strlen(group_name) == 0) {
        printf(">> Group name cannot be empty\n");
        return;
    }

    // Send ACCEPT command
    snprintf(command, sizeof(command), "ACCEPT %s", group_name);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_leave(int sockfd, conn_state_t *state) {
    // TODO: Implement leave
    printf("Function not implemented yet\n");
}

void do_kick(int sockfd, conn_state_t *state) {
    char username[100];
    char command[256];
    char response[BUFF_SIZE];

    printf("\n=== KICK MEMBER FROM GROUP ===\n");
    printf("Enter username to kick: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return;
    }
    username[strcspn(username, "\n")] = 0;

    // Validate input
    if (strlen(username) == 0) {
        printf(">> Username cannot be empty\n");
        return;
    }

    // Send KICK command
    snprintf(command, sizeof(command), "KICK %s", username);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_list_groups(int sockfd, conn_state_t *state) {
    // TODO: Implement list groups
    printf("Function not implemented yet\n");
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
    char old_name[MAX_PATH];
    char new_name[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== RENAME FILE ===\n");
    printf("Enter current file path: ");
    if (fgets(old_name, sizeof(old_name), stdin) == NULL) {
        return;
    }
    old_name[strcspn(old_name, "\n")] = 0;

    printf("Enter new file name: ");
    if (fgets(new_name, sizeof(new_name), stdin) == NULL) {
        return;
    }
    new_name[strcspn(new_name, "\n")] = 0;

    // Validate input
    if (strlen(old_name) == 0 || strlen(new_name) == 0) {
        printf(">> File names cannot be empty\n");
        return;
    }

    // Send RENAME_FILE command
    snprintf(command, sizeof(command), "RENAME_FILE %s %s", old_name, new_name);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_delete_file(int sockfd, conn_state_t *state) {
    char path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== DELETE FILE ===\n");
    printf("Enter file path to delete: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        return;
    }
    path[strcspn(path, "\n")] = 0;

    // Validate input
    if (strlen(path) == 0) {
        printf(">> File path cannot be empty\n");
        return;
    }

    // Send DELETE_FILE command
    snprintf(command, sizeof(command), "DELETE_FILE %s", path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_copy_file(int sockfd, conn_state_t *state) {
    char src_path[MAX_PATH];
    char dest_path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== COPY FILE ===\n");
    printf("Enter source file path: ");
    if (fgets(src_path, sizeof(src_path), stdin) == NULL) {
        return;
    }
    src_path[strcspn(src_path, "\n")] = 0;

    printf("Enter destination file path: ");
    if (fgets(dest_path, sizeof(dest_path), stdin) == NULL) {
        return;
    }
    dest_path[strcspn(dest_path, "\n")] = 0;

    // Validate input
    if (strlen(src_path) == 0 || strlen(dest_path) == 0) {
        printf(">> Paths cannot be empty\n");
        return;
    }

    // Send COPY_FILE command
    snprintf(command, sizeof(command), "COPY_FILE %s %s", src_path, dest_path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_move_file(int sockfd, conn_state_t *state) {
    char src_path[MAX_PATH];
    char dest_dir[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== MOVE FILE ===\n");
    printf("Enter source file path: ");
    if (fgets(src_path, sizeof(src_path), stdin) == NULL) {
        return;
    }
    src_path[strcspn(src_path, "\n")] = 0;

    printf("Enter destination directory: ");
    if (fgets(dest_dir, sizeof(dest_dir), stdin) == NULL) {
        return;
    }
    dest_dir[strcspn(dest_dir, "\n")] = 0;

    // Validate input
    if (strlen(src_path) == 0 || strlen(dest_dir) == 0) {
        printf(">> Paths cannot be empty\n");
        return;
    }

    // Send MOVE_FILE command
    snprintf(command, sizeof(command), "MOVE_FILE %s %s", src_path, dest_dir);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_mkdir(int sockfd, conn_state_t *state) {
    char path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== CREATE FOLDER ===\n");
    printf("Enter folder path: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        return;
    }
    path[strcspn(path, "\n")] = 0;

    // Validate input
    if (strlen(path) == 0) {
        printf(">> Folder path cannot be empty\n");
        return;
    }

    // Send MKDIR command
    snprintf(command, sizeof(command), "MKDIR %s", path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_rename_folder(int sockfd, conn_state_t *state) {
    char old_name[MAX_PATH];
    char new_name[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== RENAME FOLDER ===\n");
    printf("Enter current folder path: ");
    if (fgets(old_name, sizeof(old_name), stdin) == NULL) {
        return;
    }
    old_name[strcspn(old_name, "\n")] = 0;

    printf("Enter new folder name: ");
    if (fgets(new_name, sizeof(new_name), stdin) == NULL) {
        return;
    }
    new_name[strcspn(new_name, "\n")] = 0;

    // Validate input
    if (strlen(old_name) == 0 || strlen(new_name) == 0) {
        printf(">> Folder names cannot be empty\n");
        return;
    }

    // Send RENAME_FOLDER command
    snprintf(command, sizeof(command), "RENAME_FOLDER %s %s", old_name, new_name);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_rmdir(int sockfd, conn_state_t *state) {
    char path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== DELETE FOLDER ===\n");
    printf("Enter folder path to delete: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        return;
    }
    path[strcspn(path, "\n")] = 0;

    // Validate input
    if (strlen(path) == 0) {
        printf(">> Folder path cannot be empty\n");
        return;
    }

    // Send RMDIR command
    snprintf(command, sizeof(command), "RMDIR %s", path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_copy_folder(int sockfd, conn_state_t *state) {
    char src_path[MAX_PATH];
    char dest_path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== COPY FOLDER ===\n");
    printf("Enter source folder path: ");
    if (fgets(src_path, sizeof(src_path), stdin) == NULL) {
        return;
    }
    src_path[strcspn(src_path, "\n")] = 0;

    printf("Enter destination path: ");
    if (fgets(dest_path, sizeof(dest_path), stdin) == NULL) {
        return;
    }
    dest_path[strcspn(dest_path, "\n")] = 0;

    // Validate input
    if (strlen(src_path) == 0 || strlen(dest_path) == 0) {
        printf(">> Paths cannot be empty\n");
        return;
    }

    // Send COPY_FOLDER command
    snprintf(command, sizeof(command), "COPY_FOLDER %s %s", src_path, dest_path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_move_folder(int sockfd, conn_state_t *state) {
    char src_path[MAX_PATH];
    char dest_dir[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== MOVE FOLDER ===\n");
    printf("Enter source folder path: ");
    if (fgets(src_path, sizeof(src_path), stdin) == NULL) {
        return;
    }
    src_path[strcspn(src_path, "\n")] = 0;

    printf("Enter destination directory: ");
    if (fgets(dest_dir, sizeof(dest_dir), stdin) == NULL) {
        return;
    }
    dest_dir[strcspn(dest_dir, "\n")] = 0;

    // Validate input
    if (strlen(src_path) == 0 || strlen(dest_dir) == 0) {
        printf(">> Paths cannot be empty\n");
        return;
    }

    // Send MOVE_FOLDER command
    snprintf(command, sizeof(command), "MOVE_FOLDER %s %s", src_path, dest_dir);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

void do_list_content(int sockfd, conn_state_t *state) {
    char path[MAX_PATH];
    char command[BUFF_SIZE];
    char response[BUFF_SIZE];

    printf("\n=== LIST FOLDER CONTENT ===\n");
    printf("Enter folder path: ");
    if (fgets(path, sizeof(path), stdin) == NULL) {
        return;
    }
    path[strcspn(path, "\n")] = 0;

    // Default to root if empty
    if (strlen(path) == 0) {
        strcpy(path, "/");
    }

    // Send LIST_CONTENT command
    snprintf(command, sizeof(command), "LIST_CONTENT %s", path);
    if (tcp_send(sockfd, command) <= 0) {
        printf(">> Failed to send command\n");
        return;
    }

    // Receive response
    if (tcp_receive(sockfd, state, response, BUFF_SIZE) > 0) {
        print_response(response);
    } else {
        printf(">> Failed to receive response\n");
    }
}

