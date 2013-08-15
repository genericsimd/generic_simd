/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_ul_derandomizer.h



   Function:   Declare the functions  for the derandomizer in the FEC decoding 



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#ifndef __PHY_UL_DERANDOMIZER_H__

#define __PHY_UL_DERANDOMIZER_H__

#include "phy_ul_fec_decoding.h"



int32_t phy_ul_derandomizer(struct phy_ul_fec_para *para,
                            u_int8_t *p_seed, 
			    u_int8_t *p_data, 
			    u_int8_t *p_randomized_data);

#endif

