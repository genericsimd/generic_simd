/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */




#ifndef __PHY_DL_SUBCARRANDOM_H__
#define __PHY_DL_SUBCARRANDOM_H__

#include "phy_dl_tx_interface.h"


int phy_dl_subcarrandom(struct phy_dl_tx_syspara *para,
                        int32_t *wk,
                        const float *subcar_r,
                        const float *subcar_i,
                        float *subcarrandom_r,
                        float *subcarrandom_i);
#endif

