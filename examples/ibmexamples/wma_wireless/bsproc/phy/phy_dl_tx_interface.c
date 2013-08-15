/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_tx_interface.c

   Function: The interface functions for PHY-FRM and Adapter - PHY.

   Change Activity:

   Date              Description of Change                            By
   ------------      -------------------------------                  --------
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "flog.h"
#include "phy_proc.h"

#include "phy_dl_tx.h"
#include "adapter_bs_dl_interface_data.h"
#include "phy_dl_utility.h"
#include "phy_dl_wk_generator.h"
#include "phy_dl_tx_common.h"
#include "dump_util.h"
#include "phy_dl_interleaver.h"

#ifdef IPP_OPT_TEMP730 /* To use Intel IPP FFT */
IppsFFTSpec_C_32f* g_pFFTSpecInv;
Ipp8u* g_BufInv;
#else                  /* Does not use Intel IPP FFT */
#include "fft.h"
#endif                 /* End of 'ifdef IPP_OPT_TEMP730' */


/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_init()

   Description: Initialize the transmitter includes dl system parameters.

   Parameters:
                Input/Output- [struct phy_dl_tx_syspara *para] The pointer 
                              refers to the struct of system parameters.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */



int32_t phy_dl_tx_init(struct phy_global_param *phyconf,
                       struct phy_dts_info *dts_para,
                       struct phy_dl_tx_syspara *para)
{
    if (phyconf == NULL ||para == NULL ||dts_para ==NULL) 
    {
        FLOG_FATAL("phy_dl_tx_init: The pointer refers to input is null!\n");
        return ERROR_CODE;
    }

    para->tx_div = phyconf->tx_div;
    if (para->tx_div > 2)
    {
        FLOG_WARNING("phy_dl_tx_init: Up to mimo 2x2 is supported!\n");
        return ERROR_CODE;
    }
     
    para->fch_bitmap = (u_int8_t *) malloc (sizeof(u_int8_t)*(6));
    para->active_band = (char *)malloc(sizeof(char) * 21 );
    if (para->fch_bitmap  == NULL || para->active_band == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_init");
        return 1;
    }

    int32_t err_code;
#ifdef _MULTIBAND_
    u_int32_t usedsc;
    u_int32_t i,j;
    float center;
#endif    

    para->frame_index = phyconf->dl_frame_index;  //0   /* The index of frame, start from 0 */
    para->symbol_offset = phyconf->dl_symbol_offset;//0  /* OFDMA Symbol offset, defined in DL-MAP_IE */
    para->first_zone = phyconf->dl_first_zone; //1    /* 1 - first dl zone, 0 - other dl zones */
    para->mimo_mode = phyconf->dl_mimo_mode;  //0    /* 0=siso, 1=stca, 2=stcb */
    para->symbolnum_per_dl_frame = phyconf->symbolnum_per_dl_frame; //9 /* Number of symbols per dl frame */
    para->zone_len = phyconf->dl_zone_len;//9
    para->permutation_type = phyconf->dl_permutation_type; /* For broadcast region in HARQ zone [0]
                                  0 = PUSC, 1 = FUSC,
                                  2 = Optional FUSC,
                                  3 = AMC */
    para->ofdma_nfft = phyconf->ofdma_nfft;//1024  /* Size of the FFT [1024] */
    para->ofdma_g = phyconf->ofdma_g;  //1/16       /* Guard interval index[1/8] */
    para->cdd_num = phyconf->cdd_num;//2 /* cdd cyclic sample number */
    para->dl_permbase = phyconf->dl_permbase; //7 /* Table 326, Table 437 */
    para->max_cellnum = phyconf->max_cellnum;//32 /* Maximal Cell Number */
    para->max_segnum = phyconf->max_segnum;//3 /*Maximal Segment Number */
    para->prbs_id = phyconf->prbs_id;     /* Used in initialization vector of the PRBS
                              generator for subchannel randomization
                              - 0, 1, 2 *///0
    para->pilot_insert = phyconf->pilot_insert;  /* Indicate whether or not the pilots
                                are inserted *///1

    /* Preamble related parameters */
    para->preamble_index = phyconf->preamble_index;//39
    para->preamble_guard_band = phyconf->preamble_guard_band;//134
    para->preamble_max_running_index = phyconf->preamble_max_running_index;//283


    /* dts_info initial-- for UT only */
//    dts_para->active_band = (int8_t *)malloc(sizeof(int8_t) * 21);
//    memcpy(dts_para->active_band, active_band, 21*sizeof(int8_t));
//    dts_para->dl_unused_subch = 4;//4

