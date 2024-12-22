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

    sprintf(conninfo, "dbname=%s user=%s password=%s hostaddr=%s port=%s", DB_NAME, DB_USER, DB_PASSWORD, DB_HOST, DB_PORT);

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        logger("ERROR", "Connection to database failed: %s", PQerrorMessage(conn));
        disconnect_db(conn);
        return NULL;
    }else {
        logger("INFO", "Database connection established.\nDB_NAME: %s, DB_USER: %s, DB_PASSWORD: %s, DB_HOST: %s, DB_PORT: %s", DB_NAME, DB_USER, DB_PASSWORD, DB_HOST, DB_PORT);
        return conn;
    }
}


/* 
--- Disconnection from DB
*/
void disconnect_db(PGconn *conn) {
    PQfinish(conn);
    logger("INFO", "Database connection closed.");
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
            printf("[DISCONNECTED] Connection closed by client [%s]\n",client_ip);
            logger("INFO", "[DISCONNECTED] Connection closed by client [%s]",client_ip);
            remove_session(client_ip);
            close(client_fd);
            break;
        }
        
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
        if (strstr(buffer, "GET_CUSTOMERS_COMBOBOX") != NULL) {
            fetch_customers_combobox(client_fd);
        }
        if (strstr(buffer, "GET_PRODUCTS_COMBOBOX") != NULL) {
            fetch_products_combobox(client_fd);
        }
        if (strstr(buffer, "GET_ORDERS") != NULL) {
            fetch_orders(client_fd);
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

        if (strstr(buffer, "CREATE_CUSTOMER|") != NULL) {
            add_customer(buffer, client_fd);
        }
        if (strstr(buffer, "UPDATE_CUSTOMER|") != NULL) {
            update_customer(buffer, client_fd);
        }
        if (strstr(buffer, "DELETE_CUSTOMER|") != NULL) {
            delete_customer(buffer, client_fd);
        }

        if (strstr(buffer, "CREATE_ORDER|") != NULL) {
            add_order(buffer, client_fd);
        }
        if (strstr(buffer, "DELETE_ORDER|") != NULL) {
            delete_order(buffer, client_fd);
        }
        if (strstr(buffer, "CREATE_USER|") != NULL) {
            add_user(buffer, client_fd);
        }
        if (strstr(buffer, "UPDATE_USER|") != NULL) {
            update_user(buffer, client_fd);
        }
        if (strstr(buffer, "DELETE_USER|") != NULL) {
            delete_user(buffer, client_fd);
        }
    }
}


/* ******* USERS AUTH ******* */

