/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_assistfunc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 10-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_assistfunc.h"
#include <strings.h>
#include <string.h>

BYTE *  mallocnode(int usize)
{
    BYTE *pdata = malloc(usize);
    bzero(pdata,usize); 
    return pdata;
}

