#ifndef LOGIN_H

#include <gtk/gtk.h>
#include <stdio.h>


void on_login_button_clicked(GtkButton *button, gpointer* user_data[]);

GtkWidget *create_login_window(void);


#endif