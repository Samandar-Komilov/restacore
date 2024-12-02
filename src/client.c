#include <stdio.h>
#include <string.h>
#include <gtk/gtkx.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "utils.h"
#include "client_functions.h"
#include "logger/logger.h"

// Global variables
int sock;
User current_user = {0};
char id[100];
char username[50];
char password[100];
char role[20];
char created_at[64];
char first_name[255];
char last_name[255];
char email[255];
char phone_number[20];
char status[10];

GtkBuilder *auth_builder;
GtkBuilder *main_builder;
GtkBuilder *products_builder;

GtkWidget *WelcomePage, *RegisterPage, *LoginPage, *EmptyField, *IncorrectPassword;
GtkWidget *MainPage;
GtkWidget *ProductListPage;


/* GTK handlers */
void on_login_button_main_clicked();
void on_register_button_main_clicked();

void on_register_button_clicked();
void on_register_back_clicked();
void on_login_button_clicked();
void on_login_back_clicked();

void on_orders_btn_clicked();

void on_products_btn_clicked();
void setup_products_treeview(GtkTreeView *products_tv);

void on_tables_btn_clicked();

void on_users_btn_clicked();


/* ----------------- MAIN */

int main(int argc, char* argv[]){
    gtk_init(&argc, &argv);
    init_config();

    set_log_file("logs/client.log");

    auth_builder = gtk_builder_new_from_file("glade/register.glade");
    main_builder = gtk_builder_new_from_file("glade/main_page.glade");
    products_builder = gtk_builder_new_from_file("glade/products.glade");

    WelcomePage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "welcome_page"));
    RegisterPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Register"));
    LoginPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Login"));
    IncorrectPassword = GTK_WIDGET(gtk_builder_get_object(auth_builder, "IncorrectPassword"));
    EmptyField = GTK_WIDGET(gtk_builder_get_object(auth_builder, "EmptyField"));

    MainPage = GTK_WIDGET(gtk_builder_get_object(main_builder, "MainPage"));

    ProductListPage = GTK_WIDGET(gtk_builder_get_object(products_builder, "ProductListPage"));

    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(RegisterPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); //..
    g_signal_connect(LoginPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); // will be changed
    g_signal_connect(MainPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ProductListPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Welcome Page data
    GtkWidget *register_button_main = GTK_WIDGET(gtk_builder_get_object(auth_builder, "register_button_main"));
    GtkWidget *login_button_main = GTK_WIDGET(gtk_builder_get_object(auth_builder, "login_button_main"));
    g_signal_connect(register_button_main, "clicked", G_CALLBACK(on_register_button_main_clicked), NULL);
    g_signal_connect(login_button_main, "clicked", G_CALLBACK(on_login_button_main_clicked), NULL);

    // Register Page data
    GtkWidget *register_button = GTK_WIDGET(gtk_builder_get_object(auth_builder, "register_button"));
    GtkWidget *register_back = GTK_WIDGET(gtk_builder_get_object(auth_builder, "register_back"));
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), NULL);
    g_signal_connect(register_back, "clicked", G_CALLBACK(on_register_back_clicked), NULL);

    // Login Page data
    GtkWidget *login_button = GTK_WIDGET(gtk_builder_get_object(auth_builder, "login_button"));
    GtkWidget *login_back = GTK_WIDGET(gtk_builder_get_object(auth_builder, "login_back"));
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), NULL);
    g_signal_connect(login_back, "clicked", G_CALLBACK(on_login_back_clicked), NULL);

    // Main Page data
    GtkWidget *profile_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_btn"));
    GtkWidget *profile_label = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_label"));
    GtkWidget *orders_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "orders_btn"));
    GtkWidget *products_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "products_btn"));
    GtkWidget *tables_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "tables_btn"));
    GtkWidget *users_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "users_btn"));
    g_signal_connect(orders_btn, "clicked", G_CALLBACK(on_orders_btn_clicked), NULL);
    g_signal_connect(products_btn, "clicked", G_CALLBACK(on_products_btn_clicked), NULL);
    g_signal_connect(tables_btn, "clicked", G_CALLBACK(on_tables_btn_clicked), NULL);
    g_signal_connect(users_btn, "clicked", G_CALLBACK(on_users_btn_clicked), NULL);

    // Products Page data
    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(products_builder, "product_liststore"));
    GtkWidget *profile_btn_products = GTK_WIDGET(gtk_builder_get_object(products_builder, "profile_btn"));
    GtkWidget *profile_label_products = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_label"));
    GtkWidget *active_sessions_btn_products = GTK_WIDGET(gtk_builder_get_object(products_builder, "active_sessions_btn"));
    // GtkWidget *orders_btn_products = GTK_WIDGET(gtk_builder_get_object(products_builder, "orders_btn_products"));


    sock = connect_to_server();

    gtk_widget_show_all(WelcomePage);
    gtk_main();

    return 0;
}




