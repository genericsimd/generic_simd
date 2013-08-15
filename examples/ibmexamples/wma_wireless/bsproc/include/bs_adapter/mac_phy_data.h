/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_phy_data.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef __MAC_PHY_DATA_H__
#define __MAC_PHY_DATA_H__


#include "adapter_config.h"
#include "mac_modulation_adapter.h"



#define   DATACARY_PERSLOT_IN_BITS                48
/** Definition of supported permutation schemes-- added for OFDMA  */
enum permutation_scheme{
  PUSC = 0,     /* Partial usage of Subcarriers */
  FUSC,         /* Fully usage of Subcarriers */
  AMC,          /* Contiguous Subcarriers */
  OPUSC,        /* Optional PUSC, uplink only */
  OFUSC,        /* Optional FUSC, downlink only */
  PERM_LAST
};

enum permutation_scheme dl_perm ;

enum permutation_scheme ul_perm ;

/* Store information about different combinations 
 * of bandwidth and permutation
 * - Permutation
 * - Channel bandwidth 
 * - FFT size
 * - Number of used subcarriers
 * - Number of subchannels
 * - Number of subcarrier per subchannel per symbol (Average)
 * - Number of data subcarrier per subchannel per symbol (Average)
 * - Data slot width (subchannel)
 * - Data slot length (OFDM symbol)
 */
#define OFDMA_PHY_PARAM_NUM 9
enum ofdma_attri {
  OFDMA_PERM = 0,
  OFDMA_BW,
  OFDMA_FFT,
  OFDMA_USED_SUBCAR_NUM,
  OFDMA_SUBCH_NUM,
  OFDMA_SUBCAR_NUM_PERSUBCH_PERSYM,
  OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM,
  OFDMA_SLOT_WIDTH_INSUBCH,
  OFDMA_SLOT_LENGTH_INSYM
};

//Note: Currently comment 20MHz data since the propagation model
//      does not support it.
//static const unsigned long DL_OFDMA_DATA [][OFDMA_PHY_PARAM_NUM] = 
//  {{PUSC, 5000000, 512, 421, 15, 28, 24, 1, 2},
//   {PUSC, 10000000, 1024, 841, 30, 28, 24, 1, 2},
//   {PUSC, 20000000, 2048, 1681, 60, 28, 24, 1, 2},
//   {FUSC, 5000000, 512, 426, 8, 48, 48, 1, 1},
//   {FUSC, 10000000, 1024, 851, 16, 48, 48, 1, 1},
   /*{FUSC, 20000000, 2048, 1703, 32, 48, 48, 1, 1},*/
//   {OFUSC, 5000000, 512, 433, 8, 48, 48, 1, 1},
//   {OFUSC, 10000000, 1024, 865, 16, 48, 48, 1, 1},
   /*{OFUSC, 20000000, 2048, 1729, 32, 48, 48, 1, 1},*/
//   {AMC, 5000000, 512, 433, 8, 48, 48, 1, 1},
//   {AMC, 10000000, 1024, 865, 16, 48, 48, 1, 1}
   /*{AMC, 20000000, 2048, 1729, 32, 48, 48, 1, 1}*/
//  };
static const unsigned long DL_OFDMA_DATA [][OFDMA_PHY_PARAM_NUM] = 
  {{PUSC, 5000000, 512, 421, 42, 28, 16, 1, 3},
   /*{AMC, 20000000, 2048, 1729, 32, 48, 48, 1, 1}*/
  };

/*
 * Note for PUSC: The slot allocation is 1 Subchannel x 3 OFDM symbols
 *  Each subchannel is 6 tiles wide and each tile has 4 pilot subcarrier 
 *  and 8 data subcarrier. Therefore each slot has 24 pilot and 48 data 
 *  over 3 OFDM symbols. This makes an average of 8 pilot subcarrier and 
 *  16 data subcarrier per symbol (24 total). 
 * Note for OPUSC: The slot allocation is 1 Subchannel x 3 OFDM symbols
 *  Each subchannel is 6 tiles wide and each tile has 1 pilot subcarrier 
 *  and 8 data subcarrier. Therefore each slot has 6 pilot and 48 data 
 *  over 3 OFDM symbols. This makes an average of 2 pilot subcarrier and 
 *  16 data subcarrier per symbol (18 total). 
 */
