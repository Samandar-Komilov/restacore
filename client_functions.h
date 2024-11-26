#ifndef CLIENT_FUNCTIONS_H

/* General Handlers */
int connect_to_server();
int send_to_server(int sock, const char* data);
void disconnect_from_server(int sock);

/* User Auth handlers */
int register_user(int sock_fd, const char *username, const char *password);
int login_user(int sock_fd, const char *username, const char *password);

#endif