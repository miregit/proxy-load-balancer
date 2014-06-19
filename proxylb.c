/*

 * This is modified work by Miroslav Madzarevic githubcode@osadmin.com as of May 1st 2002
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

#include "proxylb.h"

#define MAXFD                   64
#define TRANSFER_BUFFER_SIZE    10000
#define ACCEPT                  0
#define DENY                    1
#define DEFAULT_DENY            2
#define NO_RULE                 3

struct proxy_t {
  int fd;
};

extern int   optind;
extern char *optarg;

static int show_help;
static int show_version;
static int nodaemon;
static int debug;

char *conf_file = NULL;

/* SETUP LONG_OPTS ARRAY FOR ARGUMENTS */
static struct option const long_opts[] = {
    {"nodaemon" , no_argument      , &nodaemon    , 1  },
    {"debug"    , no_argument      , &debug       , 1  },
    {"srcport"  , required_argument, NULL         , 's'},
    {"conf_file", required_argument, NULL         , 'c'},
    {"srcip"    , required_argument, NULL         , 'S'},
    {"help"     , no_argument      , &show_help   , 1  },
    {"version"  , no_argument      , &show_version, 1  },
    {NULL       , 0, NULL, 0}
};

/* void CLOSE()
 * 
 * WRAPPER FOR CLOSE() FUNCTION, DOES ERROR CHECKING.
 */
void Close(int descriptor) {
    if (close (descriptor) == -1)
        syslog (LOG_ERR, "Close problems %m");
}

/* VOID PRINTUSAGE()
 *
 * PRINTS OUT USAGE DIRECTIONS FOR PROXY.
 */
void
PrintUsage (int n) {

    if (n == 1) {

        fprintf (stderr, "Try `proxy --help' for more information.\n");

    } else if (n == 2) {

        fprintf (stderr, "\nUsage: proxylb OPTIONS\n"
           "Proxy and load balance connections from source to dest\n\n"
           "-s, --srcport            Source port        (required)\n"
           "-S, --srcip              Source IP          (optional)\n"
           "-c, --config             Configuration file (optional)\n"
           "    if not given ./config.txt will be used\n"
           "-n, --nodaemon           Run proxy _NOT_ as UNIX daemon (defaults to daemon mode)\n"
           "-d, --debug              Turn on syslog debugging\n"
           "    --help               display this help and exit\n"
           "    --version            output version information and exit\n\n"
           "This is " PROXYLB_VERSION "\n\n"

           "Report bugs, success stories to Miroslav Madzarevic, githubcode@osadmin.com\n"
           "This is modified work as of May 1st 2002, "
           "based originally on proxy-2.2.4 program by sparlin@openpro.org "
           "using GPL licence version 2.\n"
           "The licence can be found in LICENCE file in this programs directory.\n\n"
        );
    } else {
        fprintf (stderr, PROXYLB_VERSION "\n" );
    }
    exit (0);
}

/*
 * INT GETLARGESTSOCKET()
 *
 * COMPARES THE TWO SOCKET DESCRIPTORS AND RETURNS THE ONE WITH THE
 * LARGEST VALUE, PLUS ONE
 */
int GetLargestSocket(int sd1, int sd2) {
    if (sd1 > sd2)
        return(sd1 + 1);
    else
        return(sd2 + 1);
}

/*
 * VOID * PROXY()
 *
 * READS DATA FROM SOCKET DESCRIPTOR CREATED BY NEW CLIENT 
 * AND PASSES IT TO SOCKET DESCRIPTOR WE CREATE. SYNCHRONOUS
 * I/O MULTIPLEXING IS DONE VIA SELECT. SEE SELECT(2) FOR DETAILS.
 */
