/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_dl_tx_interface.h

   Function:  Define the data structure and APIs for physical layer interface.

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          -----------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_TX_INTERFACE_H__
#define __PHY_DL_TX_INTERFACE_H__

#include <sys/types.h>

#include "phy_dl_enum.h"
#include "phy_dl_tx_common.h"

#include "phy_proc.h"

#ifdef IPP_OPT_TEMP730 /* To use Intel IPP FFT */
#include "ipp.h"
extern IppsFFTSpec_C_32f* g_pFFTSpecInv;
extern Ipp8u* g_BufInv;
#endif

/* rru symbole */

struct phy_dl_rru_symbol
{
    u_int32_t symbol_offset; /* The offset of a symbol from the start
                                of a frame, the symbol_offset for preamble
                                is 0, and the index of dl other symbols
                                are start from 1 */
    u_int32_t frame_index;/* The index of frame, start from 0 */
    u_int32_t symbol_num; /* The number of ofdma symbols in
                             one phy_rru_symbol */
    u_int32_t symbol_len; /* The number of samples of one ofdma symbol,
                             i.e. 1152 for WiMAX profile 2.0*/
    float *symbol_i;      /* The pointer refers to the real part data of
                             a struct phy_dl_rru_symbol */
    float *symbol_q;      /* The pointer refers to the imaginary part data
                           of a struct phy_dl_rru_symbol */
    u_int32_t dl_perscan_flag;/* flag to indicate the perodical scanning */
//    void              *p_dts_info;
    void              *p_ulmap;
    u_int32_t          ul_map_len;
};

struct phy_dts_info
{
    char active_band[21]; //interference information from spectrum scanning, 1--interference
    int dl_unused_subch;  //unused subchannel number of DL,
                               //  including the interference and active forbidden channel
    int ul_unused_subch; //unused subchannel number of DL,
                               //  including the interference and active forbidden channel
};


/* System paraters for dl */
struct phy_dl_tx_syspara
{
    /** Basic Parameters obtained from WMA Config */
    u_int32_t tx_div; /* number of Tx Antenna, default is 2 */
    u_int32_t frame_index;    /* The index of frame, start from 0 */
    u_int32_t symbol_offset;  /* OFDMA Symbol offset, defined in DL-MAP_IE */
    u_int32_t first_zone;     /* 1 - first dl zone, 0 - other dl zones */
    u_int32_t mimo_mode;      /* 0=siso, 1=stca, 2=stcb */
    u_int32_t symbolnum_per_dl_frame;  /* Number of symbols per dl frame */
    u_int32_t zone_len;
    u_int32_t permutation_type; /* For broadcast region in HARQ zone [0]
                                  0 = PUSC, 1 = FUSC,
                                  2 = Optional FUSC,
                                  3 = AMC */
    u_int32_t ofdma_nfft;  /* Size of the FFT [1024] */
    float ofdma_g;         /* Guard interval index[1/8] */
    u_int32_t cdd_num; /* cdd cyclic sample number */
    u_int32_t dl_permbase; /* Table 326, Table 437 */
    u_int32_t max_cellnum ; /* Maximal Cell Number */
    u_int32_t max_segnum ; /*Maximal Segment Number */
    u_int32_t prbs_id;     /* Used in initialization vector of the PRBS
                              generator for subchannel randomization
                              - 0, 1, 2 */
    u_int32_t pilot_insert;  /* Indicate whether or not the pilots
                                are inserted */

/* Preamble related parameters */
    u_int32_t preamble_index;
    u_int32_t preamble_guard_band;
    u_int32_t preamble_max_running_index;

    /* FCH related parameters */
    u_int8_t *fch_bitmap; //fch bitmap selection
    char *active_band;
    u_int32_t dl_unused_subch;
    char fixed_active_band[21];

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

    u_int32_t num_subcar_of_subch;  /* Number of data subcarriers
                                       per subchannel */
    u_int32_t num_subch;            /* Number of subchannel in dl frame */
    u_int32_t numofsubcarpercluster;/* Table442 number of subcarriers
                                       per cluster */
    u_int32_t numofcluster;         /* Table442 number of clusters */

    u_int32_t num_pilot;     /* Table 442 number of pilot subcarriers
                                per symbol */
    
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

    int32_t *wk;
   
    u_int32_t repetition_coding_indication; /* Table 320 */

    u_int32_t fre_central; /* used for multiband single/dual mode */
    
    u_int32_t dlusesc[18*15]; 

};

int32_t phy_dl_tx_init(struct phy_global_param *phyconf,
                       struct phy_dts_info *dts_para,
                       struct phy_dl_tx_syspara *para);

int32_t phy_dl_tx_deinit(struct phy_dl_tx_syspara *para);
                         
int32_t phy_dl_tx(struct phy_dl_tx_syspara *para,
                  const u_int32_t num_in_que,
                  const u_int32_t *in_que_id,
                  const u_int32_t num_out_que,
                  const u_int32_t *out_que_id);

int32_t phy_dl_deinit_rrusymbol(struct phy_dl_rru_symbol *p_rru_symbol);

int dump_phy_tx_interface_wk(int flag, char * name, FILE * fd, int len, void *p_buf);
#endif
