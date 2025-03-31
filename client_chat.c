#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8082
#define BUFFER_SIZE 1024
#define SI struct sockaddr_in
#define SA struct sockaddr

int sock;
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
    SI server_address;
    char message[BUFFER_SIZE];
    char full_message[BUFFER_SIZE + 50];
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

    if (connect(sock, (SA *)&server_address, sizeof(server_address)) < 0) {
        exit_with_error("Connexion échouée");
    }
    printf("Connecté au serveur. Vous pouvez commencer à chatter !\n");

    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock);

    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        snprintf(full_message, sizeof(full_message), "%s: %s", username, message);
        send(sock, full_message, strlen(full_message), 0);
    }

    return 0;
}