void *
proxy (void *ptr) {

    struct proxy_t *pt;
    int    sd, sdOut;
    char   buf[TRANSFER_BUFFER_SIZE];
    int    nbytes;
    fd_set rset;
    int    selectval;
    int    iMaxSocket;
    char   *ip;
    char   *port;
    int    nr_nodes = 0; 
    struct node *head = NULL;
    // mire

    // set seed for random numbers

    srand((unsigned int) time(NULL));
    
    debug==1?syslog(LOG_NOTICE,"%s\n","proxy() started"):NULL; 

    read_config(&head,&nr_nodes,conf_file,debug);
    
    // end-mire 

    // call get_random_connection_params 
    get_random_connection_params(&ip,&port,head,nr_nodes,debug);
    
    pt = (struct proxy_t *) ptr;
    sd = pt->fd;
    free(ptr);

    /*
     * CONNECT TO REQUESTED HOST
     */

    debug==1?syslog(LOG_INFO,"about to connect to '%s':'%s'",ip,port):NULL;
    sdOut = tcp_connect(ip, port);
    debug==1?syslog(LOG_INFO,"passed tcp connect"):NULL;

    iMaxSocket = GetLargestSocket(sd, sdOut);

    /*
     * SET UP A SELECT LOOP FOR READING AND WRITING
     */
    for (;;) {
        FD_SET (sd   , &rset);
        FD_SET (sdOut, &rset);
        selectval = select (iMaxSocket, &rset, NULL, NULL, NULL);
        if (selectval == -1) {
            syslog (LOG_ERR, "select(): %m");
            Close(sdOut);
            Close(sd);
            break;
    	}
      
        /*
        * IS CLIENT READABLE? IF SO, READ FROM CLIENT, 
        * WRITE TO DESTINATION.
        */
        if (FD_ISSET (sd, &rset)) {
            nbytes = read(sd, &buf, TRANSFER_BUFFER_SIZE);
            if (nbytes <= 0) {
                Close(sdOut);
                Close(sd); 
                break;
	        }
            if (write (sdOut, &buf, nbytes) != nbytes)
                syslog(LOG_ERR, "error on write %m");
	        nbytes -= nbytes;
	    }
      
        /*
         * IS DESTINATION READABLE? IF SO, READ FROM DESTINATION, 
         * WRITE TO CLIENT. 
        */

        if (FD_ISSET (sdOut, &rset)) {
            nbytes = read(sdOut, &buf, TRANSFER_BUFFER_SIZE);
            if (nbytes <= 0) {
                Close(sdOut);
                Close(sd);
                break;
            }
            if (write (sd, &buf, nbytes) != nbytes)
                syslog(LOG_ERR, "error on write %m");
            nbytes -= nbytes;
        }
    } // end of for eternal read loop

    // mire
    kill_list(&head,&nr_nodes,debug);
    free(ip);
    free(port);
    debug==1?syslog(LOG_NOTICE,"proxy() %s","finished"):NULL;

    // end-mire
    return NULL;
}

/*
 * INT DAEMON_INIT()
 *
 * CAUSES PROXY TO RUN AS A UNIX DAEMON
 */
int
daemon_init (const char *pname) {
    int   i;
    pid_t pid;

    if ((pid = fork ()) != 0)
        exit (0);

    setsid ();

    signal (SIGHUP, SIG_IGN);

    if ((pid = fork ()) != 0)
        exit (0);

    // no no no chdir ("/");

    umask (0);

    /* 
     * CLOSE THE FIRST 64 DESCRIPTORS INHERITED FROM THE PROCESS
     * THAT EXECUTED THE DAEMON. THERE ARE PROBABLY NOT 64 OPEN
     * DESCRIPTORS, SO I DON'T CHECK THE RETURN VALUE OF CLOSE().
     * SEE "UNIX NETWORK PROGRAMMING VOLUME 1" FOR MORE INFORMATION
     * ON THIS SUBJECT... (PAGE 337 PARAGRAPH 3)
     */
    for (i = 0; i < MAXFD; i++)
        close(i);

    return 0;
}

/*
 * INT FILTERIP()
 *
 * BASIC IP FILTERING
 */
int
FilterIP (char *clientip) {
    FILE *access;
    char str[MAXLINE];
    int  deny = 0 , accept = 0;
    char *deny_str, *access_str ;
    int  i;

    access = fopen ("/etc/proxy.filters", "r");

    if (access == NULL) {
        debug==1?syslog (LOG_ERR, "fopen problems on filters file /etc/proxy.filters (harmless) %m"):NULL;
        return 0;
    }

    access_str = (char *) malloc (sizeof (char) * 25);
    sprintf (access_str, "ACCEPT.%s", clientip);

    deny_str = (char *) malloc (sizeof (char) * 25);
    sprintf (deny_str, "DENY.%s", clientip);

    while (fgets (str, MAXLINE, access) != NULL) {
        if (str[0] == '#') 
            continue;

        for (i = 0; str[i] != '\n'; i++);
        str[strlen (str) - 1] = '\0';

        for (i = 0; str[i] != '.'; i++)
            str[i] = toupper (str[i]);

        if (strstr (access_str, str) != NULL) {
            free (access_str);
            free (deny_str);
            return ACCEPT;
        }

        if (strstr (deny_str, str) != NULL) {
            free (access_str);
            free (deny_str);
            return DENY;
        }

        if (strcmp (str, "ACCEPT;") == 0)
	        accept++;
        else if (strcmp (str, "DENY;") == 0)
        	deny++;
    }

    free   (access_str);
    free   (deny_str);
    fclose (access);

    if (deny > 0)
        return DEFAULT_DENY;
    else if (accept > 0)
        return ACCEPT;
    else
        return NO_RULE;
}

