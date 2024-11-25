#ifndef CLIENT_FUNCTIONS_H

/* General Handlers */
int connect_to_server();
void send_to_server(int sock, const char* data);
void disconnect_from_server(int sock);

/* User Auth handlers */
void register_user(int sock_fd, const char *username, const char *password);
void login_user(int sock_fd, const char *username, const char *password);

#endif