void handle_login(const char *data, int client_fd){
    set_log_file("logs/server.log");

    char username[100], password[100];
    if (sscanf(data, "LOGIN|%99[^|]|%99[^|]", username, password) != 2) {
        logger("ERROR", "Invalid login data format: %s", data);
        fprintf(stderr, "Invalid login data format: %s\n", data);
        return;
    }

    // printf("Received login request with username: %s, password: %s\n", username, password);
    logger("INFO", "Received login request with username: %s, password: %s", username, password);

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    // Prepare a parameterized SQL query to prevent SQL injection
    const char *query = "SELECT id, username, password, role, created_at, first_name, last_name, email, phone_number FROM \"User\" WHERE username = $1 AND password = $2";
    const char *param_values[2] = {username, password};

    // Execute the query with parameters
    PGresult *res = PQexecParams(conn, query, 2, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Failed to execute query: %s", PQerrorMessage(conn));
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

        username_str = (username_str && *username_str) ? username_str : "-";
        password_str = (password_str && *password_str) ? password_str : "-";
        role_str = (role_str && *role_str) ? role_str : "-";
        created_at_str = (created_at_str && *created_at_str) ? created_at_str : "-";
        first_name_str = (first_name_str && *first_name_str) ? first_name_str : "-";
        last_name_str = (last_name_str && *last_name_str) ? last_name_str : "-";
        email_str = (email_str && *email_str) ? email_str : "-";
        phone_number_str = (phone_number_str && *phone_number_str) ? phone_number_str : "-";

        char response[512];
        snprintf(response, sizeof(response), "true|%s|%s|%s|%s|%s|%s|%s|%s|%s", 
                 client_id_str, username_str, password_str, role_str, created_at_str, first_name_str, last_name_str, email_str, phone_number_str);

        send(client_fd, response, strlen(response), 0);
        logger("INFO", "Sent user credentials to client: %s", response);

        // Get client IP address
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);
        getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, client_ip, sizeof(client_ip));

        // Add session
        bool is_added = add_session(client_ip, username);
        if (!is_added) {
            // Max session reached condition
            char response[] = "false";
            send(client_fd, response, strlen(response), 0);
            logger("ERROR", "Failed to add session for IP %s - Max sessions reached: %d/%d", client_ip, active_session_count, MAX_SESSIONS);
        } else {
            logger("INFO", "Added session for IP %s (user: %s). Active sessions: %d", 
                  client_ip, username, active_session_count);
            list_sessions();
        }

        
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
        logger("ERROR", "Invalid registration data format: %s", data);
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Get current timestamp for created_at
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(created_at, sizeof(created_at), "%Y-%m-%d %H:%M:%S", t);  // Format: YYYY-MM-DD HH:MM:SS

    // Connect to the PostgreSQL database
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        char response[] = "register_failed|db_connection_error";
        logger("ERROR", "Failed to connect to the database");
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Insert the user into the database
    const char *query = "INSERT INTO \"User\" (username, password, role, created_at) VALUES ($1, $2, $3, $4) RETURNING id";
    const char *param_values[4] = { username, password, default_role, created_at };

    PGresult *res = PQexecParams(conn, query, 4, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Failed to insert new user: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Failed to insert new user: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        char response[] = "register_failed|db_error";
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Retrieve the newly created customerID
    char *customer_id = PQgetvalue(res, 0, 0);
    logger("INFO", "Successfully registered user with ID: %s", customer_id);

    // Send success response to the client
    char response[256];
    snprintf(response, sizeof(response), "register_success|%s", customer_id);
    send(client_fd, response, strlen(response), 0);

    // Cleanup
    PQclear(res);
    PQfinish(conn);
}


/* ----------- PRODUCTS FUNCTIONS ------------ */

void fetch_products(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, name, price FROM \"Product\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
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

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}



