/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole v1 « Stop-and-Wait » avec acquittement négatif   *
*                                                            *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);

        /* construction d'un  paquet pour envoyer l'acquitement*/
        paquet_t paquet_reponse;

        /* verification du controle */
        if (verifier_controle(paquet))
        {
            /* extraction des donnees du paquet recu */
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
                    
            /* remise des données à la couche application */
            fin = vers_application(message, paquet.lg_info);

            /* Aquitement positif*/
            paquet_reponse.type = ACK;
            
            /* Envoie du paquet contenant l'acquitement*/
            vers_reseau(&paquet_reponse);
        }
        else
        {
            /* Aquitement négatif*/
            paquet_reponse.type = NACK;
            
            /* Envoie du paquet contenant l'acquitement*/
            vers_reseau(&paquet_reponse);
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
