/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "phy_ul_rx_interface.h"


int32_t phy_ul_power_adjust_single(struct phy_ul_rx_syspara *para,
                                   const float * input_r,
                                   const float * input_i,
                                   const float *dgain,
                                   float * output_r,
                                   float * output_i);



int32_t phy_ul_power_adjust(struct phy_ul_rx_syspara *para,
                            const float * input_ant0_r,
                            const float * input_ant0_i,
                            const float * input_ant1_r,
                            const float * input_ant1_i,
	                    const float * dgain,
                            float * output_ant0_r,
                            float * output_ant0_i,
                            float * output_ant1_r,
                            float * output_ant1_i); 
