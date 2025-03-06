#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define MAX_SEQ 16 // Capacité de numérotation
#define DEFAULT_WINDOW_SIZE 4 // Taille de fenêtre par défaut

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    int taille_fenetre = DEFAULT_WINDOW_SIZE; // Taille de la fenêtre par défaut

    // Vérification des arguments pour définir la taille de la fenêtre
    if (argc == 2) {
        taille_fenetre = atoi(argv[1]);
    }

    if (argc > 2 || taille_fenetre <= 0 || taille_fenetre > MAX_SEQ / 2) {
        fprintf(stderr, "Erreur d'utilisation : %s <taille fenêtre>\n", argv[0]);
        return 1;
    }

    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t tab_p[MAX_SEQ]; // Tableau de paquets
    int temporisateurs[MAX_SEQ] = {0}; // Temporisateurs pour chaque paquet
    unsigned int borneInf = 0; // Borne inférieure de la fenêtre
    unsigned int curseur = 0; // Pointeur pour l'envoi de paquets
    int evt; // Événement

    init_reseau(EMISSION);

    printf("[TRP] Initialisation réseau : OK.\n");
    printf("[TRP] Début d'exécution du protocole transport.\n");

    // Lecture initiale des données
    de_application(message, &taille_msg);

    while (taille_msg != 0 || borneInf != curseur) {
        // Si on peut envoyer un paquet
        if (dans_fenetre(borneInf, curseur, taille_fenetre) && taille_msg != 0) {
            // Construction du paquet
            tab_p[curseur].lg_info = taille_msg;
            tab_p[curseur].type = DATA;
            tab_p[curseur].num_seq = curseur;

            for (int i = 0; i < taille_msg; i++) {
                tab_p[curseur].info[i] = message[i];
            }

            // Générer la somme de contrôle
            generer_controle(&tab_p[curseur]);

            // Envoyer le paquet vers le réseau
            vers_reseau(&tab_p[curseur]);
            printf("Paquet %d envoyé.\n", curseur);

            // Démarrer le temporisateur pour ce paquet
            temporisateurs[curseur] = 500;
            depart_temporisateur_num(curseur, 500);

            // Incrémenter le curseur pour avancer dans la fenêtre
            curseur = incr(curseur, MAX_SEQ);
            de_application(message, &taille_msg); // Lire de nouvelles données
        } else {
            evt = attendre();
            if (evt >= 0 && evt < MAX_SEQ) { // Timeout d'un paquet spécifique
                printf("Timeout du paquet %d, retransmission.\n", evt);
                vers_reseau(&tab_p[evt]); // Retransmettre seulement le paquet en timeout
                depart_temporisateur_num(evt, 500); // Redémarrer le temporisateur pour ce paquet
            } else if (evt == -1) { // Paquet reçu
                paquet_t ack;
                de_reseau(&ack);

                // Si l'ACK est valide
                if (verifier_controle(ack) && ack.type == ACK && dans_fenetre(borneInf, ack.num_seq, taille_fenetre)) {
                    printf("ACK %d reçu.\n", ack.num_seq);
                    temporisateurs[ack.num_seq] = 0; // Arrêter le temporisateur pour ce paquet

                    // Décaler la fenêtre si possible
                    while (borneInf != curseur && temporisateurs[borneInf] == 0) {
                        borneInf = incr(borneInf, MAX_SEQ);
                    }
                }
            }
        }
    }

    printf("[TRP] Fin d'exécution du protocole Selective Repeat.\n");
    return 0;
}
