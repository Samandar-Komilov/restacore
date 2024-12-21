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
int add_product(int sock_fd, const char *name, const int price);
int update_product(int sock_fd, int id, const char *name, const int price);
int delete_product(int sock_fd, int id);

/* Customers handlers */
char* get_customers(int sock_fd);
int add_customer(int sock_fd, const char *fname, const char *lname, const char *phone_number);
int update_customer(int sock_fd, int id, const char *fname, const char *lname, const char *phone_number);
int delete_customer(int sock_fd, int id);

/* Orders handlers */
char* get_orders(int sock_fd);
char* get_customers_combobox(int sock_fd);
char* get_products_combobox(int sock_fd);
char* get_order_details(int sock_fd);
int add_order(int sock_fd, const int user_id, const int customer_id, const int product_id, const int price);
int delete_order(int sock_fd, int id);

/* Users handlers */
char* get_users(int sock_fd);
int add_user(
    int sock_fd, 
    const char *username, 
    const char *password, 
    const char *role,
    const char *first_name,
    const char *last_name,
    const char *email,
    const char *phone_number
    );
int update_user(int sock_fd, int id, const char *username, const char *password, const char *role, const char *first_name, const char *last_name, const char *email, const char *phone_number);
int delete_user(int sock_fd, int id);

#endif