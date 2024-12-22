#include <stdlib.h>
#include <stdio.h>
#include "dotenv/dotenv.h"
#include "utils.h"

const char* SERVER_ADDRESS = NULL;
int PORT = 0;
int MAX_BUFFER = 0;

Session active_sessions[MAX_SESSIONS] = {0};
int active_session_count = 0;
pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_config() {
    const char* env_path = ".env";

    load_env(env_path);

    SERVER_ADDRESS = getenv("SERVER_ADDRESS");
    if (!SERVER_ADDRESS) {
        fprintf(stderr, "Error: SERVER_ADDRESS not found in .env\n");
        exit(EXIT_FAILURE);
    }

    const char* port_str = getenv("PORT");
    if (!port_str) {
        fprintf(stderr, "Error: PORT not found in .env\n");
        exit(EXIT_FAILURE);
    }
    PORT = atoi(port_str);

    const char* max_buffer_str = getenv("MAX_BUFFER");
    if (!max_buffer_str) {
        fprintf(stderr, "Error: MAX_BUFFER not found in .env\n");
        exit(EXIT_FAILURE);
    }
    MAX_BUFFER = atoi(max_buffer_str);
}


// ACTIVE SESSIONS


bool add_session(const char *ip, const char *username) {
    bool added = false;
    
    pthread_mutex_lock(&sessions_mutex);
    
    // for (int i = 0; i < MAX_SESSIONS; i++) {
    //     if (!active_sessions[i].active) {
    //         strncpy(active_sessions[i].ip_address, ip, IP_LENGTH - 1);
    //         strncpy(active_sessions[i].username, username, USERNAME_LENGTH - 1);
    //         active_sessions[i].active = true;
    //         added = true;
    //         active_session_count++;
    //         break;
    //     }
    // }
    
    FILE *file = fopen("sessions.txt", "a+");
    if (file) {
        if (fseek(file, 0, SEEK_END) == 0) {
            fseek(file, 0, SEEK_SET);
        }
        if (ftell(file) == 0) {
            // File is empty, create new session
            fprintf(file, "%s %s \n", ip, username);
        } else {
            // File is not empty, append new session
            fseek(file, -1, SEEK_END);
            fprintf(file, "\n%s %s ", ip, username);
        }
        fclose(file);
        added = true;
    }
    
    pthread_mutex_unlock(&sessions_mutex);
    return added;
}

void remove_session(const char *ip) {
    pthread_mutex_lock(&sessions_mutex);
    
    FILE *file = fopen("sessions.txt", "r");
    if (!file) {
        pthread_mutex_unlock(&sessions_mutex);
        return;
    }
    
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        fclose(file);
        pthread_mutex_unlock(&sessions_mutex);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char line_copy[256];
        strncpy(line_copy, line, sizeof(line_copy));
        char *token = strtok(line_copy, " ");
        if (strcmp(token, ip) != 0 || strtok(NULL, " ") == NULL) {
            fprintf(temp, "%s", line);
        }
    }
    
    fclose(file);
    fclose(temp);
    remove("sessions.txt");
    rename("temp.txt", "sessions.txt");
    
    pthread_mutex_unlock(&sessions_mutex);
}


void list_sessions() {
    pthread_mutex_lock(&sessions_mutex);
    
    printf("-----------------------\nActive sessions:\n");
    FILE *file = fopen("sessions.txt", "r");
    if (file) {
        char line[256];
        int i = 1;
        while (fgets(line, sizeof(line), file)) {
            char ip[IP_LENGTH];
            char username[USERNAME_LENGTH];
            sscanf(line, "%s %s", ip, username);
            printf("%d) IP: %s, Username: %s\n", i++, ip, username);
        }
        fclose(file);
    }
    printf("-----------------------\n");
    
    pthread_mutex_unlock(&sessions_mutex);
}


/**
 * Returns a char array in the format:
 * "IP1 USERNAME1 IP2 USERNAME2 ... IPN USERNAMEN "
 * where N is the number of active sessions.
 */
char* list_sessions_client() {
    char *buffer = malloc((MAX_SESSIONS * (IP_LENGTH + USERNAME_LENGTH + 2)) * sizeof(char));
    if (!buffer) {
        return NULL;
    }
    
    FILE *file = fopen("sessions.txt", "r");
    if (!file) {
        free(buffer);
        return NULL;
    }
    
    char line[256];
    int buffer_pos = 0;
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, " ");
        if (token) {
            strncpy(buffer + buffer_pos, token, IP_LENGTH);
            buffer_pos += strlen(token);
            token = strtok(NULL, " ");
            if (token) {
                strncpy(buffer + buffer_pos, " ", 1); // add a space separator
                buffer_pos++;
                strncpy(buffer + buffer_pos, token, USERNAME_LENGTH);
                buffer_pos += strlen(token);
            }
            strncpy(buffer + buffer_pos, "\n", 1); // add a newline character
            buffer_pos++;
        }
    }
    buffer[buffer_pos] = '\0';
    
    fclose(file);
    return buffer;
}
