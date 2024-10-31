#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../shared/auth.h"
#include "../shared/network.h"

void *handle_client(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[MAX_BUFFER];

    // Receive credentials
    recv(client_sock, buffer, MAX_BUFFER, 0);

    // Parse credentials
    char username[50], password[50];
    sscanf(buffer, "%s %s", username, password);

    // Authenticate
    printf("Server scanned credentials...");
    if (authenticate_user(username, password)) {
        send(client_sock, "Login Successful", 16, 0);
        printf("Server sent successful login response! %d", client_sock);
    } else {
        send(client_sock, "Login Failed", 12, 0);
        printf("Server sent failed login response! %d", client_sock);
    }

    close(client_sock);
    free(socket_desc);
    return 0;
}

int main() {
    int server_sock, client_sock, c;
    struct sockaddr_in server, client;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, 3);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t*)&c))) {
        pthread_t client_thread;
        int *new_sock = malloc(1);
        *new_sock = client_sock;

        pthread_create(&client_thread, NULL, handle_client, (void*) new_sock);
    }

    close(server_sock);
    return 0;
}
