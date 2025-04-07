#define SERVER
#include "chat_shared.h"

#define PORT 8082

int server_fd;
Client clients[MAX_CLIENTS];
Channel channels[MAX_CHANNELS];
int channel_count = 0;

void exit_with_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void close_all_sockets() {
    printf("\nArrêt du serveur...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0) {
            close(clients[i].socket);
        }
    }
    close(server_fd);
    printf("Serveur arrêté proprement.\n");
}

void handle_sigint(int sig) {
    close_all_sockets();
    exit(0);
}

void save_message_to_file(const char *message) {
    FILE *file = fopen("history.txt", "a");
    if (file) {
        fprintf(file, "%s\n", message);
        fclose(file);
    } else {
        perror("Erreur d'ouverture du fichier");
    }
}

void init_channels() {
    // Créer le canal général par défaut
    strcpy(channels[0].name, DEFAULT_CHANNEL);
    channels[0].client_count = 0;
    channel_count = 1;
}

int find_channel(const char *name) {
    for (int i = 0; i < channel_count; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_client_index(int socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == socket) {
            return i;
        }
    }
    return -1;
}

int add_client_to_channel(int channel_idx, int client_socket) {
    if (channel_idx < 0 || channel_idx >= channel_count) return -1;
    
    // Retirer le client de son canal actuel
    int client_idx = find_client_index(client_socket);
    if (client_idx == -1) return -1;
    
    if (clients[client_idx].current_channel != -1) {
        Channel *old_channel = &channels[clients[client_idx].current_channel];
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (old_channel->clients[i].socket == client_socket) {
                old_channel->clients[i].socket = 0;
                old_channel->client_count--;
                break;
            }
        }
    }
    
    // Ajouter au nouveau canal
    Channel *channel = &channels[channel_idx];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (channel->clients[i].socket == 0) {
            channel->clients[i] = clients[client_idx];
            channel->client_count++;
            clients[client_idx].current_channel = channel_idx;
            return 0;
        }
    }
    return -1;
}

void remove_client_from_all_channels(int client_socket) {
    int client_idx = find_client_index(client_socket);
    if (client_idx == -1) return;
    
    if (clients[client_idx].current_channel != -1) {
        Channel *channel = &channels[clients[client_idx].current_channel];
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (channel->clients[i].socket == client_socket) {
                channel->clients[i].socket = 0;
                channel->client_count--;
                break;
            }
        }
    }
    clients[client_idx].current_channel = -1;
}

void broadcast_to_channel(int channel_idx, const char *message, int sender_socket) {
    if (channel_idx < 0 || channel_idx >= channel_count) return;
    
    Channel *channel = &channels[channel_idx];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (channel->clients[i].socket != 0 && channel->clients[i].socket != sender_socket) {
            send(channel->clients[i].socket, message, strlen(message), 0);
        }
    }
}

void send_to_client(int socket, const char *message) {
    send(socket, message, strlen(message), 0);
}

void handle_new_connection() {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

    if (new_socket < 0) {
        perror("Erreur lors de l'acceptation");
        return;
    }

    // Recevoir le nom d'utilisateur
    char username[50];
    int valread = read(new_socket, username, sizeof(username)-1);
    if (valread <= 0) {
        close(new_socket);
        return;
    }
    username[valread] = '\0';

    // Trouver un emplacement libre
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = new_socket;
            strncpy(clients[i].username, username, sizeof(clients[i].username)-1);
            clients[i].current_channel = -1; // Pas encore dans un canal
            
            // Ajouter au canal par défaut
            add_client_to_channel(0, new_socket);
            
            printf("Nouvelle connexion: %s (socket: %d)\n", username, new_socket);
            
            // Envoyer un message de bienvenue
            char welcome_msg[BUFFER_SIZE];
            snprintf(welcome_msg, sizeof(welcome_msg), "Bienvenue %s! Vous êtes dans le salon %s", 
                    username, channels[0].name);
            send_to_client(new_socket, welcome_msg);
            
            // Annoncer l'arrivée aux autres
            snprintf(welcome_msg, sizeof(welcome_msg), "%s a rejoint le salon", username);
            broadcast_to_channel(0, welcome_msg, new_socket);
            
            return;
        }
    }

    // Si le serveur est plein
    char *msg = "Serveur plein, connexion refusée.\n";
    send(new_socket, msg, strlen(msg), 0);
    close(new_socket);
}

