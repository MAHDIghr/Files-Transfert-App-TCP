#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...



/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}

/*******************************************************
 * Générer et renvoie la somme de controle du paquet p *
 *******************************************************/
void generer_controle(paquet_t *p){
    uint8_t somme_ctrl = 0;

    // Additionner tous les octets du paquet sauf la somme de contrôle elle-même
    somme_ctrl ^= p->type;
    somme_ctrl ^= p->num_seq;
    somme_ctrl ^= p->lg_info;

    // Ne pas inclure p->somme_ctrl dans le calcul de la somme de contrôle
    // pour ne pas inclure la valeur elle-même.

    for (int i = 0; i < p->lg_info; ++i) {
        somme_ctrl ^= p->info[i];
    }

    // Mettre à jour le champ somme_ctrl du paquet avec la somme de contrôle calculée
    p->somme_ctrl = somme_ctrl;
}

/**********************************************
 * vérifier la somme de controle du paquet p. *
   renvoie vrai si pas d'erreuru, faux sinon. *
 **********************************************/
bool verifier_controle(paquet_t p){
    uint8_t somme_ctrl_calculée = 0;

    // Calculer la somme de contrôle en excluant le champ somme_ctrl du paquet
    somme_ctrl_calculée ^= p.type;
    somme_ctrl_calculée ^= p.num_seq;
    somme_ctrl_calculée ^= p.lg_info;

    for (int i = 0; i < p.lg_info; ++i) {
        somme_ctrl_calculée ^= p.info[i];
    }

    // Comparer la somme de contrôle calculée avec la somme de contrôle stockée dans le paquet
    return somme_ctrl_calculée == p.somme_ctrl;
}

/*--------------------------------------*/
/*       Fonction d'incrementation      */
/*--------------------------------------*/
unsigned int incr(unsigned int n, int capacite){
    return (n+1) % capacite;
}
