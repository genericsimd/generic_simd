/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_params.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PHY_PARAMS_H_
#define _PHY_PARAMS_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mac.h"

// parameters in DLMAP PHY synchronization field (Table 273)
// FCH takes 1 slot (min. data allocation unit)
// Repeated over 4 slots for diversity
// Sections 8.4.4.3 and 8.4.4.4
#define NUM_FCH_SLOTS 4

// Choose the FDC for this simulation
#define FRAME_DURATION_CODE 8 
// array mapping codes to frame duration in milliseconds (Table 274)
extern const float frame_duration[9];
long int frame_number;

#define SPECTRUM_SENSING_GAP 100 //spectrum sensing period in milliseconds.

#define DL_UL_RATIO 0.667 // Ratio of DL to UL subframe size. Possible values are 0.667, 0.389, 1.083, 1.778 
#define FS 1.4 //Sampling Frequency in MHz
#define NUM_FFT 1024

// In number of OFDM symbols, This is chosen for the worst case
#define TTG_RTG_ALLOWANCE 2 
#define NUM_PREAMBLE_SYMBOLS 1 // Number of OFDM symbols in Preamble

// This value depends on several parameters like DL_UL_RATIO, OFDM
// Symbol duration, RTG/TTG durations etc. Should be calculated from there
// Value in terms of PS, 1 PS=4/Fs
#define ALLOCATION_START_TIME 16000

#if NUM_FFT==1024
#define NUM_UL_SUBCHANNELS 31
#define NUM_DL_SUBCHANNELS 42
#endif

extern const float bits_per_car[NUM_SUPPORTED_MCS];
 
// Permutation mode assumed is PUSC
#define UL_PERMUTATION_TYPE 0 //0 for PUSC, 1 for FUSC, 2 for AMC

#define DL_PERMUTATION_TYPE 2 //0 for PUSC, 1 for FUSC, 2 for AMC

// define parameters for all permutation types
#if (UL_PERMUTATION_TYPE==0)
#define UL_SYMBOLS_PER_SLOT 3
#define UL_SUBCHANNEL_PER_SLOT 1
#define UL_DATA_CAR_PER_SLOT 48
#define UL_PILOT_CAR_PER_SLOT 24

#endif

#if (DL_PERMUTATION_TYPE==2)
#define DL_DATA_CAR_PER_SLOT 48
#define DL_SYMBOLS_PER_SLOT 3 
#define DL_SUBCHANNEL_PER_SLOT 1
#define NUM_DL_SLOTS_IN_FREQ (NUM_DL_SUBCHANNELS/DL_SUBCHANNEL_PER_SLOT)
#endif

// DL-MAP is sent using QPSK rate 1/2
// Ref: Section 8.4.4.3 (description of coding_indication in Table 268)
// assuming the same for ULMAP - not explicitly given anywhere but 
// inference is that it is in the same broadcast burst as DLMAP, 
// hence same (robust) MCS  
#define BROADCAST_DBPS 48
#define BROADCAST_DIUC 0

#define DL_UL_MAP_DATA_BITS_PER_SLOT BROADCAST_DBPS

// UL overhead, in number of slots, for contention/ranging subchannels 
// and any other overheads. 
#define UL_SLOT_OVERHEAD 0 

#define NUM_RANGING_SUBCHANNELS 6
#define NUM_IR_SYMBOLS 2
#define NUM_PR_SYMBOLS 1
#define NUM_RANGING_SYMBOLS (NUM_IR_SYMBOLS + NUM_PR_SYMBOLS)

#endif
