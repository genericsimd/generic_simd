/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: test_routines.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Zhen Bo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _TEST_ROUTINES_H
#define _TEST_ROUTINES_H

#include "mac_message.h"

#define ERR_PRINT(a, b, c) \
        printf("FIELD %s: ULMAP 1: %d, ULMAP 2:%d\n", a, b, c); \

#define ERR_POINTER(a, b, c) \
        printf("%d ie: %s pointer is :%p\n", a, b, c); \

int compare_ul_map(ul_map_msg *p_1, ul_map_msg *p_2);

int compare_ucd(ucd_msg *ucd1, ucd_msg *ucd2);
int compare_dcd(dcd_msg *dcd1, dcd_msg *dcd2);

#endif
