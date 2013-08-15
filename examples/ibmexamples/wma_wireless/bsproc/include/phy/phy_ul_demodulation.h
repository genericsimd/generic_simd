/* ----------------------------------------------------------------------------
  
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_demodulation.h

   Function:   Declare the functions  for the modulation 

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------

   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */
 

#ifndef __PHY_UL_DEMODULATION_H__

#define __PHY_UL_DEMODULATION_H__

//#include "phy_ul_fec_decoding.h"
#include "phy_ul_rx_interface.h"

int32_t phy_ul_noise_est(const struct union_burst_ie *burstpara,
                         const float *pilot_data_r,
                         const float *pilot_data_i,
                         const float *pilot_est_r,
                         const float *pilot_est_i,
                         float *noise_est,
                         float *snr_est);
						 
int32_t phy_ul_demodulation_single(const struct union_burst_ie *burstpara,
                                   const float *ant1_r,
                                   const float *ant1_i,
                                   const float *hest1_r,
                                   const float *hest1_i,
                                   const float *noise_power1,
                                   float *softbit);
						 
int32_t phy_ul_demodulation(const struct union_burst_ie *burstpara,
                            const float *ant1_r,
                            const float *ant1_i,
                            const float *ant2_r,
                            const float *ant2_i,
                            const float *hest1_r,
                            const float *hest1_i,
                            const float *hest2_r,
                            const float *hest2_i,
                            float *noise_power1,
                            float *noise_power2,
                            float *softbit);						  

#endif





