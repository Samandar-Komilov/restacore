#include "utility.h"

#ifndef NETWORK_H
#define NETWORK_H

#define PORT 8080
#define MAX_BUFFER 1024

void send_command(int sockfd, Message *msg);

void receive_command(int sockfd, Message *msg);

#endif
