
/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: resolve.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _RESOLVE_H_
#define _RESOLVE_H_

#ifdef _cplusplus
extern "C" {
#endif

struct addrinfo*  resolveaddress(char *addr, char *port, int af, int type, int proto);

#ifdef _cplusplus
}
#endif

#endif
