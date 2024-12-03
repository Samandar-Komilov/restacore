#ifndef CLIENT_FUNCTIONS_H

/* General Handlers */
int connect_to_server();
int send_to_server(int sock, const char* data);
int disconnect_from_server(int sock);

/* User Auth handlers */
int register_user(int sock_fd, const char *username, const char *password);
char* login_user(int sock_fd, const char *username, const char *password);

/* Product handlers */
char* get_products(int sock_fd);

/* Customers handlers */
char* get_customers(int sock_fd);

#endif