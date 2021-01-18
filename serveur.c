/**
 * @file    serveur.c
 * @author  Coulais Alexandre
 * @brief   Fichier source des fonctions du serveur \n
 *          Les commentaires de description des fonctions,
 *          avec leurs parametres et valeurs de retour sont dans
 *          le fichier d'entete. \n Ici figurent les commentaires
 *          de description du code des fonctions.
 * @version 1.2
 * @date    2020-12-13
 * 
 * @copyright Copyright (c) 2020
 */

#include "serveur.h"

/* Variables cachees */

/* le socket d'ecoute */
int socketEcoute;
/* longueur de l'adresse */
socklen_t longeurAdr;
/* le socket de service */
int socketService;
/* le tampon de reception */
char tamponClient[LONGUEUR_TAMPON];
int debutTampon;
ssize_t finTampon;

int Initialisation() {
    return InitialisationAvecService("13214");
}

int InitialisationAvecService(char *service) {
    int n;
    const int on = 1;
    struct addrinfo hints, *res, *ressave;

    #ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
        printf("WSAStartup() n'a pas fonctionne, erreur : %d\n", WSAGetLastError()) ;
        WSACleanup();
        exit(1);
    }
    memset(&hints, 0, sizeof(struct addrinfo));
    #else
    bzero(&hints, sizeof(struct addrinfo));
    #endif

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(NULL, service, &hints, &res)) != 0)  {
        fprintf(stderr, "Initialisation, erreur de getaddrinfo : %s\n", gai_strerror(n));
        return 0;
    }

    ressave = res;

    do {
        socketEcoute = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (socketEcoute < 0)
            continue;       /* error, try next one */

        setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
#ifdef BSD
        setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
#endif
        if (bind(socketEcoute, res->ai_addr, res->ai_addrlen) == 0)
            break;          /* success */

        close(socketEcoute);    /* bind error, close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        perror("Initialisation, erreur de bind.");
        return 0;
    }

    /* conserve la longueur de l'addresse */
    longeurAdr = res->ai_addrlen;

    freeaddrinfo(ressave);
    /* attends au max 4 clients */
    listen(socketEcoute, 4);
    printf("Creation du serveur reussie sur %s.\n", service);

    return 1;
}

int AttenteClient() {
    struct sockaddr *clientAddr;
    char machine[NI_MAXHOST];

    clientAddr = (struct sockaddr *) malloc(longeurAdr);
    socketService = accept(socketEcoute, clientAddr, &longeurAdr);

    if (socketService == -1) {
        perror("AttenteClient, erreur de accept.");
        return 0;
    }

    if (getnameinfo(clientAddr, longeurAdr, machine, NI_MAXHOST, NULL, 0, 0) == 0) {
        printf("Client sur la machine d'adresse %s connecte.\n", machine);
    } else {
        printf("Client anonyme connecte.\n");
    }

    free(clientAddr);
    /*
     * Reinit buffer
     */
    debutTampon = 0;
    finTampon = 0;

    return 1;
}

char *Reception() {
    char message[LONGUEUR_TAMPON];
    int index = 0;
    int fini = FALSE;
    ssize_t retour = 0;

    while (!fini) {
        /* on cherche dans le tampon courant */
        while ((finTampon > debutTampon) &&
            (tamponClient[debutTampon] != '\n')) {
            message[index++] = tamponClient[debutTampon++];
        }
        /* on a trouve ? */
        if ((index > 0) && (tamponClient[debutTampon] == '\n')) {
            message[index++] = '\n';
            message[index] = '\0';
            debutTampon++;
            fini = TRUE;
#ifdef WIN32
            return _strdup(message);
#else
            return strdup(message);
#endif
        } else {
            /* il faut en lire plus */
            debutTampon = 0;
            retour = recv(socketService, tamponClient, LONGUEUR_TAMPON, 0);

            if (retour < 0) {
                perror("Reception, erreur de recv.");
                return NULL;
            } else if (retour == 0) {
                fprintf(stderr, "Reception, le client a ferme la connexion.\n");
                return NULL;
            } else {
                /*
                 * on a recu "retour" octets
                 */
                finTampon = retour;
            }
        }
    }

    return NULL;
}

int Emission(char *message) {
    size_t taille;

    if (strstr(message, "\n") == NULL) {
        fprintf(stderr, "Emission, Le message n'est pas termine par \\n.\n");
        return 0;
    }

    taille = strlen(message);

    if (send(socketService, message, taille, 0) == -1) {
        perror("Emission, probleme lors du send.");
        return 0;
    }

    return 1;
}

