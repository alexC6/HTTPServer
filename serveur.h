/**
 * @file    serveur.h
 * @author  Coulais Alexandre
 * @brief   Fichier de declaration des fonctions du serveur \n
 *          Les commentaires de descriptions du code
 *          sont dans le fichier source. \n Ici figurent les commentaires
 *          de description des fonctions, avec parametres et valeurs de retour
 * @version 1.2
 * @date    2020-12-13
 * 
 * @copyright Copyright (c) 2020
 */

#ifndef __SERVEUR_H__
#define __SERVEUR_H__

#include <ctype.h>
#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

/* Constantes */
#define TRUE 1
#define FALSE 0
#define LONGUEUR_TAMPON 8192

#define STR_SERVER "Server: Coulais Mortier/1.0.0\n"

#ifdef WIN32
#define perror(x) printf("%s : code d'erreur : %d\n", (x), WSAGetLastError())
#define close closesocket
#define socklen_t int
#endif

typedef int bool;

/**
 * @brief   Creation du serveur.
 * @return  int -> Retourne 1 si ca c'est bien passe 0 sinon
 */
int Initialisation(void);

/**
 * @brief Creation du serveur en precisant le service ou numero de port
 * 
 * @param service   Numero de port sur lequel ecouter
 * @return          int -> Retourne 1 si ca c'est bien passe 0 sinon
 */
int InitialisationAvecService(char *service);

/**
 * @brief Attend qu'un client se connecte
 * 
 * @return int -> Retourne 1 si ca c'est bien passe 0 sinon
 */
int AttenteClient(void);

/**
 * @brief   Recoit un message envoye par le client \n
 *          Note : penser a liberer la memoire apres traitement
 * 
 * @return char* -> Retourne NULL en cas d'erreur, sinon l'adresse de la chaine
 */
char *Reception(void);

/**
 * @brief   Envoie un message au client \n
 *          Note : le message doit se terminer par \\n
 * @param message   Chaine de caracteres a envoyer au client
 * @return          int -> Retourne 1 si ca c'est bien passe 0 sinon
 */
int Emission(char *message);

/**
 * @brief Recoit des donnees envoyees par le client
 * 
 * @param donnees   Destination de stockage des donnees recues
 * @param tailleMax Nombre de caracteres max de donnees
 * @return          ssize_t -> Retourn le nombre d'octets recus, 0 si la connexion est fermee,
 *                  un nombre negatif en cas d'erreur
 */
ssize_t ReceptionBinaire(char *donnees, ssize_t tailleMax);

/**
 * @brief Envoie des donnees au client en precisant leur taille
 * 
 * @param donnees   Chaine de caracteres a envoyer au client
 * @param taille    Nombre de caracteres contenus dans la chaine donnees
 * @return          ssize_t -> Retourn le nombre d'octets recus, 0 si la connexion est fermee,
 *                  un nombre negatif en cas d'erreur
 */
ssize_t EmissionBinaire(char *donnees, ssize_t taille);

/**
 * @brief Verification de la constitution de la requete a l'aide d'une regex
 * 
 * @param requete   Requete a verifier emise par le client
 * @return          bool -> Retourne TRUE si la requete est correcte, FALSE sinon
 */
bool verifierRequete(char *requete);

/**
 * @brief Extraction du nom du fichier de la requete
 * 
 * @param requete       Requete du client verifiee
 * @param nomFichier    Destination de stockage du nom de fichier
 * @param maxNomFichier Nombre de caracteres max du nom du fichier
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int extraitFichier(char *requete, char *nomFichier, size_t maxNomFichier);

/**
 * @brief Extraction de l'extension du nom du fichier
 * 
 * @param nomFichier        Nom du fichier demande par le client
 * @param extensionFichier  Destination de stockage de l'extension
 * @param maxExtension      Nombre de caracteres max de l'extension
 * @return                  int -> Retourne 1 si ce s'est bien passe, 0 sinon
 */
int extraitExtension(char *nomFichier, char *extension, size_t maxExtension);

/**
 * @brief Verification de l'existence du fichier et son accessibilite
 * 
 * @param nomFichier    Nom du fichier a rechercher
 * @return              bool -> Retourne TRUE si le fichier est accessible, FALSE sinon
 */
bool verifierAccesFichier(char *nomFichier);

/**
 * @brief Calcul de la taille en octet d'un fichier
 * 
 * @param nomFichier    Nom du fichier dont on veut calculer la taille
 * @return              ssize_t -> Retourne la taille du fichier en octet, -1 si erreur
 */
ssize_t calculTailleFichier(char *nomFichier);

/**
 * @brief Envoie de donnees en provenance d'un fichier texte
 * 
 * @param nomFichier    Source des donnees a envoyer
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerContenuFichierTexte(char *nomFichier);

/**
 * @brief Envoie de donnees en provenance d'un fichier binaire
 * 
 * @param nomFichier    Source des donnees a envoyer
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerContenuFichierBinaire(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 200 Ok pour un fichier html
 * 
 * @param nomFichier    Nom du fichier contenant les donnees qui vont etre envoyees
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse200HTML(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 200 Ok pour un fichier css
 * 
 * @param nomFichier    Nom du fichier contenant les donnees qui vont etre envoyees
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse200CSS(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 200 Ok pour un fichier javascript
 * 
 * @param nomFichier    Nom du fichier contenant les donnees qui vont etre envoyees
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse200JS(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 200 Ok pour un fichier jpg/jpeg
 * 
 * @param nomFichier    Nom du fichier contenant les donnees qui vont etre envoyees
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse200JPG(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 200 Ok pour un fichier ico
 * 
 * @param nomFichier    Nom du fichier contenant les donnees qui vont etre envoyees
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse200ICO(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 404 Not Found
 * 
 * @param nomFichier    Nom du fichier html contenant la page 404
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse404HTML(char *nomFichier);

/**
 * @brief Envoie d'une reponse HTTP 500 Internal Server Error
 * 
 * @param nomFichier    Message a envoyer au client
 * @return              int -> Retourne 1 si ca s'est bien passe, 0 sinon
 */
int envoyerReponse500(char *message);

/**
 * @brief Fermeture de la connexion avec le client
 */
void TerminaisonClient(void);

/**
 * @brief Fermeture du serveur
 */
void Terminaison(void);

#endif
