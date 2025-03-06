#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[]) {
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t paquet_reponse; /* paquet utilisé pour les acquittements */
    int fin = 0; /* condition d'arrêt */
    uint8_t dernier_ack = 0; /* dernier acquittement envoyé */
    uint8_t num_seq_attendu = 0; /* numéro de séquence attendu */

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation réseau : OK.\n");
    printf("[TRP] Début d'exécution du protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while (!fin) {
        de_reseau(&paquet);

        /* Vérification du contrôle */
        if (verifier_controle(paquet)) {
            if (paquet.num_seq == num_seq_attendu) {
                /* extraction des données du paquet reçu */
                for (int i = 0; i < paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }

                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);
                /* Mettre à jour le numéro de séquence attendu */
                num_seq_attendu = incr(num_seq_attendu, SEQ_NUM_SIZE);

                /* Envoyer un accusé de réception pour ce paquet */
                paquet_reponse.num_seq = paquet.num_seq; /* ACK pour le paquet reçu */
                paquet_reponse.type = ACK;
                paquet_reponse.lg_info = 0;
                generer_controle(&paquet_reponse);

                vers_reseau(&paquet_reponse);
                dernier_ack = paquet_reponse.num_seq; /* Sauvgarder le dernier num ack envoyé */
            } else {
                /* Si le numéro de séquence ne correspond pas, renvoyer le dernier ACK */
                paquet_reponse.num_seq = dernier_ack;
                paquet_reponse.type = ACK;
                paquet_reponse.lg_info = 0;
                generer_controle(&paquet_reponse);
                vers_reseau(&paquet_reponse);
            }
        }
    }

    printf("[TRP] Fin d'exécution du protocole transport.\n");
    return 0;
}
