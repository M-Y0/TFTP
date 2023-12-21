# TFTP

#### 1. Utilisation des Arguments Passés à la Ligne de Commande
- **gettftp et puttftp** : Les programmes prennent trois arguments de la ligne de commande : l'adresse du serveur (`serverName`), le port (`port`), et le nom du fichier (`fileName`). Ces arguments sont utilisés pour déterminer le serveur avec lequel se connecter et le fichier à télécharger ou à envoyer.

#### 2. Appel à getaddrinfo pour Obtenir l’Adresse du Serveur
- **Fonction `getServerAddress`** : Cette fonction utilise `getaddrinfo` pour convertir l'adresse du serveur et le numéro de port en informations utilisables pour établir une connexion. Elle gère les cas d'erreur et renvoie les détails de l'adresse dans une structure `struct addrinfo`.

#### 3. Réservation d’un Socket de Connexion vers le Serveur
- **Fonction `createUDPSocket`** : Crée un socket UDP pour la communication avec le serveur TFTP. Cette étape est essentielle pour envoyer et recevoir des données via le réseau.

#### 4. Pour gettftp
a) **Construction et Envoi de Requête RRQ**
   - **Fonction `sendRequest`** : Construit et envoie une requête RRQ (Read Request) pour demander le fichier spécifié.

b) **Réception d’un Fichier (Un Seul Paquet DAT)**
   - **Gestion dans `receiveFile`** : Le programme est capable de recevoir un fichier même s'il est constitué d'un seul paquet DAT. Il écrit les données dans un fichier local et envoie un ACK en réponse.

c) **Réception d’un Fichier (Plusieurs Paquets DAT)**
   - **Traitement Itératif dans `receiveFile`** : Si le fichier est plus grand, le programme reçoit plusieurs paquets DAT et envoie un ACK pour chaque paquet reçu, jusqu'à la fin du transfert.

#### 5. Pour puttftp
a) **Construction et Envoi de Requête WRQ**
   - **Utilisation de `sendRequest`** : Similaire à gettftp, mais envoie une requête WRQ (Write Request) pour indiquer un souhait d'envoyer un fichier au serveur.

b) **Envoi d’un Fichier (Un Seul Paquet DAT) et Réception d’ACK**
   - **Fonction `sendFile`** : Capable d'envoyer un fichier même s'il tient dans un seul paquet DAT. Le programme attend ensuite l'ACK du serveur.

c) **Envoi d’un Fichier (Plusieurs Paquets DAT) et Réception des ACKs**
   - **Gestion des Plusieurs Paquets dans `sendFile`** : Pour des fichiers plus grands, `sendFile` envoie plusieurs paquets DAT et attend un ACK pour chaque paquet, assurant ainsi la fiabilité du transfert.
