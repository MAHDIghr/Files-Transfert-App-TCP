#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define MAX_SEQ 16 // Capacité de numérotation
#define DEFAULT_WINDOW_SIZE 4 // Taille de fenêtre par défaut

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[]) {
    int taille_fenetre = DEFAULT_WINDOW_SIZE; // Taille de la fenêtre par défaut

    // Vérification des arguments pour définir la taille de la fenêtre de réception
    if (argc == 2) {
        taille_fenetre = atoi(argv[1]);
    }

    if (argc > 2 || taille_fenetre <= 0 || taille_fenetre > MAX_SEQ / 2) {
        fprintf(stderr, "Erreur d'utilisation : %s <taille fenêtre>\n", argv[0]);
        return 1;
    }

    unsigned char message[MAX_INFO];
    paquet_t tab_p[MAX_SEQ]; // Table pour bufferiser les paquets hors séquence
    int recu[MAX_SEQ] = {0}; // Table pour savoir quels paquets ont été reçus
    paquet_t paquet; // Paquet utilisé pour recevoir les données
    paquet_t paquet_reponse; // Paquet utilisé pour les acquittements
    int fin = 0;
    uint8_t num_seq_attendu = 0; // Numéro de séquence attendu

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation réseau : OK.\n");
    printf("[TRP] Début d'exécution du protocole Selective Repeat.\n");

    while (!fin) {
        de_reseau(&paquet);

        // Si le paquet est valide et dans la fenêtre de réception
        if (verifier_controle(paquet) && dans_fenetre(num_seq_attendu, paquet.num_seq, taille_fenetre)) {
            // Accepter le paquet (en séquence ou hors séquence)
            if (paquet.num_seq == num_seq_attendu) {
                // Remettre les données à la couche application pour le paquet en séquence
                for (int i = 0; i < paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                fin = vers_application(message, paquet.lg_info);

                // Traiter les paquets bufferisés (en avance)
                num_seq_attendu = incr(num_seq_attendu, MAX_SEQ);
                while (recu[num_seq_attendu]) {
                    for (int i = 0; i < tab_p[num_seq_attendu].lg_info; i++) {
                        message[i] = tab_p[num_seq_attendu].info[i];
                    }
                    fin = vers_application(message, tab_p[num_seq_attendu].lg_info);
                    recu[num_seq_attendu] = 0; // Marquer le paquet comme traité
                    num_seq_attendu = incr(num_seq_attendu, MAX_SEQ);
                }
            } else {
                // Bufferiser le paquet hors séquence
                tab_p[paquet.num_seq] = paquet;
                recu[paquet.num_seq] = 1;
                printf("Paquet %d reçu et bufferisé.\n", paquet.num_seq);
            }

            // Envoyer un accusé de réception pour ce paquet
            paquet_reponse.num_seq = paquet.num_seq;
            paquet_reponse.type = ACK;
            paquet_reponse.lg_info = 0;
            generer_controle(&paquet_reponse);
            vers_reseau(&paquet_reponse);
            printf("ACK %d envoyé.\n", paquet.num_seq);
        }
    }

    printf("[TRP] Fin d'exécution du protocole Selective Repeat.\n");
    return 0;
}
