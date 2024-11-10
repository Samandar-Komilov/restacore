#include <gtk/gtk.h>
#include "./login.h"

#include "../shared/network.h"
#include "../shared/utility.h"

// Callback for when the login button is clicked
void on_login_button_clicked(GtkButton *button, gpointer data) {
    GtkEntry *username_entry = GTK_ENTRY(data[0]);
    GtkEntry *password_entry = GTK_ENTRY(data[1]);

    const char *username = gtk_entry_get_text(username_entry);
    const char *password = gtk_entry_get_text(password_entry);

    // Create a LoginMessage structure and populate it
    LoginMessage login_msg;
    strncpy(login_msg.username, username, MAX_USERNAME_LEN - 1);
    strncpy(login_msg.password, password, MAX_PASSWORD_LEN - 1);

    // Send the message to the server or validate (this part is in another module, e.g., network.c)
    // For example: send_login_message(&login_msg);
}

// Function to create the login window
GtkWidget *create_login_window(void) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *username_label = gtk_label_new("Username:");
    GtkWidget *password_label = gtk_label_new("Password:");
    GtkWidget *username_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);  // Hide password

    printf("DEBUG::: %s %s", username_entry, password_entry);

    GtkWidget *login_button = gtk_button_new_with_label("Login");

    gtk_grid_attach(GTK_GRID(grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), login_button, 1, 2, 1, 1);

    // Store both entry widgets in an array for the callback
    gpointer entries[2] = {username_entry, password_entry};

    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), entries);

    return window;
}