ssize_t ReceptionBinaire(char *donnees, ssize_t tailleMax) {
    ssize_t dejaRecu = 0;
    ssize_t retour = 0;
    // on commence par recopier tout ce qui reste dans le tampon
    while ((finTampon > debutTampon) && (dejaRecu < tailleMax)) {
        donnees[dejaRecu] = tamponClient[debutTampon];
        dejaRecu++;
        debutTampon++;
    }
    /**
     * si on n'est pas arrive au max
     * on essaie de recevoir plus de donnees
     */
    if (dejaRecu < tailleMax) {
        retour = recv(socketService, donnees + dejaRecu, (size_t) (tailleMax - dejaRecu), 0);

        if (retour < 0) {
            perror("ReceptionBinaire, erreur de recv.");
            return -1;
        } else if (retour == 0) {
            fprintf(stderr, "ReceptionBinaire, le client a ferme la connexion.\n");
            return 0;
        } else {
            // on a recu "retour" octets en plus
            return dejaRecu + retour;
        }
    } else {
        return dejaRecu;
    }
}

ssize_t EmissionBinaire(char *donnees, ssize_t taille) {
    ssize_t retour = 0;
    retour = send(socketService, donnees, (size_t) taille, 0);

    if (retour == -1) {
        perror("Emission, probleme lors du send.");
        return -1;
    } else {
        return retour;
    }
}

bool verifierRequete(char *requete) {
    int match;
    regex_t preg;
    const char *str_regex = "^GET \\/\\S* HTTP\\/1\\.1";

    // On fabrique la regex
    if (regcomp(&preg, str_regex, REG_EXTENDED) != 0) {
        fprintf(stderr, "Erreur a la construction de l'expression reguliere\n");
        return FALSE;
    }

    // On cherche les correspondances dans la chaine de caracteres
    match = regexec(&preg, requete, 0, NULL, 0);
    regfree(&preg);

    // Si pas de correspondance on retourne FALSE
    if (match == REG_NOMATCH) {
        fprintf(stderr, "%s n'est pas une requete valide\n", requete);
        return FALSE;
    }

    return TRUE;
}

int extraitFichier(char *requete, char *nomFichier, size_t maxNomFichier) {
    size_t offsetStart = 0;
    size_t offsetEnd = 0;
    size_t longueur = 0;

    // Recherche du debut du nom du fichier
    while (requete[offsetStart] != '/') {
        offsetStart++;
    }

    // On ajoute 1 à offsetStart pour se placer sur le premier caractere
    offsetEnd = ++offsetStart;

    // On recherche la fin du nom du fichier
    while (requete[offsetEnd] != ' ') {
        offsetEnd++;
    }

    longueur = offsetEnd - offsetStart;

    // Si la longueur calculee ne rentre pas dans nomFichier, on arrete la recherche
    if (longueur >= maxNomFichier) {
        fprintf(stderr, "Nom fichier demande trop long\n");
        return 0;
    }

    // Sinon, on met quelque chose dans nomFichier
    if (longueur == 0) {
        // S'il n'y a aucun caractere apres le / on renvoie par defaut index.html
        strncpy(nomFichier, "index.html", maxNomFichier);
        nomFichier[strlen("index.html")] = '\0';
    } else {
        // Sinon on recopie le nom du fichier
        strncpy(nomFichier, &requete[offsetStart], longueur);
        nomFichier[longueur] = '\0';
    }

    return 1;
}

int extraitExtension(char *nomFichier, char *extensionFichier, size_t maxExtension) {
    size_t offsetStart = 0;
    size_t offsetEnd = strlen(nomFichier);
    size_t longueur = 0;

    // On recherche le debut de l'extension
    while (nomFichier[offsetStart] != '.') {
        offsetStart++;
    }

    // On incremente offsetStart pour se placer sur le premier caractere de l'extension
    longueur = offsetEnd - ++offsetStart;

    // Si la longueur calculee depasse la capacite de la destination on arrete la recherche
    if (longueur >= maxExtension) {
        fprintf(stderr, "Extension du fichier demande trop longue\n");
        return 0;
    }

    // On recopie dans extensionFichier l
    strncpy(extensionFichier, &nomFichier[offsetStart], longueur);
    extensionFichier[longueur] = '\0';

    return 1;
}

bool verifierAccesFichier(char *nomFichier) {
    // Si le fichier n'existe pas ou qu'il n'est pas lisible on retourne FALSE
    if (access(nomFichier, F_OK | R_OK) < 0) {
        fprintf(stderr, "Fichier %s non accessible\n", nomFichier);
        return FALSE;
    }

    return TRUE;
}

