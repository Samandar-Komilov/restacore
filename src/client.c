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

GtkBuilder *auth_builder;
GtkBuilder *main_builder;
GtkBuilder *products_builder;
GtkBuilder *customers_builder;
GtkBuilder *users_builder;
GtkBuilder *profile_builder;

GtkWidget *WelcomePage, *RegisterPage, *LoginPage, *EmptyField, *IncorrectPassword;
GtkWidget *MainPage;
GtkWidget *ProductListPage, *AddProductPopup, *UpdateDeleteProductPopup;
GtkWidget *CustomersListPage, *AddCustomerPopup, *UpdateDeleteCustomerPopup;
GtkWidget *UsersListPage;
GtkWidget *ProfilePage;


/* Auth handler prototypes */
void on_login_button_main_clicked();
void on_register_button_main_clicked();

void on_register_button_clicked();
void on_register_back_clicked();
void on_login_button_clicked();
void on_login_back_clicked();


/* Orders handler prototypes */
void on_orders_btn_clicked();

/* Products handler prototypes */
void on_products_btn_clicked();
void on_products_back_clicked();
void setup_products_treeview(GtkTreeView *treeview, char *response);
void on_add_product_btn_clicked();
void on_save_product_btn_clicked();
void on_cancel_product_btn_clicked();
void on_product_row_double_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_update_product_btn_clicked();
void on_delete_product_btn_clicked();
void on_cancel2_product_btn_clicked();

/* Customers handler prototypes */
void on_customers_btn_clicked();
void on_customers_back_btn_clicked();
void setup_customers_treeview(GtkTreeView *treeview, char *response);
void on_add_customer_btn_clicked();
void on_save_customer_btn_clicked();
void on_cancel_customer_btn_clicked();
void on_customer_row_double_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_update_customer_btn_clicked();
void on_delete_customer_btn_clicked();
void on_cancel2_customer_btn_clicked();

/* Users handler prototypes */
void on_users_btn_clicked();
void on_users_back_btn_clicked();
void setup_users_treeview(GtkTreeView *treeview, char *response);
void on_add_user_btn_clicked();
void on_save_user_btn_clicked();
void on_cancel_user_btn_clicked();
void on_user_row_double_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_update_user_btn_clicked();
void on_delete_user_btn_clicked();
void on_cancel2_user_btn_clicked();

/* Profile handler prototypes */
void on_profile_btn_clicked();
void on_profile_back_btn_clicked();


/* ----------------- MAIN */

