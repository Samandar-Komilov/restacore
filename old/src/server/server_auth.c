#include "../shared/auth.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 256

bool authenticate_user(const char *username, const char *password) {
    FILE *file = fopen("data/users.txt", "r");
    if (!file) {
        perror("Failed to open users file");
        return false;
    }

    char line[MAX_LINE];
    bool authenticated = false;

    // Format in users.txt: username password
    while (fgets(line, sizeof(line), file)) {
        char file_username[50], file_password[50];
        
        // Parse line
        if (sscanf(line, "%s %s", file_username, file_password) == 2) {
            // Check if username and password match
            if (strcmp(file_username, username) == 0 && strcmp(file_password, password) == 0) {
                authenticated = true;
                break;
            }
        }
    }

    fclose(file);
    return authenticated;
}
