#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../shared/utility.h"
#include "../shared/network.h"


void send_command(int sockfd, Message *msg){
    size_t msg_size = sizeof(Message);
    char buffer[msg_size];
    memcpy(buffer, msg, msg_size);

    if (send(sockfd, buffer, msg_size, 0) == -1){
        perror("Message Send Failed.");
    }
}


void receive_command(int sockfd, Message *msg){
    size_t msg_size = sizeof(Message);
    char buffer[msg_size];

    if (recv(sockfd, buffer, msg_size, 0) == -1){
        perror("Message Receive Failed.");
    }

    memcpy(msg, buffer, msg_size);
}