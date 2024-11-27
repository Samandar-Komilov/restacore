#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../shared/network.h"
#include "../shared/utility.h"

#include <gtk/gtk.h>
// #include "../gui/login.h"


int main(int argc, char* argv[]) {
    // gtk_init(&argc, &argv);

    // GtkWidget *login_window = create_login_window();
    // gtk_widget_show_all(login_window);

    // g_signal_connect(login_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // gtk_main();

    int sock;
    struct sockaddr_in server;
    // char message[MAX_BUFFER], response[MAX_BUFFER];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    // Take user input
    Message send_msg, receive_msg;
    send_msg.type = LOGIN;
    printf("Enter username: ");
    scanf("%s", send_msg.content.login.username);
    printf("Enter password: ");
    scanf("%s", send_msg.content.login.password);

    send_command(sock, &send_msg);
    receive_command(sock, &receive_msg);

    printf("\nSuccess: %d\nMessage: %s\n", receive_msg.content.response.success, receive_msg.content.response.message);
    close(sock);
    return 0;
}
