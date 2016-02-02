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
    struct answer reponse;
    struct stat stat_fich;

    /*  Construction de la reponse */
    reponse.errnum = 0;
    reponse.nbbytes = 0;
    reponse.ack = ANSWER_OK;

    /* Creation du fichier */
    printf("Creation du fichier \n");
    int fichier = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(fichier != -1)
    {
        printf("Copies \n");
        /* Echange des données  */
        copy_n_bytes (clientfd, fichier, length);
        printf("Fermeture \n");
        /* Fermeture du fichier */
        close (fichier);
    }
    else
    {
        printf ("Erreur lors de la creation du fichier \n");
    }

    /*  Envoi de la réponse */
    if(write (clientfd, &reponse, sizeof(reponse)) == -1)
    {
        fprintf (stderr, "Erreur lors de l'écriture du fichier\n");
        exit (errno);
    }

} /* put_file */


/*
 * Envoi d'un fichier (requete GET).
 * 'clientfd' est un socket connecte a un client.
 * 'filename' est le nom du fichier a envoyer.
 */
void get_file (int clientfd, char *filename)
{
    struct answer reponse;
    struct stat buf;

    /*  Construction de la reponse */
    reponse.errnum = 0;
    reponse.nbbytes = 0;
    reponse.ack = ANSWER_OK;

    /* Récupération du fichier */
    printf("Récupération du fichier \n");
    int fichier = open(filename, O_RDONLY);
    /*Verifie si le fichier existe*/
    if (fichier != -1){
        reponse.ack = ANSWER_OK;
    }else{
        printf ("Le fichier n'existe pas\n");
        reponse.ack = ANSWER_ERROR;
        reponse.errnum = errno;
    }
    stat(filename, &buf);
    reponse.nbbytes = (int) buf.st_size;

    /*  Envoi de l'accusé de réponse */
    if(write (clientfd, &reponse, sizeof(struct answer)) == -1)
    {
        fprintf (stderr, "Erreur lors de l'écriture de la reponse\n");
        exit (errno);
    }
    /*  Envoi de le fichier */
    copy_all_bytes (fichier, clientfd);
    close (fichier);

} /* get_file */


/*
 * Destruction d'un fichier (requete DEL).
 * 'clientfd' est un socket connecte a un client.
 * 'filename' est le nom du fichier a detruire.
 */
void del_file (int clientfd, char *filename)
{
    /*Création réponse*/
    struct answer rep_del;
    rep_del.ack = ANSWER_OK;

    /*  Construction de la reponse */
    rep_del.errnum = 0;

    /* Verification de l'existence fichier a supprimer */
    FILE* file_exist = fopen(filename,"r");
    if (file_exist != NULL)
    {
        rep_del.ack = ANSWER_OK;
    }
    else
    {
        printf ("Le fichier n'existe pas\n");
        printf("%s",filename);
        rep_del.ack = ANSWER_ERROR;
        rep_del.errnum = errno;
    }
    fclose (file_exist);

    /*  Envoi de la réponse */
    printf("Envoi de la réponse\n");
    if(write (clientfd, &rep_del, sizeof(rep_del)) == -1)
    {
        fprintf (stderr, "Erreur lors de l'écriture de la reponse\n");
        exit (errno);
    }

    /*Suppression du fichier*/
    unlink(filename);

    puts("suppression reussi");

    /**** A COMPLETER ****/

} /* del_file */

/*
 * ls sur un fichier/repertoire (requete DIR).
 * 'clientfd' est un socket connecte a un client.
 * 'pathname' est le nom du fichier ou repertoire a lister.
 */
void dir_file (int clientfd, char *pathname)
{
    struct answer reponse;
    struct stat stat_fich;

    reponse.errnum = 0;
    reponse.nbbytes = 0;
    FILE* file_exist = fopen(pathname,"r");

    if (file_exist!=NULL)
    {
        reponse.ack = ANSWER_OK;
    }
    else
    {
        printf ("Le dossier n'existe pas\n");
        reponse.ack = ANSWER_ERROR;
        reponse.errnum = errno;
    }

    if(stat(pathname, &stat_fich) == -1)
    {
        reponse.errnum = errno;
        fprintf (stderr, "Erreur, impossible de récupéré les informations\n");
        exit (errno);
    }
    else
    {
        reponse.nbbytes = stat_fich.st_size;
    }

    fclose (file_exist);

    if(write (clientfd, &reponse, sizeof(reponse)) == -1)
    {
        fprintf (stderr, "Erreur lors de l'écriture du fichier\n");
        exit (errno);
    }

    if(reponse.ack != ANSWER_ERROR)
    {
        /*  Sortie standard*/
        dup2(clientfd, STDOUT_FILENO);
        /*  Sortie d' erreur*/
        dup2(clientfd, STDERR_FILENO);
        execlp("ls", "ls", "-l", pathname, 0);
    }
    printf ("Fin");

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

    /* Lecture de la requete */
    read (f, &r, sizeof(r));

    /* Identification de la requete*/
    switch(r.kind)
    {
        case REQUEST_PUT :
            put_file (f, r.path, r.nbbytes);
            break;
        case REQUEST_GET :
            get_file (f, r.path);
            break;
        case REQUEST_DEL :
            del_file (f, r.path);
            break;
        case REQUEST_DIR :
            dir_file(f, r.path);
            break;
        default :
            printf ("Erreur, la requete n'a pas été identifiée\n");
            break;
    }
}


int main ()
{
    struct sockaddr_in soc_in;
    struct sockaddr_in info_client;
    int val;
    int ss;                     /* socket d'ecoute */
    int new, addrlen;
    struct hostent *host;
    /* creation d'un socket 'ss' : famille IP et type TCP */
    ss = socket (AF_INET, SOCK_STREAM, 0);

    if (ss <= 0)
    {
        fprintf (stderr, "Probleme lors de la creation du socket\n");
        exit(errno);
    }

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
    soc_in.sin_family = 		AF_INET;
    soc_in.sin_addr.s_addr = INADDR_ANY;
    soc_in.sin_port = htons(PORT);

    if(bind (ss, (struct sockaddr*)&soc_in, sizeof(struct sockaddr_in)) != 0)
    {
        fprintf (stderr, "Probleme de bind\n");
        exit (errno);
    }

    /* Prepare le socket a la reception de connexions */
    listen(ss, 10);
    addrlen = sizeof(struct sockaddr_in);

    /* Attend une demande de connexion */
    while (1)
    {
        printf ("Attente... \n");
        new = accept(ss,(struct sockaddr*)&info_client, &addrlen);

        if(!fork())
        {
            /* Processus Fils */
            /* Fermeture du processus pére */
            close(ss);
            handle_request(new);
            close(new);
            exit(0);
        }
        else
        {
            /* Processus Pére */
            /* Fermeture du processus fils*/
            close(new);
            wait (NULL);
            printf (" Processus fils termine\n");
        }
    }
    return 0;
}
