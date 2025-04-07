#define CLIENT
#include "chat_shared.h"
// configuration du client

#define SERVER_IP "127.0.0.1"
#define PORT 8082
#define BUFFER_SIZE 1024
// variables globales

int sock;
GtkWidget *chat_display;
GtkWidget *entry;
GtkWidget *channel_label;
char username[50];
char current_channel[CHANNEL_NAME_LEN] = DEFAULT_CHANNEL;

// Fonction pour quitter avec un message d'erreur
void exit_with_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
//Gestionnaire de signal pour quitter proprement

void handle_exit(int sig) {
    printf("\nDéconnexion en cours...\n");
    close(sock);
    exit(EXIT_SUCCESS);
}
// Callback GTK pour mettre à jour l'affichage du chat

static gboolean update_chat_display(gpointer data) {
    char *msg = (char *)data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, msg, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    free(msg);
    return G_SOURCE_REMOVE;
}

static gboolean update_channel_label(gpointer data) {
    char *channel = (char *)data;
    gtk_label_set_text(GTK_LABEL(channel_label), channel);
    free(channel);
    return G_SOURCE_REMOVE;
}
// Thread pour recevoir les messages du serveur
void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int valread;
    
    while (1) {
        valread = read(sock, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0';

            if (strstr(buffer, "Vous avez rejoint le salon:")) {
                char *channel_start = strrchr(buffer, ' ') + 1;
                if (channel_start) {
                    strncpy(current_channel, channel_start, sizeof(current_channel)-1);
                    g_idle_add(update_channel_label, strdup(current_channel));
                }
            }

            g_idle_add(update_chat_display, strdup(buffer));
        } else if (valread == 0) {
            printf("Le serveur a fermé la connexion.\n");
            exit(EXIT_SUCCESS);
        }
    }
    return NULL;
}
// Callback pour quitter l'application
void quit_application(GtkWidget *widget, gpointer data) {
    printf("\nDéconnexion en cours...\n");
    close(sock);
    gtk_main_quit();
    exit(EXIT_SUCCESS);  
}

void send_message(GtkWidget *widget, gpointer data) {
    const char *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(message) > 0) {
        send(sock, message, strlen(message), 0);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}
//Fonction pour créer l'interface graphique
void create_gui() {
    GtkWidget *window, *vbox, *scroll, *send_button, *quit_button;
    GtkWidget *text_view, *hbox;
    GtkTextBuffer *buffer;

    gtk_init(NULL, NULL);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 500, 400);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *channel_text = gtk_label_new("Salon actuel:");
    channel_label = gtk_label_new(current_channel);
    gtk_box_pack_start(GTK_BOX(hbox), channel_text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), channel_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    chat_display = text_view;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    send_button = gtk_button_new_with_label("Envoyer");
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), NULL);
    
    quit_button = gtk_button_new_with_label("Quitter");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(quit_application), NULL);
    
    gtk_box_pack_start(GTK_BOX(button_box), send_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), quit_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    gtk_widget_grab_focus(entry);
    g_signal_connect(entry, "activate", G_CALLBACK(send_message), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(quit_application), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}

int main() {
    struct sockaddr_in server_address;
    pthread_t recv_thread;

    signal(SIGINT, handle_exit);

    printf("Entrez votre Pseudo: ");
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

    send(sock, username, strlen(username), 0);

    printf("Connecté au serveur. Commandes disponibles:\n");
    printf("/list - Liste des salons de discussions\n");
    printf("/join <nom> - Rejoindre un salon\n");
    printf("Vous êtes dans le salon: %s\n", current_channel);
 // Lance le thread pour recevoir les messages
    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock);
    create_gui();

    pthread_join(recv_thread, NULL);
    return 0;
}