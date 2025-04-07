#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <gtk/gtk.h>


// Definitions de macros
#define SERVER_IP "172.16.80.70"
#define PORT 8082
#define BUFFER_SIZE 1024
#define SI struct sockaddr_in
#define SA struct sockaddr

#define MAX_CLIENTS 10
#define MAX_CHANNELS 10
#define CHANNEL_NAME_LEN 50
#define DEFAULT_CHANNEL "GENERAL"

// Abstraction de la gestion d'erreur et du retour 
void exit_with_error(const char *msg);

void handle_exit(int sig);

// Gestion de la receiption et laffachage des messages sur l'interface
void *receive_messages(void *socket_desc);

// Fermer l'interface
void quit_application(GtkWidget *widget, gpointer data);

// Envoyer les message saisit dans l'Entry
void send_message(GtkWidget *widget, gpointer data);

// Creation de l'interface
void create_gui();
