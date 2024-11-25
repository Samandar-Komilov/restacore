#include <sys/types.h>      // For data types like `socklen_t`
#include <sys/socket.h>     // For socket-related functions like `getpeername`
#include <netinet/in.h>     // For `struct sockaddr_in` and `AF_INET`
#include <arpa/inet.h>      // For `inet_ntop` and `INET_ADDRSTRLEN`
#include <unistd.h>         // For `recv` and `close`
#include <string.h>         // For `memset`
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <time.h>

#include "utils.h"
#include "server_functions.h"


/* 
--- Database connection
*/
PGconn *connect_to_db(){
    const char *conninfo = "dbname=bistrosync_db user=postgres password=voidpostgres hostaddr=127.0.0.1 port=5432";

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        disconnect_db(conn);
        return NULL;
    }else {
        printf("Database connection established.\n");
        return conn;
    }
}


/* 
--- Disconnection from DB
*/
void disconnect_db(PGconn *conn) {
    PQfinish(conn);
    exit(1);
}


/* 
--- General Handler for commands
 */
void *handle_command(void *client_fd_ptr){
    int client_fd = *((int*)client_fd_ptr);
    free(client_fd_ptr); 
    char buffer[MAX_BUFFER];

    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_size);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        ssize_t received_bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        if (received_bytes <= 0) {
            if (received_bytes == 0) {
                printf("[DISCONNECTED] Connection closed by client [%s]\n",client_ip);
            } else {
                perror("Error receiving data from the client");
            }
            break;
        }

        printf("[CLIENT] %s\n", buffer);
        
        /* Respond to commands accordingly */
        if (strstr(buffer, "REGISTER|") != NULL) {
            handle_register(buffer, client_fd);
        }
        if (strstr(buffer, "LOGIN|") != NULL) {
            handle_login(buffer, client_fd);
        }
    }
}


/* ******* USERS AUTH ******* */

void handle_login(const char *data, int client_fd){
    char username[100], password[100];
    if (sscanf(data, "CLIENT_LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        printf("LOGIN_DATA: [%s][%s]\n", username, password);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    printf("Received login request with username: %s, password: %s\n", username, password);

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare a parameterized SQL query to prevent SQL injection
    const char *query = "SELECT customerID FROM customers WHERE username = $1 AND password = $2";
    const char *param_values[2] = {username, password};

    // Execute the query with parameters
    PGresult *res = PQexecParams(conn, query, 2, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int num_rows = PQntuples(res);

    if (num_rows > 0) {
        // Get the customerID
        const char *client_id_str = PQgetvalue(res, 0, 0);
        int client_id = atoi(client_id_str);

        // Send success response
        char response[256];
        snprintf(response, sizeof(response), "true|%d", client_id);
        send(client_fd, response, strlen(response), 0);
        printf("[LOGIN_SUCCESS] Sent 'true' response with client ID (%d) to client\n", client_id);
    } else {
        // Send failure response
        char response[] = "false";
        send(client_fd, response, strlen(response), 0);
        printf("[LOGIN_FAILURE] Sent 'false' response to client\n");
    }

    // Clean up
    PQclear(res);
    PQfinish(conn);
}

void handle_register(const char* data, int client_fd) {
    char username[255], password[255];
    const char *default_role = "user";
    char created_at[64];

    // Parse the incoming data
    if (sscanf(data, "REGISTER|%254[^|]|%254[^|]", username, password) != 2) {
        fprintf(stderr, "Invalid registration data format: %s\n", data);
        char response[] = "register_failed|invalid_format";
        send(client_fd, response, strlen(response), 0);
        return;
    }

    printf("Registering user: %s\n", username);

    // Get current timestamp for created_at
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(created_at, sizeof(created_at), "%Y-%m-%d %H:%M:%S", t);  // Format: YYYY-MM-DD HH:MM:SS

    // Connect to the PostgreSQL database
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        char response[] = "register_failed|db_connection_error";
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Insert the user into the database
    const char *query = "INSERT INTO customers (username, password, role, created_at) VALUES ($1, $2, $3, $4) RETURNING customerID";
    const char *param_values[4] = { username, password, default_role, created_at };

    PGresult *res = PQexecParams(conn, query, 4, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to insert new user: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        char response[] = "register_failed|db_error";
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Retrieve the newly created customerID
    char *customer_id = PQgetvalue(res, 0, 0);
    printf("Successfully registered user with ID: %s\n", customer_id);

    // Send success response to the client
    char response[256];
    snprintf(response, sizeof(response), "register_success|%s", customer_id);
    send(client_fd, response, strlen(response), 0);

    // Cleanup
    PQclear(res);
    PQfinish(conn);
}



/* ******* USERS CRUD operations ******* */

void add_user(PGconn *conn, const char *username, const char *password, const char* role) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO \"User\" (username, password, role) VALUES ('%s', '%s', %s);",
             username, password, role);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("User added successfully.\n");
    PQclear(res);
}


void fetch_users(PGconn *conn) {
    PGresult *res = PQexec(conn, "SELECT id, username, role_id FROM \"User\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        printf("ID: %s, Username: %s, Role ID: %s\n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2));
    }

    PQclear(res);
}

