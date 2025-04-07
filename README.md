# Chat Client-Serveur en C

Un système de chat en temps réel avec interface graphique GTK pour le client et serveur multi-utilisateurs avec gestion de salons de discussion.
## Structure du projet
Le projet est divisé en deux parties principales:
- *Le serveur*(serveur_chat.c)

 -*Le client*(client_chat.c)



## Fonctionnalités principales
Le projet est structuré avec des fichiers .h (fichier d'en tête) et des fichiers.c(code source).
### Fichier d'en tête
- Les *fichiers .h* contiennent :
  - Les *macros* et *constantes* utilisées dans le projet
  - Les *définitions de types* (structures représentant un client, un canal, etc.)
  - Les *prototypes* des fonctions principales

### Serveur

- Gestion multi-clients simultanés
- Création dynamique de salons de discussion
- Historique des messages sauvegardé dans `history.txt`
- Commandes spéciales :
  - `/list` - Lister les salons disponibles
  - `/join <salon>` - Changer de salon
  - `/quit` - Quitter proprement

### Client
- Interface graphique intuitive avec GTK+3
- Affichage en temps réel des messages
- Notification des changements de salon
- Gestion propre de la déconnexion

## Architecture Technique

### Serveur (serveur_chat.c)
- **Modèle** : Monothread avec multiplexage via `select()`
- **Structures de données** :
  - `Client` : Pseudo + socket + salon actuel
  - `Channel` : Nom + liste des clients + compteur
- **Fonctions clés** :
  - `handle_new_connection()` : Gère les nouvelles connexions
  - `broadcast_to_channel()` : Diffusion aux membres d'un salon
  - `add_client_to_channel()` : Changement de salon

### Client (client_chat.c)
- **Threads** :
  - Main thread : Interface GTK
  - Receiver thread : Écoute les messages serveur
- **GTK Components** :
  - `GtkTextView` : Affichage des messages
  - `GtkEntry` : Saisie des messages
  - `GtkLabel` : Affichage du salon actuel

### Commandes de Compilation

#### Serveur
bash
gcc -o serveur_chat serveur_chat.c


#### Client
bash
gcc -o client_chat client_chat.c $(pkg-config --cflags --libs gtk+-3.0) -pthread

  
## Fonctionnalités Non Réalisées

- *Chiffrement de bout en bout (E2EE)* :
  - Idée : rendre les messages lisibles uniquement par les clients
  - Tentatives :
    - Chiffrement César (trop simple)
    - OpenSSL (tentative de mise en place d’un chiffrement standardisé)
  - Statut : *non implémenté*

## Difficultés Rencontrées

- Complexité du langage C :
  - Gestion manuelle de la mémoire
  - Manipulation précise des tailles et buffers
  - Grand nombre de fonctions systèmes à retenir
- Intégration avec GTK+ et gestion multi-thread via pthreads