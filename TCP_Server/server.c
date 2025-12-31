#include "common.h"

/* ==================== MAIN COMMAND PROCESSOR ==================== */

/**
 * @function process_command: Process and route client commands
 * @param state: Connection state of the client
 * @param command: Command string received from client
 * @return: None
 **/
void process_command(conn_state_t *state, char *command) {
    char cmd[20];
    
    /* Parse command */
    if (sscanf(command, "%s", cmd) != 1) {
        tcp_send(state->sockfd, "300");
        return;
    }
    
    /* Route to appropriate handler */
    if (strcmp(cmd, "REGISTER") == 0) {
        handle_register(state, command);
    } else if (strcmp(cmd, "LOGIN") == 0) {
        handle_login(state, command);
    } else if (strcmp(cmd, "LOGOUT") == 0) {
        handle_logout(state, command);
    } else if (strcmp(cmd, "UPLOAD") == 0) {
        handle_upload(state, command);
    } else if (strcmp(cmd, "DOWNLOAD") == 0) {
        handle_download(state, command);
    } else if (strcmp(cmd, "CREATE") == 0) {
        handle_create_group(state, command);
    } else if (strcmp(cmd, "JOIN") == 0) {
        handle_join_group(state, command);
    } else if (strcmp(cmd, "APPROVE") == 0) {
        handle_approve(state, command);
    } else if (strcmp(cmd, "INVITE") == 0) {
        handle_invite(state, command);
    } else if (strcmp(cmd, "ACCEPT") == 0) {
        handle_accept(state, command);
    } else if (strcmp(cmd, "LEAVE") == 0) {
        handle_leave(state, command);
    } else if (strcmp(cmd, "KICK") == 0) {
        handle_kick(state, command);
    } else if (strcmp(cmd, "LIST_GROUPS") == 0) {
        handle_list_groups(state, command);
    } else if (strcmp(cmd, "LIST_MEMBERS") == 0) {
        handle_list_members(state, command);
    } else if (strcmp(cmd, "LIST_REQUESTS") == 0) {
        handle_list_requests(state, command);
    } else if (strcmp(cmd, "RENAME_FILE") == 0) {
        handle_rename_file(state, command);
    } else if (strcmp(cmd, "DELETE_FILE") == 0) {
        handle_delete_file(state, command);
    } else if (strcmp(cmd, "COPY_FILE") == 0) {
        handle_copy_file(state, command);
    } else if (strcmp(cmd, "MOVE_FILE") == 0) {
        handle_move_file(state, command);
    } else if (strcmp(cmd, "MKDIR") == 0) {
        handle_mkdir(state, command);
    } else if (strcmp(cmd, "RENAME_FOLDER") == 0) {
        handle_rename_folder(state, command);
    } else if (strcmp(cmd, "RMDIR") == 0) {
        handle_rmdir(state, command);
    } else if (strcmp(cmd, "COPY_FOLDER") == 0) {
        handle_copy_folder(state, command);
    } else if (strcmp(cmd, "MOVE_FOLDER") == 0) {
        handle_move_folder(state, command);
    } else if (strcmp(cmd, "LIST_CONTENT") == 0) {
        handle_list_content(state, command);
    } else {
        tcp_send(state->sockfd, "300");
    }
}

/* ==================== THREAD FUNCTION ==================== */

/**
 * @function handle_client: Thread function to handle a client connection
 * @param arg: Pointer to conn_state_t structure for this client
 * @return: NULL after client disconnects
 **/
void *handle_client(void *arg) {
    conn_state_t *state = (conn_state_t *)arg;
    char buffer[BUFF_SIZE];
    
    /* Send welcome message */
    tcp_send(state->sockfd, "100");
    
    /* Process commands */
    while (1) {
        int ret = tcp_receive(state->sockfd, state, buffer, BUFF_SIZE);
        if (ret <= 0) {
            break; /* Connection closed or error */
        }
        
        printf("Received from %s: %s\n", 
               state->is_logged_in ? state->logged_user : "anonymous", buffer);
        process_command(state, buffer);
    }
    
    /* Auto logout if logged in */
    if (state->is_logged_in) {
        pthread_mutex_lock(&account_mutex);
        for (int i = 0; i < account_count; i++) {
            if (strcmp(accounts[i].username, state->logged_user) == 0) {
                accounts[i].is_logged_in = 0;
                printf("User %s disconnected (auto logout)\n", state->logged_user);
                write_log_detailed(state->client_addr, "", "+INFO User disconnected (auto logout)");
                break;
            }
        }
        pthread_mutex_unlock(&account_mutex);
    }
    
    close(state->sockfd);
    free(state);
    pthread_detach(pthread_self());
    return NULL;
}

/* ==================== MAIN FUNCTION ==================== */

/**
 * @function main: Main server function to initialize and accept connections
 * @param argc: Number of command line arguments
 * @param argv: Array of command line arguments (argv[1] is port number)
 * @return: 0 on normal exit, 1 on error
 **/
int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    pthread_t tid;
    int port;
    
    if (argc != 2) {
        printf("Usage: %s Port_Number\n", argv[0]);
        return 1;
    }
    
    port = atoi(argv[1]);
    
    /* Load data from files */
    printf("Loading data...\n");
    load_accounts();
    load_groups();
    load_requests();
    load_invites();
    
    /* Create necessary directories if not exist */
    mkdir("data", 0755);
    mkdir("groups", 0755);
    mkdir("logs", 0755);
    
    /* Create socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error");
        return 1;
    }
        
    /* Bind */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(listenfd);
        return 1;
    }
    
    /* Listen */
    if (listen(listenfd, BACKLOG) == -1) {
        perror("listen() error");
        close(listenfd);
        return 1;
    }
    
    printf("===========================================\n");
    printf("  FILE SHARING SERVER STARTED\n");
    printf("  Port: %d\n", port);
    printf("  Waiting for connections...\n");
    printf("===========================================\n");
    
    write_log_detailed("SERVER", "", "+INFO Server started");
    
    /* Accept connections */
    while (1) {
        sin_size = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &sin_size);
        if (connfd == -1) {
            perror("accept() error");
            continue;
        }
        
        printf("\n[NEW CONNECTION] %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        /* Create state for this connection */
        conn_state_t *state = malloc(sizeof(conn_state_t));
        memset(state, 0, sizeof(conn_state_t));
        state->sockfd = connfd;
        state->user_group_id = -1;
        
        /* Store client address for logging */
        snprintf(state->client_addr, sizeof(state->client_addr), "%s:%d",
                 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        /* Create thread to handle client */
        if (pthread_create(&tid, NULL, handle_client, state) != 0) {
            perror("pthread_create() error");
            close(connfd);
            free(state);
        }
    }
    
    close(listenfd);
    return 0;
}
