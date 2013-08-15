/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_ofdma_ul_deinterleaver.h

   Function:   Declare the functions  for the deinterleaver in the FEC decoding 


   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#ifndef __PHY_UL_DEINTERLEAVER_H__

#define __PHY_UL_DEINTERLEAVER_H__

//#include "phy_ul_rx_interface.h"



struct deinterleave_setting
{
    int   table_kj[12000];
    //table_array_index={48,96,144,192,240,288,384,480,576}; 
    int   table_index[50];
    int   table_offset[50];
    int   map_count;
};

//DeInterleaver
struct phy_ul_fec_para;
int32_t phy_ul_deinterleaver(struct phy_ul_fec_para *para, 

	                     float *interleave_output, 

	                     float *deinterleave_output);

int phy_ul_init_deinterleave_setting();

#endif
