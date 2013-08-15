/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_ZONEPERMUTATION_H__
#define __PHY_DL_ZONEPERMUTATION_H__
#include "phy_ul_rx_interface.h"

							   
int32_t phy_ul_zonepermutation(struct phy_ul_rx_syspara *para,
                               char *dts_selbit,
                               u_int32_t *ava_subch,
                               u_int32_t *rotation_posindex,
                               u_int32_t *pilot_allocation,
                               u_int32_t *data_allocation,
                               u_int32_t *ranging_allocation);
#endif
