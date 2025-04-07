#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// gestion des sélections de sockets et des signaux
#ifdef CLIENT

#include <gtk/gtk.h>
#include <pthread.h>
#endif


// Définitions des constantes
#define MAX_CLIENTS 10
#define MAX_CHANNELS 10
#define CHANNEL_NAME_LEN 50
#define DEFAULT_CHANNEL "GENERAL"
#define BUFFER_SIZE 1024

// Structures
typedef struct {
    int socket;
    char username[50];
    int current_channel;
} Client;

typedef struct {
    char name[CHANNEL_NAME_LEN];
    Client clients[MAX_CLIENTS];
    int client_count;
} Channel;

// Prototypes pour la gestion des canaux
#ifdef SERVER
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
void init_channels();
int find_channel(const char *name);
int find_client_index(int socket);
int add_client_to_channel(int channel_idx, int client_socket);
void remove_client_from_all_channels(int client_socket);
void broadcast_to_channel(int channel_idx, const char *message, int sender_socket);
void send_to_client(int socket, const char *message);
#endif