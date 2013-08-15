/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_decoder_cctailbiting.h

   Function:   Declare the functions  for the randomizer in the FEC coding 

   Change Activity:

   Date             Description of Change                            By

   -----------      ---------------------                            --------




   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#ifndef __PHY_UL_DECODER_CCTAILBITING_H__

#define __PHY_UL_DECODER_CCTAILBITING_H__

#include "phy_ul_fec_decoding.h"


int32_t phy_ul_decoder_cctailbiting(struct phy_ul_fec_para *para,
            	         	    u_int8_t *symbols, 
			            u_int8_t * data);


#endif

