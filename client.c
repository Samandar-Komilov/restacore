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

// Global variables
int sock;
int id;
char* role;

GtkBuilder *auth_builder;
GtkWidget *WelcomePage, *RegisterPage, *LoginPage, *EmptyField, *InvalidPassword;


/* GTK handlers */
void on_login_button_main_clicked();
void on_register_button_main_clicked();

void on_register_button_clicked();
void on_login_button_clicked();




/* ----------------- MAIN */

int main(int argc, char* argv[]){
    gtk_init(&argc, &argv);

    auth_builder = gtk_builder_new_from_file("register.glade");
    WelcomePage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "welcome_page"));
    RegisterPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Register"));
    LoginPage = GTK_WIDGET(gtk_builder_get_object(auth_builder, "Login"));
    g_signal_connect(WelcomePage, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(RegisterPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); //..
    g_signal_connect(LoginPage, "destroy", G_CALLBACK(gtk_main_quit), NULL); // will be changed

    // Welcome Page data
    GtkWidget *register_button_main = GTK_WIDGET(gtk_builder_get_object(auth_builder, "register_button_main"));
    GtkWidget *login_button_main = GTK_WIDGET(gtk_builder_get_object(auth_builder, "login_button_main"));
    g_signal_connect(register_button_main, "clicked", G_CALLBACK(on_register_button_main_clicked));
    g_signal_connect(login_button_main, "clicked", G_CALLBACK(on_login_button_main_clicked));

    


    sock = connect_to_server();

    gtk_widget_show_all(register_window);

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

    if (strlen(username_register) == 0 || strlen(password_register) == 0 || strlen(confirm_password_register) == 0) {
        g_print("All fields must be filled out.\n");
        return;
    }

    if (strcmp(password_register, confirm_password_register) != 0) {
        g_print("Passwords do not match.\n");
        return;
    }

    // Send registration data to the server
    // register_user(sock, username_register, password_register);
}

void on_sign_up_back_clicked(GtkButton *button, gpointer user_data) {
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

    if (strlen(username_login) == 0 || strlen(password_login) == 0) {
        g_print("All fields must be filled out.\n");
        return;
    }

    // Send login data to the server
    // login_user(sock, username_login, password_login);
}

void on_login_back_clicked() {
    gtk_widget_hide (GTK_WIDGET(LoginPage));
 	gtk_widget_show (GTK_WIDGET(WelcomePage));
}