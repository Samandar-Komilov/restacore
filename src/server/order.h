#include "../shared/utility.h"

#ifndef ORDER_H
#define ORDER_H


typedef struct {
    int order_id;
    char customer_name[50];
    char order_status[20];
} Order;


void handle_order_requests(int client_sock, Message *send_msg, Message *receive_msg);

void create_order(int client_sock, Message *send_msg, Message *receive_msg);
void read_order(int client_sock, Message *send_msg, Message *receive_msg);
void update_order(int client_sock, Message *send_msg, Message *receive_msg);
void delete_order(int client_sock, Message *send_msg, Message *receive_msg);


#endif