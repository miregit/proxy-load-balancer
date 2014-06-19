
/*

 * This is modified work by Miroslav Madzarevic, githubcode@osadmin.com as of May 1st 2002
 * based originally on proxy-2.2.4 program by sparlin@openpro.org

 * COPYRIGHT (C) 2002 MIROSLAV MADZAREVIC, githubcode@osadmin.com
 * COPYRIGHT (C) 1998, 1999, 2000  SONNY PARLIN
 * 
 * THIS PROGRAM IS FREE SOFTWARE; YOU CAN REDISTRIBUTE IT AND/OR
 * MODIFY IT UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE
 * AS PUBLISHED BY THE FREE SOFTWARE FOUNDATION; EITHER VERSION 2
 * OF THE LICENSE, OR ANY LATER VERSION.
 *
 * THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 * BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
 * GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF THE GNU GENERAL PUBLIC LICENSE
 * ALONG WITH THIS PROGRAM; IF NOT, WRITE TO THE FREE SOFTWARE
 * FOUNDATION, INC., 59 TEMPLE PLACE - SUITE 330, BOSTON, MA  
 * 02111-1307, USA.

 */


/*-----------------------------------------------*/
/* Universal header for network programming code */
/* Sonny Parlin 12/98				             */
/*-----------------------------------------------*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <getopt.h>

#define PROXYLB_VERSION "proxylb-1.0.3"

/* Define bzero() as a macro incase it's not in std C library */
#define bzero(ptr,n) memset(ptr, 0, n)

/* Miscellaneous constants */
#define MAXLINE         4096    /* max text line length */
#define MAXSOCKADDR  	128     /* max socket address structure size */
#define BUFFSIZE        8192    /* buffer size for reads and writes */

/* uncomment next line for non-posix compliant systems */
#define socklen_t unsigned int

/* Following shortens all the type casts of pointer arguments */
#define SA        struct sockaddr

#define LISTENQ 1024 

// mire
/* definition of a data node holding host information */
struct node {
    char   *host_name;
    int    port;
    struct node *next;
};

/* Function declorations for proxy */
void Close(int descriptor);
void PrintUsage(int n);
void *proxy(void *ptr); // main proxy function
int  GetLargestSocket(int sd1, int sd2);
int  daemon_init(const char *pname); // makes our program an unix daemon
int  FilterIP(char *);  // we have our own filtering if you don't use ipchains or similar
void finish(int sig);   // exit pthread
int  tcp_connect(const char *, const char *);
int  tcp_listen(const char *, const char *, socklen_t *);
char *sock_ntop(const SA *, socklen_t);
char *Sock_ntop(const SA *, socklen_t);

// mire

//      list.c
int         insert_node ( char *host_name, int port, int *ptr_nr_nodes, struct node **ptr_head, int debug );
struct node *get_node   ( int position, int nr_nodes, struct node *head );
int         kill_list   ( struct node **ptr_head, int *ptr_nr_nodes, int debug );

//      config.c
int read_config ( struct node **ptr_head, int *ptr_nr_nodes, char *conf_file, int debug );

//      util.c
int get_random_connection_params( char **_ip, char **_port, struct node *head, int nr_nodes, int debug );