/*
 * STATIC VOID FINISH()
 *
 * FUNCTION TO HANDLE SIGNAL GENERATED BY CTRL-C
 */
void
finish (int sig) {
    pthread_exit(NULL);
}

/*
 * INT MAIN()
 */
int
main (int argc, char *argv[]) {
    int       listenfd, connfd;
    socklen_t clilen, clientaddrlen;
    socklen_t addrlen;
    struct    sockaddr *cliaddr;
    struct    sockaddr *clientaddr;
    struct    proxy_t *ptr;
    pthread_t ptConnection;
    int   FilterCheck;
    int   c;
    char  *srcip   = NULL;
    char  *srcport = NULL;

    (void) signal (SIGINT, finish);

    while ((c = getopt_long (argc, argv, "nds:S:c:", long_opts, NULL)) != -1) {
        switch (c) {
        
            case 0:
        	  break;

        	case 'n':
        	  nodaemon = 1;
        	  break;

        	case 'd':
        	  debug = 1;
        	  break;

        	case 's':
        	  srcport = optarg;
        	  break;

        	case 'c':
        	  conf_file = optarg;
        	  break;

        	case 'S':
        	  srcip = optarg;
        	  break;

        	default:
        	  PrintUsage (1);
              exit(1);
    	}
    }

    if (show_help)
        PrintUsage (2);

    if (show_version)
        PrintUsage (3);

    if (argc < 3) {
        fprintf (stderr, "proxylb: too few arguments\n");
        PrintUsage (1);
    }

    if (!srcport)
        PrintUsage(1);

    /* PRINT STARTUP MESSAGE */
    fprintf (stderr, "\nproxylb started: listening on port [%s]\n\n", srcport);

    /* RUN PROXY AS UNIX DAEMON */
    if (!nodaemon)
        daemon_init (argv[0]);

    if (srcip)
        listenfd = tcp_listen (srcip, srcport, &addrlen);
    else
        listenfd = tcp_listen (NULL, srcport, &addrlen);

    if ( (cliaddr = malloc(addrlen)) == NULL)
        syslog(LOG_ERR, "memory allocation error %m");

    // eternal socket accept loop
    for (;;) {

        ptr = (struct proxy_t *) malloc (sizeof (struct proxy_t));

        clilen = addrlen;
        connfd = accept (listenfd, cliaddr, &clilen);

        if (connfd == -1)
            continue;

        ptr->fd = connfd;

        if ((clientaddr = malloc (MAXSOCKADDR)) == NULL)
            syslog (LOG_ERR, "memory allocation error %m");

        clientaddrlen = MAXSOCKADDR;

        if (getpeername (ptr->fd, clientaddr, &clientaddrlen) < 0)
    	    syslog (LOG_ERR, "getpeername error %m");

        /*
         * BASIC IP FILTERING
         */
        FilterCheck = FilterIP ((char *) Sock_ntop (clientaddr, clientaddrlen));

        if (FilterCheck > 0) {

            switch (FilterCheck) {
                case DENY:
            		debug==1?syslog (LOG_INFO, "found deny rule for %s, dropping connection!",
        	    		Sock_ntop (clientaddr, clientaddrlen)):NULL;
                    break;
                case DEFAULT_DENY:
                	debug==1?syslog (LOG_INFO, "%s matched default policy of DENY, " 
                        "dropping connection!",
        	    		Sock_ntop (clientaddr, clientaddrlen)):NULL;
                     break;
                case NO_RULE:
               		debug==1?syslog (LOG_ERR, "no rule matching %s and no default policy, "
                		"dropping!", Sock_ntop (clientaddr, clientaddrlen)):NULL;
                    break;
            }

            Close (ptr->fd);
        } else {
            debug==1?syslog (LOG_INFO, "allowing connection from %s",
		        Sock_ntop (clientaddr, clientaddrlen)):NULL;
        }

        free(clientaddr);

        if (pthread_create (&ptConnection, NULL, proxy, (void *) ptr) != 0) {
    	    syslog (LOG_ERR, "pthread creation problems %m");
	        return 1;
        }

        if ( (pthread_detach(ptConnection)) != 0)
    	    syslog(LOG_ERR, "pthread_detach %m");
    }
} // end of main

// _END_

