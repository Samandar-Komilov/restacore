#include <stdio.h>
#include <string.h>
// #include <gtk/gtkx.h>
// #include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "utils.h"


int id; // user_id global variable
char* role; // user role global variable


/* Prototypes */
int connect_to_server();
void send_to_server(int sock, const char* data);
void disconnect_from_server(int sock);


int main(int argc, char* argv[]){
    int sock = connect_to_server();

    // GTK related data

    return 0;
}


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
    }

    printf("Connection established.\n");
    return sock;
}

void send_to_server(int sock, const char *data) {
    size_t data_len = strlen(data);
    if (send(sock, data, data_len, 0) != data_len) {
        fprintf(stderr, "Error sending data.\n");
        // Handle the error, close the connection, or take appropriate action
    } else {
        printf("Data sent successfully. The data: %s\n", data);
    }
}

void disconnect_from_server(int sock) {
    // Close the socket
    close(sock);
    printf("Disconnected from the server.\n");
}