int main(int argc, char* argv[]){
    gtk_init(&argc, &argv);
    init_config();

    set_log_file("logs/client.log");

    auth_builder = gtk_builder_new_from_file("glade/register.glade");
    main_builder = gtk_builder_new_from_file("glade/main_page.glade");
    products_builder = gtk_builder_new_from_file("glade/products.glade");
    customers_builder = gtk_builder_new_from_file("glade/customer.glade");
    users_builder = gtk_builder_new_from_file("glade/users.glade");
    profile_builder = gtk_builder_new_from_file("glade/profile.glade");

    WelcomePage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "welcome_page"));
    RegisterPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Register"));
    LoginPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Login"));
    IncorrectPassword = GTK_WIDGET(gtk_builder_get_object(auth_builder, "IncorrectPassword"));
    EmptyField = GTK_WIDGET(gtk_builder_get_object(auth_builder, "EmptyField"));

    MainPage = GTK_WIDGET(gtk_builder_get_object(main_builder, "MainPage"));

    ProductListPage = GTK_WIDGET(gtk_builder_get_object(products_builder, "ProductListPage"));
    AddProductPopup = GTK_WIDGET(gtk_builder_get_object(products_builder, "AddProductPopup"));
    UpdateDeleteProductPopup = GTK_WIDGET(gtk_builder_get_object(products_builder, "UpdateDeleteProductPopup"));

    CustomersListPage = GTK_WIDGET(gtk_builder_get_object(customers_builder, "CustomerListPage"));
    AddCustomerPopup = GTK_WIDGET(gtk_builder_get_object(customers_builder, "AddCustomerPopup"));
    UpdateDeleteCustomerPopup = GTK_WIDGET(gtk_builder_get_object(customers_builder, "UpdateDeleteCustomerPopup"));

    UsersListPage = GTK_WIDGET(gtk_builder_get_object(users_builder, "MainPage"));

    ProfilePage = GTK_WIDGET(gtk_builder_get_object(profile_builder, "MainPage"));

    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(RegisterPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); //..
    g_signal_connect(LoginPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); // will be changed
    g_signal_connect(MainPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ProductListPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(AddProductPopup, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(UpdateDeleteProductPopup, "destroy", G_CALLBACK(on_cancel2_product_btn_clicked), NULL);

    g_signal_connect(CustomersListPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(AddCustomerPopup, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(UpdateDeleteCustomerPopup, "destroy", G_CALLBACK(on_cancel2_customer_btn_clicked), NULL);

    g_signal_connect(UsersListPage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(ProfilePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);

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
    GtkWidget *products_back_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "products_back_btn"));
    GtkWidget *customers_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "customers_btn"));
    GtkWidget *users_btn = GTK_WIDGET(gtk_builder_get_object(main_builder, "users_btn"));
    g_signal_connect(orders_btn, "clicked", G_CALLBACK(on_orders_btn_clicked), NULL);
    g_signal_connect(products_btn, "clicked", G_CALLBACK(on_products_btn_clicked), NULL);
    g_signal_connect(products_back_btn, "clicked", G_CALLBACK(on_products_back_clicked), NULL);
    g_signal_connect(customers_btn, "clicked", G_CALLBACK(on_customers_btn_clicked), NULL);
    g_signal_connect(users_btn, "clicked", G_CALLBACK(on_users_btn_clicked), NULL);
    g_signal_connect(profile_btn, "clicked", G_CALLBACK(on_profile_btn_clicked), NULL);

    // Products Page data
    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(products_builder, "product_liststore"));
    GtkWidget *profile_btn_products = GTK_WIDGET(gtk_builder_get_object(products_builder, "profile_btn"));
    GtkWidget *profile_label_products = GTK_WIDGET(gtk_builder_get_object(main_builder, "profile_label"));
    GtkWidget *add_product_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "add_product_btn"));
    GtkWidget *active_sessions_btn_products = GTK_WIDGET(gtk_builder_get_object(products_builder, "active_sessions_btn"));
    g_signal_connect(add_product_btn, "clicked", G_CALLBACK(on_add_product_btn_clicked), NULL);
    g_signal_connect(products_tv, "row-activated", G_CALLBACK(on_product_row_double_clicked), NULL);
    // CRUD product Popup data
    GtkWidget *create_product_popup_save_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "create_product_popup_save_btn"));
    GtkWidget *create_product_popup_cancel_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "create_product_popup_cancel_btn"));
    GtkWidget *update_delete_product_popup_apply_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_apply_btn"));
    GtkWidget *update_delete_product_popup_delete_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_delete_btn"));
    GtkWidget *update_delete_product_popup_cancel_btn = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_cancel_btn"));
    g_signal_connect(create_product_popup_save_btn, "clicked", G_CALLBACK(on_save_product_btn_clicked), NULL);
    g_signal_connect(create_product_popup_cancel_btn, "clicked", G_CALLBACK(on_cancel_product_btn_clicked), NULL);
    g_signal_connect(update_delete_product_popup_apply_btn, "clicked", G_CALLBACK(on_update_product_btn_clicked), NULL);
    g_signal_connect(update_delete_product_popup_delete_btn, "clicked", G_CALLBACK(on_delete_product_btn_clicked), NULL);
    g_signal_connect(update_delete_product_popup_cancel_btn, "clicked", G_CALLBACK(on_cancel2_product_btn_clicked), NULL);

    // Customers Page data
    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));
    GtkListStore *customer_liststore = GTK_LIST_STORE(gtk_builder_get_object(customers_builder, "customer_liststore"));
    GtkWidget *profile_btn_customers = GTK_WIDGET(gtk_builder_get_object(customers_builder, "profile_btn"));
    GtkWidget *profile_label_customers = GTK_WIDGET(gtk_builder_get_object(customers_builder, "profile_label"));
    GtkWidget *add_customer_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "add_customer_btn"));
    GtkWidget *active_sessions_btn_customers = GTK_WIDGET(gtk_builder_get_object(customers_builder, "active_sessions_btn"));
    GtkWidget *customers_back_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "customers_back_btn"));
    g_signal_connect(add_customer_btn, "clicked", G_CALLBACK(on_add_customer_btn_clicked), NULL);
    g_signal_connect(customers_back_btn, "clicked", G_CALLBACK(on_customers_back_btn_clicked), NULL);
    g_signal_connect(customers_tv, "row-activated", G_CALLBACK(on_customer_row_double_clicked), NULL);
    // CRUD customer Popup data
    GtkWidget *create_customer_popup_save_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "create_customer_popup_save_btn"));
    GtkWidget *create_customer_popup_cancel_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "create_customer_popup_cancel_btn"));
    GtkWidget *update_delete_customer_popup_apply_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_apply_btn"));
    GtkWidget *update_delete_customer_popup_delete_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_delete_btn"));
    GtkWidget *update_delete_customer_popup_cancel_btn = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_cancel_btn"));
    g_signal_connect(create_customer_popup_save_btn, "clicked", G_CALLBACK(on_save_customer_btn_clicked), NULL);
    g_signal_connect(create_customer_popup_cancel_btn, "clicked", G_CALLBACK(on_cancel_customer_btn_clicked), NULL);
    g_signal_connect(update_delete_customer_popup_apply_btn, "clicked", G_CALLBACK(on_update_customer_btn_clicked), NULL);
    g_signal_connect(update_delete_customer_popup_delete_btn, "clicked", G_CALLBACK(on_delete_customer_btn_clicked), NULL);
    g_signal_connect(update_delete_customer_popup_cancel_btn, "clicked", G_CALLBACK(on_cancel2_customer_btn_clicked), NULL);


    // Users Page data
    GtkWidget *users_back_btn = GTK_WIDGET(gtk_builder_get_object(users_builder, "users_back_btn"));
    GtkListStore *user_liststore = GTK_LIST_STORE(gtk_builder_get_object(users_builder, "user_liststore"));
    g_signal_connect(users_back_btn, "clicked", G_CALLBACK(on_users_back_btn_clicked), NULL);

    // Profile Page data
    GtkWidget *profile_back_btn = GTK_WIDGET(gtk_builder_get_object(profile_builder, "profile_back_btn"));

    sock = connect_to_server();

    gtk_widget_show_all(WelcomePage);
    gtk_main();

    return 0;
}




