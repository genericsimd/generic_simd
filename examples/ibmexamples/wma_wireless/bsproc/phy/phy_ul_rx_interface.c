/* ----------------------------------------------------------------------------

  IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved. 

   File Name: phy_ul_rx_interface.c



   Function:



   Change Activity:



   Date             Description of Change                          By

   -------------    ---------------------                          ------------


   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */



#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <math.h>

#include "phy_periodic_sensing.h"

#include "phy_proc.h"
#include "flog.h"
#include "bs_cfg.h"
#include "prephy_proc.h"


#include "phy_ul_rx_interface.h"
#include "phy_ul_rx.h"
#include "phy_ul_rx_common.h"
#include "phy_ul_rx_ranging.h"
#include "fft.h"
#include "phy_ul_deinterleaver.h"

#ifdef _DEBUG_WMAPHY_
#include "phy_ul_rx_utility.h"
#endif

#include "viterbicore.h"


#ifdef IPP_OPT_TEMP730
#include "ipp.h"
    IppsFFTSpec_C_32f* pFFTSpecInv_ul;
    Ipp8u* BufInv_ul;
    IppsFFTSpec_C_32f* pFFTSpecFwd_ul;
    Ipp8u* BufFwd_ul;
#endif



/**----------------------------------------------------------------------------
 *
 *    Function:    phy_ul_init_rru_symbol()
 *
 *    Description: Memory malloc and initilize for rru symbol.
 *
 *    Parameters:
 *       Input-   [uint32_t frame_index]  The index of frame.
 *                [uint32_t symbol_offset] The offset of symbols
 *                                        counting from start of frame.
 *                [uint32_t symbol_num] The number of ofdma symbols in                                                                                                                        current rru symbol.
                  [uint32_t symbol_len] The length in byte for one
 *                                      ofdma symbol.
 *                                                                                                                                            Return Value:
 *                                                                                                                                             0       Success
 *                                                                                                                                            150     Error
 *                                                                                                                                             Notes:
                                                                                                                                         ----------------------------------------------------------------------------
 *                                                                                                                                          LOG END TAG zYx                                                            */

struct phy_ul_rru_symbol* phy_ul_init_rrusymbol(u_int32_t frame_index,
                                                u_int32_t symbol_offset,
                                                u_int32_t symbol_num,
                                                u_int32_t symbol_len,
                                                u_int8_t endflag)
{
    struct phy_ul_rru_symbol *p_rru_symbol;
    p_rru_symbol =(struct phy_ul_rru_symbol *)calloc(1, sizeof(struct phy_ul_rru_symbol));
    if (p_rru_symbol == NULL)
    {
        FLOG_FATAL("phy_ul_init_rru_symbol: Error in calloc memory for rru_symbol!\n");
        return NULL;
    }

    p_rru_symbol->frame_index = frame_index;
    p_rru_symbol->symbol_len = symbol_len; 
    p_rru_symbol->symbol_num = symbol_num;
    p_rru_symbol->symbol_offset = symbol_offset;
    p_rru_symbol->ul_subframe_endflag = endflag;
    p_rru_symbol->ul_fake_flag = 0;
    p_rru_symbol->ul_map = NULL;
    p_rru_symbol->ul_map_len = 0;

    p_rru_symbol->symbol_i =(float *)calloc(p_rru_symbol->symbol_num*p_rru_symbol->symbol_len, sizeof(float));
    p_rru_symbol->symbol_q =(float *)calloc(p_rru_symbol->symbol_num*p_rru_symbol->symbol_len, sizeof(float));

    if (p_rru_symbol->symbol_i == NULL || p_rru_symbol->symbol_q == NULL)
    {
        FLOG_FATAL("phy_ul_init_rru_symbol: Error in calloc memory for rru_symbol!\n");
        return NULL;
    }

    return p_rru_symbol;

}


/**----------------------------------------------------------------------------

   Function:    phy_ul_rx_init()

   Description: Initialize the receiver includes ul system parameters.

   Parameters:
                Input/Output-  [phy_ul_rx_syspara *para] The pointer refers to
                               the struct of system parameters.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */

int32_t phy_ul_rx_init(struct phy_global_param *phyconf,
                       struct phy_dts_info *dts_para,
                       struct phy_ul_rx_syspara *para)
                   
