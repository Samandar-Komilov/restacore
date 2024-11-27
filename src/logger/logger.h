#ifndef LOGGER_H

void set_log_file(const char *log_file_path);
void logger(const char* log_level, const char* formatted_message, ...);

#endif