#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "utils.h"
#include "client_functions.h"


int connect_to_server(){
    struct sockaddr_in serv_addr;
    int sock;

    printf("Creating client socket...\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    printf("Defining client socket family: (address, port)...\n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("Client is connecting to the server...\n");
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed. Retrying...");
        sleep(1); // Wait for a while before retrying
        printf("Cennection failed. Retrying...");
    }

    printf("Connection established.\n");
    return sock;
}

int send_to_server(int sock, const char *data) {
    size_t data_len = strlen(data);
    if (send(sock, data, data_len, 0) != data_len) {
        fprintf(stderr, "Error sending data.\n");
        disconnect_from_server(sock);
        return 1;
    }
    printf("Data sent successfully. The data: %s\n", data);
    return 0;
}

int disconnect_from_server(int sock) {
    // Close the socket
    close(sock);
    printf("Disconnected from the server.\n");
    return 0;
}

int register_user(int sock_fd, const char *username, const char *password) {
    // Construct the registration message
    char message[512];
    snprintf(message, sizeof(message), "REGISTER|%s|%s", username, password);

    // Send the registration request to the server
    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send register request");
        return 1;
    }

    // Receive the server's response
    char response[512];
    int bytes_received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive register response");
        return 2;
    }
    response[bytes_received] = '\0';

    // Process the response
    if (strncmp(response, "register_success", 16) == 0) {
        printf("Registration successful! Assigned customerID: %s\n", response + 17);
        return 0;
    }
    printf("Registration failed: %s\n", response);
    return -1;
}


int login_user(int sock_fd, const char *username, const char *password) {
    // Construct the login message
    char message[512];
    snprintf(message, sizeof(message), "LOGIN|%s|%s", username, password);

    // Send the login request to the server
    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send login request");
        return 1;
    }

    // Receive the server's response
    char response[512];
    int bytes_received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive login response");
        return 2;
    }
    response[bytes_received] = '\0';

    // Process the response
    if (strncmp(response, "true", 4) == 0) {
        printf("Login successful! Assigned customerID: %s\n", response + 5);
        return 0;
    }
    printf("Login failed: Invalid username or password.\n");
    return -1;
}