{

    if (para == NULL||phyconf == NULL)
    {
        FLOG_FATAL("E000_init: The pointer refer to input sys_para structure is null!\n");
        return ERROR_CODE;
    }
     if (dts_para == NULL)
    {
        FLOG_FATAL("dts_init: The pointer refers to input is null!\n");
        return ERROR_CODE;
    }

   // char active_band[21] = {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0};

    /* System framework parameters */
    para->bandwidth = phyconf->bandwidth;
    para->ofdma_fs = phyconf->ofdma_fs;
    para->oversampling = phyconf->oversampling;
    para->multiple_zone = phyconf->multiple_zone;
    para->rx_div = phyconf->rx_div;

    para->frame_index = phyconf->dl_frame_index;  //0   /* The index of frame, start from 0 */
    para->frame_increased = phyconf->frame_increased; //1
    para->symbol_offset = phyconf->ul_zone_begin;//0  /* OFDMA Symbol offset, defined in DL-MAP_IE */
    para->mimo_mode = phyconf->ul_mimo_mode;  //0    /* 0=siso, 1=stca, 2=stcb */
    para->symbolnum_per_ul_frame = phyconf->symbolnum_per_ul_frame; //9 /* Number of symbols per dl frame */
    para->zone_duration= phyconf->ul_zone_len;//9
    para->permutation_type = phyconf->ul_permutation_type; /* For broadcast region in HARQ zone [0]
                                  0 = PUSC, 1 = FUSC,
                                  2 = Optional FUSC,
                                  3 = AMC */
    para->ofdma_nfft = phyconf->ofdma_nfft;//1024  /* Size of the FFT [1024] */
    para->ofdma_g = phyconf->ofdma_g;  //1/16       /* Guard interval index[1/8] */
    para->cdd_num = phyconf->cdd_num;//2 /* cdd cyclic sample number */
    para->ul_permbase = phyconf->ul_permbase; //7 /* Table 326, Table 437 */
    para->max_cellnum = phyconf->max_cellnum;//32 /* Maximal Cell Number */
    para->max_segnum = phyconf->max_segnum;//3 /*Maximal Segment Number */
    para->prbs_id = phyconf->prbs_id;     /* Used in initialization vector of the PRBS
                              generator for subchannel randomization
                              - 0, 1, 2 *///0
    para->pilot_insert = phyconf->pilot_insert;  /* Indicate whether or not the pilots
                                are inserted *///1

    /* Preamble related parameters */
    para->preamble_index = phyconf->preamble_index;//39


    /* dts_info initial-- for UT only */
//    dts_para->active_band = (int8_t *)malloc(sizeof(int8_t) * 21);
//    memcpy(dts_para->active_band, active_band, 21*sizeof(int8_t));
//    dts_para->ul_unused_subch = 6;//6

    /* UL_bitmap related parameters */
    memcpy(para->ul_bitmap, phyconf->ul_bitmap, sizeof(u_int8_t)*35);
    para->ul_unused_subch = dts_para->ul_unused_subch;
    memcpy(para->active_band, dts_para->active_band, 21*sizeof(char));
    memcpy(para->fixed_active_band, dts_para->active_band, 21*sizeof(char));

   /* DL_bitmap setting */
    memcpy(para->fch_bitmap, phyconf->fch_bitmap, sizeof(u_int8_t)*6);


   /* FEC related parameters */
    para->stc_rate = phyconf->stc_rate;  //1     /* Always be 1 in current version */
    para->stclayer_num = phyconf->stclayer_num; //1  /* Always be 1 in current version */
    para->coding_type = phyconf->ul_coding_type;//0

    para->max_num_symbol_per_frame = 48;
  //  para->zone_begin = phyconf->ul_zone_begin;

