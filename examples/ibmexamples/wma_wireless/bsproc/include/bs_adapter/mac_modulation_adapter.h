/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_modulation_adapter.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _MAC_MODULATION_ADAPTER_H_
#define _MAC_MODULATION_ADAPTER_H_

#define ERROR_TRACE1(str)   printf("%s",str)

typedef enum mdulationtype { 
OFDM_BPSK_1_2 = 0, 
OFDM_QPSK_1_2 = 0, 
OFDM_QPSK_3_4 = 1, 
OFDM_16QAM_1_2 = 2, 
OFDM_16QAM_3_4 = 3,
OFDM_64QAM_1_2 = 4,
OFDM_64QAM_2_3 = 5,
OFDM_64QAM_3_4 = 6
}modulation_type;

#define OFDM_BPSK_1_2_BITS           0.5
#define OFDM_QPSK_1_2_BITS           1.0
#define OFDM_QPSK_3_4_BITS           1.5
#define OFDM_16QAM_1_2_BITS          2.0
#define OFDM_16QAM_3_4_BITS          3.0
#define OFDM_64QAM_1_2_BITS          3.0
#define OFDM_64QAM_2_3_BITS          4.0
#define OFDM_64QAM_3_4_BITS          4.5



#define GET_MODULATION_RATE(mc_type)                  if(mc_type == OFDM_BPSK_1_2)\
	                                                  return OFDM_BPSK_1_2_BITS;\
						      else if(mc_type == OFDM_QPSK_1_2)\
                                                          return OFDM_QPSK_1_2_BITS;\
						      else if(mc_type == OFDM_QPSK_3_4)\
                                                          return OFDM_QPSK_3_4_BITS;\
						      else if(mc_type == OFDM_16QAM_1_2)\
                                                          return OFDM_16QAM_1_2_BITS;\
						      else if(mc_type == OFDM_16QAM_3_4)\
                                                          return OFDM_16QAM_3_4_BITS;\
						      else if(mc_type == OFDM_64QAM_2_3)\
                                                          return OFDM_64QAM_2_3_BITS;\
						      else if(mc_type == OFDM_64QAM_3_4)\
                                                          return OFDM_64QAM_3_4_BITS;\
                                                      ERROR_TRACE1 ("get modulation rate modulation unknown\n");\
                                                      return 0.0;\
                                             \
                                            
double get_modulation_rate(modulation_type mc_type);


#endif
