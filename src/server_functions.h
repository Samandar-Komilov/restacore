#ifndef SERVER_FUNCTIONS_H

#include <libpq-fe.h>

/* General Handlers */
PGconn *connect_to_db();
void disconnect_db(PGconn *conn);
void *handle_command(void *client_fd_ptr);
void send_response(const char* data);

/* User Auth */
void handle_login(const char *data, int client_fd);

/* User handlers */
void add_user(PGconn *conn, const char *username, const char *password, const char* role);

/* Product handlers */
void fetch_products(int sock);
void add_product(const char *data, int sock);
void update_product(const char *data, int sock);
void delete_product(const char *data, int sock);

/* Customer handlers */
void fetch_customers(int sock);

/* User handlers */
void fetch_users(int sock);

#endif