void handle_client_message(int client_index) {
    int sock = clients[client_index].socket;
    char buffer[BUFFER_SIZE];
    int valread = read(sock, buffer, BUFFER_SIZE);

    if (valread == 0) {
        // Déconnexion
        printf("%s s'est déconnecté (socket: %d)\n", clients[client_index].username, sock);
        
        // Annoncer la déconnexion
        if (clients[client_index].current_channel != -1) {
            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s a quitté le salon", clients[client_index].username);
            broadcast_to_channel(clients[client_index].current_channel, msg, sock);
        }
        
        close(sock);
        remove_client_from_all_channels(sock);
        clients[client_index].socket = 0;
    } else {
        buffer[valread] = '\0';
        
        // Gestion des commandes
        if (buffer[0] == '/') {
            char *command = strtok(buffer, " \n");
            
            if (strcmp(command, "/list") == 0) {
                char list_msg[BUFFER_SIZE] = "salons disponibles:\n";
                for (int i = 0; i < channel_count; i++) {
                    snprintf(list_msg + strlen(list_msg), sizeof(list_msg) - strlen(list_msg),
                            "- %s (%d utilisateurs)\n", channels[i].name, channels[i].client_count);
                }
                send_to_client(sock, list_msg);
            } 
            else if (strcmp(command, "/join") == 0) {
                char *channel_name = strtok(NULL, " \n");
                if (channel_name) {
                    int channel_idx = find_channel(channel_name);
                    
                    // Créer le canal s'il n'existe pas
                    if (channel_idx == -1 && channel_count < MAX_CHANNELS) {
                        strncpy(channels[channel_count].name, channel_name, CHANNEL_NAME_LEN-1);
                        channels[channel_count].client_count = 0;
                        channel_idx = channel_count;
                        channel_count++;
                    }
                    
                    if (channel_idx != -1) {
                        // Quitter l'ancien canal
                        if (clients[client_index].current_channel != -1) {
                            char msg[BUFFER_SIZE];
                            snprintf(msg, sizeof(msg), "%s a quitté le salon", clients[client_index].username);
                            broadcast_to_channel(clients[client_index].current_channel, msg, sock);
                        }
                        
                        // Rejoindre le nouveau
                        add_client_to_channel(channel_idx, sock);
                        
                        // Confirmation
                        char msg[BUFFER_SIZE];
                        snprintf(msg, sizeof(msg), "Vous avez rejoint le salon: %s", channels[channel_idx].name);
                        send_to_client(sock, msg);
                        
                        // Annonce
                        snprintf(msg, sizeof(msg), "%s a rejoint le salon", clients[client_index].username);
                        broadcast_to_channel(channel_idx, msg, sock);
                    } else {
                        send_to_client(sock, "Nombre maximum de salons atteint");
                    }
                }
            } else {
                send_to_client(sock, "Commande inconnue");
            }
        } else {
            // Message normal
            if (clients[client_index].current_channel != -1) {
                char full_msg[BUFFER_SIZE + CHANNEL_NAME_LEN + 50];
                snprintf(full_msg, sizeof(full_msg), "[%s] %s: %s", 
                         channels[clients[client_index].current_channel].name, 
                         clients[client_index].username, buffer);
                
                save_message_to_file(full_msg);
                broadcast_to_channel(clients[client_index].current_channel, full_msg, sock);
            }
        }
    }
}

int main() {
    struct sockaddr_in address;
    fd_set readfds;
    int opt = 1, max_sd, activity;

    signal(SIGINT, handle_sigint);

    // Initialisation
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].current_channel = -1;
    }
    
    init_channels();

    // Création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        exit_with_error("Erreur lors de la création du socket");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        exit_with_error("setsockopt échoué");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        exit_with_error("Echec de la liaison");
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        exit_with_error("Echec de l'écoute");
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);
    printf("channels initiaux: %s\n", DEFAULT_CHANNEL);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0) {
                FD_SET(clients[i].socket, &readfds);
            }
            if (clients[i].socket > max_sd) {
                max_sd = clients[i].socket;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("Erreur de select");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            handle_new_connection();
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0 && FD_ISSET(clients[i].socket, &readfds)) {
                handle_client_message(i);
            }
        }
    }

    return 0;
}