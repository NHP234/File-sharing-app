#include "common.h"

/* ==================== MAIN FUNCTION ==================== */

/**
 * @function main: Main client function to connect to server and handle user interaction
 * @param argc: Number of command line arguments
 * @param argv: Array of command line arguments (argv[1] is IP, argv[2] is port)
 * @return: 0 on normal exit, 1 on error
 **/
int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char ip_addr[50];
    int port;
    conn_state_t state;
    char buffer[BUFF_SIZE];
    int choice;
    int is_logged_in = 0;
    
    if (argc != 3) {
        printf("Usage: %s IP_Address Port_Number\n", argv[0]);
        return 1;
    }
    
    strcpy(ip_addr, argv[1]);
    port = atoi(argv[2]);
    
    /* Create socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        return 1;
    }
    
    /* Connect to server */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        close(sockfd);
        return 1;
    }
    
    /* Initialize state */
    memset(&state, 0, sizeof(conn_state_t));
    
    /* Receive welcome message */
    if (tcp_receive(sockfd, &state, buffer, BUFF_SIZE) > 0) {
        print_response(buffer);
    }
    
    /* Main loop */
    while (1) {
        print_main_menu();
        
        if (scanf("%d", &choice) != 1) {
            /* Clear input buffer */
            while (getchar() != '\n');
            printf("Invalid choice\n");
            continue;
        }
        getchar(); /* Consume newline */
        
        switch (choice) {
            case 0: /* Exit */
                if (is_logged_in) {
                    do_logout(sockfd, &state, &is_logged_in);
                }
                printf("Goodbye!\n");
                close(sockfd);
                return 0;
                
            case 1: /* Register */
                do_register(sockfd, &state);
                break;
                
            case 2: /* Login */
                do_login(sockfd, &state, &is_logged_in);
                break;
                
            case 3: /* Logout */
                do_logout(sockfd, &state, &is_logged_in);
                break;
                
            case 4: /* Create group */
                do_create_group(sockfd, &state);
                break;
                
            case 5: /* Join group */
                do_join_group(sockfd, &state);
                break;
                
            case 6: /* Accept invite */
                do_accept(sockfd, &state);
                break;
                
            case 7: /* Leave group */
                do_leave(sockfd, &state);
                break;
                
            case 8: /* List groups */
                do_list_groups(sockfd, &state);
                break;
                
            case 9: /* List members */
                do_list_members(sockfd, &state);
                break;
                
            case 10: /* Approve */
                do_approve(sockfd, &state);
                break;
                
            case 11: /* Invite */
                do_invite(sockfd, &state);
                break;
                
            case 12: /* Kick */
                do_kick(sockfd, &state);
                break;
                
            case 13: /* List requests */
                do_list_requests(sockfd, &state);
                break;
                
            case 14: /* Upload */
                do_upload(sockfd, &state);
                break;
                
            case 15: /* Download */
                do_download(sockfd, &state);
                break;
                
            case 16: /* List content */
                do_list_content(sockfd, &state);
                break;
                
            case 17: /* Create folder */
                do_mkdir(sockfd, &state);
                break;
                
            case 18: /* Rename file */
                do_rename_file(sockfd, &state);
                break;
                
            case 19: /* Delete file */
                do_delete_file(sockfd, &state);
                break;
                
            case 20: /* Copy file */
                do_copy_file(sockfd, &state);
                break;
                
            case 21: /* Move file */
                do_move_file(sockfd, &state);
                break;
                
            case 22: /* Rename folder */
                do_rename_folder(sockfd, &state);
                break;
                
            case 23: /* Delete folder */
                do_rmdir(sockfd, &state);
                break;
                
            case 24: /* Copy folder */
                do_copy_folder(sockfd, &state);
                break;
                
            case 25: /* Move folder */
                do_move_folder(sockfd, &state);
                break;
                
            default:
                printf("Invalid choice\n");
        }
    }
    
    close(sockfd);
    return 0;
}

