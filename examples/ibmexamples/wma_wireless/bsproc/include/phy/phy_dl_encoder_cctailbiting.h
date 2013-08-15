/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_encoder_cctailbiting.h 

   Function:   Declare the functions  for the randomizer in the FEC coding

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_ENCODER_CCTAILBITING_H__
#define __PHY_DL_ENCODER_CCTAILBITING_H__

#include "phy_dl_tx.h" 

/* CC encoder and decoder polynomials for r=1/2 k=7 */
#define CCPOLYB  0x6d
#define CCPOLYA  0x4f

int phy_dl_encoder_cctailbiting(const struct phy_dl_fec_para *para,
                                unsigned char *bit,
                                unsigned char *symbol);

#endif

