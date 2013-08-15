/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_DL_MUXOFDMSYM_H__
#define __PHY_DL_MUXOFDMSYM_H__

#include "phy_dl_tx_interface.h"

int phy_dl_muxofdmsym (struct phy_dl_tx_syspara  *para,
                       u_int32_t num_unused_subch,
                       const float (*modulation_r),
                       const float (*modulation_i),
                       const u_int32_t (*datasubcar),
                       const u_int32_t (*pilotsubcar),
                       float *muxofdmsym_r,
                       float *muxofdmsym_i);
#endif