    /* FCH related parameters */
    memcpy(para->fch_bitmap, phyconf->fch_bitmap, sizeof(u_int8_t)*6);
    para->dl_unused_subch = dts_para->dl_unused_subch;
    memcpy(para->active_band, dts_para->active_band, 21*sizeof(char));
    memcpy(para->fixed_active_band, dts_para->active_band, 21*sizeof(char));

   /* FEC related parameters */

    para->stc_rate = phyconf->stc_rate;  //1     /* Always be 1 in current version */
    para->stclayer_num = phyconf->stclayer_num; //1  /* Always be 1 in current version */
    para->coding_type = phyconf->coding_type;//0 


    para->max_symbolnum_per_frame = 48;
    para->repetition_coding_indication = 3;
            
    if (para->preamble_index > 113)
    {
        FLOG_ERROR ("phy_dl_tx_init: The 'index' is out of range!\n");
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
  //  printf("-----------segment = %d\n-------------------", para->segment_index);

    switch (para->permutation_type)
    {
        case PUSC:
        {
            para->symbol_per_slot = 2;
            if ( para->ofdma_nfft == 1024)
            {
                para->num_subch = 30;
                para->ofdma_nused = 841;
                para->ofdma_dc = 512;
                para->numofsubcarpercluster = 14;
                para->numofcluster = 60;
                para->num_subcar_of_subch = 24;
                para->num_pilot = para->numofcluster * 2;
                para->num_phyband = 21;//24
 
            }
            break;
          
        }
        case FUSC:
        case OFUSC:
        {
            FLOG_WARNING("Unsupported permutation type !:\n");
            return ERROR_CODE;
        }
        case AMC:
        {
            para->symbol_per_slot = 3;
            if ( para->ofdma_nfft == 1024)
            {
#ifndef _MULTIBAND_                
                para->num_subch = 42;//the standard subch is 48, but in FOAK, it's 42
                para->ofdma_nused = 757;//865
                para->ofdma_dc = 512;
                para->numofbins_of_phyband = 4;
                para->num_phyband = 21;//24
                para->num_subcar_of_subch = 48; //datasubcarrier number per slot
                para->num_pilot = para->num_phyband * 4; //standard: 96, FOAK: 84
                para->num_data_subcar = 672; //768
	        para->bins_perslot = 2; //one subchannel contains 2 bins for AMC 2X3
		para->datasub_persubsym = 16; //data subcarriers number of one subchannel in 1 ofdm symbol
	        para->pilot_persubsym = 2;//pilot subcarriers number of one subchannel in 1 ofdm symbol
		para->num_pilot_of_subch = 6; //pilot number per slot
		para->pilot_period = 3; //pilot period in time direction for one symbol slot
		para->num_bins = para->num_subch * 2; //number of bins
		para->left_guard = 134;
		para->right_guard = para->left_guard-1; //133
#else
                para->num_subch = 15;//the standard subch is 48, but in FOAK, it's 42
                para->ofdma_nused = 757;//865
                para->ofdma_dc = 512;
                para->numofbins_of_phyband = 2;
                para->num_phyband = 15;//24
                para->datasub_persubsym = 16; //data subcarriers number of one subchannel in 1 ofdm symbol
                para->pilot_persubsym = 2;//pilot subcarriers number of one subchannel in 1 ofdm symbol
                para->num_subcar_of_subch = 48; //datasubcarrier number per slot
                para->num_pilot = para->num_phyband * 2; //standard: 96, FOAK: 84
                para->num_data_subcar = para->num_subch * para->datasub_persubsym; //768
                para->bins_perslot = 2; //one subchannel contains 2 bins for AMC 2X3
                para->num_pilot_of_subch = 6; //pilot number per slot
                para->pilot_period = 3; //pilot period in time direction for one symbol slot
                para->num_bins = para->num_subch * 2; //number of bins
                para->left_guard = 134;
                para->right_guard = para->left_guard-1; //133
#endif
            }
            break;

        }
    }

    para->ofdma_ncp = para->ofdma_nfft * para->ofdma_g;
    para->ofdma_symlen_with_guard = para->ofdma_nfft + para->ofdma_ncp;
    para->ofdma_nused_no_dc = para->ofdma_nused - 1;

#ifdef _MULTIBAND_
    u_int32_t freq_dual[30] = {223525,223675,223725,223850,223950,224025,224125,
                               224175,224225,224325,224425,224475,224525,224575,
                               224650,230525,230675,230725,230850,230950,231025,
                               231125,231175,231225,231325,231425,231475,231525,
                               231575,231650};
    u_int32_t freq_single[10] = {228075,228125,228175,228250,228325,
                                 228400,228475,228550,228675,228750};
                                 

    para->fre_central = phyconf->fre_central;
    usedsc = 18;
    memset(para->dlusesc, 0, (usedsc*para->num_subch)*sizeof(u_int32_t));
    if (para->fre_central == 228450)
    {
        for (i=0; i<10; i++)
        {
            center = (((float)freq_single[i] - (float)para->fre_central)* 1000)/(phyconf->ofdma_fs/para->ofdma_nfft);
            printf("---------------------center = %f\n-----------------------", center);
            for (j=0; j<usedsc/2; j++)
            {
                para->dlusesc[i*usedsc+j] = floor(center) - usedsc/2 + j+1 + 512;
                para->dlusesc[i*usedsc+usedsc/2+j] = ceil(center)+j + 512;
            }
        }

    }
    else
    {
        for (i=0; i<para->num_subch; i++)
        {
            center = ((freq_dual[i] - para->fre_central)*1000)/(phyconf->ofdma_fs/para->ofdma_nfft);
            for (j=0; j<usedsc/2; j++)
            {
                para->dlusesc[i*usedsc+j] = floor(center) - usedsc/2 + j+1 + 512;
                para->dlusesc[i*usedsc+usedsc/2+j] = ceil(center)+j + 512;
            }
        }
    }   


#if 0
    FILE *fp0;
    fp0 = fopen("./data_dst/0_dlusedsc.out","w+t");
    dl_dump_uinteger(fp0, para->dlusesc,para->num_subch*18);
    fclose(fp0);
#endif


#endif

    /* Initialize Wk */
    para->wk = (int32_t *) malloc (sizeof(int32_t)*(para->ofdma_nused_no_dc + para->symbol_per_slot + 32));
    if ( para->wk == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_init");
        return 1;
    }
    err_code = phy_dl_wk_generator(para,
                                   para->wk); 
			                 
    if (err_code)
    {
	FLOG_ERROR("E008_tx: Error in wk generation!\n");
	return err_code;
    }

    DO_DUMP(DUMP_PHY_TX_INTERFACE_WK_ID, 0, para, 0);

#ifdef IPP_OPT_TEMP730
    ippsFFTInitAlloc_C_32f(&g_pFFTSpecInv, 
                           (int32_t)(log(para->ofdma_nfft)/log(2)), 
                           IPP_FFT_DIV_INV_BY_N, 
                           ippAlgHintFast);
    int32_t BufSize;
    ippsFFTGetBufSize_C_32f(g_pFFTSpecInv, &BufSize);
    g_BufInv = malloc(BufSize*sizeof(Ipp8u));
    if ( g_BufInv == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_init");
        return 1;
    }
#else
    fft_init (para->ofdma_nfft, &para->XX, &para->x, &para->X);
#endif

    phy_dl_init_interleave_setting();
    return SUCCESS_CODE;
}


/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_deinit()

