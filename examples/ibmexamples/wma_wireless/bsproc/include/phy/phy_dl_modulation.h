/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_dl_modulation.h

   Function:   Declare the functions  for the modulation

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_MODULATION_H__
#define __PHY_DL_MODULATION_H__

#include "phy_dl_tx.h" 

int phy_dl_modulation(struct phy_dl_fec_para *para,
                      unsigned char *bit,
                      float *symbol_r,
                      float *symbol_i);


#endif
