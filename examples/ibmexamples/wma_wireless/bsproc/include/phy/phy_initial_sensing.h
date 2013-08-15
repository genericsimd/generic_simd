/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
   

   File Name: phy_initial_sensing.h

   Function: Declare the functions to form DL frame in transmitter.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_INITIAL_SENSING_H__
#define __PHY_INITIAL_SENSING_H__

struct spectrum_scan_state
{
    float dgain[2];
    int cali_ana_pwr[2];
    int cali_digi_pwr[2];
    int is_nfft;
    float power[1024];
    float intf[21];
};

int spectrum_ed_scan( float *input_r,
                      float *input_i,
                      struct spectrum_scan_state * conf );

int init_spectrum_ed_scan(void);
int release_spectrum_ed_scan(void);

#endif //__PHY_INITIAL_SENSING_H__
