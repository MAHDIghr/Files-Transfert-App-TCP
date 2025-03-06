#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define DEFAULT_WINDOW_SIZE 4
/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    int taille_fenetre = DEFAULT_WINDOW_SIZE; 
    if (argc == 2) {
        taille_fenetre = atoi(argv[1]);
    }

    if (argc > 2 || taille_fenetre <= 0) {
        fprintf(stderr, "Erreur d'utilisation : %s <taille fenêtre>\n", argv[0]);
        return 1;
    }

    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t tab_p[SEQ_NUM_SIZE]; /* Tableau de paquets utilisés par le protocole */
    paquet_t paquet_recu; /* paquet utilisé pour recevoir l'acquitement */
    unsigned int borneInf = 0; /* début de la fenêtre */
    unsigned int curseur = 0;
    int evt; /* Événement */

    init_reseau(EMISSION);

    printf("[TRP] Initialisation réseau : OK.\n");
    printf("[TRP] Début d'exécution du protocole transport.\n");

    /* lecture de données provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while (taille_msg != 0) {
        if (dans_fenetre(borneInf, curseur, taille_fenetre) && taille_msg != 0) {
            /* construction du paquet */
            tab_p[curseur].lg_info = taille_msg;
            tab_p[curseur].type = DATA;
            tab_p[curseur].num_seq = curseur;

            for (int i = 0; i < taille_msg; i++) {
                tab_p[curseur].info[i] = message[i];
            }

            /* Générer la somme de contrôle */
            generer_controle(&tab_p[curseur]);

            /* Envoyer le paquet vers le réseau */
            vers_reseau(&tab_p[curseur]);

            /* Démarrer le temporisateur pour le premier paquet de la fenêtre */
            if (borneInf == curseur) {
                depart_temporisateur(500);
            }

            /* Incrémenter le curseur pour avancer dans la fenêtre */
            curseur = incr(curseur, SEQ_NUM_SIZE);
            /* Lire de nouvelles données */
            de_application(message, &taille_msg);
        } else {
            evt = attendre();
            if (evt == -1) { /* evt == Paquet arrivé */
                de_reseau(&paquet_recu);
                if (verifier_controle(paquet_recu) && dans_fenetre(borneInf, paquet_recu.num_seq, taille_fenetre)) {
                    /* Mettre à jour la borne inférieure de la fenêtre */
                    borneInf = incr(paquet_recu.num_seq, SEQ_NUM_SIZE);

                    /* Arrêter le temporisateur si tous les paquets sont acquittés */
                    if (borneInf == curseur) {
                        arret_temporisateur();
                    }
                }
            } else { /* Timeout - Retransmission */
                unsigned int i = borneInf;
                depart_temporisateur(500); /* Redémarrer le temporisateur */
                while (i != curseur) {
                    vers_reseau(&tab_p[i]); /* Retransmettre tous les paquets dans la fenêtre */
                    i = incr(i, SEQ_NUM_SIZE);
                }
            }
        }
    }
    printf("[TRP] Fin d'exécution du protocole de transfert de données (TDD).\n");
    return 0;
}
