#define CLIENT
#include "chat_shared.h"

#define SERVER_IP "127.0.0.1"
#define PORT 8082
#define BUFFER_SIZE 1024

int sock;
GtkWidget *chat_display;
GtkWidget *entry;
char username[50];

void exit_with_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void handle_exit(int sig) {
    printf("\nDéconnexion en cours...\n");
    close(sock);
    exit(EXIT_SUCCESS);
}

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int valread;
    while (1) {
        valread = read(sock, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0';

            GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(text_buffer, &iter);
            gtk_text_buffer_insert(text_buffer, &iter, buffer, -1);
            gtk_text_buffer_insert(text_buffer, &iter, "\n", -1); 
        } else if (valread == 0) {
            printf("Le serveur a fermé la connexion.\n");
            exit(EXIT_SUCCESS);
        }
    }
    return NULL;
}

void quit_application(GtkWidget *widget, gpointer data) {
    close(sock);  // Ferme la connexion
    printf("Déconnexion en cours...\n");
    gtk_main_quit();  // Ferme l'interface graphique
}

void send_message(GtkWidget *widget, gpointer data) {
    const char *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(message) > 0) {
        if (strcmp(message, "exit") == 0) {
            close(sock);
            printf("Déconnexion en cours...\n");
            gtk_main_quit();
            return;
        }

        char full_message[BUFFER_SIZE + 50];
        snprintf(full_message, sizeof(full_message), "%s: %s", username, message);
        send(sock, full_message, strlen(full_message), 0);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}

void create_gui() {
    GtkWidget *window, *vbox, *scroll, *send_button, *quit_button;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    gtk_init(NULL, NULL);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 400, 300);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    chat_display = text_view;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);  

   
    send_button = gtk_button_new_with_label("Envoyer");
    gtk_box_pack_start(GTK_BOX(button_box), send_button, TRUE, TRUE, 0);
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), NULL);

    
    quit_button = gtk_button_new_with_label("Quitter");
    gtk_box_pack_start(GTK_BOX(button_box), quit_button, TRUE, TRUE, 0);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(quit_application), NULL);

    
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(handle_exit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}

int main() {
    struct sockaddr_in server_address;
    pthread_t recv_thread;

    signal(SIGINT, handle_exit);

    printf("Entrez votre nom: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit_with_error("Erreur lors de la création du socket");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        exit_with_error("Adresse invalide");
    }

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        exit_with_error("Connexion échouée");
    }
    printf("Connecté au serveur. Vous pouvez commencer à chatter !\n");

    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock);
    create_gui();

    return 0;
}
