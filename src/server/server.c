#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../shared/auth.h"
#include "../shared/network.h"
#include "order.h"


void *handle_client(void *socket_desc);
void *handle_login(int client_sock, Message *send_msg, Message *receive_msg);
void *handle_command(int client_sock, Message *send_msg, Message *receive_msg);

void handle_inventory_requests(int client_sock, Message *send_msg, Message *receive_msg);

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



void *handle_client(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[MAX_BUFFER];

    Message receive_msg, send_msg;

    // Receive credentials
    receive_command(client_sock, &receive_msg);
    printf("Server scanned the message...\n");

    switch (receive_msg.type){
        case LOGIN:
            handle_login(client_sock, &send_msg, &receive_msg);
            break;
        case COMMAND:
            handle_command(client_sock, &send_msg, &receive_msg);
            break;
        default:
            send_msg.content.response.success = false;
            strcpy(send_msg.content.response.message, "Invalid message type.");
            send_command(client_sock, &send_msg);
            printf("Invalid message received from client %d\n", client_sock);
            break;
    }

    close(client_sock);
    free(socket_desc);
    return 0;
}


void *handle_login(int client_sock, Message *send_msg, Message *receive_msg){
    send_msg->type = RESPONSE;
    
    if (authenticate_user(receive_msg->content.login.username, receive_msg->content.login.password)) {
        send_msg->content.response.success = true;
        strcpy(send_msg->content.response.message, "Login Successful!");
        send_command(client_sock, send_msg);
        printf("Server sent successful login response! %d\n", client_sock);
    } else {
        send_msg->content.response.success = false;
        strcpy(send_msg->content.response.message, "Login Failed - Wrong credentials!");
        // send(client_sock, "Login Failed", 12, 0);
        send_command(client_sock, send_msg);
        printf("Server sent failed login response! %d\n", client_sock);
    }
    return 0;
}


void *handle_command(int client_sock, Message *send_msg, Message *receive_msg){
    
    switch (receive_msg->content.command.unit){
        case ORDER:
            handle_order_requests(client_sock, send_msg, receive_msg);
            break;
        case INVENTORY:
            handle_inventory_requests(client_sock, send_msg, receive_msg);
            break;
        default:
            send_msg->content.response.success = false;
            strcpy(send_msg->content.response.message, "Invalid message type.");
            send_command(client_sock, send_msg);
            printf("Invalid message received from client %d\n", client_sock);
            break;
    }
}

void handle_inventory_requests(int client_sock, Message *send_msg, Message *receive_msg){
}