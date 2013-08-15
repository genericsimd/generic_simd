/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_dl_interleaver.h

   Function:   Declare the functions  for the interleaver in the FEC coding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_INTERLEAVER_H__
#define __PHY_DL_INTERLEAVER_H__
//#include "phy_dl_tx.h" 

struct interleave_setting
{
    int   table_jk[12000];
    //table_array_index={48,96,144,192,240,288,384,480,576}; 
    int   table_index[50];
    int   table_offset[50];
    int   map_count;
    
};

struct phy_dl_fec_para;

int phy_dl_init_interleave_setting();
int phy_dl_interleaver(const struct phy_dl_fec_para *para,
                       unsigned char *interleave_input,
                       unsigned char *interleave_output);
//extern struct interleave_setting g_interleave_setting;
#endif
