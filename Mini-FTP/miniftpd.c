/*
 * miniftpd.c
 * Xavier Delord
 * 12.11.2002
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

#include "requests.h"
#include "common.h"


/* Mettre 1 avec un debugger. */
#if 0
#define fork() (0)
#endif


/* get_callername(fd) returns the hostname (or its ip address) on the other side
 * of the  connected socket fd. The name is contained in a static area.
 * (note: this is rather tricky)
 * This is used only to print nice messages about what's going on.
 */
char *get_callername (int fd)
{
    struct sockaddr_in sins;
    int len;
    struct hostent *sp;

    len = sizeof (sins);
    if (getpeername (fd, (struct sockaddr *)&sins, &len) == -1) {
        perror ("getpeername");
        return "Noname";
    }
    sp = gethostbyaddr ((char *)&(sins.sin_addr.s_addr), 4, AF_INET);
    if (sp == NULL) {
        char *rep;
        rep = inet_ntoa (sins.sin_addr);
        fprintf (stderr, "can't reverse address for %s\n", rep);
        return rep;
    }
    return sp->h_name;
} /* get_callername */


/*
 * Reception d'un fichier (requete PUT).
 * 'clientfd' est un socket connecte a un client.
 * 'filename' est le nom du fichier a creer.
 * 'length' est le nombre d'octets attendus.
 */
void put_file (int clientfd, char *filename, int length)
{

    /**** A COMPLETER ****/

} /* put_file */


/*
 * Envoi d'un fichier (requete GET).
 * 'clientfd' est un socket connecte a un client.
 * 'filename' est le nom du fichier a envoyer.
 */
void get_file (int clientfd, char *filename)
{

    /**** A COMPLETER ****/

} /* get_file */


/*
 * Destruction d'un fichier (requete DEL).
 * 'clientfd' est un socket connecte a un client.
 * 'filename' est le nom du fichier a detruire.
 */
void del_file (int clientfd, char *filename)
{

    /**** A COMPLETER ****/

} /* del_file */

/*
 * ls sur un fichier/repertoire (requete DIR).
 * 'clientfd' est un socket connecte a un client.
 * 'pathname' est le nom du fichier ou repertoire a lister.
 */
void dir_file (int clientfd, char *pathname)
{

    /**** A COMPLETER ****/

} /* dir_file */



/*
 * Lit une requete sur le descripteur 'f', et appelle la procedure
 * correspondante pour gerer cette requete.
 */
void handle_request (int f)
{
    struct request r;

    printf ("Process %d, handling connection from %s\n",
            getpid(), get_callername (f));


    /**** A COMPLETER ****/

}


int main ()
{
    struct sockaddr_in soc_in;
    int val;
    int ss;                     /* socket d'ecoute */

    /* creation d'un socket 'ss' : famille IP et type TCP */


    /**** A COMPLETER ****/
 

    /* Force la reutilisation de l'adresse si non allouee */
    val = 1;
    if (setsockopt(ss,SOL_SOCKET,SO_REUSEADDR,(char*)&val,sizeof(val)) == -1) {
        perror ("setsockopt");
        exit (1);
    }

    /*
     * Nomme localement le socket :
     * socket inet, port local PORT, adresse IP locale quelconque
     */


    /**** A COMPLETER ****/


    /* Prepare le socket a la reception de connexions */


    /**** A COMPLETER ****/


    while (1) {

        /**** A COMPLETER ****/

    } /* while (1) */
    /* NOTREACHED */
    return 0;
}
