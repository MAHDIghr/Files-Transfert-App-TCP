# Files-Transfert-App-TCP

Application de transfert de fichiers réalisée en C, utilisant les mécanismes de la couche transport pour envoyer et recevoir des fichiers entre deux machines. Le projet inclut également des outils de test, d’analyse réseau et de mesure de performance.

---

## Fonctionnalités

- Envoi de fichiers depuis un client vers un serveur
- Réception de fichiers côté serveur
- Communication via sockets (mode non connecté)
- Mesure des performances (débit, pertes, temps de transfert)
- Analyse réseau via captures Wireshark
- Tests automatisés pour valider le protocole
- Configuration simple via fichier `config.txt`

---

## Structure du projet

.
├── src/                # Code source en C

├── tests/              # Scripts de tests automatisés

├── wireshark/          # Captures réseau pour analyse

├── perf/               # Scripts Python pour tracer les performances

├── config.txt          # Paramètres de l'application

├── Makefile            # Compilation du projet

└── README.md           # Documentation


---

## Compilation

Assurez-vous d’être sur un système Linux avec `gcc` installé.


Cela génère les exécutables du client et du serveur.

---

## Utilisation

### 1. Lancer le serveur

./server <port>
### 2. Envoyer un fichier depuis le client

./client <adresse_serveur> <port> <fichier>

Le fichier sera transmis au serveur et sauvegardé dans le répertoire prévu.

---

## Tests automatisés

Le projet inclut un script permettant de vérifier automatiquement :

- la transmission correcte des fichiers
- l’intégrité des données
- le comportement en cas de pertes

Pour lancer les tests :

./tests/run_tests.sh

---

## Analyse des performances

Un script Python permet de tracer les performances du protocole :


python3 perf/plot_perf.py


Les résultats incluent :

- débit moyen
- temps de transfert
- taux de perte

---

## Analyse réseau

Le dossier `wireshark/` contient des captures permettant d’étudier :

- les échanges client/serveur
- la structure des paquets
- les retransmissions éventuelles

Ces analyses ont servi à valider le fonctionnement du protocole.

---

## Objectifs pédagogiques

Ce projet m’a permis de :

- comprendre en profondeur les mécanismes de la couche transport
- manipuler les sockets réseau en C
- automatiser des tests et analyser des traces réseau
- mesurer et optimiser les performances d’un protocole simple
- renforcer mes compétences Linux, scripting et diagnostic réseau

