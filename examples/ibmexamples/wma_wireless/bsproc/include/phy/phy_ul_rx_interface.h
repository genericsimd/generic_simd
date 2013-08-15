
/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_rx_interface.h

   Function:  

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_UL_RX_INTERFACE_H__
#define __PHY_UL_RX_INTERFACE_H__

#include <stdlib.h>
#include <sys/types.h>

#include "phy_proc.h"
#include "adapter_bs_ul_map_interface.h"
#include "phy_ul_enum.h"
#include "phy_ul_rx_common.h"
#include "phy_dl_tx_interface.h"

//#ifdef IPP_OPT_TEMP730
//#include "ipp.h"
//#endif

////////////////System Parameter Init for Rx/////////////////////////

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
    u_int32_t blkerr_num;
};


struct phy_ul_rx_syspara
{
	/////////////////////System Configuration ////////////////////
     /** Basic Parameters obtained from WMA Config */
    u_int32_t bandwidth;
    u_int32_t ofdma_fs;
    u_int32_t oversampling;
    u_int32_t bs_id;            //BS ID [0]
    u_int32_t multiple_zone; /* 1-multiple zone support */

    u_int32_t rx_div; /* number of Tx Antenna, default is 2 */
    u_int32_t frame_index;    /* The index of frame, start from 0 */
    u_int32_t symbol_offset;  /* OFDMA Symbol offset, defined in DL-MAP_IE */
    u_int32_t first_zone;     /* 1 - first dl zone, 0 - other dl zones */
    u_int32_t mimo_mode;      /* 0=CDD, 1=STCA, 2=STCB */
    u_int32_t symbolnum_per_ul_frame;  /* Number of symbols per dl frame */
    u_int32_t zone_len;
    u_int32_t permutation_type; /* For broadcast region in HARQ zone [0]
                                  0 = PUSC, 1 = FUSC,
                                  2 = Optional FUSC,
                                  3 = AMC */
    u_int32_t ofdma_nfft;  /* Size of the FFT [1024] */
    float ofdma_g;         /* Guard interval index[1/8] */
    u_int32_t cdd_num; /* cdd cyclic sample number */
    u_int32_t ul_permbase; /* Table 326, Table 437 */
    u_int32_t max_cellnum ; /* Maximal Cell Number */
    u_int32_t max_segnum ; /*Maximal Segment Number */
    u_int32_t prbs_id;     /* Used in initialization vector of the PRBS
                              generator for subchannel randomization
                              - 0, 1, 2 */
    u_int32_t pilot_insert;  /* Indicate whether or not the pilots
                                are inserted */
    int32_t frame_increased;    // (Yes,No) specifies whether
                               // the frame number for the DL/UL subframe is increased (Yes or No)
    u_int32_t preamble_index;
    u_int32_t zone_duration; 

    /* FCH related parameters */
    u_int8_t ul_bitmap[35]; //ul_bitmap selection
    char active_band[21];
    char fixed_active_band[21];
    u_int32_t ul_unused_subch;
    u_int32_t avasubch_num;
    u_int32_t usedsubch_num;
    u_int32_t chan_sum;
    u_int8_t fch_bitmap[6];


   /* FEC related parameters */
    u_int8_t stc_rate;       /* Always be 1 in current version */
    u_int8_t stclayer_num;   /* Always be 1 in current version */
    u_int32_t coding_type;   /* CC CTC */

    /******************************************************/
    /* Parameter derived from the phy_config */
    u_int32_t segment_index;  /* Segment index - 0, 1, 2 */
    u_int32_t id_cell;        /* Preamble IDCell - 0~31 */

    float **XX;               /* For fft */
    float **x;
    float **X;

    /** Fixed after the system initialization */
    u_int32_t max_symbolnum_per_frame; /* Max number of symbols per frame */
    u_int32_t symbol_per_slot;


    u_int32_t ofdma_ncp;   /* Length of cyclic prefix = G* OfdmaNfft [128] */
    u_int32_t ofdma_symlen_with_guard;  /* OfdmaNfft+OfdmaNcp */
    u_int32_t ofdma_nused; /* Table 442, with the DC subcarrier [841] */
    u_int32_t ofdma_nused_no_dc;  /* Without the DC subcarrier[840] */
    u_int32_t ofdma_dc;    /* [512] */

    /* PUSC related parameters */

    u_int32_t num_subcar_of_subch;  /* Number of data subcarriers
                                      per subchannel */
    u_int32_t num_subch;            /* Number of subchannel in dl frame */
    u_int32_t numofsubcarpercluster;/* Table442 number of subcarriers
                                       per cluster */
    u_int32_t numofcluster;         /* Table442 number of clusters */
    u_int32_t numoftiles;

