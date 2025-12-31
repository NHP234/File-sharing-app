#include "common.h"

/* ==================== UI FUNCTIONS ==================== */

/**
 * @function print_main_menu: Display the main menu
 * @return: None
 **/
void print_main_menu() {
    printf("\n========================================\n");
    printf("       FILE SHARING CLIENT\n");
    printf("========================================\n");
    printf("  ACCOUNT MANAGEMENT\n");
    printf("  1.  Register\n");
    printf("  2.  Login\n");
    printf("  3.  Logout\n");
    printf("\n  GROUP MANAGEMENT\n");
    printf("  4.  Create group\n");
    printf("  5.  Join group\n");
    printf("  6.  Accept invite\n");
    printf("  7.  Leave group\n");
    printf("  8.  List all groups\n");
    printf("  9.  List members in my group\n");
    printf("\n  LEADER FUNCTIONS\n");
    printf("  10. Approve join request\n");
    printf("  11. Invite user to group\n");
    printf("  12. Kick member\n");
    printf("  13. List join requests\n");
    printf("\n  FILE OPERATIONS\n");
    printf("  14. Upload file\n");
    printf("  15. Download file\n");
    printf("  16. List content\n");
    printf("  17. Create folder\n");
    printf("\n  LEADER FILE OPERATIONS\n");
    printf("  18. Rename file\n");
    printf("  19. Delete file\n");
    printf("  20. Copy file\n");
    printf("  21. Move file\n");
    printf("  22. Rename folder\n");
    printf("  23. Delete folder\n");
    printf("  24. Copy folder\n");
    printf("  25. Move folder\n");
    printf("\n  0.  Exit\n");
    printf("========================================\n");
    printf("Your choice: ");
}

/**
 * @function print_response: Translate and display server response
 * @param response: Response string from server
 * @return: None
 **/
void print_response(char *response) {
    char code[10];
    sscanf(response, "%s", code);
    
    /* Success codes */
    if (strcmp(code, "100") == 0) {
        printf(">> Connected to server successfully\n");
    } else if (strcmp(code, "110") == 0) {
        printf(">> Login successful\n");
    } else if (strcmp(code, "120") == 0) {
        printf(">> Registration successful\n");
    } else if (strcmp(code, "130") == 0) {
        printf(">> Logout successful\n");
    } else if (strcmp(code, "140") == 0) {
        printf(">> Upload successful\n");
    } else if (strcmp(code, "141") == 0) {
        printf(">> Server ready to receive file\n");
    } else if (strcmp(code, "150") == 0) {
        printf(">> Download successful\n");
    } else if (strcmp(code, "151") == 0) {
        printf(">> Server ready to send file\n");
    } else if (strcmp(code, "160") == 0) {
        printf(">> Join request sent successfully\n");
    } else if (strcmp(code, "170") == 0) {
        printf(">> Member approved successfully\n");
    } else if (strcmp(code, "180") == 0) {
        printf(">> Invite sent successfully\n");
    } else if (strcmp(code, "190") == 0) {
        printf(">> Joined group successfully\n");
    } else if (strcmp(code, "200") == 0) {
        printf(">> Left group successfully\n");
    } else if (strcmp(code, "201") == 0) {
        printf(">> Member kicked successfully\n");
    } else if (strcmp(code, "202") == 0) {
        printf(">> Group created successfully\n");
    } else if (strcmp(code, "203") == 0) {
        printf(">> List of groups:\n");
        printf("%s\n", response + 4); /* Print data after code */
    } else if (strcmp(code, "204") == 0) {
        printf(">> List of members:\n");
        printf("%s\n", response + 4);
    } else if (strcmp(code, "205") == 0) {
        printf(">> List of join requests:\n");
        printf("%s\n", response + 4);
    } else if (strcmp(code, "210") == 0) {
        printf(">> File renamed successfully\n");
    } else if (strcmp(code, "211") == 0) {
        printf(">> File deleted successfully\n");
    } else if (strcmp(code, "212") == 0) {
        printf(">> File copied successfully\n");
    } else if (strcmp(code, "213") == 0) {
        printf(">> File moved successfully\n");
    } else if (strcmp(code, "220") == 0) {
        printf(">> Folder created successfully\n");
    } else if (strcmp(code, "221") == 0) {
        printf(">> Folder renamed successfully\n");
    } else if (strcmp(code, "222") == 0) {
        printf(">> Folder deleted successfully\n");
    } else if (strcmp(code, "223") == 0) {
        printf(">> Folder copied successfully\n");
    } else if (strcmp(code, "224") == 0) {
        printf(">> Folder moved successfully\n");
    } else if (strcmp(code, "225") == 0) {
        printf(">> Content list:\n");
        printf("%s\n", response + 4);
    }
    /* Error codes */
    else if (strcmp(code, "300") == 0) {
        printf(">> Error: Invalid command syntax\n");
    } else if (strcmp(code, "400") == 0) {
        printf(">> Error: Not logged in\n");
    } else if (strcmp(code, "401") == 0) {
        printf(">> Error: Wrong username or password\n");
    } else if (strcmp(code, "402") == 0) {
        printf(">> Error: Account does not exist\n");
    } else if (strcmp(code, "403") == 0) {
        printf(">> Error: Already logged in\n");
    } else if (strcmp(code, "404") == 0) {
        printf(">> Error: Not in any group\n");
    } else if (strcmp(code, "406") == 0) {
        printf(">> Error: Not group leader\n");
    } else if (strcmp(code, "407") == 0) {
        printf(">> Error: Already in a group\n");
    } else if (strcmp(code, "408") == 0) {
        printf(">> Error: Leader must remove all members first\n");
    } else if (strcmp(code, "500") == 0) {
        printf(">> Error: Resource does not exist\n");
    } else if (strcmp(code, "501") == 0) {
        printf(">> Error: Name already exists\n");
    } else if (strcmp(code, "502") == 0) {
        printf(">> Error: File write error on server\n");
    } else if (strcmp(code, "503") == 0) {
        printf(">> Error: Invalid destination path\n");
    } else if (strcmp(code, "504") == 0) {
        printf(">> Error: Cannot perform file operation on folder\n");
    } else if (strcmp(code, "505") == 0) {
        printf(">> Error: File is being used (uploading/downloading)\n");
    } else if (strcmp(code, "506") == 0) {
        printf(">> Error: Cannot perform folder operation on file\n");
    } else {
        printf(">> Response: %s\n", response);
    }
}

