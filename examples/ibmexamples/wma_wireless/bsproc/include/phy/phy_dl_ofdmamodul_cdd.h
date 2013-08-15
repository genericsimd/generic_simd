/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_ofdmamodul.h


   Function: Declare the functions to do OFDMA modulation.


      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */





#ifndef __PHY_DL_OFDMAMODUL_H__
#define __PHY_DL_OFDMAMODUL_H__

#include "phy_dl_tx_interface.h"


int phy_dl_ofdmamodul_cdd(const struct phy_dl_tx_syspara *para,
                            const float *input_r,
                            const float *input_i,
                            float *output_r,
                            float *output_i);


#endif

