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
void check_answer (struct answer *a)
{
    switch (a->ack) {
        case ANSWER_OK:
            break;
        case ANSWER_UNKNOWN:
            fprintf (stderr, "?? unknown request ?\n");
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
            fprintf (stderr, "Bad answer from server %d\n", a->ack);
            exit (1);
    }
}

/*
 * Obtention d'un fichier distant.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'distname' est le nom du fichier a demander, 'localname' est le nom du
 * fichier resultat.
 */
void get_file (int serverfd, char *servername, char *distname, char *localname)
{
    struct request requete;
    struct answer reponse;
    struct stat buf;

    /*  Construction de la requete */
    requete.kind = REQUEST_GET;
    strcpy(requete.path, distname);

    /*  Envoie la requete au serveur */
    if (write(serverfd, &requete,sizeof(struct request)) == -1)
    {
        fprintf(stdout,"[%i] : Erreur lors de l'envoi de la requete au serveur\n",getpid());
        exit(EXIT_FAILURE);
    }

    /*  Lit la r�ponse du serveur */
    if (read(serverfd, &reponse, sizeof(struct answer)) == -1)
    {
        fprintf(stdout, "[%i] : Erreur lors de la lecture de la reponse\n", getpid());
        exit(EXIT_FAILURE);
    }

    /*  V�rification de la r�ponse */
    check_answer(&reponse);

    int fd = open(localname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if( fd == -1)
    {
        perror("Probleme de copie");
        exit(1);
    }

    /*  Echange de donn�es */
    copy_all_bytes(serverfd, fd);
    close(fd);
    close(serverfd);
    printf("Fichier %s recu \n", localname);

}


/*
 * Envoi d'un fichier a distance.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'localname' est le nom du fichier a envoyer, 'distname' est le nom du
 * fichier resultat (sur la machine distante).
 */
void put_file (int serverfd, char *servername, char *localname, char *distname)
{
    struct request requete;
    struct answer reponse;
    struct stat buf;

    /*  Construction de la requete */
    requete.kind = REQUEST_PUT;
    strcpy(requete.path, distname);

    if(stat(localname,&buf) == -1)
    {
        perror(localname);
        exit(1);
    }

    requete.nbbytes = buf.st_size;

    /*  Envoie la requete au serveur */
    if (write(serverfd, &requete,sizeof(requete)) == -1)
    {
        fprintf(stdout,"[%i] : Erreur lors de l'envoi de la requete au serveur\n",getpid());
        exit(EXIT_FAILURE);
    }

    /*  Lit la r�ponse du serveur */
    if (read(serverfd, &reponse, sizeof(reponse) == -1) == -1 )
    {
        fprintf(stdout, "[%i] : Erreur lors de la lecture de la reponse\n", getpid());
        exit(EXIT_FAILURE);
    }

    /*  V�rification de la r�ponse */
    check_answer(&reponse);

    int fd = open(localname, O_RDONLY);
    if( fd == -1)
    {
        perror("Probleme de copy");
        exit(1);
    }

    /*  Echange de donn�es */
    copy_all_bytes(fd, serverfd);
    close(fd);
    printf("Fichier %s envoye \n", localname);

}


/*
 * Destruction d'un fichier distant.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'distname' est le nom du fichier a detruire.
 */
void del_file (int serverfd, char *servername, char *distname)
{
    struct request Client_DEL;
    struct answer Reponse_DEL;
    int del, test;

    Client_DEL.kind = REQUEST_DEL;
    strcpy(Client_DEL.path,distname);
    /*Envoie la requete sur le serveur*/
    test = write(serverfd, &Client_DEL, sizeof(Client_DEL));
    if (test == -1)
    {
        perror("Erreur requete");
        exit(EXIT_FAILURE);
    }
    /*Lit la r�ponse du serveur*/
    test = read(serverfd, &Reponse_DEL, sizeof(Reponse_DEL));
    if (test == -1)
    {
        perror("Erreur r�ponse");
        exit(EXIT_FAILURE);
    }

    check_answer(&Reponse_DEL);

    printf("Suppression fichier OK \n");
    /**** A COMPLETER ****/

}

/*
 * Obtention d'un dir distant.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'distname' est le nom du fichier ou repertoire distant a lister.
 */
void dir_file (int serverfd, char *servername, char *distname)
{
    struct request requete;
    struct answer reponse;

    /*  Construction de la requete */
    requete.kind = REQUEST_DIR;
    strcpy(requete.path, distname);

    /*  Envoie la requete au serveur */
    if (write(serverfd, &requete, sizeof(requete)) == -1)
    {
        fprintf(stdout,"[%i] : Erreur lors de l'envoi de la requete au serveur\n",getpid());
        exit(EXIT_FAILURE);
    }

    /*  Lit la r�ponse du serveur */
    if (read(serverfd,&reponse, sizeof(reponse)) == -1)
    {
        fprintf(stdout, "[%i] : Erreur lors de la lecture de la reponse\n", getpid());
        exit(EXIT_FAILURE);
    }

    /*  V�rification de la r�ponse */
    check_answer(&reponse);

    /*  Echange de donn�es */
    copy_all_bytes(serverfd, STDOUT_FILENO);
    close(serverfd);
    exit(EXIT_SUCCESS);
    /**** A COMPLETER ****/

}


/*
 * Retourne un socket connecte la machine 'serverhost' sur le port 'port',
 * ou termine le processus en cas d'echec.
 */
int connection (char *serverhost, unsigned int port)
{
    /* Descripteur de la socket */
    int retour;
    struct sockaddr_in adresse;

    struct hostent *sp;

    sp = gethostbyname(serverhost);
    if (sp == NULL)
    {
        fprintf(stdout, "Machine %s inconnue !\n", serverhost);
        exit(EXIT_FAILURE);
    }

    int sock = socket(PF_INET,SOCK_STREAM,0);

    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    memcpy(&adresse.sin_addr.s_addr, sp->h_addr_list[0],sp->h_length);

    if (sock != -1)
    {
        fprintf(stdout, "[%i] [desc %i]: Le socket client est rattache au port %i \n", getpid(), sock, ntohs(adresse.sin_port));
    }
    else
    {
        fprintf(stdout, "[%i] [desc %i]: Le socket client n'a pas reussi a se rattacher au port %i \n", getpid(), sock, ntohs(adresse.sin_port));
        exit(EXIT_FAILURE);
    }

    /* Connection de la socket client au serveur */
    retour = connect(sock,(struct sockaddr *)&adresse, sizeof(adresse));

    if (retour == -1)
    {
        fprintf(stdout, "[%i] connection impossible !\n", getpid());
        exit(EXIT_FAILURE);
    }
    else {
        printf("connection etablie\n");
    }

    /* Retourne le_socket_connecte_au_serveur  */
    return(sock);


}

/**** A COMPLETER ****/
void usage (void)
{
    fprintf (stderr, "miniftp: hostname get distfilename localfilename\n");
    fprintf (stderr, "miniftp: hostname put localfilename distfilename\n");
    fprintf (stderr, "miniftp: hostname del distfilename\n");
    fprintf (stderr, "miniftp: hostname dir pathname\n");
    exit (2);
}

int main (int argc, char **argv)
{
    char *serverhost;
    int cmde;
    int serverfd;

    if (argc < 3)
        usage ();

    serverhost = argv[1];

    if ((strcmp (argv[2], "put") == 0) && (argc == 5))
        cmde = REQUEST_PUT;
    else if ((strcmp (argv[2], "get") == 0) && (argc == 5))
        cmde = REQUEST_GET;
    else if ((strcmp (argv[2], "del") == 0) && (argc == 4))
        cmde = REQUEST_DEL;
    else if ((strcmp (argv[2], "dir") == 0) && (argc == 4))
        cmde = REQUEST_DIR;
    else
        usage();

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

