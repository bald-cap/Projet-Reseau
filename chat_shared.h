#pragma once

// bibliothèques pour la communication réseau et la gestion des sockets
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// bibliothèque GTK pour le client permettant l'interface graphique
#ifdef CLIENT
#include <gtk/gtk.h>
#endif

// gestion des sélections de sockets et des signaux
#ifdef SERVER
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#endif
