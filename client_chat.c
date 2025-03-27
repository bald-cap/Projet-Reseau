#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int sock;

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
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("%s", buffer);
        } else if (valread == 0) {
            printf("Le serveur a fermé la connexion.\n");
            exit(EXIT_SUCCESS);
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_address;
    char message[BUFFER_SIZE];
    pthread_t recv_thread;

    // Gestion du signal pour une fermeture propre
    signal(SIGINT, handle_exit);

    // Création du socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        perror("Adresse invalide");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connexion échouée");
        exit(EXIT_FAILURE);
    }
    printf("Connecté au serveur. Vous pouvez commencer à chatter !\n");

    // Lancer un thread pour recevoir les messages
    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock);

    // Lire les messages de l'utilisateur et les envoyer
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        send(sock, message, strlen(message), 0);
    }

    return 0;
}
