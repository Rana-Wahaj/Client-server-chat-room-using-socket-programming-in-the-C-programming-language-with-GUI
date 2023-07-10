#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

GtkWidget *chat_view;
GtkTextBuffer *chat_buffer;
GtkWidget *username_entry;
GtkWidget *message_entry;
int server_socket;

void add_message_to_view(char *message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(chat_buffer, &iter);
    gtk_text_buffer_insert(chat_buffer, &iter, message, -1);
}

void connect_to_server(GtkButton *button, gpointer user_data) {
    char *server_ip = (char *)gtk_entry_get_text(GTK_ENTRY(user_data));
    int server_port = 5000;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {

        perror("Invalid server IP address");
        exit(1);
    }

    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        exit(1);
    }

    char username[BUFFER_SIZE];
    sprintf(username, "%s", gtk_entry_get_text(GTK_ENTRY(username_entry)));
    send(server_socket, username, strlen(username), 0);

    gtk_widget_set_sensitive(button, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(username_entry), FALSE);
}

void send_message(GtkWidget *widget, gpointer user_data) {
    char message[BUFFER_SIZE];
    sprintf(message, "%s\n", gtk_entry_get_text(GTK_ENTRY(user_data)));
    send(server_socket, message, strlen(message), 0);
    gtk_entry_set_text(GTK_ENTRY(user_data), "");
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *chat_scroll;
    GtkWidget *chat_label;
    GtkWidget *message_box;
    GtkWidget *username_label;
    GtkWidget *message_label;
    GtkWidget *connect_button;
    GtkWidget *send_button;
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;
    GError *error = NULL;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, "entry, label, button { font-family: Sans; font-size: 12px; }", -1, &error);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    chat_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chat_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    chat_view = gtk_text_view_new();
    chat_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_view), FALSE);
    gtk_container_add(GTK_CONTAINER(chat_scroll), chat_view);
    chat_label = gtk_label_new("Chat Messages:");
    gtk_box_pack_start(GTK_BOX(main_box), chat_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), chat_scroll, TRUE, TRUE, 0);

    message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    username_label = gtk_label_new("Username:");
    gtk_box_pack_start(GTK_BOX(message_box), username_label, FALSE, FALSE, 0);
    username_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(message_box), username_entry, FALSE, FALSE, 0);
    message_label = gtk_label_new("Message:");
    gtk_box_pack_start(GTK_BOX(message_box), message_label, FALSE, FALSE, 0);
    message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(message_box), message_entry, TRUE, TRUE, 0);
    send_button = gtk_button_new_with_label("Send");
    g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_message), message_entry);
    gtk_box_pack_start(GTK_BOX(message_box), send_button, FALSE, FALSE, 0);
    connect_button = gtk_button_new_with_label("Connect");
    g_signal_connect(G_OBJECT(connect_button), "clicked", G_CALLBACK(connect_to_server), username_entry);
    gtk_box_pack_start(GTK_BOX(message_box), connect_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), message_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), main_box);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

