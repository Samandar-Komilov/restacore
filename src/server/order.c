#include <stdio.h>

#include "../shared/utility.h"
#include "order.h"


void handle_order_requests(int client_sock, Message *send_msg, Message *receive_msg){
    
    switch(receive_msg->content.command.command){
        case CREATE:
            create_order(client_sock, &send_msg, &receive_msg);
            break;
        case READ:
            read_order(client_sock, &send_msg, &receive_msg);
            break;
        case UPDATE:
            update_order(client_sock, &send_msg, &receive_msg);
            break;
        case DELETE:
            delete_order(client_sock, &send_msg, &receive_msg);
            break;
        default:
            send_msg->content.response.success = false;
            strcpy(send_msg->content.response.message, "Invalid command.");
            send_command(client_sock, &send_msg);
            printf("Invalid command received from client %d\n", client_sock);
            break;
    }
    
    return 0;
}



void create_order(int client_sock, Message *send_msg, Message *receive_msg){
    return 0;
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