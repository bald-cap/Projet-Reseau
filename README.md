Plan de développement du projet
1. Comprendre le fonctionnement des sockets en C
Avant de coder, assure-toi de bien comprendre :

La création d'un socket (avec socket())

La liaison du socket à une adresse (bind())

L'écoute des connexions entrantes (listen())

L'acceptation d'une connexion client (accept())

L'envoi et la réception de messages (send() et recv())

La fermeture d'une connexion (close())

2. Implémentation du serveur
Le serveur devra :

Accepter plusieurs connexions simultanément (utilisation de select(), poll() ou pthread pour la gestion des clients).

Gérer l'envoi et la réception de messages.

Diffuser les messages reçus à tous les clients connectés.

Gérer les déconnexions proprement.

3. Implémentation du client
Le client devra :

Se connecter au serveur (connect())

Lire les messages saisis par l'utilisateur et les envoyer au serveur

Afficher les messages reçus en temps réel

Gérer les erreurs de connexion et de déconnexion

4. Améliorations possibles
Ajouter des pseudonymes

Implémenter des salons de discussion

Sauvegarder l'historique des messages

Ajouter un chiffrement basique des messages

### COMMUNICATION
#### Connection
- saissis son pseudo

serveur ecoute le message d'un client - lire()
serveur broadcats le message reçu aux autres clients
serveur transfer le message lu à l'autre client



### GRACFUL SHUTDOWN
Liberer le port avant la fermeture / se deconnecter en tappant le mot exit
Sauvgarder chaque info dans le fichier concerné

``` bash
gcc -o serveur serveur_chat.c
gcc -o client client_chat.c
```

### CURRENT CODE
- Broadcast des clients (terminé)

#### Problem
- Traquer les Psuedo + message
- Graceful Shutdown (gestion de liberation de port au kill SIGINT / SIGTERM)