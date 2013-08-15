/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name:phy_dl_zonepermutation.h 

   Function: Claim for the phy_dl_zonepermutation() function

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#ifndef __PHY_DL_ZONEPERMUTATION_H__
#define __PHY_DL_ZONEPERMUTATION_H__
#include "phy_dl_tx_interface.h"

int32_t phy_dl_zonepermutation(struct phy_dl_tx_syspara *para,
                               char *dst_selbit,
                               u_int32_t *pilot_allocation,
                               u_int32_t *data_allocation);

int32_t phy_dl_zonepermutation_multi(struct phy_dl_tx_syspara *para,
                                     char *dst_selbit,
                                     u_int32_t *pilot_allocation,
                                     u_int32_t *data_allocation);

#endif
