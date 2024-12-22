#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <libpq-fe.h>
#include <time.h>
#include <errno.h>

#include "utils.h"
#include "server_functions.h"
#include "logger/logger.h"


int main() {
    /* dotenv and logger initialization */
    init_config();
    set_log_file("logs/server.log");

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        logger("ERROR", "Socket creation failed: %s (errno: %d)", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        logger("ERROR", "Bind failed: %s (errno: %d)", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("\033[1;32m[LISTENING] Port Number: %d\033[0m\n", PORT);
    logger("INFO", "Listening on Port Number: %d", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }
        
        /* Convert the client's IP address from binary to a readable string */ 
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

        printf("\033[1;32m[CONNECTED] New connection from %s\033[0m\n", client_ip);
        logger("INFO", "[CONNECTED] New connection from %s\n", client_ip);

        int* client_fd_copy = (int*)malloc(sizeof(int));
        if (client_fd_copy == NULL) {
            perror("Failed to allocate memory");
            close(client_fd);
            continue;
        }

        *client_fd_copy = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_command, client_fd_copy) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(client_fd_copy);  // Free the memory in case of failure
            continue;
        }

        // printf("[INFO] Thread created for client %s\n", client_ip);
        printf("\033[1;32m[INFO] Thread created for client %s\033[0m\n", client_ip);

        pthread_detach(client_thread);
    }

    close(server_fd);

    return 0;
}