/* -------------------- AUTH HANDLERS */

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



/* ----------------- PRODUCTS FUNCTIONS */


void on_products_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(MainPage));
    gtk_widget_show (GTK_WIDGET(ProductListPage));

    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));

    GtkCellRenderer *renderer;

    // ID Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(products_tv),
                                                -1, "ID", renderer, "text", 0, NULL);

    // Name Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(products_tv),
                                                -1, "Name", renderer, "text", 1, NULL);

    // Price Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(products_tv),
                                                -1, "Price", renderer, "text", 2, NULL);
                                                
    GtkTreeViewColumn *col_id = gtk_tree_view_get_column(products_tv, 0);
    GtkTreeViewColumn *col_name = gtk_tree_view_get_column(products_tv, 1);
    GtkTreeViewColumn *col_price = gtk_tree_view_get_column(products_tv, 2);

    gtk_tree_view_column_set_fixed_width(col_id, 200);   // Set width for ID column
    gtk_tree_view_column_set_fixed_width(col_name, 300); // Set width for Name column
    gtk_tree_view_column_set_fixed_width(col_price, 300); // Set width for Price column

    char *response = get_products(sock);

    printf("CLIENT.C: %s\n", response);
    setup_products_treeview(products_tv, response);
}

void setup_products_treeview(GtkTreeView *treeview, char *response) {
    printf("SETUP PRODUCTS TREEVIEW is working.\n");
    GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(products_builder, "product_liststore"));
    gtk_list_store_clear(liststore);

    char *token = strtok(response, "|"); // skip true
    token = strtok(NULL, "|");

    while (token != NULL) {
        printf("Current token: %s\n", token);
        guint id;
        char* name = NULL;
        gint price;

        sscanf(token, "%d,%m[^,],%d", &id, &name, &price);

        printf("ID: %d, Name: %s, Price: %d\n", id, name, price);

        // Add the product to the list store
        GtkTreeIter iter;
        gtk_list_store_append(liststore, &iter);
        gtk_list_store_set(liststore, &iter,
                           0, id,
                           1, name,
                           2, price,
                           -1);

        g_free(name);
        token = strtok(NULL, "|");
    }
}