ssize_t calculTailleFichier(char *nomFichier) {
    FILE *file = NULL;
    ssize_t size = 0;

    // On test l'ouverture du fichier, et on retourne -1 si probleme
    if ((file = fopen(nomFichier, "rb")) == NULL) {
        fprintf(stderr, "Erreur lors de la tentative d'ouverture du fichier %s\n", nomFichier);
        return -1;
    }

    // On place le curseur a la fin du buffer et on demande sa position
    fseek(file, 0, SEEK_END);
    size = ftell(file);

    fclose(file);

    return size;
}

int envoyerContenuFichierTexte(char *nomFichier) {
    FILE *file = NULL;
    char *tampon = NULL;
    ssize_t tailleFichier;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On ouvre le fichier en controlant les erreurs
    if ((file = fopen(nomFichier, "rb")) == NULL) {
        fprintf(stderr, "Erreur a l'ouverture du fichier %s\n", nomFichier);
        return 0;
    }

    // On alloue la taille necessaire pour stocker le contenu du fichier en controlant les erreurs
    if ((tampon = malloc((size_t) (tailleFichier+1) * sizeof(char))) == NULL) {
        fprintf(stderr, "Erreur d'allocation memoire\n");
        return 0;
    }

    // On lit le contenu du fichier en le stockant dans la memoire allouee en verifiant l'absence d'erreur
    if (fread(tampon, sizeof(char), (size_t) tailleFichier, file) != (size_t) tailleFichier) {
        fprintf(stderr, "Erreur a la lecture du fichier %s\n", nomFichier);
        return 0;
    }
    
    tampon[tailleFichier] = '\0';

    // On emet le contenu du fichier en controlant les erreurs
    if (!(Emission(tampon))) {
        fprintf(stderr, "Erreur lors de l'emission des donnees\n");
        return 0;
    }

    free(tampon);
    fclose(file);

    return 1;
}

int envoyerContenuFichierBinaire(char *nomFichier) {
    FILE *file = NULL;
    char *tampon = NULL;
    ssize_t tailleFichier;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On ouvre le fichier en controlant les erreurs
    if ((file = fopen(nomFichier, "rb")) == NULL) {
        fprintf(stderr, "Erreur a l'ouverture du fichier %s\n", nomFichier);
        return 0;
    }

    // On alloue la taille necessaire pour stocker le contenu du fichier en controlant les erreurs
    if ((tampon = malloc((size_t) (tailleFichier+1) * sizeof(char))) == NULL) {
        fprintf(stderr, "Erreur d'allocation memoire\n");
        return 0;
    }

    // On lit le contenu du fichier en le stockant dans la memoire allouee en verifiant l'absence d'erreur
    if (fread(tampon, sizeof(char), (size_t) tailleFichier, file) != (size_t) tailleFichier) {
        fprintf(stderr, "Erreur a la lecture du fichier %s\n", nomFichier);
        return 0;
    }

    tampon[tailleFichier] = '\0';

    // On emet le contenu du fichier en controlant les erreurs
    if (EmissionBinaire(tampon, tailleFichier+1) == -1) {
        fprintf(stderr, "Erreur lors de l'emission des donnees\n");
        return 0;
    }

    free(tampon);
    fclose(file);

    return 1;
}

int envoyerReponse200HTML(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 200 OK\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: text/html\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse200CSS(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 200 OK\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: text/css\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse200JS(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 200 OK\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: application/javascript\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse200JPG(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 200 OK\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: image/jpeg\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse200ICO(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 200 OK\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: image/x-icon\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse404HTML(char *nomFichier) {
    char aux[50];
    ssize_t tailleFichier = 0;

    // On recupere la taille du fichier en retournant 0 si une erreur se produit
    if ((tailleFichier = calculTailleFichier(nomFichier)) == -1) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 404 Not Found\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: text/html\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du fichier en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleFichier) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    return Emission(aux);
}

int envoyerReponse500(char *message) {
    char aux[50];
    size_t tailleMessage = 0;

    // On recupere la longueur de la chaine en retournant 0 si une erreur se produit
    if ((tailleMessage = strlen(message)) == 0) {
        return 0;
    }

    // On emet sequentiellement les differentes lignes de la reponse
    // On s'arrete si une erreur se produit
    if (!(Emission("HTTP/1.1 500 Internal Server Error\n"))) {
        return 0;
    }

    if (!(Emission(STR_SERVER))) {
        return 0;
    }

    if (!(Emission("Content-type: text/html\n"))) {
        return 0;
    }

    // On copie dans la chaine auxiliaire la taille du message en verifiant une potentielle erreur
    if (snprintf(aux, 49, "Content-length: %lu\n\n", tailleMessage) < 0) {
        fprintf(stderr, "Erreur au remplissage de content-length\n");
        return 0;
    }

    if (!(Emission(aux))) {
        return 0;
    }

    return Emission(message);
}

void TerminaisonClient() {
    close(socketService);
}

void Terminaison() {
    close(socketEcoute);
}