//Note: Currently comment 20MHz data since the propagation model
//      does not support it.
static const unsigned long UL_OFDMA_DATA [][OFDMA_PHY_PARAM_NUM] = 
  {{PUSC, 5000000, 512, 409, 17, 24, 16, 1, 3},
   {PUSC, 10000000, 1024, 841, 35, 24, 16, 1, 3},
   /*{PUSC, 20000000, 2048, 1681, 92, 24, 16, 1, 3},*/
   {OPUSC, 5000000, 512, 433, 24, 18, 16, 1, 3},
   {OPUSC, 10000000, 1024, 865, 48, 18, 16, 1, 3},
   /*{OPUSC, 20000000, 2048, 1729, 96, 18, 16, 1, 3},*/
   {AMC, 5000000, 512, 433, 8, 48, 48, 1, 1},
   {AMC, 10000000, 1024, 865, 16, 48, 48, 1, 1}
   /*{AMC, 20000000, 2048, 1729, 32, 48, 48, 1, 1}*/
  };

#define PHYTHER_DL_MODE                   0     


#define CURRENT_DL_OFDMA_PERM                            DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_PERM]
#define CURRENT_DL_OFDMA_BW                              DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_BW]
#define CURRENT_DL_OFDMA_FFT                             DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_FFT]
#define CURRENT_DL_OFDMA_USED_SUBCAR_NUM                 DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_USED_SUBCAR_NUM]
#define CURRENT_DL_OFDMA_SUBCH_NUM                       DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_SUBCH_NUM]
#define CURRENT_DL_OFDMA_SUBCAR_NUM_PERSUBCH_PERSYM      DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_SUBCAR_NUM_PERSUBCH_PERSYM]
#define CURRENT_DL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM  DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM]
#define CURRENT_DL_OFDMA_SLOT_WIDTH_INSUBCH              DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_SLOT_WIDTH_INSUBCH]
#define CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM               DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_SLOT_LENGTH_INSYM]

#define GET_DL_BITS_PERSUBCH_PERSYM(mc_type)             CURRENT_DL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM * \
                                                         get_modulation_rate(mc_type)


#define    UL_SYMBOL_NUMBER                              15            //define current UL width of frame in symbol that is a solid value

#define PHYTHER_UL_MODE                                  1
#define CURRENT_UL_OFDMA_PERM                            UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_PERM]
#define CURRENT_UL_OFDMA_BW                              UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_BW]
#define CURRENT_UL_OFDMA_FFT                             UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_FFT]
#define CURRENT_UL_OFDMA_USED_SUBCAR_NUM                 UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_USED_SUBCAR_NUM]
#define CURRENT_UL_OFDMA_SUBCH_NUM                       UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_SUBCH_NUM]
#define CURRENT_UL_OFDMA_SUBCAR_NUM_PERSUBCH_PERSYM      UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_SUBCAR_NUM_PERSUBCH_PERSYM]
#define CURRENT_UL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM  UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM]
#define CURRENT_UL_OFDMA_SLOT_WIDTH_INSUBCH              UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_SLOT_WIDTH_INSUBCH]
#define CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM               UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_SLOT_LENGTH_INSYM]

#define GET_UL_BITS_PERSUBCH_PERSYM(mc_type)             CURRENT_UL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM* \
                                                         get_modulation_rate(mc_type)
// the working frequency of the phy layer
double frequency;



int set_dl_perm(enum permutation_scheme dlperm);

int set_ul_perm(enum permutation_scheme ulperm);

int get_subch_num(u_int8_t is_dl, int* subchannel_num);

int get_datasubcar_persubch_persym(u_int8_t is_dl, int* data_carrier_num);

// in the unit of ofdma symbol
int get_slot_length(u_int8_t is_dl, int* slot_length);

int get_bits_persubch_persym(u_int8_t is_dl, modulation_type mc_type, int* bit_num);

#endif