void on_add_product_btn_clicked() {
    gtk_widget_show(AddProductPopup);
}


void on_save_product_btn_clicked(){
    GtkEntry *name_entry = GTK_ENTRY(gtk_builder_get_object(products_builder, "create_product_popup_name"));
    GtkEntry *price_entry = GTK_ENTRY(gtk_builder_get_object(products_builder, "create_product_popup_price"));

    const char *name = gtk_entry_get_text(name_entry);
    const char *price_str = gtk_entry_get_text(price_entry);

    int price = atoi(price_str);

    if (strlen(name) == 0 || price <= 0) {
        g_print("Invalid input: Name cannot be empty and Price must be greater than 0.\n");
        return;
    }

    int result = add_product(sock, name, price);

    if (result == 1) {
        g_print("Product added successfully: %s, %d\n", name, price);
    } else {
        g_print("Failed to add product.\n");
    }

    // Refreshing table content
    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    char *response = get_products(sock);
    setup_products_treeview(products_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(products_builder, "AddProductPopup"));
    gtk_widget_hide(popup);
}

void on_cancel_product_btn_clicked(){
    gtk_widget_hide(AddProductPopup);
}

void on_product_row_double_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data){
    // GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    // GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(products_builder, "product_liststore"));
    GtkListStore *liststore = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));
    GtkTreeIter iter;

    // Get the selected row data
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(liststore), &iter, path)) {
        guint id;
        gchar *name;
        gint price;

        // Retrieve data from the selected row
        gtk_tree_model_get(GTK_TREE_MODEL(liststore), &iter,
                           0, &id,
                           1, &name,
                           2, &price,
                           -1);

        // Set the data in the entry fields of the popup window
        GtkWidget *id_label = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_id"));
        GtkWidget *name_entry = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_name"));
        GtkWidget *price_entry = GTK_WIDGET(gtk_builder_get_object(products_builder, "update_delete_product_popup_price"));

        gtk_label_set_text(GTK_LABEL(id_label), g_strdup_printf("%d", id));
        gtk_entry_set_text(GTK_ENTRY(name_entry), name);
        gtk_entry_set_text(GTK_ENTRY(price_entry), g_strdup_printf("%d", price));

        // Show the update/delete popup window
        gtk_widget_show(UpdateDeleteProductPopup);

        // Free the allocated memory for name
        g_free(name);
    }
    gtk_widget_show(UpdateDeleteProductPopup);
}

void on_update_product_btn_clicked(){
    GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(products_builder, "update_delete_product_popup_id"));
    GtkEntry *name_entry = GTK_ENTRY(gtk_builder_get_object(products_builder, "update_delete_product_popup_name"));
    GtkEntry *price_entry = GTK_ENTRY(gtk_builder_get_object(products_builder, "update_delete_product_popup_price"));

    const char *id_str = gtk_label_get_text(id_label);
    const char *name = gtk_entry_get_text(name_entry);
    const char *price_str = gtk_entry_get_text(price_entry);

    int id = atoi(id_str);
    int price = atoi(price_str);

    if (strlen(name) == 0 || price <= 0) {
        g_print("Invalid input: Name cannot be empty and Price must be greater than 0.\n");
        return;
    }

    int result = update_product(sock, id, name, price);

    if (result == 0) {
        g_print("Product added successfully: %d %s, %d\n", id, name, price);
    } else {
        g_print("Failed to update product.\n");
    }

    // Refreshing table content
    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    char *response = get_products(sock);
    setup_products_treeview(products_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(products_builder, "UpdateDeleteProductPopup"));
    gtk_widget_hide(popup);
}

