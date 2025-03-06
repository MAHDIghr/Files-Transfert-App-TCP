/*************************************************************
* proto_tdd_v2 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole v2 « Stop-and-Wait » ARQ                         *
*                                                            *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t paquet_recu; /* paquet utilisé pour recevoir l'acquitement */
    int evt; /* évènement : paquet reçu ou timeout */
    uint8_t num_paquet_next = 0; /* numéro du prochain paquet */

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    while ( taille_msg != 0 ) {
        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.num_seq = num_paquet_next; 

        /* Générer la somme de controle*/
        generer_controle(&paquet); 

        /* remise à la couche reseau */
        vers_reseau(&paquet);

        /* départ temporisateur*/
        depart_temporisateur(100);

        evt = attendre(); /* Attente d'un aquitement : paquet arrive | timeOut */
        
        while (evt != -1) // -1 si un paquet reçu est disponible
        {
            /* retransmission du paquet */
            vers_reseau(&paquet);
            depart_temporisateur(100);
            evt = attendre();
        }
        
        /* dans ce cas le paquet est bien arrivé */
        /* recevoire le paquet contenant l'aquitement  */
        de_reseau(&paquet_recu);

        /* arreter le temporisateur*/
        arret_temporisateur();

        /* incrémentation du num du prochain paquet*/
        num_paquet_next ++;

        /* lecture de donnees provenant de la couche application pour le prochain envoie */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
