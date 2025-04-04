#define SERVER
#include"chat_shared.h"

#define PORT 8082
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
} Client;

int server_fd;
Client clients[MAX_CLIENTS];

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

void broadcast_message(const char *message, int sender_socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

void handle_new_connection() {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

    if (new_socket < 0) {
        perror("Erreur lors de l'acceptation");
        return;
    }

    printf("Nouvelle connexion acceptée, socket: %d\n", new_socket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = new_socket;
            
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
        printf("Client déconnecté, socket: %d\n", sock);
        close(sock);
        clients[client_index].socket = 0;
    } else {
        buffer[valread] = '\0';
        printf("Message reçu de %s: %s", clients[client_index].username, buffer);
        save_message_to_file(buffer);
        broadcast_message(buffer, sock);
    }
}

int main() {
    struct sockaddr_in address;
    fd_set readfds;
    int opt = 1, max_sd, activity;

    signal(SIGINT, handle_sigint);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
    }

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