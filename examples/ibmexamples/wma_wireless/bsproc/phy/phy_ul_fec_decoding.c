/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ofdma_ul_fec_decoding.c

   Function:  API functions for the FEC decoding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "phy_ul_rx_common.h"
#include "phy_ul_rx_interface.h"

#include "phy_ul_deinterleaver.h"
#include "phy_ul_depuncture.h"
#include "clip.h"
#include "phy_ul_decoder_cctailbiting.h"
#include "phy_ul_derepetition.h"
#include "phy_ul_derandomizer.h"

#include "phy_ul_fec_decoding.h"

#include "flog.h"
#include "prof_util.h"


/* turn on/off dump according to DUMP_PHY_UL_FEC setting */
#ifndef DUMP_PHY_UL_FEC

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

#ifdef DUMP_PHY_UL_FEC
#include "phy_ul_rx_utility.h"
#endif

//#define _SOFTBIT_ONE_
//#define _SOFTBIT_TWO_
//#define _SOFTBIT_THREE_


/*
 Data randomization is performed on all data transmitted on the DL and UL, except the FCH. The
 randomization is initialized on each FEC block.
required parameters
Ns    is the allocated slots number for data burst 30
Data allocated      30 * 24 * 4 /2  (DL_PUSC)
R    is the repetition factor
FEC block size = (modulation, coding rate, Ns)
*/
int32_t set_fec_blocksize( struct phy_ul_rx_syspara *para, 
                           struct union_burst_ie *burstpara,
                           struct phy_ul_fec_para *fecpara)
{
    int j,n,k,m;
    int ceilslotnum, floorslotnum;
    int tablej[7] ={6, 4, 3, 2, 2 ,1 ,1};
    int slotsizetable[7] = {6, 9, 12, 18, 18, 24,27};
    fecpara->code_id = burstpara->code_id;
    int slotsize = 8*slotsizetable[fecpara->code_id];
    j = tablej[fecpara->code_id]; //j slot number 
  //FLOG_DEBUG(" repetition = %d\n", burstpara-> repetition_coding_indication);
    n = (int)floor(burstpara->slots_num * para->stc_rate / (burstpara-> repetition_coding_indication* para->stclayer_num)); 
   //FLOG_DEBUG(" number n = %d\n", n);
    k = (int)floor(n / j);  //block number
  // FLOG_DEBUG(" number k = %d\n", k);
    m = (int) n % j; 
    ceilslotnum = (int) ceil((double)(m+j)/2); 
  // FLOG_DEBUG(" number ceilslotnum = %d\n", ceilslotnum);
    floorslotnum = (int) floor((m+j)/2);
  // FLOG_DEBUG(" number floorslotnum = %d\n", floorslotnum); 

    if(n<=j)
    {
        fecpara->blockjslot = 1;            
        fecpara->blocksizejslot = n * slotsize;
        fecpara->blocksizeceilslot = 0;     
        fecpara->blocksizefloorslot = 0;

         para->blk_per_frame = 1;
    }
    else
    {
        if(m ==0) {
            fecpara->blockjslot = k;            
            fecpara->blocksizejslot = j * slotsize;
            fecpara->blocksizeceilslot = 0;     
            fecpara->blocksizefloorslot = 0;

             para->blk_per_frame = k;
        }
        else {
            fecpara->blockjslot = k-1;            
	    fecpara->blocksizejslot = j * slotsize;
	    fecpara->blocksizeceilslot = ceilslotnum * slotsize;     
	    fecpara->blocksizefloorslot = floorslotnum * slotsize;

             para->blk_per_frame = k + 1;
        }
    }
   
    burstpara->coding_rate = 2;
    fecpara->bits_slotsymbol = fecpara->blockjslot* fecpara->blocksizejslot + fecpara->blocksizeceilslot + fecpara->blocksizefloorslot;
    //this values will be updated in the fec functions
    fecpara->burst_len = fecpara->bits_slotsymbol; 
    fecpara->burst_len_encoded = fecpara->bits_slotsymbol * burstpara->coding_rate; //1/2 rate as default
    fecpara->burst_len_punctured = fecpara->burst_len_encoded; //should be changed for other coding rate, by WQ
//    fecpara->burst_len_repeated = fecpara->burst_len_punctured * burstpara-> repetition_coding_indication;
    fecpara->decoder_type = 2;    //0 hard decision, 1 soft decision, 2 csi
    fecpara->slots_num = burstpara->slots_num;
    fecpara->repetition_code = burstpara-> repetition_coding_indication;

