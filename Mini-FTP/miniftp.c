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

    /**** A COMPLETER ****/

}


/*
 * Envoi d'un fichier a distance.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'localname' est le nom du fichier a envoyer, 'distname' est le nom du
 * fichier resultat (sur la machine distante).
 */
void put_file (int serverfd, char *servername, char *localname, char *distname)
{

    /**** A COMPLETER ****/

}


/*
 * Destruction d'un fichier distant.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'distname' est le nom du fichier a detruire.
 */
void del_file (int serverfd, char *servername, char *distname)
{

    /**** A COMPLETER ****/

}

/*
 * Obtention d'un dir distant.
 * 'serverfd' est un socket connecte a la machine 'servername'.
 * 'distname' est le nom du fichier ou repertoire distant a lister.
 */
void dir_file (int serverfd, char *servername, char *distname)
{

    /**** A COMPLETER ****/

}


/*
 * Retourne un socket connecte la machine 'serverhost' sur le port 'port',
 * ou termine le processus en cas d'echec.
 */
int connection (char *serverhost, unsigned int port)
{

    /**** A COMPLETER ****/

    /*
    return le_socket_connecte_au_serveur;
    */
}


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

