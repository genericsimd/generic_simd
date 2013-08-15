/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_fec_decoding.h



   Function:   Declare the functions  for the FEC encoding 



   Change Activity:



   Date             Description of Change                          By

   -------------    ---------------------                          ------------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */



#ifndef __PHY_UL_FEC_DECODING_H__

#define __PHY_UL_FEC_DECODING_H__

#include "phy_ul_rx_interface.h"
#include "viterbicore.h"
#include "adapter_bs_ul_map_interface.h"


int32_t phy_ul_fec_decoding(struct phy_ul_rx_syspara *para,
                            struct union_burst_ie *burstpara,
			    float *input);
							

#endif



