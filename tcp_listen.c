/*

 * This is modified work by Miroslav Madzarevic githubcode@osadmin.com as of May 1st 2002
 * based originally on proxy-2.2.4 program by sparlin@openpro.org
   
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

int
tcp_listen (const char *host, const char *serv, socklen_t * addrlenp) {
    int       listenfd, n;
    const int on = 1;
    struct    addrinfo hints, *res, *ressave;

    bzero (&hints, sizeof (struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo (host, serv, &hints, &res)) != 0) {
        syslog(LOG_ERR, "tcp_listen error for host '%s', serv '%s'", host, serv);
    }

    ressave = res;

    do {
        listenfd = socket (AF_INET, res->ai_socktype, res->ai_protocol);
        if (listenfd < 0)
        	continue;

        if (setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) != 0) {
            syslog(LOG_ERR, "setsockopt error %m");
        }

        if (bind (listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;      /* success */

        close (listenfd);
    }  while ((res->ai_next) != NULL);

    if (res == NULL) {
      syslog (LOG_ERR, "tcp_listen error for host '%s', serv '%s'", host, serv);
      exit (1);
    }

    if (listen (listenfd, LISTENQ) != 0) {
        syslog(LOG_ERR, "listen error %m");
    }

    if (addrlenp)
        *addrlenp = res->ai_addrlen;	/* return size of protocol address */

    freeaddrinfo (ressave);

    return listenfd;
}