   Description: De-initialize the dl transmitter include system parameters.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para] The pointer refers 
                        to the struct of system parameters.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */
int32_t phy_dl_tx_deinit(struct phy_dl_tx_syspara *para)
                         
{
    if (para == NULL) 
    {
        FLOG_ERROR("phy_dl_tx_deinit: The pointer refer to input is null!\n");
        return ERROR_CODE;
    }

    free(para->wk);
    free(para->fch_bitmap);
    free(para->active_band);    
#ifdef IPP_OPT_TEMP730
    ippsFFTFree_C_32f(g_pFFTSpecInv);
    if(g_BufInv!=NULL) 
    {
        free(g_BufInv);
        g_BufInv = NULL;
    }
#else
    fft_quit (para->XX, para->x, para->X);
#endif

    return SUCCESS_CODE;
}

/**----------------------------------------------------------------------------
 *    Function:    phy_dl_tx()
 *
 *    Description: To genarate dl sub frame.
 *
 *    Parameters:
 *        Input-  [struct phy_dl_tx_syspara *para]  The pointer refer 
 *                to the struct of system parameters.
 *                [const u_int32_t num_in_que] The number of input 
 *                queues.
 *                [const u_int32_t *in_que_id] The buffer contains the 
 *                ids of input queues.
 *
 *        Output- [const u_int32_t num_out_que] The number of output 
 *                [const u_int32_t *out_que_id] The buffer contains the
 *                ids of output queues.
 *                                                                                                                                             Return Value:
 *         0       Success
 *         150     Error
 *
 *                                                                                                                                                                                                                                                                                ----------------------------------------------------------------------------
 * LOG END TAG zYx                                                            */