     switch (para->permutation_type)
    {
        case PUSC:
        {
            para->symbol_per_slot = 3;
            if ( para->ofdma_nfft == 1024)
            {
                para->num_subch = 31; //35 for standard;
                para->ofdma_nused = 757; //841 for standard
                para->ofdma_dc = 512;
                para->num_subcar_of_subch = 24;
                para->num_tiles_perslot = 6;//6
                para->num_subcar_group = 48; //48
                para->num_datasubcar_perslot = 48;
		para->num_pilot_perslot = 24;
                para->num_subcar_pertileF = 4;
		para->zone_len = para->symbolnum_per_ul_frame/para->symbol_per_slot;
                para->numoftiles = para->num_subch * para->num_tiles_perslot; 
            }
            break;

        }
        case OPUSC:
        {
            FLOG_WARNING("Unsupported permutation type !:\n");
            return ERROR_CODE;
        }
        case AMC://not standard, just for FOAK
        {
            para->symbol_per_slot = 3;
            if ( para->ofdma_nfft == 1024)
            {
                para->num_subch = 42;//the standard subch is 48, but in FOAK, it's 42
                para->ofdma_nused = 757;//865
                para->ofdma_dc = 512;
                para->numofbins_of_phyband = 4;
                para->num_phyband = 21;//24
                para->num_subcar_of_subch = 48;
                para->num_pilot = para->num_phyband * 4; //standard: 96, FOAK: 84
                para->num_data_subcar = 672; //768
            }
            break;
        }
    }
                                                                
    para->ofdma_nused_no_dc = para->ofdma_nused - 1;
    para->ofdma_ncp = para->ofdma_nfft * para->ofdma_g;
    para->ofdma_sym_len_with_guard = para->ofdma_nfft + para->ofdma_ncp;
   
    para->sample_per_frame = para->ofdma_sym_len_with_guard * para->symbolnum_per_ul_frame;
    para->sample_per_slotsymbol = para->ofdma_sym_len_with_guard * para->symbol_per_slot;
    para->frame_num = para->frame_index;
    para->usedsubch_num = para->num_subch-para->ul_unused_subch;
    para->demod_gain = 1;
	
	/* ranging related information */
    para->ranging_symoffset = phyconf->ranging_symoffset;//0
    para->ranging_subchoffset = phyconf->ranging_subchoffset;//0
    para->ranging_sym = phyconf->ranging_sym;//3
    para->ranging_subch =  phyconf->ranging_subch;//6


    if (para->preamble_index > 113)
    {
        FLOG_WARNING ("phy_dl_tx_init: The 'index' is out of range!\n");
        return ERROR_CODE;
    }
    else if (para->preamble_index < 32)
        para->segment_index = 0;
    else if (para->preamble_index < 64)
        para->segment_index = 1;
    else if (para->preamble_index < 96)
        para->segment_index = 2;
    else
        para->segment_index = para->preamble_index % 3;

    para->id_cell = para->preamble_index % 32;

    para->repetition_coding_indication = 3;

   /* fec init */

    struct phy_ul_fec_para *fecpara;
    fecpara = &para->fec_para;

    //for WiMAX 16e the minimum fec size is 48, the maximum fec size is 432 

    if((fecpara->vp = (void*) create_viterbi(500)) == NULL)
    {
        FLOG_FATAL("create_viterbi27 failed\n");
        return ERROR_CODE;
    }

#ifdef IPP_OPT_TEMP730
    ippsFFTInitAlloc_C_32f(&pFFTSpecFwd_ul, (int)(log(para->ofdma_nfft)/log(2)), IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    int BufSizeFwd;
    ippsFFTGetBufSize_C_32f(pFFTSpecFwd_ul, &BufSizeFwd);
    BufFwd_ul = malloc(BufSizeFwd*sizeof(Ipp8u));
    if(BufFwd_ul == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_rx_init\n");
        return -1;
    }
#else /* non IPP_OPT_TEMP730 */
    fft_init(para->ofdma_nfft, &para->XX, &para->x, &para->X);
#endif

    /*parameters for power adjustment */
    para->CaliDgain0 = (float)phyconf->cali_dgain[0]; 
    para->CaliAnaPwr0 = (float)phyconf->cali_ana_pwr[0]; //-60.0;
    para->CaliDigiPwr0 = (float)phyconf->cali_digi_pwr[0];// 10000.0;
    para->CaliDgain1 = (float)phyconf->cali_dgain[1];// 0.0; 
    para->CaliAnaPwr1 = (float)phyconf->cali_ana_pwr[1];// -60.0;
    para->CaliDigiPwr1 = (float)phyconf->cali_digi_pwr[1]; //11000.0; 

    /* paramters with spectrum sensing */
    para->is_nfft = 1024;
    para->ps_nfft = 1024;

    /* added softbit threshold selection */
    para->softbit_threshold =((float)phyconf->softbit_threshold)/1000.0F;
    para->softbit_shift = phyconf->softbit_shift;
 
    para->blkerr_num = 0;

   /* add for the PS threshold setting */
    para->ps_thd = phyconf->ps_thd;

    phy_ps_init_history();

    phy_ul_init_deinterleave_setting();

#ifdef _BER_TEST_
    int ret = 0;
    ret = get_global_param ("FAKE_ULMAP_DURATION", &g_fake_ulmap_duration);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_DURATION error\n");
    }

