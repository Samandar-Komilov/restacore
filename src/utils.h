#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "dotenv/dotenv.h"

#ifndef UTILS_H
#define UTILS_H

#define MAX_SESSIONS 10
#define USERNAME_LENGTH 50
#define IP_LENGTH 16
// #define PORT 8088
// #define MAX_BUFFER 1024
// #define SERVER_ADDRESS "192.168.0.105"

extern const char* SERVER_ADDRESS;
extern int PORT;
extern int MAX_BUFFER;
extern int active_session_count;
extern pthread_mutex_t sessions_mutex;

void init_config();

/* -------------------------------------- */

typedef struct {
    char ip_address[IP_LENGTH];
    char username[USERNAME_LENGTH];
    bool active;
} Session;

extern Session active_sessions[MAX_SESSIONS];
void copy_sessions(Session* active_sessions_copy);

bool add_session(const char *ip, const char *username);
void remove_session(const char *ip);
void list_sessions();
char* list_sessions_client();


typedef struct {
    int id;
    char username[50];
    char password[100];
    char role[20];
    char created_at[64];
    char first_name[255];
    char last_name[255];
    char email[255];
    char phone_number[20];
} User;

typedef struct {
    int id;
    char name[255];
    int price;
} Product;

typedef struct {
    int id;
    int user_id;
    double total_price;
    time_t created_at;
    int customer_id;
} Order;

typedef struct {
    int id;
    int order_id;
    int quantity;
    int price;
    int product_id;
} OrderItem;

typedef struct {
    int id;
    char first_name[255];
    char last_name[255];
    char phone_number[20];
    time_t visited_at;
} Customer;

#endif