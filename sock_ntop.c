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

#include	"proxylb.h"

char *
sock_ntop (const struct sockaddr *sa, socklen_t salen) {
    char        portstr[7];
    static char str[128];		/* Unix domain is largest */

    struct sockaddr_in *sin = (struct sockaddr_in *) sa;

    if (inet_ntop (AF_INET, &sin->sin_addr, str, sizeof (str)) == NULL)
        return (NULL);

    if (ntohs (sin->sin_port) != 0) {
        snprintf (portstr, sizeof (portstr), ".%d", ntohs (sin->sin_port));
        strcat (str, portstr);
    }
    return (str);
}

char *
Sock_ntop (const struct sockaddr *sa, socklen_t salen) {
    char *ptr;

    if ((ptr = sock_ntop (sa, salen)) == NULL) {
        syslog (LOG_ERR,"sock_ntop error %m");	/* inet_ntop() sets errno */
        exit (1);
    }

    return (ptr);
}
