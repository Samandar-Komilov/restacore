#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>

#include "utils.h"


/* Prototypes */
void connect_to_db();
void disconnect_db(PGconn *conn);

int main() {
    

    PGresult *res = PQexec(conn, "SELECT id, name FROM my_table");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        disconnect_db(conn);
    }

    int nrows = PQntuples(res);
    int nfields = PQnfields(res);
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < nfields; j++) {
            printf("%s\t", PQgetvalue(res, i, j));
        }
        printf("\n");
    }

    // Clean up
    PQclear(res);
    PQfinish(conn);
    return 0;
}

void connect_to_db(){
    const char *conninfo = "dbname=bistrosync_db user=postgres password=voidpostgres hostaddr=127.0.0.1 port=5432";

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        exit_nicely(conn);
        return NULL;
    }else {
        printf("Database connection established.\n");
    }
}

void disconnect_db(PGconn *conn) {
    PQfinish(conn);
    exit(1);
}


/* --- USERS CRUD --- */


void add_user(PGconn *conn, const char *username, const char *password, int role_id) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO \"User\" (username, password, role_id) VALUES ('%s', '%s', %d);",
             username, password, role_id);

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


/* --- ORDERS CRUD --- */

void create_order(PGconn *conn, int user_id, double total_price) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO \"Order\" (user_id, total_price) VALUES (%d, %.2f);",
             user_id, total_price);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert Order failed: %s\n", PQerrorMessage(conn));
    } else {
        printf("Order created successfully.\n");
    }
    PQclear(res);
}


void fetch_orders(PGconn *conn) {
    PGresult *res = PQexec(conn, "SELECT id, user_id, total_price, created_at FROM \"Order\";");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Fetch Orders failed: %s\n", PQerrorMessage(conn));
    } else {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            printf("ID: %s, User ID: %s, Total Price: %s, Created At: %s\n",
                   PQgetvalue(res, i, 0),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 2),
                   PQgetvalue(res, i, 3));
        }
    }
    PQclear(res);
}


void update_order(PGconn *conn, int order_id, double new_total_price) {
    char query[256];
    snprintf(query, sizeof(query),
             "UPDATE \"Order\" SET total_price = %.2f WHERE id = %d;",
             new_total_price, order_id);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Update Order failed: %s\n", PQerrorMessage(conn));
    } else {
        printf("Order updated successfully.\n");
    }
    PQclear(res);
}


void delete_order(PGconn *conn, int order_id) {
    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM \"Order\" WHERE id = %d;", order_id);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Delete Order failed: %s\n", PQerrorMessage(conn));
    } else {
        printf("Order deleted successfully.\n");
    }
    PQclear(res);
}


/* --- ORDER ITEMS CRUD --- */

void create_order_item(PGconn *conn, int order_id, int menu_item_id, int quantity, double price) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO OrderItem (order_id, menu_item_id, quantity, price) VALUES (%d, %d, %d, %.2f);",
             order_id, menu_item_id, quantity, price);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert OrderItem failed: %s\n", PQerrorMessage(conn));
    } else {
        printf("OrderItem created successfully.\n");
    }
    PQclear(res);
}

void fetch_order_items(PGconn *conn, int order_id) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT id, menu_item_id, quantity, price FROM OrderItem WHERE order_id = %d;", order_id);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Fetch OrderItems failed: %s\n", PQerrorMessage(conn));
    } else {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            printf("ID: %s, Menu Item ID: %s, Quantity: %s, Price: %s\n",
                   PQgetvalue(res, i, 0),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 2),
                   PQgetvalue(res, i, 3));
        }
    }
    PQclear(res);
}


/* --- INVENTORY CRUD --- */

void create_inventory(PGconn *conn, const char *name, int quantity, const char *unit, int threshold) {
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO Inventory (name, quantity, unit, threshold) VALUES ('%s', %d, '%s', %d);",
             name, quantity, unit, threshold);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert Inventory failed: %s\n", PQerrorMessage(conn));
    } else {
        printf("Inventory item created successfully.\n");
    }
    PQclear(res);
}

void fetch_inventory(PGconn *conn) {
    PGresult *res = PQexec(conn, "SELECT id, name, quantity, unit, threshold FROM Inventory;");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Fetch Inventory failed: %s\n", PQerrorMessage(conn));
    } else {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
            printf("ID: %s, Name: %s, Quantity: %s, Unit: %s, Threshold: %s\n",
                   PQgetvalue(res, i, 0),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 2),
                   PQgetvalue(res, i, 3),
                   PQgetvalue(res, i, 4));
        }
    }
    PQclear(res);
}

/* --- INVENTORY USAGE CRUD --- */