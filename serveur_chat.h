#pragma once

// Les imports
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>

// Definitions de macros
#define PORT 8080
#define DEFAULT_CHANNEL "GENERAL"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_CHANNELS 10
#define CHANNEL_NAME_LEN 50


#define BUFFER_SIZE 1024

// Abstraction et representation d'un utilisateur
// Structures, definition du type Client
typedef struct {
    int socket;
    char username[50];
} Client;

// definition du type Canal
typedef struct {
    char name[CHANNEL_NAME_LEN];
    Client clients[MAX_CLIENTS];
    int client_count;
} Channel;

// Declaration de functions principales
void exit_with_error(const char *msg);

void close_all_sockets() ;

void handle_sigint(int sig);

void save_message_to_file(const char *message) ;

void broadcast_message(const char *message, int sender_socket);

void handle_new_connection();

void handle_client_message(int client_index);
