/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_ofdmademodul.h



   Function: Declare the functions to do OFDMA demodulation.



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */





#ifndef __PHY_UL_DEMOD_H__

#define __PHY_UL_DEMOD_H__



#include "phy_ul_rx_interface.h"



int32_t phy_ul_ofdmademodul(  const struct phy_ul_rx_syspara *para,

                              const float *input_r,

                              const float *input_i,

                              float *output_r,

                              float *output_i);



#endif //__PHY_UL_DEMOD_H__

