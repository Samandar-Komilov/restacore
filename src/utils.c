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
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!active_sessions[i].active) {
            strncpy(active_sessions[i].ip_address, ip, IP_LENGTH - 1);
            strncpy(active_sessions[i].username, username, USERNAME_LENGTH - 1);
            active_sessions[i].active = true;
            added = true;
            active_session_count++;
            break;
        }
    }
    
    pthread_mutex_unlock(&sessions_mutex);
    return added;
}

void remove_session(const char *ip) {
    pthread_mutex_lock(&sessions_mutex);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].active && strcmp(active_sessions[i].ip_address, ip) == 0) {
            active_sessions[i].active = false;
            memset(active_sessions[i].ip_address, 0, IP_LENGTH);
            memset(active_sessions[i].username, 0, USERNAME_LENGTH);
            active_session_count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&sessions_mutex);
}

void list_sessions() {
    pthread_mutex_lock(&sessions_mutex);
    
    printf("-----------------------\nActive sessions:\n");
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (active_sessions[i].active) {
            printf("%d) IP: %s, Username: %s\n", i+1, active_sessions[i].ip_address, active_sessions[i].username);
        }
    }
    
    pthread_mutex_unlock(&sessions_mutex);
}