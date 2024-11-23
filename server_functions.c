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
                // Client disconnected
                printf("[DISCONNECTED] Connection closed by client [%s]\n",client_ip);
            } else {
                perror("Error receiving data from the client");
            }
            break; // Exit the loop when the client disconnects
        }

        printf("[CLIENT] %s\n", buffer);
        // switch (receive_msg->content.command.unit){
        //     case ORDER:
        //         handle_order_requests(client_sock, send_msg, receive_msg);
        //         break;
        //     case INVENTORY:
        //         handle_inventory_requests(client_sock, send_msg, receive_msg);
        //         break;
        //     default:
        //         send_msg->content.response.success = false;
        //         strcpy(send_msg->content.response.message, "Invalid message type.");
        //         send_command(client_sock, send_msg);
        //         printf("Invalid message received from client %d\n", client_sock);
        //         break;
        // }
    }
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

