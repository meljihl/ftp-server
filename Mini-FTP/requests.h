/*
 * requests.h
 * Xavier Delord
 * 12.11.2002
 */

#define MAXPATH 256

/* Client requests */
#define REQUEST_PUT 1
#define REQUEST_GET 2
#define REQUEST_DEL 3
#define REQUEST_DIR 4

/* Server answers */
#define ANSWER_OK      0
#define ANSWER_UNKNOWN 1        /* unknown request */
#define ANSWER_ERROR   2        /* error while processing request */

struct request {
    int  kind;
    char path[MAXPATH];
    int  nbbytes;               /* for PUT only */
};

struct answer {
    int  ack;
    int  nbbytes;               /* for GET only */
    int  errnum;                /* significant if ack == ERROR and != 0 */
};

/* Port number deduced from UID allowing many users on a sigle host */
#define PORT (IPPORT_USERRESERVED + getuid())