void on_delete_product_btn_clicked(){
    GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(products_builder, "update_delete_product_popup_id"));
    const char *id_str = gtk_label_get_text(id_label);

    int id = atoi(id_str);

    int result = delete_product(sock, id);

    if (result == 0) {
        g_print("Product added successfully: %d\n", id);
    } else {
        g_print("Failed to update product.\n");
    }

    // Refreshing table content
    GtkTreeView *products_tv = GTK_TREE_VIEW(gtk_builder_get_object(products_builder, "products_tv"));
    char *response = get_products(sock);
    setup_products_treeview(products_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(products_builder, "UpdateDeleteProductPopup"));
    gtk_widget_hide(popup);
}

void on_cancel2_product_btn_clicked(){
    gtk_widget_hide(UpdateDeleteProductPopup);
}



/* ----------------- CUSTOMERS FUNCTIONS */


void on_customers_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(MainPage));
    gtk_widget_show (GTK_WIDGET(CustomersListPage));

    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));


    GtkCellRenderer *renderer;

    // ID Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(customers_tv),
                                                -1, "ID", renderer, "text", 0, NULL);

    // First Name Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(customers_tv),
                                                -1, "First Name", renderer, "text", 1, NULL);

    // Last Name Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(customers_tv),
                                                -1, "Last Name", renderer, "text", 2, NULL);
    // Phone Number Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(customers_tv),
                                                -1, "Phone Number", renderer, "text", 3, NULL);
    // Visited At Column
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(customers_tv),
                                                -1, "Visited At", renderer, "text", 4, NULL);
                                                
    GtkTreeViewColumn *col_id = gtk_tree_view_get_column(customers_tv, 0);
    GtkTreeViewColumn *col_first_name = gtk_tree_view_get_column(customers_tv, 1);
    GtkTreeViewColumn *col_last_name = gtk_tree_view_get_column(customers_tv, 2);
    GtkTreeViewColumn *col_phone_number = gtk_tree_view_get_column(customers_tv, 3);
    GtkTreeViewColumn *col_visited_at = gtk_tree_view_get_column(customers_tv, 4);

    gtk_tree_view_column_set_fixed_width(col_id, 50);
    gtk_tree_view_column_set_fixed_width(col_first_name, 200);
    gtk_tree_view_column_set_fixed_width(col_last_name, 200);
    gtk_tree_view_column_set_fixed_width(col_phone_number, 150); 
    gtk_tree_view_column_set_fixed_width(col_visited_at, 200); 

    char *response = get_customers(sock);
    setup_customers_treeview(customers_tv, response);

    printf("Customers List response: %s\n", response);
}


void setup_customers_treeview(GtkTreeView *treeview, char *response) {
    printf("SETUP CUSTOMERS TREEVIEW is working.\n");
    GtkListStore *liststore = GTK_LIST_STORE(gtk_builder_get_object(customers_builder, "customer_liststore"));
    gtk_list_store_clear(liststore);

    char *token = strtok(response, "|"); // skip true
    token = strtok(NULL, "|");

    while (token != NULL) {
        printf("Current token: %s\n", token);
        guint id;
        char* name = NULL, *first_name = NULL, *last_name = NULL, *phone_number = NULL, *visited_at = NULL;

        sscanf(token, "%d,%m[^,],%m[^,],%m[^,],%m[^,]", &id, &first_name, &last_name, &phone_number, &visited_at);

        printf("First Name: %s, Last Name: %s, Phone Number: %s, Visited At: %s\n", first_name, last_name, phone_number, visited_at);

        // Add the customer to the list store
        GtkTreeIter iter;
        gtk_list_store_append(liststore, &iter);
        gtk_list_store_set(liststore, &iter,
                           0, id,
                           1, first_name,
                           2, last_name,
                           3, phone_number,
                           4, visited_at,
                           -1);

        g_free(name);
        token = strtok(NULL, "|");
    }
}

