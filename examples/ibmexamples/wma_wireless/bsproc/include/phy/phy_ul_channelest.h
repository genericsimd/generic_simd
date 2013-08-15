/* ----------------------------------------------------------------------------
   
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_ul_chanlest.h



   Function: Declare the functions to do channel estimation for single channel,

             STC Matrix A and STC Matrix B.



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------




   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */





#ifndef __PHY_UL_CHANLEST_H__
#define __PHY_UL_CHANLEST_H__

#include "phy_ul_rx_interface.h"

#define NC (60) /* Number of clusters */

#define NL (14) /* Length of clusters (the number of subcarriers in one cluster) */

//#define BP_4NTX1 (8)   /* The first pilot position of antenna 1 in one 4n OFDMA symbol */

//#define EP_4NTX1 (834) /* The last pilot position of antenna 1 in one 4n OFDMA symbol */

//#define BP_4NTX2 (4)   /* The last pilot position of antenna 1 in one 4n OFDMA symbol */

//#define EP_4NTX2 (830) /* The last pilot position of antenna 1 in one 4n OFDMA symbol */



int32_t phy_ul_single_chanlest(const struct phy_ul_rx_syspara *para,
                               const float *rx_r,
                               const float *rx_i,
                               float *h_r,
                               float *h_i);


#endif //__PHY_DL_CHANLEST_H__



