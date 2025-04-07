# Projet Chat Client-Serveur en C avec GTK

## Structure du Projet

Le projet est divisé en deux parties principales :
- **Le serveur** (`serveur_chat.c`)
- **Le client** (`client_chat.c`)

### Organisation du Code

Le projet est structuré avec des fichiers `.h` (fichiers d’en-tête) et des fichiers `.c` (code source).

- Les **fichiers `.h`** contiennent :
  - Les **macros** et **constantes** utilisées dans le projet
  - Les **prototypes** des fonctions principales

- Le **fichier `serveur_chat.c`** inclut :
  - Une abstraction pour la gestion des erreurs
  - L’implémentation des fonctions principales du serveur

- Le **fichier `client_chat.c`** utilise :
  - La bibliothèque **GTK+ 3.0** pour créer une interface graphique
  - Une encapsulation des actions utilisateurs via des boutons :
    - Envoyer un message
    - Se déconnecter proprement

## Fonctionnalités

- Interface graphique cliente :
  - Envoi de messages via un bouton "Envoyer"
  - Déconnexion propre via un bouton dédié
- Transmission et stockage des messages :
  - Les messages envoyés sont reçus par le serveur
  - Ils sont redistribués à tous les clients connectés
  - Ils sont enregistrés dans un fichier `history.txt`

## Protocoles et Technologies

- **Protocole de communication :** TCP/IP
- **Bibliothèque d'interface graphique :** GTK+ 3.0
- **Langage utilisé :** C

### Commandes de Compilation

#### Serveur
```bash
gcc -o serveur_chat serveur_chat.c
```

#### Client
```bash
gcc -o client_chat client_chat.c $(pkg-config --cflags --libs gtk+-3.0) -pthread
```

## Fonctionnalités Non Réalisées

- **Chiffrement de bout en bout (E2EE)** :
  - Idée : rendre les messages lisibles uniquement par les clients
  - Tentatives :
    - Chiffrement César (trop simple)
    - OpenSSL (tentative de mise en place d’un chiffrement standardisé)
  - Statut : **non implémenté**

## Difficultés Rencontrées

- Complexité du langage C :
  - Gestion manuelle de la mémoire
  - Manipulation précise des tailles et buffers
  - Grand nombre de fonctions systèmes à retenir
- Intégration avec GTK+ et gestion multi-thread via pthreads