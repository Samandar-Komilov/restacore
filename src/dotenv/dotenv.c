#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dotenv.h"

void load_env(const char *env_file_path) {
    FILE *file = fopen(env_file_path, "r");
    if (!file) {
        perror("Failed to open .env file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines or comments
        if (line[0] == '\0' || line[0] == '#') continue;

        // Split into key and value
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (key && value) {
            setenv(key, value, 1);
        }
    }

    fclose(file);
}
