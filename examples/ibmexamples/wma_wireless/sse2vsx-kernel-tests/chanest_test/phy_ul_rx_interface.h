
/* ----------------------------------------------------------------------------
   (C)Copyright 2009
   International Business Machines Corporation,
   All Rights Reserved.

   This file is IBM Confidential and is not to be distributed.

   File Name: phy_ul_rx_interface.h

   Function:

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------
   18-May 2009    Created                                        Jianwen Chen


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_UL_RX_INTERFACE_H__
#define __PHY_UL_RX_INTERFACE_H__

#include <stdlib.h>
#include <sys/types.h>


struct phy_ul_fec_para
{
    u_int32_t slots_num;             /* slots number */
    u_int32_t bits_slotsymbol;       /* original bits length for the burst*/
    u_int32_t blockjslot;            /* */
    u_int32_t blocksizejslot;        /* */
    u_int32_t blocksizeceilslot;     /* */
    u_int32_t blocksizefloorslot;    /* */

    u_int32_t burst_len;             /* the uncoded length of the burst */
    u_int32_t burst_len_encoded;     /* For Convolutional coding it should be 2* bits_slotsymbol, for CTC, it should be 3*bits_slotsymbol */
    u_int32_t burst_len_punctured;   /* Puncture  length, this is the lenth for the  interleaving */
    u_int32_t burst_len_repeated;    /* Repeated  length, this is the lenth for the  modulation and  the following following */
    u_int32_t burst_len_modulated;   /* Modulated  length, this is the final lenth of the fec module*/

    u_int32_t repetition_code;       /* Table 320 - RepetitionIndication*2 */
    u_int32_t code_id;               /*modulation, rate */
    u_int32_t ncpc;                  /* The number of coded bits per subcarrier, 2, 4, or 6 for QPSK, 16-QAM or 64-QAM [4] */

    u_int32_t decoder_type;          /*0 hard decision, 1 soft decision*/
    u_int32_t padding;               /* */

    //for viterbi decoder vp
    void *vp;
};


struct phy_ul_rx_syspara
{
	/////////////////////System Configuration ////////////////////
	u_int32_t bw;              //System bandwidth, this system uses 10*10^6 Hz as defualt  [10M]
	/*antenna diversity, currently only support 2*2 configurations;*/
	u_int32_t tx_div;           //Transmission antenna number  [2]
	u_int32_t rx_div;           //Receive antenna number  [1]
	u_int32_t bs_id;            //BS ID [0]
	u_int32_t permutation_type;  // for broadcast region in HARQ zone, 0 = PUSC, 1 = FUSC, 2 = Optional FUSC, 3 = AMC  [0]
	u_int32_t init_ranging_code;  //size of the code set for initial ranging [128]
	u_int32_t ranging_length;   //length of the Ranging Code [144]
    u_int32_t max_num_symbol_per_frame;
    u_int32_t preamble_index;

	u_int32_t num_pilot;             /* Table 442 Number of pilot subcarriers for one symbol */
	u_int32_t pilot_insert;          /* Indicate whether or not the pilots are inserted */

	/////////////////////From MAC Parameters //////////////////
	u_int32_t duration_code ;  //Table 318
	u_int32_t frame_num;       //Table 318
	u_int32_t uiuc;            //
	u_int32_t cid;             //
	u_int32_t symbol_offset;   //OFDMA Symbol offset, defined in UL-MAP_IE or DL-MAP_IE
	u_int32_t coding_indication;    //Table 311
	int32_t subchannel_offset;
	u_int32_t num_symbol_per_dl_frame;   //number of data symbols for DL per frames DL_MAP
	u_int32_t repetition_coding_indication; /* Table 320 */

	u_int32_t num_symbol_per_ul_frame;    // number of data symbols for UL per frames UL_MAP
	u_int32_t ofdma_nfft;        // size of the FFT ,  index 512 is DC Subcarriers  [1024]
    float ofdma_g;                  //2009-04-30 ryan fix bug: u_int32_t ofdma_g; //guard interval index (1/32, 1/16, 1/8, 1/4)  [1/8]
	u_int32_t id_cell;           // (0~31) preamble IDCell in the first zone
	u_int32_t ul_permbase;       // (0~69) specifies the basis of uplink permutation
	float papr_oversample;          // oversampling factor, 1 means Nyquist sampling rate
    float snr;

	////////////////////Derivated Parameters/////////////////////
	int32_t frame_increased;           // (Yes,No) specifies whether the frame number for the DL/UL subframe is increased (Yes or No)
				                   //need fftsize & permutation type (PUSC, FUSC)
	u_int32_t num_subcar_of_subch; // number of data subcarriers per subchannel [24]
	u_int32_t num_subch;         // number of all subchannel  Table 442  [30]
	u_int32_t repetition_code;   // Table 320 = RepetitionIndication *2
	u_int32_t sample_per_frame;
	u_int32_t sample_per_slotsymbol;
    u_int32_t frame_index;

	//OFDMA parameters
	float ofdma_n;                  //sampling factor derivated from BW accroding to Table 245
	u_int32_t ofdma_ncp;         // length of Cyclic Prefix, derivated from G* OfdmaNfft [128]
	u_int32_t ofdma_sym_len_with_guard;         // OfdmaNfft+OfdmaNcp
	u_int32_t ofdma_nused;       // Table 442, 841   with the DC subcarrier
	u_int32_t ofdma_nused_no_dc;       // 840
	u_int32_t ofdma_frame_time;   // T_frame= 5ms.  Table 319 (DurationCode)
	float ofdma_fs;          //sampling frequency 8.3.2.2
	float ofdma_delta_f;            //subcarrier spacing
	float ofdma_tb;                 //useful symbol time
	u_int32_t ofdma_tg;          //CP time
	u_int32_t ofdma_sym_time;     //OFDM symbol time
	float ofdma_sample_time;         //Sampling time
	u_int32_t ofdma_dc;           //512

    u_int32_t frame_start_off;
    float demod_gain;
    float stcb_sync_pow_gain1, stcb_sync_pow_gain2, stcb_sync_pow_gain3;
    float stcb_sync_const1, stcb_sync_const2;

	////////////////To MAC Parameters//////////////////////
    u_int32_t ranging_code;
    u_int32_t symbol_per_slot;  //symbols number for each slot for DL_PUSC it is 2
    u_int32_t mimo_mode;
    u_int32_t coding_type;      //0 CC 1 CTC
    u_int32_t decoder_type;     //0 hard decision, 1 soft decision

  //for stc
    u_int32_t stc_rate;         //stc rate
    u_int32_t stclayer_num;     //stc layer number

  //for receving process
    int32_t last_slotsymbol;

  //for fft
    float **XX;
    float **x;
    float **X;

  //parameters for fec
    struct phy_ul_fec_para fec_para;

	//for ranging
    u_int32_t *pilot_allocation;
    u_int32_t *data_allocation;
    u_int32_t *range_allocation;
    u_int32_t *data_only_allocation;
    u_int32_t num_tiles_subchannel; /* added by wq for zone permutation, 6 */
    u_int32_t num_subcar_group; //48
    u_int32_t ranging_channel_offset;//0

    u_int32_t numoftiles;

};



#endif


