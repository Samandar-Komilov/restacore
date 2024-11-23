#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef UTILS_H

#define PORT 8080
#define MAX_BUFFER 1024
#define SERVER_ADDRESS "192.168.43.154"


typedef struct {
    int id;
    char username[50];
    char password[100];
    char role;
    time_t created_at;
} User;

typedef struct {
    int id;
    int user_id;
    double total_price;
    time_t created_at;
} Order;

typedef struct {
    int id;
    int order_id;
    int menu_item_id;
    int quantity;
    double price;
} OrderItem;

typedef struct {
    int id;
    char name[100]; 
    double price; 
    int inventory_id;
} MenuItem;


typedef struct {
    int id;
    char name[100];
    int quantity;
    char unit[20];
    int threshold;
} Inventory;

typedef struct {
    int id;
    int menu_item_id; // Foreign key to MenuItem.id
    int inventory_id; // Foreign key to Inventory.id
    int quantity_used; // Quantity of inventory used
} InventoryUsage;

#endif