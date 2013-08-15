/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_PROC_H_
#define __PHY_PROC_H_

#include <sys/types.h>
#include "global.h"

extern pthread_mutex_t mutex_tx_phy_en_flag;
extern int tx_phy_en_flag;

extern pthread_mutex_t mutex_phy_frame_info;
extern struct phy_frame_info gs_phy_frame_info;

extern pthread_mutex_t mutex_ranging_en_flag;
extern int ranging_en_flag;

extern struct phy_dl_tx_syspara *phy_dl_para;
extern struct phy_ul_rx_syspara *phy_ul_para;

struct phy_frame_info
{
    u_int32_t info_flag;
    void * ul_map[UL_MAP_STORE];
    u_int32_t ul_map_len[UL_MAP_STORE];
    void * dts_info[UL_MAP_STORE];
    u_int32_t dl_perscan_flag[UL_MAP_STORE];
    u_int32_t frame_number[UL_MAP_STORE];
};

struct phy_global_param
{
        /** DL system parameters */

        u_int32_t bandwidth;
        u_int32_t ofdma_fs;
        u_int32_t oversampling;
        /* 1-multiple zone support */
        u_int32_t multiple_zone;
        /* number of Tx Antenna, default is 2 */
        u_int32_t tx_div;
        /* The index of frame, start from 0 */
        u_int32_t dl_frame_index;
        /* OFDMA Symbol offset, defined in DL-MAP_IE */
        u_int32_t dl_symbol_offset;
        /* 1 - first dl zone, 0 - other dl zones */
        u_int32_t dl_first_zone;
        /* 0=cdd, 1=stca, 2=stcb */
        u_int32_t dl_mimo_mode;
        /* without counting preamble */
        u_int32_t symbolnum_per_dl_frame;

        u_int32_t dl_zone_len;
        /* For broadcast region in HARQ zone [0] */
        /*    0 = PUSC, 1 = FUSC, */
        /*    2 = Optional FUSC,  */
        /*    3 = AMC             */
        u_int32_t dl_permutation_type;
        /* Size of the FFT [1024] */
        u_int32_t ofdma_nfft;
        /* Guard interval index[1/8] */
        float ofdma_g;
        /* cdd cyclic sample number */
        u_int32_t cdd_num;
        /* Table 326, Table 437 */
        u_int32_t dl_permbase;
        /* Maximal Cell Number */
        u_int32_t max_cellnum;
        /*Maximal Segment Number */
        u_int32_t max_segnum;
        /* Used in initialization vector of the PRBS*/
        /* generator for subchannel randomization */
        /* 0, 1, 2 */
        u_int32_t prbs_id;
        /* Indicate whether or not the pilots are inserted */
        u_int32_t pilot_insert;

        /* Preamble related parameters */
        u_int32_t preamble_index;
        u_int32_t preamble_guard_band;
        u_int32_t preamble_max_running_index;

        /* FCH related parameters */
        u_int8_t fch_bitmap[6];

        /* FEC related parameters */

        /* Always be 1 in current version */
        u_int32_t stc_rate;
        /* Always be 1 in current version */
        u_int32_t stclayer_num;
        /* CC CTC */
        u_int32_t coding_type;

        /** UL system parameters */

        /* Receive antenna number */
        u_int32_t rx_div;

        /* BS ID [0] */
        u_int32_t bs_id;

        /* specifies whether the frame number for the DL/UL subframe */
        /* is increased (0: TRUE; 1: FALSE) */
        u_int32_t frame_increased;

        u_int32_t ul_zone_begin;
        u_int32_t ul_zone_len;

        /* for broadcast region in HARQ zone */
        /* 0 = PUSC, 1 = FUSC, 2 = Optional FUSC, 3 = AMC  [0] */
        u_int32_t ul_permutation_type;
        /* number of data symbols for UL per frames UL_MAP */
        u_int32_t symbolnum_per_ul_frame;
        /* (0~69) specifies the basis of uplink permutation */
        u_int32_t ul_permbase;
        /* UL subchannel selection bitmap, 1: active 0: inactive */
        u_int8_t ul_bitmap[35];

        /* ranging related information */
        u_int32_t ranging_symoffset;
        u_int32_t ranging_subchoffset;
        u_int32_t ranging_sym;
        u_int32_t ranging_subch;

        /* FEC related */
        u_int32_t ul_mimo_mode; /* 0-CDD, 1-STCA, 2-STCB */
        /* 0 CC 1 CTC */
        u_int32_t ul_coding_type;

        int      cali_ana_pwr[2]; // It will be set when system is initiated
        int      cali_digi_pwr[2]; //It will be set when system is initiated.
        int      cali_dgain[2]; //added by WQ

        /* for PHY clip algorithm selection*/
        u_int32_t softbit_threshold;
        u_int8_t softbit_shift;
        /* for Periodical Sensing */
        float ps_thd;
        u_int32_t fre_central; 


};

int phy_bs_process (void);

int phy_bs_release (void);

int cfg_update_phy_interference(int type, int len, void * buf);

int cfg_update_ranging_exp_power(int type, int len, void * buf);

int cfg_update_ranging_algorithms(int type, int len, void * buf);


#endif /* __PHY_H_ */
