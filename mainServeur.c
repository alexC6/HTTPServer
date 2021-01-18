/**
 * @file    mainServeur.c
 * @author  Coulais Alexandre
 * @brief   Fichier source du serveur
 * @version 1.2
 * @date    2020-12-13
 * 
 * @copyright Copyright (c) 2020
 */
#include "serveur.h"

int main() {
    char *message = NULL;

    // On initialise le service avec ouverture du port
    Initialisation();

    while (1) {
        int fini = 0;

        // On attend une connexion client
        AttenteClient();

        // Tant que le client emet des requetes
        while (!fini) {
            if (message != NULL) {
                free(message);
                message = NULL;
            }

            message = Reception();

            // On verifie que message contient quelque chose
            if (message != NULL) {
                char nomFichier[256], extension[5];

                // Si la requete n'est pas connue par le serveur on emet une erreur
                if (!(verifierRequete(message))) {
                    envoyerReponse500("Erreur serveur : le serveur n'est pas capable de traiter la requete\n");
                    continue;
                }

                // Si l'extraction du fichier rencontre une erreur on emet un message d'erreur
                if (!(extraitFichier(message, nomFichier, 256))) {
                    envoyerReponse500("Erreur serveur : probleme detecte avec le nom du fichier\n");
                    continue;
                }

                // Si le fichier n'est pas accessible on emet une erreur 404
                if (!(verifierAccesFichier(nomFichier))) {
                    envoyerReponse404HTML("page404.html");

                    if (!(envoyerContenuFichierTexte("page404.html"))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }

                    continue;
                }

                // Si l'extraction de l'extension rencontre un probleme, on emet un message d'erreur
                if (!(extraitExtension(nomFichier, extension, 5))) {
                    envoyerReponse500("Erreur serveur : probleme lors de la recherche de l'extension\n");
                    continue;
                }

                // En fonction de l'extension trouvee, on emet les reponses et les fichiers correspondants
                // Chaque appel de fonction est soumis au controle d'erreur avec emission d'un message si probleme
                if (!(strcmp(extension, "html"))) {
                    if (!(envoyerReponse200HTML(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie de la reponse\n");
                        continue;
                    }

                    if (!(envoyerContenuFichierTexte(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }
                } else if (!(strcmp(extension, "css"))) {
                    if (!(envoyerReponse200CSS(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie de la reponse\n");
                        continue;
                    }

                    if (!(envoyerContenuFichierTexte(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }
                } else if (!(strcmp(extension, "js"))) {
                    if (!(envoyerReponse200JS(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie de la reponse\n");
                        continue;
                    }

                    if (!(envoyerContenuFichierTexte(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }
                } else if ((!(strcmp(extension, "jpg"))) || (!(strcmp(extension, "jpeg")))) {
                    if (!(envoyerReponse200JPG(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie de la reponse\n");
                        continue;
                    }

                    if (!(envoyerContenuFichierBinaire(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }
                } else if (!(strcmp(extension, "ico"))) {
                    if (!(envoyerReponse200ICO(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie de la reponse\n");
                        continue;
                    }

                    if (!(envoyerContenuFichierBinaire(nomFichier))) {
                        envoyerReponse500("Erreur serveur : probleme rencontre lors de l'envoie du contenu\n");
                        continue;
                    }
                } else {
                    fprintf(stderr, "Erreur : extension inconnue\n");
                    envoyerReponse500("Erreur serveur : extension de fichier inconnue\n");
                    continue;
                }

                free(message);
                message = NULL;
            } else {
                fini = 1;
            }
        }

        // On ferme la connexion avec le client lorsqu'il n'y a plus de requete a traiter
        TerminaisonClient();
    }

    if (message != NULL) {
        free(message);
        message = NULL;
    }

    return 0;
}