    u_int32_t num_pilot;     /* Table 442 number of pilot subcarriers
                                per symbol */
    u_int32_t num_tiles_perslot; 
    u_int32_t num_subcar_group; //48
    u_int32_t num_datasubcar_perslot;//48
    u_int32_t num_pilot_perslot;//24
    u_int32_t num_subcar_pertileF;//4


    /* Parameters of AMC Band */
    u_int32_t numofbins_of_phyband;/*number of bins per physical band */
    u_int32_t num_phyband; /* number of physical band */
    u_int32_t num_data_subcar;  /* data subcarrier number */
    u_int32_t bins_perslot; //one subchannel contains 2 bins for AMC 2X3
    u_int32_t datasub_persubsym; //data subcarriers number of one subchannel in 1 ofdm symbol
    u_int32_t pilot_persubsym;//pilot subcarriers number of one subchannel in 1 ofdm symbol
    u_int32_t num_pilot_of_subch; //pilot number per slot
    u_int32_t pilot_period; //pilot period in time direction for one symbol slot
    u_int32_t num_bins; //number of bins
    u_int32_t left_guard;
    u_int32_t right_guard; //133


    u_int32_t repetition_coding_indication; /* Table 320 */
/**********************************************************************************/
    u_int32_t max_num_symbol_per_frame;
    u_int32_t repetition_code;   // Table 320 = RepetitionIndication *2 
    u_int32_t sample_per_slotsymbol;
    u_int32_t frame_num;
    u_int32_t sample_per_frame;
    float demod_gain;
    u_int32_t ofdma_sym_len_with_guard;

    /* ranging related information */
    u_int32_t ranging_symoffset;
    u_int32_t ranging_subchoffset;
    u_int32_t ranging_sym;
    u_int32_t ranging_subch;
	
    /*power adjustment parameters */
    float CaliDgain0; 
    float CaliAnaPwr0;
    float CaliDigiPwr0;
    float CaliDgain1; 
    float CaliAnaPwr1;
    float CaliDigiPwr1;
    float dgain0;
    float dgain1;

   /* spectrum sensing related parameters */
    u_int32_t is_nfft;
    u_int32_t ps_nfft;

     
  //for receving process
    int32_t last_slotsymbol;
  //parameters for fec 
    struct phy_ul_fec_para fec_para;
    float softbit_threshold;
    u_int8_t softbit_shift;
    u_int32_t blkerr_num;
    u_int32_t blk_per_frame;

  /*parameter for Periodic Sensing */
    float ps_thd;

    float noise_figure;
    float noise_maxhold;
};


struct phy_ul_rru_symbol
{
   u_int32_t    symbol_offset;
   u_int32_t    frame_index;
   u_int32_t    symbol_num; /* Number of symbol */
   u_int32_t    symbol_len; /* Length in float for each symbol  */
   u_int8_t     ul_subframe_endflag; //the last ofdma symbol in the dl subframe
                                      //1 end,  0 not end
   u_int8_t  is_broadcast;
   u_int8_t  mimo_mode;    //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
   u_int32_t slotlength;   //number of slots in slotsymbol

   float  *symbol_i;        /* Real part */
   float  *symbol_q;        /* Imaginary part */
   unsigned char *ul_map; /* ul_map for the current frame */
   u_int8_t       ul_map_len;
   float dgain;
   u_int8_t     ul_perscan_flag ; /* 1--periodical scanning */
//   void         *p_dts_info;
   u_int8_t    ul_fake_flag;
};

/*
struct dts_info
{
    char *active_band; //interference information from spectrum scanning, 1--interference
    u_int32_t dl_unused_subch;  //unused subchannel number of DL,
                               //  including the interference and active forbidden channel
    u_int32_t ul_unused_subch; //unused subchannel number of UL,
                               //  including the interference and active forbidden channel
};
*/

struct phy_ul_rru_symbol* phy_ul_init_rrusymbol(u_int32_t frame_index,
                                                u_int32_t symbol_offset,
                                                u_int32_t symbol_num,
                                                u_int32_t symbol_len,
                                                u_int8_t endflag);

int32_t phy_ul_rx_init(struct phy_global_param *phyconf,
                       struct phy_dts_info *dts_para,
                       struct phy_ul_rx_syspara *para);


int32_t phy_ul_rx_deinit(struct phy_ul_rx_syspara *para); 

						 
int32_t phy_ul_rx(struct phy_ul_rx_syspara *para,
                  const u_int32_t num_in_que,
                  const u_int32_t *in_que_id,
                  const u_int32_t num_out_que,
                  const u_int32_t *out_que_id);
				  

#endif 