void add_product(const char *data, int sock) {
    char name[255];
    char priceStr[10];
    if (sscanf(data, "CREATE_PRODUCT|%254[^|]|%9[^|]", name, priceStr) != 2) {
        logger("ERROR", "Invalid product creation data format: %s", data);
        fprintf(stderr, "Invalid product creation data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
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
    logger("INFO", "Product added successfully.");
    PQclear(res);
}

void update_product(const char *data, int sock) {

    char name[255], priceStr[10], idStr[10];
    if (sscanf(data, "UPDATE_PRODUCT|%9[^|]|%254[^|]|%9[^|]", idStr, name, priceStr) != 3) {
        logger("ERROR", "Invalid product update data format: %s", data);
        fprintf(stderr, "Invalid product update data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    int id = atoi(idStr);
    int price = atoi(priceStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "UPDATE \"Product\" SET name = '%s', price = %d WHERE id = %d;",
         name, price, id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Update failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Update failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Product updated successfully.");
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
        logger("ERROR", "Failed to connect to the database");
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
        logger("ERROR", "Delete failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Product deleted successfully.");
    PQclear(res);
}

/* ----------- CUSTOMERS FUNCTIONS ------------ */

void fetch_customers(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, first_name, last_name, phone_number, visited_at FROM \"Customer\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
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

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}


void add_customer(const char *data, int sock){
    char fname[255], lname[255], pnumber[20], visited_at[64];
    if (sscanf(data, "CREATE_CUSTOMER|%254[^|]|%254[^|]|%19[^|]", fname, lname, pnumber) != 3) {
        printf("CREATE_CUSTOMER: [%s][%s][%s]\n", fname, lname, pnumber);
        logger("ERROR", "Invalid product creation data format: %s", data);
        fprintf(stderr, "Invalid product creation data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(visited_at, sizeof(visited_at), "%Y-%m-%d %H:%M:%S", t);

    const char *query = "INSERT INTO \"Customer\" (first_name, last_name, phone_number, visited_at) VALUES ($1, $2, $3, $4) RETURNING id";
    const char *param_values[4] = { fname, lname, pnumber, visited_at };

    PGresult *res = PQexecParams(conn, query, 4, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Insert failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Customer added successfully.");
    PQclear(res);
}


void update_customer(const char *data, int sock) {
    char idStr[10], fname[255], lname[255], pnumber[20];
    if (sscanf(data, "UPDATE_CUSTOMER|%9[^|]|%254[^|]|%254[^|]|%19[^|]", idStr, fname, lname, pnumber) != 4) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid customer update data format: %s", data);
        fprintf(stderr, "Invalid customer update data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "UPDATE \"Customer\" SET first_name = '%s', last_name = '%s', phone_number = '%s' WHERE id = %d;",
         fname, lname, pnumber, id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Update failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Update failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Customer updated successfully.");
    PQclear(res);
}

void delete_customer(const char *data, int sock) {
    char idStr[10];
    if (sscanf(data, "DELETE_CUSTOMER|%9[^|]", idStr) != 1) {
        // printf("LOGIN_DATA: [%s][%s]\n", username, password);
        logger("ERROR", "Invalid customer delete data format: %s", data);
        fprintf(stderr, "Invalid customer delete data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "DELETE FROM \"Customer\" WHERE id = %d;", id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Delete failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Customer deleted successfully.");
    PQclear(res);
    PQfinish(conn);
}


/* ----------- ORDERS FUNCTIONS ------------ */

void fetch_orders(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    const char *query = 
        "SELECT o.id, "
        "CONCAT(c.first_name, ' ', c.last_name) AS \"Customer\", "
        "p.name AS \"Product Name\", "
        "o.price AS \"Price\", "
        "u.username AS \"User username\", "
        "o.created_at AS \"Created At\" "
        "FROM \"Order\" o "
        "JOIN \"Customer\" c ON o.customer_id = c.id "
        "JOIN \"User\" u ON o.user_id = u.id "
        "JOIN \"Product\" p ON o.product_id = p.id;";

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s,%s,%s,%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1),
                 PQgetvalue(res, i, 2),
                 PQgetvalue(res, i, 3),
                 PQgetvalue(res, i, 4),
                 PQgetvalue(res, i, 5));
    }

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}

void fetch_customers_combobox(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, CONCAT(first_name, ' ', last_name) AS \"Full Name\" FROM \"Customer\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1));
    }

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}

void fetch_products_combobox(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, name FROM \"Product\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1));
    }

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}

void add_order(const char *data, int sock){
    int user_id, customer_id, product_id, price;
    char created_at[64];
    if (sscanf(data, "CREATE_ORDER|%d|%d|%d|%d", &user_id, &customer_id, &product_id, &price) != 4) {
        logger("ERROR", "Invalid order creation data format: %s", data);
        fprintf(stderr, "Invalid order creation data format: %s\n", data);
        return;
    }

    logger("INFO", "CREATE ORDER: [%d][%d][%d][%d]", user_id, customer_id, product_id, price);

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(created_at, sizeof(created_at), "%Y-%m-%d %H:%M:%S", t);

    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO \"Order\" (user_id, customer_id, product_id, price, created_at) VALUES (%d, %d, %d, %d, '%s')",
             user_id, customer_id, product_id, price, created_at);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Insert failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Order added successfully.");
    PQclear(res);
}

void delete_order(const char *data, int sock) {
    char idStr[10];
    if (sscanf(data, "DELETE_ORDER|%9[^|]", idStr) != 1) {
        logger("ERROR", "Invalid order delete data format: %s", data);
        fprintf(stderr, "Invalid order delete data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "DELETE FROM \"Order\" WHERE id = %d;", id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Delete failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "Order deleted successfully.");
    PQclear(res);
    PQfinish(conn);
}


void fetch_order_details(int sock){
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, name FROM \"Product\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
    char buffer[5096];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "true");

    for (int i = 0; i < rows; i++) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 
                 "|%s,%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1));
    }

    send(sock, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}


/* ----------- USERS FUNCTIONS ------------ */

void fetch_users(int client_socket) {
    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    PGresult *res = PQexec(conn, "SELECT id, username, password, role, first_name, last_name, email, phone_number, created_at FROM \"User\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT query failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "SELECT query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    int rows = PQntuples(res);
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

    send(client_socket, buffer, strlen(buffer), 0);

    PQclear(res);
    PQfinish(conn);
}


void add_user(const char *data, int sock){
    char username[50], password[100], role[20], created_at[64], first_name[255], last_name[255], email[255], phone_number[20];
    if (sscanf(data, "CREATE_USER|%49[^|]|%99[^|]|%19[^|]|%254[^|]|%254[^|]|%254[^|]|%19[^|]", username, password, role, first_name, last_name, email, phone_number) != 7) {
        printf("CREATE_USER: [%s][%s][%s][%s][%s][%s][%s]\n", username, password, role, first_name, last_name, email, phone_number);
        logger("ERROR", "Invalid user creation data format: %s", data);
        fprintf(stderr, "Invalid user creation data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database in add_user\n");
        logger("ERROR", "Failed to connect to the database in add_user");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(created_at, sizeof(created_at), "%Y-%m-%d %H:%M:%S", t);

    const char *query = "INSERT INTO \"User\" (username, password, role, created_at, first_name, last_name, email, phone_number) VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING id";
    const char *param_values[8] = { username, password, role, created_at, first_name, last_name, email, phone_number };

    PGresult *res = PQexecParams(conn, query, 8, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Insert failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "User added successfully.");
    PQclear(res);
}


void update_user(const char *data, int sock){
    char idStr[10], username[50], password[10], role[20], firstname[255], lastname[255], email[255], phone_number[20];
    if (sscanf(data, "UPDATE_USER|%9[^|]|%49[^|]|%99[^|]|%19[^|]|%254[^|]|%254[^|]|%254[^|]|%19[^|]", idStr, username, password, role, firstname, lastname, email, phone_number) != 8) {
        logger("ERROR", "Invalid user update data format: %s", data);
        fprintf(stderr, "Invalid user update data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database in update_user\n");
        logger("ERROR", "Failed to connect to the database in update_user");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "UPDATE \"User\" SET username = '%s', password = '%s', role = '%s', first_name = '%s', last_name = '%s', email = '%s', phone_number = '%s' WHERE id = %d;",
         username, password, role, firstname, lastname, email, phone_number, id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Update failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Update failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "User updated successfully.");
    PQclear(res);
}


void delete_user(const char *data, int sock){
    char idStr[10];
    if (sscanf(data, "DELETE_USER|%9[^|]", idStr) != 1) {
        logger("ERROR", "Invalid user delete data format: %s", data);
        fprintf(stderr, "Invalid user delete data format: %s\n", data);
        return;
    }

    PGconn *conn = connect_to_db();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to the database\n");
        logger("ERROR", "Failed to connect to the database");
        return;
    }

    int id = atoi(idStr);

    char query[1024];
    snprintf(query, sizeof(query),
         "DELETE FROM \"User\" WHERE id = %d;", id);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete failed: %s\n", PQerrorMessage(conn));
        logger("ERROR", "Delete failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return;
    }
    logger("INFO", "User deleted successfully.");
    PQclear(res);
    PQfinish(conn);
}

