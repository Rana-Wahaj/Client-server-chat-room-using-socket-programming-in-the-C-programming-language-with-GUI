#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gtk/gtk.h>

#define PORT 5000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char *username;
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;

GtkTextBuffer *buffer;

void send_message_to_all_clients(char *message) {
    for (int i = 0; i < num_clients; i++) {
        send(clients[i].socket, message, strlen(message), 0);
    }
}

void add_message_to_view(char *message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, message, -1);
}

void handle_client(int sockfd) {
    char buffer[BUFFER_SIZE];
    int read_size;
    char message[BUFFER_SIZE];

    while ((read_size = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        sprintf(message, "[%s]: %s", clients[sockfd].username, buffer);
        send_message_to_all_clients(message);
        add_message_to_view(message);
    }

    // Client disconnected
    sprintf(message, "%s has disconnected.\n", clients[sockfd].username);
    add_message_to_view(message);

    // Remove client from list of clients
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket == sockfd) {
            for (int j = i; j < num_clients-1; j++) {
                clients[j] = clients[j+1];
            }
            num_clients--;
            break;
        }
    }

    close(sockfd);
}

void on_client_connected(int sock) {
    // Check if server is full
    if (num_clients >= MAX_CLIENTS) {
        printf("Server is full. Rejecting connection.\n");
        close(sock);
        return;
    }

    // Accept incoming connection
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);

    // Receive username from client
    char username[BUFFER_SIZE];
    int read_size = recv(client_sock, username, BUFFER_SIZE, 0);
    username[read_size] = '\0';

    // Add client to list of clients
    clients[num_clients].socket = client_sock;
    clients[num_clients].username = strdup(username);
    num_clients++;

    // Send welcome message to client
    char message[BUFFER_SIZE];
    sprintf(message, "Welcome to the chat server, %s!\n", username);
    send(client_sock, message, strlen(message), 0);

    // Notify all clients that a new client has connected
    sprintf(message, "%s has joined the chat.\n", username);
    send_message_to_all_clients(message);
    add_message_to_view(message);

    // Start handling client messages
    handle_client(client_sock);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Server");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create text view widget
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(window), text_view);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Bind socket to port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_addr.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        return 1;
    }

    // Listen for incoming connections
    if (listen(sock, MAX_CLIENTS) < 0) {
        perror("Failed to listen for connections");
        return 1;
    }

    // Add socket to GTK main loop
    GIOChannel *channel = g_io_channel_unix_new(sock);
    guint source_id = g_io_add_watch(channel, G_IO_IN, (GIOFunc)on_client_connected, NULL);

    // Show window and run GTK main loop
    gtk_widget_show_all(window);
    gtk_main();

    // Clean up
    g_source_remove(source_id);
    g_io_channel_unref(channel);
    close(sock);
    return 0;
}

