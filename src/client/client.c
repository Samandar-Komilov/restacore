#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/network.h"

int main() {
    int sock;
    struct sockaddr_in server;
    char message[MAX_BUFFER], response[MAX_BUFFER];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    // Take user input
    printf("Enter username: ");
    scanf("%s", message);
    strcat(message, " ");
    printf("Enter password: ");
    scanf("%s", message + strlen(message));

    send(sock, message, strlen(message), 0);
    recv(sock, response, MAX_BUFFER, 0);

    printf("%s\n", response);
    close(sock);
    return 0;
}