void on_add_customer_btn_clicked() {
    gtk_widget_show(AddCustomerPopup);
}


void on_save_customer_btn_clicked(){
    GtkEntry *first_name_entry = GTK_ENTRY(gtk_builder_get_object(customers_builder, "create_customer_popup_fname"));
    GtkEntry *last_name_entry = GTK_ENTRY(gtk_builder_get_object(customers_builder, "create_customer_popup_lname"));
    GtkEntry *phone_number_entry = GTK_ENTRY(gtk_builder_get_object(customers_builder, "create_customer_popup_phone_number"));

    const char *fname = gtk_entry_get_text(first_name_entry);
    const char *lname = gtk_entry_get_text(last_name_entry);
    const char *pnumber = gtk_entry_get_text(phone_number_entry);

    if (strlen(fname) == 0 || strlen(lname) == 0 || strlen(pnumber) == 0) {
        g_print("Invalid input: Name and Phone Number fields cannot be empty.\n");
        return;
    }

    int result = add_customer(sock, fname, lname, pnumber);

    if (result == 1) {
        g_print("Customer added successfully: %s, %s %s\n", fname, lname, pnumber);
    } else {
        g_print("Failed to add customer.\n");
    }

    // Refreshing table content
    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));
    char *response = get_customers(sock);
    setup_customers_treeview(customers_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(customers_builder, "AddCustomerPopup"));
    gtk_widget_hide(popup);
}

void on_cancel_customer_btn_clicked(){
    gtk_widget_hide(AddCustomerPopup);
}

void on_customer_row_double_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data){
    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));
    GtkListStore *liststore = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));
    GtkTreeIter iter;

    // Get the selected row data
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(liststore), &iter, path)) {
        guint id;
        gchar *first_name;
        gchar *last_name;
        gchar *phone_number;
        gchar *visited_at;

        // Retrieve data from the selected row
        gtk_tree_model_get(GTK_TREE_MODEL(liststore), &iter,
                           0, &id,
                           1, &first_name,
                           2, &last_name,
                           3, &phone_number,
                           4, &visited_at,
                           -1);

        // Set the data in the entry fields of the popup window
        GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_id"));
        GtkWidget *fname_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_fname"));
        GtkWidget *lname_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_lname"));
        GtkWidget *phone_number_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_phone_number"));
        GtkWidget *visited_at_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_visited_at"));

        gtk_label_set_text(GTK_LABEL(id_label), g_strdup_printf("%d", id));
        gtk_entry_set_text(GTK_ENTRY(fname_entry), first_name);
        gtk_entry_set_text(GTK_ENTRY(lname_entry), last_name);
        gtk_entry_set_text(GTK_ENTRY(phone_number_entry), phone_number);
        gtk_entry_set_text(GTK_ENTRY(visited_at_entry), visited_at);

        // Show the update/delete popup window
        gtk_widget_show(UpdateDeleteCustomerPopup);

        g_free(first_name);
        g_free(last_name);
        g_free(phone_number);
        g_free(visited_at);
    }
    gtk_widget_show(UpdateDeleteCustomerPopup);
}

void on_update_customer_btn_clicked(){
        GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_id"));
        GtkWidget *fname_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_fname"));
        GtkWidget *lname_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_lname"));
        GtkWidget *phone_number_entry = GTK_WIDGET(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_phone_number"));

    const char *id_str = gtk_label_get_text(GTK_LABEL(id_label));
    const char *fname = gtk_entry_get_text(GTK_ENTRY(fname_entry));
    const char *lname = gtk_entry_get_text(GTK_ENTRY(lname_entry));
    const char *phone_number = gtk_entry_get_text(GTK_ENTRY(phone_number_entry));

    int id = atoi(id_str);

    if (strlen(fname) == 0 || strlen(lname) == 0 || strlen(phone_number) == 0) {
        g_print("Invalid input: Name and Phone Number fields cannot be empty.\n");
        return;
    }

    int result = update_customer(sock, id, fname, lname, phone_number);

    if (result == 0) {
        g_print("Customer updated successfully: %d, %s, %s, %s\n", id, fname, lname, phone_number);
    } else {
        g_print("Failed to update product.\n");
    }

    // Refreshing table content
    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));
    char *response = get_customers(sock);
    setup_customers_treeview(customers_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(customers_builder, "UpdateDeleteCustomerPopup"));
    gtk_widget_hide(popup);
}

