#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logger.h"
#include "../dotenv/dotenv.h"

char *server_address = getenv("SERVER_ADDRESS");

void logger(const char* log_level, const char* message){
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log_file, "%s %s %s %s\n", time_buffer, SERVER_ADDRESS, log_level, message);
    fclose(log_file);
}
