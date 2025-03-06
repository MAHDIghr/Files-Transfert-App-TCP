#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {
    int taille_fenetre = 4; // taille par défaut
    if (argc == 2) {
        taille_fenetre = atoi(argv[1]);
    }

    if (argc > 2 || taille_fenetre <= 0) {
        fprintf(stderr, "Erreur d'utilisation : %s <taille fenêtre>\n", argv[0]);
        return 1;
    }

    unsigned char message[MAX_INFO]; // message de l'application
    int taille_msg; // taille du message
    paquet_t tab_p[SEQ_NUM_SIZE]; // Tableau de paquets utilisés par le protocole
    paquet_t paquet_recu; // paquet utilisé pour recevoir l'acquitement
    unsigned int borneInf = 0; // début de la fenêtre
    unsigned int curseur = 0; // pointeur courant
    int evt; // Événement
    int duplicate_ack_count = 0; // Compteur d'acquittements dupliqués
    int last_ack_received = -1; // Dernier ACK reçu

    init_reseau(EMISSION);

    printf("[TRP] Initialisation réseau : OK.\n");
    printf("[TRP] Début d'exécution du protocole transport.\n");

    // Lecture de données provenant de la couche application
    de_application(message, &taille_msg);

    // Tant que l'émetteur a des données à envoyer
    while (taille_msg != 0 || borneInf != curseur) {
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

            // Démarrer le temporisateur pour le premier paquet de la fenêtre
            if (borneInf == curseur) {
                depart_temporisateur(500);
            }

            // Incrémenter le curseur pour avancer dans la fenêtre
            curseur = incr(curseur, SEQ_NUM_SIZE);
            // Lire de nouvelles données
            de_application(message, &taille_msg);
        } else {
            evt = attendre();
            if (evt == -1) { // evt == Paquet arrivé
                de_reseau(&paquet_recu);

                // Vérifier si l'ACK reçu est dupliqué
                if (verifier_controle(paquet_recu) && paquet_recu.type == ACK) {
                    if (paquet_recu.num_seq == last_ack_received) {
                        duplicate_ack_count++;
                    } else {
                        duplicate_ack_count = 0; // Réinitialiser le compteur
                        last_ack_received = paquet_recu.num_seq; // Mettre à jour le dernier ACK reçu
                    }

                    // Si on a reçu 3 ACK dupliqués, retransmettre
                    if (duplicate_ack_count == 3 && dans_fenetre(borneInf, last_ack_received, taille_fenetre)) {
                        unsigned int i = last_ack_received; // Début de la retransmission
                        depart_temporisateur(500); // Redémarrer le temporisateur
                        while (i != curseur) {
                            vers_reseau(&tab_p[i]); // Retransmettre tous les paquets dans la fenêtre
                            i = incr(i, SEQ_NUM_SIZE);
                        }
                    }

                    // Mettre à jour la borne inférieure de la fenêtre
                    borneInf = incr(paquet_recu.num_seq, SEQ_NUM_SIZE);

                    // Arrêter le temporisateur si tous les paquets sont acquittés
                    if (borneInf == curseur) {
                        arret_temporisateur();
                    }
                }
            } else { // Timeout - Retransmission
                unsigned int i = borneInf;
                depart_temporisateur(500); // Redémarrer le temporisateur
                while (i != curseur) {
                    vers_reseau(&tab_p[i]); // Retransmettre tous les paquets dans la fenêtre
                    i = incr(i, SEQ_NUM_SIZE);
                }
            }
        }
    }
    printf("[TRP] Fin d'exécution du protocole de transfert de données (TDD).\n");
    return 0;
}
