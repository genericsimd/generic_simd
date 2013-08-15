/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PHY_UL_SUBCARDERANDOM_H_

#define _PHY_UL_SUBCARDERANDOM_H_



#include "phy_ul_rx_interface.h"

int32_t phy_ul_subcarderandom(struct phy_ul_rx_syspara *para,
                              int32_t *wk,
                              const float *subcar_r, 
                              const float *subcar_i, 
                              float *subcarderandom_r, 
                              float *subcarderandom_i);



#endif



