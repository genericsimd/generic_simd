/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: resolve.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resolve.h"


//
// Function: ResolveAddress
//
// Description:
//    This routine resolves the specified address and returns a list of addrinfo
//    structure containing SOCKADDR structures representing the resolved addresses.
//    Note that if 'addr' is non-NULL, then getaddrinfo will resolve it whether
//    it is a string listeral address or a hostname.
//
struct addrinfo *resolveaddress(char *addr, char *port, int af, int type, int proto)
{
    struct addrinfo hints,
    *res = NULL;
    int             rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    rc = getaddrinfo(
            addr,
            port,
           &hints,
           &res
            );
    if (rc != 0)
    {
        const char *errorinfo = gai_strerror(rc);
        printf("Invalid address %s, getaddrinfo failed: %s\n", addr, errorinfo);
        return NULL;
    }
    return res;
}
