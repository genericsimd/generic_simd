/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_puncture.h

   Function:   Declare the functions  for the puncture in the FEC coding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_PUNCTURE_H__
#define __PHY_DL_PUNCTURE_H__

#include "phy_dl_tx.h" 
/* Interleaver */
int phy_dl_puncture(struct phy_dl_fec_para *para,
                    unsigned char *inputbits,
                    unsigned char **outputbits);

#endif
