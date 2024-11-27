#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "../utils.h"
#include "logger.h"

static char log_file_path[256] = "default.log";

void set_log_file(const char *file_path) {
    if (file_path) {
        strncpy(log_file_path, file_path, sizeof(log_file_path) - 1);
        log_file_path[sizeof(log_file_path) - 1] = '\0'; 
    }
}

void logger(const char* log_level, const char* formatted_message, ...){
    init_config();
    FILE *log_file = fopen(log_file_path, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log_file, "[%s] [%s] ", time_buffer, log_level);
    va_list args;
    va_start(args, formatted_message);
    vfprintf(log_file, formatted_message, args);
    va_end(args);
    fprintf(log_file, "\n");

    fclose(log_file);
}