/* Welcome Page Handlers */

void on_login_button_main_clicked() {
    gtk_widget_hide(WelcomePage);
    gtk_widget_show(LoginPage);
}

void on_register_button_main_clicked() {
    gtk_widget_hide(WelcomePage);
    gtk_widget_show(RegisterPage);
}


/* Register Page Handlers */

void on_register_button_clicked() {
    GtkEntry *entry_username_register = GTK_ENTRY(gtk_builder_get_object(auth_builder, "username_register"));
    GtkEntry *entry_password_register = GTK_ENTRY(gtk_builder_get_object(auth_builder, "password_register"));
    GtkEntry *entry_confirm_password_register = GTK_ENTRY(gtk_builder_get_object(auth_builder, "confirm_password_register"));

    const char *username_register = gtk_entry_get_text(GTK_ENTRY(entry_username_register));
    const char *password_register = gtk_entry_get_text(GTK_ENTRY(entry_password_register));
    const char *confirm_password_register = gtk_entry_get_text(GTK_ENTRY(entry_confirm_password_register));

    printf("DEBUG::: Register/Register button clicked, credentials taken.");

    if (strlen(username_register) == 0 || strlen(password_register) == 0 || strlen(confirm_password_register) == 0) {
        g_print("All fields must be filled out.\n");
        gtk_widget_show(EmptyField);
        return;
    }

    if (strcmp(password_register, confirm_password_register) != 0) {
        g_print("Passwords do not match.\n");
        gtk_widget_show(IncorrectPassword);
        return;
    }

    printf("DEBUG::: Register/Register button clicked, validations approved.");

    // Send registration data to the server
    int status = register_user(sock, username_register, password_register);

    switch (status) {
    case 0:
        g_print("Registration successful. Redirecting to login page...\n");
        sleep(1);
        gtk_widget_hide (GTK_WIDGET(RegisterPage));
        gtk_widget_show (GTK_WIDGET(LoginPage));
        break;
    case 1:
        g_print("Failed to send register request.\n");
        break;
    case 2:
        g_print("Failed to receive register response.\n");
        break;
    default:
        g_print("Registration failed.\n");
        break;
    }
}

void on_register_back_clicked() {
    gtk_widget_hide (GTK_WIDGET(RegisterPage));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}


/* Login Page Handlers */

