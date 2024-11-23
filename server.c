#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <libpq-fe.h>
#include <time.h>

#include "utils.h"
#include "server_functions.h"


int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];

    // Server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[LISTENING] Port Number: %d\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }
        
        // Convert the client's IP address from binary to a readable string
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

        printf("[CONNECTED] New connection from %s\n", client_ip);

        // Allocate memory for client_fd to pass it to the thread
        int* client_fd_copy = (int*)malloc(sizeof(int));
        if (client_fd_copy == NULL) {
            perror("Failed to allocate memory");
            close(client_fd);
            continue;
        }

        *client_fd_copy = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_command, client_fd_copy) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(client_fd_copy);  // Free the memory in case of failure
            continue;
        }

        pthread_detach(client_thread);
    }

    close(server_fd);

    // PGresult *res = PQexec(conn, "SELECT id, name FROM my_table");
    // if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    //     fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
    //     PQclear(res);
    //     disconnect_db(conn);
    // }

    // int nrows = PQntuples(res);
    // int nfields = PQnfields(res);
    // for (int i = 0; i < nrows; i++) {
    //     for (int j = 0; j < nfields; j++) {
    //         printf("%s\t", PQgetvalue(res, i, j));
    //     }
    //     printf("\n");
    // }

    // // Clean up
    // PQclear(res);
    // PQfinish(conn);
    return 0;
}


/* --- ORDERS CRUD --- */

// void create_order(PGconn *conn, int user_id, double total_price) {
//     char query[256];
//     snprintf(query, sizeof(query),
//              "INSERT INTO \"Order\" (user_id, total_price) VALUES (%d, %.2f);",
//              user_id, total_price);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//         fprintf(stderr, "Insert Order failed: %s\n", PQerrorMessage(conn));
//     } else {
//         printf("Order created successfully.\n");
//     }
//     PQclear(res);
// }


// void fetch_orders(PGconn *conn) {
//     PGresult *res = PQexec(conn, "SELECT id, user_id, total_price, created_at FROM \"Order\";");

//     if (PQresultStatus(res) != PGRES_TUPLES_OK) {
//         fprintf(stderr, "Fetch Orders failed: %s\n", PQerrorMessage(conn));
//     } else {
//         int rows = PQntuples(res);
//         for (int i = 0; i < rows; i++) {
//             printf("ID: %s, User ID: %s, Total Price: %s, Created At: %s\n",
//                    PQgetvalue(res, i, 0),
//                    PQgetvalue(res, i, 1),
//                    PQgetvalue(res, i, 2),
//                    PQgetvalue(res, i, 3));
//         }
//     }
//     PQclear(res);
// }


// void update_order(PGconn *conn, int order_id, double new_total_price) {
//     char query[256];
//     snprintf(query, sizeof(query),
//              "UPDATE \"Order\" SET total_price = %.2f WHERE id = %d;",
//              new_total_price, order_id);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//         fprintf(stderr, "Update Order failed: %s\n", PQerrorMessage(conn));
//     } else {
//         printf("Order updated successfully.\n");
//     }
//     PQclear(res);
// }


// void delete_order(PGconn *conn, int order_id) {
//     char query[256];
//     snprintf(query, sizeof(query), "DELETE FROM \"Order\" WHERE id = %d;", order_id);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//         fprintf(stderr, "Delete Order failed: %s\n", PQerrorMessage(conn));
//     } else {
//         printf("Order deleted successfully.\n");
//     }
//     PQclear(res);
// }


// /* --- ORDER ITEMS CRUD --- */

// void create_order_item(PGconn *conn, int order_id, int menu_item_id, int quantity, double price) {
//     char query[256];
//     snprintf(query, sizeof(query),
//              "INSERT INTO OrderItem (order_id, menu_item_id, quantity, price) VALUES (%d, %d, %d, %.2f);",
//              order_id, menu_item_id, quantity, price);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//         fprintf(stderr, "Insert OrderItem failed: %s\n", PQerrorMessage(conn));
//     } else {
//         printf("OrderItem created successfully.\n");
//     }
//     PQclear(res);
// }

// void fetch_order_items(PGconn *conn, int order_id) {
//     char query[256];
//     snprintf(query, sizeof(query), "SELECT id, menu_item_id, quantity, price FROM OrderItem WHERE order_id = %d;", order_id);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_TUPLES_OK) {
//         fprintf(stderr, "Fetch OrderItems failed: %s\n", PQerrorMessage(conn));
//     } else {
//         int rows = PQntuples(res);
//         for (int i = 0; i < rows; i++) {
//             printf("ID: %s, Menu Item ID: %s, Quantity: %s, Price: %s\n",
//                    PQgetvalue(res, i, 0),
//                    PQgetvalue(res, i, 1),
//                    PQgetvalue(res, i, 2),
//                    PQgetvalue(res, i, 3));
//         }
//     }
//     PQclear(res);
// }


// /* --- INVENTORY CRUD --- */

// void create_inventory(PGconn *conn, const char *name, int quantity, const char *unit, int threshold) {
//     char query[256];
//     snprintf(query, sizeof(query),
//              "INSERT INTO Inventory (name, quantity, unit, threshold) VALUES ('%s', %d, '%s', %d);",
//              name, quantity, unit, threshold);

//     PGresult *res = PQexec(conn, query);
//     if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//         fprintf(stderr, "Insert Inventory failed: %s\n", PQerrorMessage(conn));
//     } else {
//         printf("Inventory item created successfully.\n");
//     }
//     PQclear(res);
// }

// void fetch_inventory(PGconn *conn) {
//     PGresult *res = PQexec(conn, "SELECT id, name, quantity, unit, threshold FROM Inventory;");

//     if (PQresultStatus(res) != PGRES_TUPLES_OK) {
//         fprintf(stderr, "Fetch Inventory failed: %s\n", PQerrorMessage(conn));
//     } else {
//         int rows = PQntuples(res);
//         for (int i = 0; i < rows; i++) {
//             printf("ID: %s, Name: %s, Quantity: %s, Unit: %s, Threshold: %s\n",
//                    PQgetvalue(res, i, 0),
//                    PQgetvalue(res, i, 1),
//                    PQgetvalue(res, i, 2),
//                    PQgetvalue(res, i, 3),
//                    PQgetvalue(res, i, 4));
//         }
//     }
//     PQclear(res);
// }

/* --- INVENTORY USAGE CRUD --- */