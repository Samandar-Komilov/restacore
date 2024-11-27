#include <stdlib.h>
#include <stdio.h>
#include "dotenv/dotenv.h"
#include "utils.h"

const char* SERVER_ADDRESS = NULL;
int PORT = 0;
int MAX_BUFFER = 0;

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