void on_login_button_clicked() {
    GtkEntry *entry_username_login = GTK_ENTRY(gtk_builder_get_object(auth_builder, "username_login"));
    GtkEntry *entry_password_login = GTK_ENTRY(gtk_builder_get_object(auth_builder, "password_login"));
    GtkEntry *entry_confirm_password_login = GTK_ENTRY(gtk_builder_get_object(auth_builder, "confirm_password_login"));

    const char *username_login = gtk_entry_get_text(GTK_ENTRY(entry_username_login));
    const char *password_login = gtk_entry_get_text(GTK_ENTRY(entry_password_login));
    const char *confirm_password_login = gtk_entry_get_text(GTK_ENTRY(entry_confirm_password_login));

    // printf("DEBUG::: Login/Login button clicked, credentials taken.");
    logger("INFO", "Login/Login button clicked, credentials taken.");

    // Validations
    if (strlen(username_login) == 0 || strlen(password_login) == 0) {
        g_print("All fields must be filled out.\n");
        gtk_widget_show(EmptyField);
        return;
    }

    if (strcmp(password_login, confirm_password_login) != 0) {
        g_print("Passwords do not match.\n");
        gtk_widget_show(IncorrectPassword);
        return;
    }

    logger("INFO", "Login/Login button clicked, validations approved.");

    char* response = login_user(sock, username_login, password_login);

    printf("RESPONSE: %s\n", response);

    if (strcmp(response, "1") == 0) {
        g_print("Failed to send login request.\n");
    } else if (strcmp(response, "2") == 0) {
        g_print("Failed to receive login response.\n");
    } else if (strcmp(response, "-1") == 0) {
        g_print("Login failed. Invalid username or password. \n");
        gtk_widget_show(IncorrectPassword);
    }

    if (strncmp(response, "true", 4) == 0) {
        // Create a copy of the response to work with strtok
        char response_copy[MAX_BUFFER];
        strncpy(response_copy, response, MAX_BUFFER);

        char *token = strtok(response_copy, "|");  // Skip "true"
        if (token == NULL) {
            printf("Invalid response format\n");
            return;
        }

        // Extract and assign each token to the corresponding field
        token = strtok(NULL, "|");  // ID
        current_user.id = token ? atoi(token) : -1;

        token = strtok(NULL, "|");  // Username
        strncpy(current_user.username, token ? token : "undefined", sizeof(current_user.username));

        token = strtok(NULL, "|");  // Password
        strncpy(current_user.password, token ? token : "undefined", sizeof(current_user.password));

        token = strtok(NULL, "|");  // Role
        strncpy(current_user.role, token ? token : "undefined", sizeof(current_user.role));

        token = strtok(NULL, "|");  // Created_at
        strncpy(current_user.created_at, token ? token : "undefined", sizeof(current_user.created_at));

        token = strtok(NULL, "|");  // First Name
        strncpy(current_user.first_name, token ? token : "undefined", sizeof(current_user.first_name));

        token = strtok(NULL, "|");  // Last Name
        strncpy(current_user.last_name, token ? token : "undefined", sizeof(current_user.last_name));

        token = strtok(NULL, "|");  // Email
        strncpy(current_user.email, token ? token : "undefined", sizeof(current_user.email));

        token = strtok(NULL, "|");  // Phone Number
        strncpy(current_user.phone_number, token ? token : "undefined", sizeof(current_user.phone_number));


        printf("Login successful! ID: %d, Username: %s\n", current_user.id, current_user.username);
        // logger("INFO", "Login successful! ID: %d, Username: %s\n", current_user->id, current_user->username);

        // Show Main Page with logged-in user credentials
        GtkWidget *profile_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_btn"));
        GtkWidget *profile_label = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_label"));
        gtk_label_set_text(GTK_LABEL(profile_label), current_user.username);

        gtk_widget_hide(LoginPage);
        gtk_widget_show(MainPage);
    };

}

void on_login_back_clicked() {
    gtk_widget_hide (GTK_WIDGET(LoginPage));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}


/* ----------------- MAIN PAGE */

void on_orders_btn_clicked(){
}

void on_products_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(MainPage));
    gtk_widget_show (GTK_WIDGET(ProductListPage));

    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    setup_products_treeview(products_tv);
}

void setup_products_treeview(GtkTreeView *treeview) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // Column for Product ID
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(treeview, column);

    // Column for Product Name
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(treeview, column);

    // Column for Product Price
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(treeview, column);
}

void on_tables_btn_clicked(){

}

void on_users_btn_clicked(){

}