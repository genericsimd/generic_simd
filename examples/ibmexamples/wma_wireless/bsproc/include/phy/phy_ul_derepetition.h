/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_derepetition.h



   Function:   Declare the functions  for the derepetition in the FEC decoding 



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#ifndef __PHY_UL_DEREPETITION_H__

#define __PHY_UL_DEREPETITION_H__

#include "phy_ul_fec_decoding.h"





int32_t phy_ul_derepetition(struct phy_ul_fec_para *para, 
                            float *inputdata, 
	                    float **outputdata);



#endif

