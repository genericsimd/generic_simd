/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_modulation_adapter.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_modulation_adapter.h"
#include <stdio.h>
#include "flog.h"

double get_modulation_rate(modulation_type mc_type){
    switch (mc_type) {
        case OFDM_QPSK_1_2:
          //  return 2.0 * 1 / 2;    
	         return OFDM_QPSK_1_2_BITS;
        case OFDM_QPSK_3_4:
          //  return 2.0 * 3 / 4;
            return OFDM_QPSK_3_4_BITS;
        case OFDM_16QAM_1_2:
	  //return 4.0 * 1 / 2;
	         return OFDM_16QAM_1_2_BITS;
        case OFDM_16QAM_3_4:
	  //return 4.0 * 3 / 4;
	         return OFDM_16QAM_3_4_BITS;

        case OFDM_64QAM_1_2:
	  //return 4.0 * 3 / 4;
	         return OFDM_64QAM_1_2_BITS;
        case OFDM_64QAM_2_3:
	  //return 6.0 * 2 / 3;
	         return OFDM_64QAM_2_3_BITS;
        case OFDM_64QAM_3_4:
	  //return 6.0 * 3 / 4; 
            return OFDM_64QAM_3_4_BITS;
        default:
            FLOG_ERROR("get modulation rate modulation unknown\n");
    }
    return 0.0;
}
