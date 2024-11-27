#include <stdbool.h>

#ifndef UTILITY_H
#define UTILITY_H

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_TOKEN_LEN 64
#define MAX_DATA_LEN 256
#define MAX_COMMAND_LEN 20



typedef enum {
    CREATE,
    READ,
    UPDATE,
    DELETE
} Command;

typedef enum {
    ORDER,
    INVENTORY,
    SALARY
} Unit;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} LoginMessage;


typedef struct {
    Unit unit;
    Command command;
    char token[MAX_TOKEN_LEN];
    char data[MAX_DATA_LEN];
} CommandMessage;


typedef struct {
    bool success;
    char message[MAX_DATA_LEN];
    char data[MAX_DATA_LEN];
} ResponseMessage;


typedef enum {
    LOGIN,
    COMMAND,
    RESPONSE
} MessageType;


typedef union {
    LoginMessage login;
    CommandMessage command;
    ResponseMessage response;
} MessageContent;


typedef struct {
    MessageType type;
    MessageContent content;
} Message;

#endif