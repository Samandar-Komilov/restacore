#include <stdbool.h>

#ifndef UTILITY_H
#define UTILITY_H

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_TOKEN_LEN 64
#define MAX_DATA_LEN 256
#define MAX_COMMAND_LEN 20



typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} LoginMessage;


typedef struct {
    char token[MAX_TOKEN_LEN];
    char command[MAX_COMMAND_LEN];
    char data[MAX_DATA_LEN];
} CommandMessage;


typedef struct {
    bool success;
    char message[MAX_DATA_LEN];
    char data[MAX_DATA_LEN];
} ResponseMessage;


typedef struct {
    LoginMessage login;
    CommandMessage command;
    ResponseMessage response;
} Message;

#endif