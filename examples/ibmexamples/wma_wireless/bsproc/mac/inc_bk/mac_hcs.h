/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_hcs.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_HCS_H__
#define __MAC_HCS_H__

#include "mac_config.h"
#include "debug.h"
// the hcs result has been set to the hcs field
int hcs_calculation(u_char const * input, int length, u_char* output);

// return 0 means the verification is correct, otherwise the verification is wrong
int hcs_verification(u_char const * input, int length, u_char * ouput);

#endif
