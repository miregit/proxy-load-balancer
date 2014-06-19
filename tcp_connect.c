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
tcp_connect(const char *host, const char *serv) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    /* 
     * Initialize addrinfo struct, and specify a socket 
     * family and a socket type for our proxy connection.
     */
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
      syslog(LOG_ERR, "tcp_connect first error for host '%s', serv '%s'", host, serv);
    }

    ressave = res;

    /* 
     * Loop through, calling socket and connect for each IP, creating
     * a linked list of addrinfo structures, once a successful connection
     * has been made, break out of loop. We then free memory and return
     * a socket descriptor.
     */
    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
            continue;

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;  /* Break on success */

        close(sockfd);
    } while ( (res = res->ai_next) != NULL); /* Build linked list */

    if (res == NULL) {
        syslog(LOG_ERR, "tcp_connect second error for %s, %s", host, serv);
    }

    freeaddrinfo(ressave);
    return (sockfd);
}