int32_t phy_dl_tx(struct phy_dl_tx_syspara *para,
                  const u_int32_t num_in_que,
                  const u_int32_t *in_que_id,
                  const u_int32_t num_out_que,
                  const u_int32_t *out_que_id)
{
    int32_t err_code;
   
    if (num_out_que != para->tx_div)
    {
        FLOG_FATAL("phy_dl_tx: The number of output queue error!\n");
        return ERROR_CODE;
    }
    if (num_out_que < num_in_que)
    {
        FLOG_ERROR("phy_dl_tx: num_out_que < num_in_que\n");
        return ERROR_CODE;
    }
    if (para == NULL || in_que_id == NULL || out_que_id == NULL) 
    {
        FLOG_FATAL("phy_dl_tx: The pointer refer to buffer is null!\n");
        return ERROR_CODE;
    }
    
    switch(para->tx_div)
    {
      	case 1:
            err_code = phy_dl_tx_div1(para,
                                      *in_que_id, 
                                      *out_que_id);
            if (err_code) {
                FLOG_ERROR("phy_dl_tx: Error in transmitter!\n");
                return err_code;
            }
            break;
      	case 2:
            if (num_in_que == 1)
            {
            	err_code = phy_dl_tx_div2(para,
            	                          *in_que_id,
            	                          *in_que_id,
            	                          out_que_id[0],
            	                          out_que_id[1]);
                if (err_code) 
                {
                    FLOG_ERROR("phy_dl_tx: Error in transmitter!\n");
                    return err_code;
                }
            }
            else
            {
            	err_code = phy_dl_tx_div2(para,
            	                          in_que_id[0],
            	                          in_que_id[1],
            	                          out_que_id[0],
            	                          out_que_id[1]);
                if (err_code) 
                {
                    FLOG_ERROR("phy_dl_tx: Error in transmitter!\n");
                    return err_code;
                }
            }
            break;
      	default:
            FLOG_WARNING("Suport up to mimo 2x2!\n");
            return ERROR_CODE;
    }
   
    return SUCCESS_CODE;
    
}
                  
/**----------------------------------------------------------------------------
 *    Function:    phy_dl_deinit_rrusymbol()
 *
 *    Description: Memory release for rru symbol.
 *
 *    Parameters:
 *       Input  -  [struct phy_dl_rru_symbol *p_rru_symbol] The pointer
 *                 refer to the rru symbol.
 *
 *    Return Value:
 *       0       Success
 *       150     Error
 *    Notes:
 *       None
 *-------------------------------------------------------------------
 * LOG END TAG zYx                                                            */

int32_t phy_dl_deinit_rrusymbol(struct phy_dl_rru_symbol *p_rru_symbol)
{
    if(p_rru_symbol != NULL) {
        if(p_rru_symbol->symbol_i != NULL)
        {
            free(p_rru_symbol->symbol_i);
            p_rru_symbol->symbol_i = NULL;
        }
            
        if(p_rru_symbol->symbol_q != NULL) 
        {
            free(p_rru_symbol->symbol_q);
            p_rru_symbol->symbol_q = NULL;
        }
        free(p_rru_symbol);
        p_rru_symbol = NULL;
    }

    return SUCCESS_CODE;
}

/**----------------------------------------------------------------------------
 *    Function:    dump_phy_tx_interface_wk
 *
 *    Description: Dump PHY tx interface wk.
 *
 *    Parameters:
 *       Input  -  [int flag] flag 
 *       	-  [char * name] file name 
 *       	-  [FILE *fd] file handle to put the dump data in 
 *       	-  [int len] data length 
 *       	-  [void *p_buf] data buffer 
 *
 *    Return Value:
 *       0       Success
 *       others  Error
 *    Notes:
 *       None
 *-------------------------------------------------------------------
 * LOG END TAG zYx                                                            */
int dump_phy_tx_interface_wk(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;
    (void)	len;

    u_int32_t i;
    struct phy_dl_tx_syspara *para = (struct phy_dl_tx_syspara *)p_buf;

    for (i=0; i< para->ofdma_nused_no_dc + para->symbol_per_slot + 32; i++)
    {
         fprintf(fd, "%d\n", para->wk[i]);
    }
    FLOG_DEBUG("complete wk generation!\n");

    return 0;
}
