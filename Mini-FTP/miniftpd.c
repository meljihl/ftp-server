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
#include <string.h>

#include "requests.h"
#include "common.h"


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


void put_file (int socket, char *filename, int length)
{
    struct answer response;
    response.errnum = 0;
    response.nbbytes = 0;
    response.ack = ANSWER_OK;

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (file == -1) {
        exit(errno);
    }

    copy_n_bytes (socket, file, length);

    if(write (socket, &response, sizeof(response)) == -1) {
        exit (errno);
    }

} /* put_file */


void get_file (int socket, char *filename)
{
    struct answer response;
    response.errnum = 0;
    response.nbbytes = 0;
    response.ack = ANSWER_OK;

    struct stat buf;

    int file = open(filename, O_RDONLY);
    if (file != -1) {
        response.ack = ANSWER_OK;
    } else{
        response.ack = ANSWER_ERROR;
        response.errnum = errno;
    }
    stat(filename, &buf);
    response.nbbytes = (int) buf.st_size;

    if(write (socket, &response, sizeof(struct answer)) == -1)
    {
        exit (errno);
    }
    copy_all_bytes (file, socket);
    close (file);

} /* get_file */


void del_file (int socket, char *filename)
{
    struct answer response;
    response.ack = ANSWER_OK;
    response.errnum = 0;

    FILE* file_exist = fopen(filename,"r");

    if (file_exist != NULL) {
        response.ack = ANSWER_OK;
    } else {
        response.ack = ANSWER_ERROR;
        response.errnum = errno;
    }

    fclose (file_exist);
    if(write (socket, &response, sizeof(response)) == -1) {
        exit (errno);
    }

    remove(filename);
} /* del_file */

void dir_file (int socket, char *pathname)
{
    struct answer response;
    response.errnum = 0;
    response.nbbytes = 0;

    struct stat file_stat;


    FILE* file_exist = fopen(pathname,"r");
    if (file_exist != NULL) {
        response.ack = ANSWER_OK;
    } else {
        response.ack = ANSWER_ERROR;
        response.errnum = errno;
    }

    if(stat(pathname, &file_stat) == -1) {
        exit (errno);
    } else {
        response.nbbytes = (int) file_stat.st_size;
    }

    fclose (file_exist);

    if(write (socket, &response, sizeof(response)) == -1) {
        exit (errno);
    }

    if(response.ack != ANSWER_ERROR) {
        // Duplicate standard error file descriptor
        dup2(socket, STDERR_FILENO);

        // Duplicate standard output file descriptor
        dup2(socket, STDOUT_FILENO);

        execlp("ls", "ls", "-l", pathname, 0);
    }
} /* dir_file */



void handle_request (int f)
{
    struct request r;

    printf ("Process %d, handling connection from %s\n",
            getpid(), get_callername (f));

    read(f, &r, sizeof(r));

    if (r.kind == REQUEST_PUT) {
        put_file(f, r.path, r.nbbytes);
    } else if (r.kind == REQUEST_DEL) {
        del_file(f, r.path);
    } else if (r.kind == REQUEST_DIR) {
        dir_file(f, r.path);
    } else {
        get_file(f, r.path);
    }
}


int main () {
    struct sockaddr_in soc_in;
    int val;
    int ss;                     /* socket d'ecoute */

    /* creation d'un socket 'ss' : famille IP et type TCP */
    ss = socket(AF_INET, SOCK_STREAM, 0);

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
    soc_in.sin_family = AF_INET;
    soc_in.sin_addr.s_addr = INADDR_ANY;
    soc_in.sin_port = htons(PORT);

    int bindResult = bind(ss, (const struct sockaddr*)&soc_in, sizeof(soc_in));
    if(bindResult != 0) {
        exit (errno);
    }


    /* Prepare le socket a la reception de connexions */

    if (listen(ss, 5) < 0) { // Fd en mode écoute, maximum 5 connexions
        exit(errno);
    }

    int confd;
    int sizeOfClient = sizeof(soc_in);

    for(;;) {
        confd = accept(ss, (struct sockaddr*) &soc_in, &sizeOfClient);

        if (confd < 0) {
            perror("Failed to accept");
            exit(errno);
        }

        struct hostent* hostp;
        hostp = gethostbyaddr(&soc_in.sin_addr.s_addr, sizeof(soc_in.sin_addr.s_addr), AF_INET);

        if (hostp == NULL) {
            perror("Failed to get host address");
            exit(errno);
        }

        char* hostaddrp = inet_ntoa(soc_in.sin_addr);
        if (hostaddrp == NULL) {
            perror("Failed to get host addr");
            exit(errno);
        }


        /**** A COMPLETER ****/

    } /* while (1) */
    /* NOTREACHED */
    return 0;

}