    ret = get_global_param ("FAKE_ULMAP_UIUC", &g_fake_ulmap_uiuc);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_UIUC error\n");
    }

#endif


    return SUCCESS_CODE;
    

}

/**----------------------------------------------------------------------------

   Function:    phy_ul_rx_deinit()

   Description: De-initialize the receiver.

   Parameters:
                Input/Output-  [phy_ul_rx_syspara *para] The pointer refers to
                               the struct of system parameters.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */

int32_t phy_ul_rx_deinit(struct phy_ul_rx_syspara *para) 
                         
{  
    delete_viterbi(para->fec_para.vp);

#ifdef IPP_OPT_TEMP730
    ippsFFTFree_C_32f(pFFTSpecFwd_ul);
    free(BufFwd_ul);
#else /* non IPP_OPT_TEMP730 */
    fft_quit(para->XX, para->x, para->X);
#endif

  phy_ps_deinit_history();

  return SUCCESS_CODE;

}


/**----------------------------------------------------------------------------

   Function:    phy_ul_rx()

   Description: To receive and decode dl sub frame by slot_symbol.

   Parameters:

                Input-  [phy_ul_rx_syspara *para]  The pointer refer to the
                        struct of system parameters.
                        [const uint32_t num_in_que] The number of input queue.
                        [const uint32_t *in_que_id] The buffer contains the
                        id of input queue.

                Output- [const uint32_t num_out_que] The number of output queue.
                        [const uint32_t *out_que_id] The buffer contains the
                        id of output queue.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                                                           */
                                  

int32_t phy_ul_rx(struct phy_ul_rx_syspara *para,
                  const u_int32_t num_in_que,
                  const u_int32_t *in_que_id,
                  const u_int32_t num_out_que,
                  const u_int32_t *out_que_id)
{
    int32_t err_code;


    if (num_in_que != para->rx_div)
    {
        FLOG_FATAL("phy_ul_rx: The number of input queue error!\n");
        return ERROR_CODE;
    }

    if (num_in_que > 2 ) 
    {
        FLOG_WARNING("phy_ul_rx: Up to mimo 2x2 is supported!\n");
        return ERROR_CODE;
    }

    if (num_out_que > num_in_que)
    {
        FLOG_ERROR("phy_ul_rx: The number of output queue is larger than number of input queue!\n");
        return ERROR_CODE;
    }

    if (para == NULL || in_que_id == NULL 
        || out_que_id == NULL) 
    {
        FLOG_FATAL("phy_ul_rx: The pointer refer to buffer is null!\n");
        return ERROR_CODE;
    }


    switch(para->rx_div)
    {
    	case 1:
            err_code = phy_ul_rx_div1(para,
                                      *in_que_id, 
                                      *out_que_id);
            if (err_code)
            {
                FLOG_ERROR("phy_ul_rx: Error in receiver for single antenna!\n");
                return err_code;
            }

            break;

    	case 2:
            err_code = phy_ul_rx_div2( para,
                                       in_que_id[0], 
                                       in_que_id[1],
                                       out_que_id[0],
                                       out_que_id[1]);
            if (err_code)
            {
                FLOG_ERROR("phy_ul_rx: Error in receiver for two antennas!\n");
                return err_code;
            }

            break;

    	default:
    	    FLOG_WARNING("Unsuport up to mimo 2x2!\n");
    	    return ERROR_CODE;
    }

    return SUCCESS_CODE;

}










