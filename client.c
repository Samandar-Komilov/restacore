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

int sock; // Server socket
int id; // user_id global variable
char* role; // user role global variable


/* GTK handlers */
void on_register_button_clicked(GtkButton *button, gpointer user_data);
void on_sign_up_back_clicked(GtkButton *button, gpointer user_data);


int main(int argc, char* argv[]){
    gtk_init(&argc, &argv);
    
    // sock = connect_to_server();

    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "glade/register.glade", NULL)) {
        g_error("Failed to load Glade file.");
    }

    // Get the main window
    GtkWidget *register_window = GTK_WIDGET(gtk_builder_get_object(builder, "Register"));
    g_signal_connect(register_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Fetch widgets
    GtkWidget *username_entry = GTK_WIDGET(gtk_builder_get_object(builder, "username"));
    GtkWidget *password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "password"));
    GtkWidget *confirm_password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "confirm_password"));
    GtkWidget *register_button = GTK_WIDGET(gtk_builder_get_object(builder, "register_button"));
    GtkWidget *sign_up_back_button = GTK_WIDGET(gtk_builder_get_object(builder, "sign_up_back"));

    // Attach signal handlers
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), 
                     (gpointer)username_entry);
    g_signal_connect(sign_up_back_button, "clicked", G_CALLBACK(on_sign_up_back_clicked), register_window);

    // Show the register window
    gtk_widget_show_all(register_window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}

/* Callback for Register button */
void on_register_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *username_entry = GTK_WIDGET(user_data);
    GtkWidget *password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "password_entry"));
    GtkWidget *confirm_password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "confirm_password_entry"));

    const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
    const char *confirm_password = gtk_entry_get_text(GTK_ENTRY(confirm_password_entry));

    if (strlen(username) == 0 || strlen(password) == 0 || strlen(confirm_password) == 0) {
        g_print("All fields must be filled out.\n");
        return;
    }

    if (strcmp(password, confirm_password) != 0) {
        g_print("Passwords do not match.\n");
        return;
    }

    // Send registration data to the server
    // register_user(sock, username, password);
}

/* Callback for Back button */
void on_sign_up_back_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *register_window = GTK_WIDGET(user_data);
    gtk_widget_hide(register_window); // Hide the register window (replace with navigation logic if needed)
    g_print("Back to the previous screen.\n");
}