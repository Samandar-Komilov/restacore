#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <time.h>

#include "utils.h"
#include "server_functions.h"
#include "logger/logger.h"


void handle_login(const char *data, int client_fd);
void handle_register(const char *data, int client_fd);


/* 
--- Database connection
*/
PGconn *connect_to_db(){
    set_log_file("logs/server.log");

    const char* DB_NAME = getenv("DB_NAME");
    const char* DB_USER = getenv("DB_USER");
    const char* DB_PASSWORD = getenv("DB_PASSWORD");
    const char* DB_HOST = getenv("DB_HOST");
    const char* DB_PORT = getenv("DB_PORT");
    char conninfo[256];

    printf(">>> %s %s %s %s %s", DB_NAME, DB_USER, DB_PASSWORD, DB_HOST, DB_PORT);

    sprintf(conninfo, "dbname=%s user=%s password=%s hostaddr=%s port=%s", DB_NAME, DB_USER, DB_PASSWORD, DB_HOST, DB_PORT);

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
    set_log_file("logs/server.log");

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
        if (strstr(buffer, "GET_PRODUCTS") != NULL) {
            fetch_products(client_fd);
        }
        if (strstr(buffer, "GET_CUSTOMERS") != NULL) {
            fetch_customers(client_fd);
        }
        if (strstr(buffer, "GET_ORDERS") != NULL) {
            fetch_customers(client_fd);
        }
        if (strstr(buffer, "GET_USERS") != NULL) {
            fetch_users(client_fd);
        }

        if (strstr(buffer, "CREATE_PRODUCT|") != NULL) {
            add_product(buffer, client_fd);
        }
        if (strstr(buffer, "UPDATE_PRODUCT|") != NULL) {
            update_product(buffer, client_fd);
        }
        if (strstr(buffer, "DELETE_PRODUCT|") != NULL) {
            delete_product(buffer, client_fd);
        }
    }
}


/* ******* USERS AUTH ******* */

void handle_login(const char *data, int client_fd){
    set_log_file("logs/server.log");

    char username[100], password[100];
    if (sscanf(data, "LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid login data format: %s", data);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    // printf("Received login request with username: %s, password: %s\n", username, password);
    logger("INFO", "Received login request with username: %s, password: %s", username, password);

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    // Prepare a parameterized SQL query to prevent SQL injection
    const char *query = "SELECT id, username, password, role, created_at, first_name, last_name, email, phone_number FROM \"User\" WHERE username = $1 AND password = $2";
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
        const char *client_id_str = PQgetvalue(res, 0, 0);
        const char *username_str = PQgetvalue(res, 0, 1);
        const char *password_str = PQgetvalue(res, 0, 2);
        const char *role_str = PQgetvalue(res, 0, 3);
        const char *created_at_str = PQgetvalue(res, 0, 4);
        const char *first_name_str = PQgetvalue(res, 0, 5);
        const char *last_name_str = PQgetvalue(res, 0, 6);
        const char *email_str = PQgetvalue(res, 0, 7);
        const char *phone_number_str = PQgetvalue(res, 0, 8);

        username_str = (username_str && *username_str) ? username_str : "undefined";
        password_str = (password_str && *password_str) ? password_str : "undefined";
        role_str = (role_str && *role_str) ? role_str : "undefined";
        created_at_str = (created_at_str && *created_at_str) ? created_at_str : "undefined";
        first_name_str = (first_name_str && *first_name_str) ? first_name_str : "undefined";
        last_name_str = (last_name_str && *last_name_str) ? last_name_str : "undefined";
        email_str = (email_str && *email_str) ? email_str : "undefined";
        phone_number_str = (phone_number_str && *phone_number_str) ? phone_number_str : "undefined";

        char response[512];
        snprintf(response, sizeof(response), "true|%s|%s|%s|%s|%s|%s|%s|%s|%s", 
                 client_id_str, username_str, password_str, role_str, created_at_str, first_name_str, last_name_str, email_str, phone_number_str);

        send(client_fd, response, strlen(response), 0);
        // printf("[LOGIN_SUCCESS] Sent user credentials to client: %s\n", response);
        logger("INFO", "Sent user credentials to client: %s", response);
    } else {
        char response[] = "false";
        send(client_fd, response, strlen(response), 0);
        logger("INFO", "Login failed - sent 'false' response to client");
    }

    // Clean up
    PQclear(res);
    PQfinish(conn);
}

void handle_register(const char* data, int client_fd) {
    set_log_file("logs/server.log");

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
    const char *query = "INSERT INTO \"User\" (username, password, role, created_at) VALUES ($1, $2, $3, $4) RETURNING id";
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


/* ----------- PRODUCTS FUNCTIONS ------------ */

void fetch_products(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, name, price FROM \"Product\";");

    printf("PG Result got\n");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    printf("ROWS response: %d\n", rows);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1),
                 PQgetvalue(res, i, 2));
    }

    printf("RESPONSE: %s\n", buffer);

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}



void add_product(const char *data, int sock) {

    char name[255];
    char priceStr[10];
    if (sscanf(data, "CREATE_PRODUCT|%254[^|]|%9[^|]", name, priceStr) != 2) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid product creation data format: %s", data);
        fprintf(stderr, "Invalid product creation data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    int price = atoi(priceStr);

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO \"Product\" (name, price) VALUES ('%s', %d);",
             name, price);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    printf("Product added successfully.\n");
    PQclear(res);
}

void update_product(const char *data, int sock) {

    char name[255], priceStr[10], idStr[10];
    if (sscanf(data, "UPDATE_PRODUCT|%9[^|]|%254[^|]|%9[^|]", idStr, name, priceStr) != 3) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid product update data format: %s", data);
        fprintf(stderr, "Invalid product update data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    int id = atoi(idStr);
    int price = atoi(priceStr);

    printf("SERVER Update function is working.\n");

    char query[1024];
    snprintf(query, sizeof(query),
         "UPDATE \"Product\" SET name = '%s', price = %d WHERE id = %d;",
         name, price, id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Update failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    printf("Product updated successfully.\n");
    PQclear(res);
}

void delete_product(const char *data, int sock) {
    char idStr[10];
    if (sscanf(data, "DELETE_PRODUCT|%9[^|]", idStr) != 1) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid product delete data format: %s", data);
        fprintf(stderr, "Invalid product delete data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "DELETE FROM \"Product\" WHERE id = %d;",
         id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    printf("Product deleted successfully.\n");
    PQclear(res);
}

/* ----------- CUSTOMERS FUNCTIONS ------------ */

void fetch_customers(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, first_name, last_name, phone_number, visited_at FROM \"Customer\";");

    printf("PG Result got\n");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    printf("ROWS response: %d\n", rows);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s,%s,%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1),
                 PQgetvalue(res, i, 2),
                 PQgetvalue(res, i, 3),
                 PQgetvalue(res, i, 4));
    }

    printf("RESPONSE: %s\n", buffer);

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}



/* ----------- USERS FUNCTIONS ------------ */

void fetch_users(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, username, password, role, created_at, first_name, last_name, email, phone_number FROM \"User\";");

    printf("PG Result got\n");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    printf("ROWS response: %d\n", rows);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s,%s,%s,%s,%s,%s,%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1),
                 PQgetvalue(res, i, 2),
                 PQgetvalue(res, i, 3),
                 PQgetvalue(res, i, 4),
                 PQgetvalue(res, i, 5),
                 PQgetvalue(res, i, 6),
                 PQgetvalue(res, i, 7),
                 PQgetvalue(res, i, 8));
    }

    printf("RESPONSE: %s\n", buffer);

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}