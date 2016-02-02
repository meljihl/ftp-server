/*
 * common.c
 * Xavier Delord
 * 12.11.2002
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

#define MSGSIZE 512

/*
 * Copie `nbbytes' octets depuis le descripteur `from' sur le descripteur `to'.
 * En cas d'echec, affiche un message explicatif.
 * Renvoie 0 si ok, -1 en cas d'erreur.
 *
 * La lecture est faite par bloc de MSGSIZE octets. L'ecriture s'attend
 * a ne pas pouvoir tout ecrire d'un seul coup.
 */
int copy_n_bytes (int from, int to, int nbbytes)
{
    char buf[MSGSIZE];
    char *p;
    while (nbbytes != 0) {
        int ecrit;
        int lu;
        lu = read (from, buf, ((nbbytes < MSGSIZE) ? nbbytes : MSGSIZE));
        if (lu == -1) {
            perror ("bad read");
            return -1;
        } else if (lu == 0) {
            fprintf (stderr, "EOF (connection closed)\n");
            return -1;
        }
        nbbytes -= lu;
        p = buf;
        while (lu != 0) {
            ecrit = write (to, p, lu);
            if (ecrit == -1) {
                perror ("bad write");
                return -1;
            }
            p += ecrit;
            lu -= ecrit;
        }
    }
    return 0;
}

/*
 * Copie depuis le descripteur `from' sur le descripteur `to' jusqu'a
 * ce qu'une lecture sur `from' renvoie end-of-file.
 * En cas d'echec, affiche un message explicatif.
 * Renvoie 0 si ok, -1 en cas d'erreur.
 *
 * La lecture est faite par bloc de MSGSIZE octets. L'ecriture s'attend
 * a ne pas pouvoir tout ecrire d'un seul coup.
 */
int copy_all_bytes (int from, int to)
{
    char buf[MSGSIZE];
    char *p;
    while (1) {
        int ecrit;
        int lu;
        lu = read (from, buf, MSGSIZE);
        if (lu == -1) {
            perror ("bad read");
            return -1;
        } else if (lu == 0) {
            return 0;
        }
        p = buf;
        while (lu != 0) {
            ecrit = write (to, p, lu);
            if (ecrit == -1) {
                perror ("bad write");
                return -1;
            }
            p += ecrit;
            lu -= ecrit;
        }
    }
    /* NOTREACHED */
    return 0;
}

/*
 * Comme perror, mais avec deux chaines de caracteres.
 * perror2 ("Echec ouverture du fichier", filename);
 */
void perror2 (char *s1, char *s2)
{
    if (s2 == NULL) {
        perror (s1);
    } else if (s1 == NULL) {
        perror (s2);
    } else {
        fprintf (stderr, "%s ", s1);
        perror (s2);
    }
}
