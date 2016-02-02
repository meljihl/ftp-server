/*
 * miniftp.c
 * Xavier Delord
 * 12.11.2002
 */

/*
 * miniftp hostname get distfile  localfile
 * miniftp hostname put localfile distfile
 * miniftp hostname del file
 * miniftp hostname dir file
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>

#include "requests.h"
#include "common.h"

/*
 * check_answer ne rend la main que si  a->ack est ANSWER_OK,
 * sinon le processus se termine avec un message d'erreur.
 */
void check_answer (struct answer *a) {
    switch (a->ack) {
        case ANSWER_OK:
            break;
        case ANSWER_UNKNOWN:
            exit (1);
        case ANSWER_ERROR:
            if (a->errnum != 0) {
                errno = a->errnum;
                perror ("Server rejection");
            } else {
                fprintf (stderr, "Server rejection\n");
            }
            exit (1);
        default:
            exit (1);
    }
}

//Obtention d'un fichier distant
void get_file (int serverfd, char *servername, char *distname, char *localname)
{
    struct request request;
    struct answer response;
    struct stat buf;

    //Construction de la request
    request.kind = REQUEST_GET;
    strcpy(request.path, distname);

    //Envoie la request au serveur
    if (write(serverfd, &request,sizeof(struct request)) == -1) {
        exit(EXIT_FAILURE);
    }

    //Lit la réponse du serveur
    if (read(serverfd, &response, sizeof(struct answer)) == -1) {
        exit(EXIT_FAILURE);
    }

    //Vérification de la réponse
    check_answer(&response);

    int fd = open(localname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if( fd == -1) {
        perror("Probleme de copie");
        exit(1);
    }

    //Echange de données
    copy_all_bytes(serverfd, fd);
    close(fd);
    close(serverfd);
    printf("Fichier %s recu \n", localname);

}


//Envoi d'un fichier a distance.
void put_file (int serverfd, char *servername, char *localname, char *distname) {
    struct request request;
    struct answer response;
    struct stat buf;

    //Construction de la request
    request.kind = REQUEST_PUT;
    strcpy(request.path, distname);

    if(stat(localname,&buf) == -1) {
        perror(localname);
        exit(1);
    }

    request.nbbytes = buf.st_size;

    //Envoie la request au serveur
    if (write(serverfd, &request,sizeof(request)) == -1) {
        exit(EXIT_FAILURE);
    }

    //Lit la réponse du serveur
    if (read(serverfd, &response, sizeof(response) == -1) == -1 ) {
        exit(EXIT_FAILURE);
    }

    //Vérification de la réponse
    check_answer(&response);

    int fd = open(localname, O_RDONLY);
    if( fd == -1) {
        perror("Probleme de copy");
        exit(1);
    }

    //Echange de données
    copy_all_bytes(fd, serverfd);
    close(fd);
    printf("Fichier %s envoye \n", localname);

}


//Destruction d'un fichier distant.
void del_file (int serverfd, char *servername, char *distname) {
    struct request Client_DEL;
    struct answer response_DEL;
    int del, test;

    Client_DEL.kind = REQUEST_DEL;
    strcpy(Client_DEL.path,distname);
    /*Envoie la request sur le serveur*/
    test = write(serverfd, &Client_DEL, sizeof(Client_DEL));
    if (test == -1) {
        perror("Erreur request");
        exit(EXIT_FAILURE);
    }
    //Lit la réponse du serveur
    test = read(serverfd, &response_DEL, sizeof(response_DEL));
    if (test == -1) {
        perror("Erreur réponse");
        exit(EXIT_FAILURE);
    }

    check_answer(&response_DEL);

    printf("Suppression fichier OK \n");

}

//Obtention d'un dir distant.
void dir_file (int serverfd, char *servername, char *distname) {
    struct request request;
    struct answer response;

    //Construction de la request
    request.kind = REQUEST_DIR;
    strcpy(request.path, distname);

    //Envoie la request au serveur
    if (write(serverfd, &request, sizeof(request)) == -1) {
        exit(EXIT_FAILURE);
    }

    //Lit la réponse du serveur
    if (read(serverfd,&response, sizeof(response)) == -1) {
        exit(EXIT_FAILURE);
    }

    //Vérification de la réponse
    check_answer(&response);

    //Echange de données
    copy_all_bytes(serverfd, STDOUT_FILENO);
    close(serverfd);
    exit(EXIT_SUCCESS);
}


//Retourne un socket ou termine le processus en cas d'echec.
int connection (char *serverhost, unsigned int port) {
    //Description de la socket
    int retour;
    struct sockaddr_in adresse;

    struct hostent *sp;

    sp = gethostbyname(serverhost);
    if (sp == NULL) {
        exit(EXIT_FAILURE);
    }

    int sock = socket(PF_INET,SOCK_STREAM,0);

    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    memcpy(&adresse.sin_addr.s_addr, sp->h_addr_list[0],sp->h_length);

    if (sock != -1) {
        fprintf(stdout, "[%i] [desc %i]: Le socket client est rattache au port %i \n", getpid(), sock, ntohs(adresse.sin_port));
    } else {
        exit(EXIT_FAILURE);
    }

    //Connection de la socket client au serveur
    retour = connect(sock,(struct sockaddr *)&adresse, sizeof(adresse));

    if (retour == -1) {
        exit(EXIT_FAILURE);
    } else {
        printf("connection etablie\n");
    }

    //Retourne le_socket_connecte_au_serveur
    return(sock);


}

void usage (void) {
    fprintf (stderr, "miniftp: hostname get distfilename localfilename\n");
    fprintf (stderr, "miniftp: hostname put localfilename distfilename\n");
    fprintf (stderr, "miniftp: hostname del distfilename\n");
    fprintf (stderr, "miniftp: hostname dir pathname\n");
    exit (2);
}

int main (int argc, char **argv) {
    char *serverhost;
    int cmde;
    int serverfd;

    if (argc < 3) {
        usage();
    }
    serverhost = argv[1];

    if ((strcmp(argv[2], "put") == 0) && (argc == 5)){
        cmde = REQUEST_PUT;
    } else if ((strcmp(argv[2], "get") == 0) && (argc == 5)){
        cmde = REQUEST_GET;
    } else if ((strcmp(argv[2], "del") == 0) && (argc == 4)){
        cmde = REQUEST_DEL;
    } else if ((strcmp(argv[2], "dir") == 0) && (argc == 4)){
        cmde = REQUEST_DIR;
    } else { usage();
    }
    serverfd = connection (serverhost, PORT);

    switch (cmde) {
        case REQUEST_GET:
            get_file (serverfd, serverhost, argv[3], argv[4]);
            break;
        case REQUEST_PUT:
            put_file (serverfd, serverhost, argv[3], argv[4]);
            break;
        case REQUEST_DEL:
            del_file (serverfd, serverhost, argv[3]);
            break;
        case REQUEST_DIR:
            dir_file (serverfd, serverhost, argv[3]);
            break;
    }

    close (serverfd);
    return 0;
}