void on_delete_customer_btn_clicked(){
    GtkLabel *id_label = GTK_LABEL(gtk_builder_get_object(customers_builder, "update_delete_customer_popup_id"));
    const char *id_str = gtk_label_get_text(GTK_LABEL(id_label));

    int id = atoi(id_str);

    int result = delete_customer(sock, id);

    if (result == 0) {
        g_print("Customer deleted successfully: %d\n", id);
    } else {
        g_print("Failed to delete customer.\n");
    }

    // Refreshing table content
    GtkTreeView *customers_tv = GTK_TREE_VIEW(gtk_builder_get_object(customers_builder, "customers_tv"));
    char *response = get_customers(sock);
    setup_customers_treeview(customers_tv, response);

    GtkWidget *popup = GTK_WIDGET(gtk_builder_get_object(customers_builder, "UpdateDeleteCustomerPopup"));
    gtk_widget_hide(popup);
}

void on_cancel2_customer_btn_clicked(){
    gtk_widget_hide(UpdateDeleteCustomerPopup);
}


/* ----------------- ORDERS FUNCTIONS */


void on_orders_btn_clicked(){
}



/* ----------------- USERS FUNCTIONS */


void on_users_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(MainPage));
    gtk_widget_show (GTK_WIDGET(UsersListPage));

    GtkTreeView *users_tv = GTK_TREE_VIEW(gtk_builder_get_object(users_builder, "users_tv"));

    char *response = get_users(sock);

    printf("Users List response: %s\n", response);
}



/* ----------------- PROFILE FUNCTIONS */


void on_profile_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(MainPage));
    gtk_widget_show (GTK_WIDGET(ProfilePage));

    GtkEntry *profile_first_name = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_first_name"));
    GtkEntry *profile_last_name = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_last_name"));
    GtkEntry *profile_username = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_username"));
    GtkEntry *profile_password = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_password"));
    GtkEntry *profile_role = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_role"));
    GtkEntry *profile_created_at = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_role"));
    GtkEntry *profile_email = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_role"));
    GtkEntry *profile_phone_number = GTK_ENTRY(gtk_builder_get_object(profile_builder, "profile_role"));


    gtk_entry_set_text(profile_first_name, current_user.first_name);
    gtk_entry_set_text(profile_last_name, current_user.last_name);
    gtk_entry_set_text(profile_username, current_user.username);
    gtk_entry_set_text(profile_password, current_user.password);
    gtk_entry_set_text(profile_role, current_user.role);
    gtk_entry_set_text(profile_created_at, current_user.created_at);
    gtk_entry_set_text(profile_email, current_user.email);
    gtk_entry_set_text(profile_phone_number, current_user.phone_number);
}


/* ------------ Helper Handlers ----------- */

void on_login_button_main_clicked() {
    gtk_widget_hide(WelcomePage);
    gtk_widget_show(LoginPage);
}

void on_register_button_main_clicked() {
    gtk_widget_hide(WelcomePage);
    gtk_widget_show(RegisterPage);
}

void on_register_back_clicked() {
    gtk_widget_hide (GTK_WIDGET(RegisterPage));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}

void on_login_back_clicked() {
    gtk_widget_hide (GTK_WIDGET(LoginPage));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}

// ---

void on_products_back_clicked(){
    gtk_widget_hide (GTK_WIDGET(ProductListPage));
    gtk_widget_show (GTK_WIDGET(MainPage));
}

void on_customers_back_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(CustomersListPage));
    gtk_widget_show (GTK_WIDGET(MainPage));
}

void on_users_back_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(UsersListPage));
    gtk_widget_show (GTK_WIDGET(MainPage));
}

void on_profile_back_btn_clicked(){
    gtk_widget_hide (GTK_WIDGET(ProfilePage));
    gtk_widget_show (GTK_WIDGET(MainPage));
}