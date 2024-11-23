#ifndef SERVER_FUNCTIONS_H

#include <libpq-fe.h>

/* General Handlers */
PGconn *connect_to_db();
void disconnect_db(PGconn *conn);
void *handle_command(void *client_fd_ptr);
void send_response(const char* data);


/* User handlers */
void add_user(PGconn *conn, const char *username, const char *password, const char* role);
void fetch_users(PGconn *conn);
void get_user(PGconn *conn);

#endif