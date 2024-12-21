#ifndef SERVER_FUNCTIONS_H

#include <libpq-fe.h>
// #include <pthread.h>
// #include <stdbool.h>

// #define MAX_SESSIONS 10
// #define IP_LENGTH 16  // For IPv4 addresses (xxx.xxx.xxx.xxx\0)
// #define USERNAME_LENGTH 100

// typedef struct {
//     char ip_address[IP_LENGTH];
//     char username[USERNAME_LENGTH];
//     bool active;
// } Session;

// extern Session active_sessions[MAX_SESSIONS];
// extern pthread_mutex_t sessions_mutex;

// void init_sessions(void);
// void cleanup_sessions(void);
// bool add_session(const char *ip, const char *username);
// void remove_session(const char *ip);
// int get_active_session_count(void);

/* General Handlers */
PGconn *connect_to_db();
void disconnect_db(PGconn *conn);
void *handle_command(void *client_fd_ptr);
void send_response(const char* data);

/* User Auth */
void handle_login(const char *data, int client_fd); 
void handle_register(const char *data, int client_fd);

/* Product handlers */
void fetch_products(int sock);
void add_product(const char *data, int sock);
void update_product(const char *data, int sock);
void delete_product(const char *data, int sock);

/* Customer handlers */
void fetch_customers(int sock);
void add_customer(const char *data, int sock);
void update_customer(const char *data, int sock);
void delete_customer(const char *data, int sock);

/* Order handlers */
void fetch_orders(int sock);
void fetch_customers_combobox(int sock);
void fetch_products_combobox(int sock);
void fetch_order_details(int sock);
void add_order(const char *data, int sock);
void delete_order(const char *data, int sock);

/* User handlers */
void fetch_users(int sock);
void add_user(const char *data, int sock);
void update_user(const char *data, int sock);
void delete_user(const char *data, int sock);

#endif