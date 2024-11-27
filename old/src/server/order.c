#include <stdio.h>

#include "../shared/utility.h"
#include "order.h"


void handle_order_requests(int client_sock, Message *send_msg, Message *receive_msg){
    
    switch(receive_msg->content.command.command){
        case CREATE:
            create_order(client_sock, send_msg, receive_msg);
            break;
        case READ:
            read_order(client_sock, send_msg, receive_msg);
            break;
        case UPDATE:
            update_order(client_sock, send_msg, receive_msg);
            break;
        case DELETE:
            delete_order(client_sock, send_msg, receive_msg);
            break;
        default:
            send_msg->content.response.success = false;
            strcpy(send_msg->content.response.message, "Invalid command.");
            send_command(client_sock, send_msg);
            printf("Invalid command received from client %d\n", client_sock);
            break;
    }
    
    return 0;
}



void create_order(int client_sock, Message *send_msg, Message *receive_msg) {
    FILE *file = fopen("orders.txt", "a");
    if (!file) {
        perror("Failed to open orders.txt");
        send_msg->content.response.success = false;
        strcpy(send_msg->content.response.message, "Failed to create order.");
        send_command(client_sock, send_msg);
        return;
    }

    Order new_order = receive_msg->content.command.order;
    fprintf(file, "%d,%s,%s\n", new_order.order_id, new_order.customer_name, new_order.order_status);

    fclose(file);

    send_msg->content.response.success = true;
    strcpy(send_msg->content.response.message, "Order created successfully.");
    send_command(client_sock, send_msg);
}



void read_order(int client_sock, Message *send_msg, Message *receive_msg){
    return 0;
}


void update_order(int client_sock, Message *send_msg, Message *receive_msg){
    return 0;
}


void delete_order(int client_sock, Message *send_msg, Message *receive_msg){
    return 0;
}