    fecpara->blkerr_num = 0;

    //Let Ncpc be the number of coded bits per subcarrier
    //i.e., 2, 4, or 6 for QPSK, 16-QAM or 64-QAM,

    switch (fecpara->code_id)
    {
	case CC_QPSK12:
        case CC_QPSK34:
             fecpara->ncpc = 2;
             fecpara->burst_len_modulated = fecpara->slots_num * 48 * 2;
             break;
	case CC_QAM16_12:
	case CC_QAM16_34:
	    fecpara->ncpc = 4;
            fecpara->burst_len_modulated = fecpara->slots_num * 48 * 4;
	    break;
	case CC_QAM64_12:
	case CC_QAM64_23:
	case CC_QAM64_34:
	    fecpara->ncpc = 6;
            fecpara->burst_len_modulated = fecpara->slots_num * 48 * 6;
	    break;
    }

    fecpara->burst_len_repeated = fecpara->burst_len_modulated * burstpara-> repetition_coding_indication;

    return SUCCESS_CODE; 
}

/*----------------------------------------------------------------------------
   Function:  phy_ofdma_ul_fec_decoding
   Description: 
   Parameters:
                Input-  [struct phy_ul_rx_syspara para]  The system parameters that 
                        include the slot length and numbers for each FEC block

                Output- 
                       
   Return Value:
                0       Success
                150     Error
   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                                 */


int32_t phy_ul_fec_decoding(struct phy_ul_rx_syspara *para,
                            struct union_burst_ie *burst,
                            float *input)                          
							 
{

    float a_bit_in_byte[144*32*15]__attribute__((aligned(16)));
    float b_bit_in_byte[144*32*15]__attribute__((aligned(16)));
    unsigned char c_bit_in_byte[144*32*15]__attribute__((aligned(16)));
    unsigned char rand_seed[]__attribute__((aligned(16))) = {0,1,1,0,1,1,1,0,0,0,1,0,1,0,1}; 


    float *derepetition_out, *deinter_out, *depuncture_out; 
    unsigned char *clip_out, *viterbi_out, *derandom_out;
    int err_code;
    struct union_burst_ie *currentburst;	

    currentburst = burst;
    struct phy_ul_fec_para *fecpara;
    fecpara = (struct phy_ul_fec_para *)&para->fec_para;

PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_OTHER ));
    
  //FLOG_DEBUG("currentburst->slots_num= %d\n", currentburst->slots_num);
		
    //get the fec parameters from the burst parameters   
    fecpara->decoder_type = para->coding_type; 
    set_fec_blocksize(para, currentburst, fecpara);
#ifdef DUMP_PHY_UL_FEC
    FLOG_DEBUG("print fecpara information!\n");
    FLOG_DEBUG("fecpara->burst_len= %d\n", fecpara->burst_len);
    FLOG_DEBUG("fecpara->code_id= %d\n", fecpara->code_id);
    FLOG_DEBUG("fecpara->burst_len_encoded= %d\n", fecpara->burst_len_encoded);
    FLOG_DEBUG("fecpara->slots_num= %d\n", fecpara->slots_num);
    FLOG_DEBUG("fecpara->blockjslot= %d\n", fecpara->blockjslot);
    FLOG_DEBUG("fecpara->blocksizejslot= %d\n", fecpara->blocksizejslot);
    FLOG_DEBUG("fecpara->blocksizeceilslot= %d\n", fecpara->blocksizeceilslot);
    FLOG_DEBUG("fecpara->repetition_code= %d\n", fecpara->repetition_code);
    FLOG_DEBUG("fecpara->blocksizefloorslot= %d\n", fecpara->blocksizefloorslot);
    FLOG_DEBUG("fecpara->burst_len_repeated= %d\n", fecpara->burst_len_repeated);
    FLOG_DEBUG("******************************************************\n");
#endif    
    /* derepetition */
    derepetition_out = a_bit_in_byte;
    err_code = phy_ul_derepetition(fecpara, input, &derepetition_out);
    if (err_code) 
    {
        FLOG_ERROR("E008_rx: Error in derepetition!\n");
        return err_code;
    }

	DO_DUMP(DUMP_PHY_FEC_DEREPETITION_ID, 0, derepetition_out, fecpara->burst_len_repeated);

    //FLOG_INFO("derepetition_finish!\n");

PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_OTHER ));
    /* deinterleaver */
PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DEINTER ));
    deinter_out = b_bit_in_byte;
    err_code = phy_ul_deinterleaver(fecpara, derepetition_out, deinter_out);
    if (err_code) 
    {
        FLOG_ERROR("E008_rx: Error in deinterleave!\n");
        return err_code;
    }


PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DEINTER ));
PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_OTHER ));
   DO_DUMP(DUMP_PHY_FEC_DEINTERLEAVER_ID, 0, deinter_out, fecpara->burst_len_repeated);

       
   // FLOG_INFO("deinterleaver_finish!\n");

    /* depuncture */
    depuncture_out = a_bit_in_byte;
    err_code = phy_ul_depuncture(fecpara, deinter_out, &depuncture_out);  
    if (err_code) {
        FLOG_ERROR("E008_rx: Error in depuncture!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_FEC_DEPUNCTURE_ID, 0, depuncture_out, fecpara->burst_len_encoded);
        
    //FLOG_INFO("depuncture_finish!\n");

PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_OTHER ));
PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_CLIP ));
#ifdef _SOFTBIT_ONE_
    float threshold1;
    clip_out = c_bit_in_byte;
    threshold1 = getmax(depuncture_out, fecpara->burst_len_encoded);
    // FLOG_DEBUG("threshold %f\n", threshold1);
    clip(depuncture_out, clip_out, fecpara->burst_len_encoded, threshold1);
#endif

#ifdef _SOFTBIT_TWO_
    float threshold1;
    clip_out = c_bit_in_byte;
    switch (fecpara->code_id)
    {
       case CC_QPSK12:
            para->softbit_threshold = 0.15;
            break;
       case CC_QPSK34:
            para->softbit_threshold = 0.05;
            break;
       case CC_QAM16_12:
            para->softbit_threshold = 0.15;
            break;
       case CC_QAM16_34:
            para->softbit_threshold = 0.075;
            break;
       case CC_QAM64_12:
            para->softbit_threshold = 0.15;
            break;
       case CC_QAM64_23:
            para->softbit_threshold = 0.09;
            break;
       case CC_QAM64_34:
            para->softbit_threshold = 0.05;
            break;
     }

    threshold1 = getmax(depuncture_out, fecpara->burst_len_encoded) * para->softbit_threshold;
  //  FLOG_DEBUG("threshold %f\n", threshold1);
    clip(depuncture_out, clip_out, fecpara->burst_len_encoded, threshold1);
#endif

/*
#ifdef _SOFTBIT_THREE_
    u_int8_t softbit_num = 3;
    u_int8_t threshold3;
    clip_out = c_bit_in_byte;
    threshold3 = pow(2, softbit_num);
   // FLOG_DEBUG("threshold %f\n", threshold1);
    clip_alg3(depuncture_out, clip_out, fecpara->burst_len_encoded, threshold3, para->softbit_shift);
#endif
*/

 
   DO_DUMP(DUMP_PHY_FEC_CLIP_ID, 0, clip_out, fecpara->burst_len_encoded);

    FLOG_INFO("clip_finish!\n");
    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_CLIP ));
    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_VIT ));

   //viterbi_out = (unsigned char*) c_bit_in_byte;
    viterbi_out = c_bit_in_byte;
    err_code = phy_ul_decoder_cctailbiting(fecpara, clip_out, viterbi_out);
    if (err_code) 
    {
        FLOG_ERROR("E008_rx: Error in viterbi decoder!\n");
        return err_code;
    }
	

   DO_DUMP(DUMP_PHY_FEC_DECODER_ID, 0, viterbi_out, fecpara->burst_len);

    FLOG_INFO("viterbi_finish!\n");
    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_VIT ));

    /* derandomizer */
    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DERAN ));
    derandom_out = currentburst->p_buf_out;
    err_code = phy_ul_derandomizer(fecpara, rand_seed, viterbi_out, derandom_out);
    if (err_code) 
    {
        FLOG_ERROR("E008_rx: Error in derandomizer!\n");
        return err_code;
    }
    
//  FLOG_DEBUG("fecpara->blkerr_num = %d\n", fecpara->blkerr_num);
    para->blkerr_num += fecpara->blkerr_num;
    DO_DUMP(DUMP_PHY_FEC_DERAND_ID, 0, derandom_out, fecpara->burst_len);       
    DBG(printf("derandom_finish!\n"));
    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DERAN ));
    return SUCCESS_CODE;
}

/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_UL_FEC

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif


