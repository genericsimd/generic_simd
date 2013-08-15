/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
   
   
   File Name: phy_dl_fec_encoding.c

   Function:  API functions for the FEC coding

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h> 

#include "phy_dl_fec_encoding.h"
#include "phy_dl_modulation.h"
#include "phy_dl_repetition.h"
#include "phy_dl_interleaver.h"
#include "phy_dl_encoder_cctailbiting.h"
#include "phy_dl_randomizer.h"
#include "phy_dl_puncture.h" 
#include "phy_dl_utility.h"
#include "flog.h"

#include "bs_debug.h"
#include "monitor_proc.h"

/* turn on/off dump according to DUMP_PHY_DL_FEC setting */
#ifndef DUMP_PHY_DL_FEC

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

/*
 Data randomization is performed on all data except the FCH. 
 The randomization is initialized on each FEC block.
 Parameters:
 Ns is the allocated slots number for data burst 30
 Data allocated      30 * 24 * 4 /2  (DL_PUSC)
 R is the repetition factor
 FEC block size = (modulation, coding rate, Ns)
*/


//#define _BS_MONITOR_



int set_fec_para(struct phy_dl_tx_syspara *para, struct phy_dl_fec_para *fecpara, int code_id)
{
    int j,n,k,m;
    int ceilslotnum, floorslotnum;
    int tablej[] = {6, 4, 3, 2, 2 ,1 ,1}; 
    int slotsizetable[] = {6, 9, 12, 18, 18, 24,27};
    int slotsize = 8*slotsizetable[code_id];

    j = tablej[code_id];
    n = (int)floor((double)fecpara->slots_num * para->stc_rate/(fecpara->repetition_code* para->stclayer_num));
    k = (int)floor((double)n / j);  //block number
    m = (int) n % j;
    ceilslotnum = (int) ceil((double)(m+j)/2);
    floorslotnum = (int) floor((double)(m+j)/2);

    if(n<=j)
    {
        fecpara->blockjslot = 1;
        fecpara->blocksizejslot = n * slotsize;
        fecpara->blocksizeceilslot = 0;
        fecpara->blocksizefloorslot = 0;
    }
    else
    {
        if(m ==0)
        {
            fecpara->blockjslot = k;
            fecpara->blocksizejslot = j * slotsize;
            fecpara->blocksizeceilslot = 0;
            fecpara->blocksizefloorslot = 0;
        }
        else
        {
            fecpara->blockjslot = k-1;
            fecpara->blocksizejslot = j * slotsize;
            fecpara->blocksizeceilslot = ceilslotnum * slotsize;
            fecpara->blocksizefloorslot = floorslotnum * slotsize;
        }
     }

    fecpara->bits_slotsymbol = fecpara->blockjslot* fecpara->blocksizejslot + fecpara->blocksizeceilslot + fecpara->blocksizefloorslot;
   
    //this values will be updaed in the fec functions
    fecpara->burst_len = fecpara->bits_slotsymbol;
    fecpara->burst_len_encoded = fecpara->bits_slotsymbol * 2; //1/2 rate as default
    fecpara->burst_len_punctured = fecpara->burst_len_encoded;

    fecpara->burst_len_repeated = fecpara->burst_len_punctured * fecpara->repetition_code;
    //Let Ncpc be the number of coded bits per subcarrier
    //i.e., 2, 4, or 6 for QPSK, 16-QAM or 64-QAM,

    switch (code_id)
    {
        case CC_QPSK12:
        case CC_QPSK34:
            fecpara->ncpc = 2;
            fecpara->fec_len = fecpara->burst_len_repeated/2;
            fecpara->burst_len_modulated = fecpara->burst_len_repeated/2;
            break;
        case CC_QAM16_12:
        case CC_QAM16_34:
            fecpara->ncpc = 4;
            fecpara->fec_len = fecpara->burst_len_repeated/4;
            fecpara->burst_len_modulated = fecpara->burst_len_repeated/4;
            break;
       case CC_QAM64_12:
       case CC_QAM64_23:
       case CC_QAM64_34:
            fecpara->ncpc = 6;
            fecpara->fec_len = fecpara->burst_len_repeated/6;
            fecpara->burst_len_modulated = fecpara->burst_len_repeated/6;
            break;
    }

    return SUCCESS_CODE; 
}

// para->repetition_code = 4;
int phy_dl_fch_fecencoding(struct phy_dl_tx_syspara *para,
                           const struct phy_dl_slot *p_first_slot,
                           const unsigned char num_of_slots,
                           float *fec_encoded_r,
                           float *fec_encoded_i,
                           unsigned int *fec_len)
{
    unsigned char a_bit_in_byte[2*2880]__attribute__((aligned(16)));
    unsigned char b_bit_in_byte[2*2880]__attribute__((aligned(16)));
    unsigned char c_bit_in_byte[2*2880]__attribute__((aligned(16)));
    unsigned char *fch_data, *tailbiting, *punctured_data, *interleaver, *repetition;
    float *mod_data_r, *mod_data_i; 
    int err_code;

    struct phy_dl_fec_para fec_parameters;
    struct phy_dl_fec_para *fecpara;
    fecpara = &fec_parameters;

    //get the fec parameters from the global parameters
    fecpara->slots_num = num_of_slots;
    fecpara->repetition_code = (p_first_slot->repetition_coding_indication)? (p_first_slot->repetition_coding_indication*2): 1;
    fecpara->code_id = p_first_slot->code_id;

    //get the fec block size and parameters from the code_id
    set_fec_para(para, fecpara, fecpara->code_id);

    *fec_len = fecpara->fec_len;

    fch_data = p_first_slot->payload;

	DO_DUMP(DUMP_PHY_TX_FEC9_FCHIN_ID, 0, fch_data, fecpara->burst_len);

#ifdef _BS_MONITOR_
    struct phy_hook_fch_result fch_result;
    fch_result.frm_num = para->frame_index;
    memcpy(fch_result.fch, fch_data, 48);
    hook_debug_trace(HOOK_FCH_IDX, &fch_result, sizeof( struct phy_hook_fch_result), 1);
#ifdef _HOOK_BS_DUMP_
    FILE *fp_hookfch;
    unsigned char fch_loop = 0;
    fp_hookfch = fopen("hook_fch.dat", "wt+");
    for (fch_loop =0; fch_loop<48; fch_loop++)
    {
        fprintf(fp_hookfch, "%d\n", fch_data[fch_loop]);
    }
    fclose(fp_hookfch);
#endif
#endif



    /* CC encoder */
    tailbiting = a_bit_in_byte;
    err_code = phy_dl_encoder_cctailbiting(fecpara,
                                           fch_data,
                                           tailbiting);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fch_fecencoding: Error in cc encoder!\n");
        return err_code;
    }

 DO_DUMP(DUMP_PHY_TX_FEC8_FCH_TAILBITING_ID, 0, tailbiting,\
              fecpara->burst_len_encoded);
 
    /* Puncture */
    punctured_data = b_bit_in_byte;
    err_code = phy_dl_puncture(fecpara,
                               tailbiting,
                               &punctured_data);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fch_fecencoding: Error in puncture!\n");
        exit(1); 
    }

	
	DO_DUMP(DUMP_PHY_TX_FEC7_FCH_PUNCTURE_ID, 0, 	punctured_data,\
					  fecpara->burst_len_punctured);
	

    /* Interleaver */
    interleaver = c_bit_in_byte;
    err_code = phy_dl_interleaver(fecpara,
                                  punctured_data,
                                  interleaver);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fch_fecencoding: Error in interleaver!\n");
        exit(1);
    }


	DO_DUMP(DUMP_PHY_TX_FEC6_FCH_INTERLEAVER_ID, 0, interleaver,\
              fecpara->burst_len_punctured);

    /* Repetition */
    repetition = a_bit_in_byte;
    err_code = phy_dl_repetition(fecpara,
                                 interleaver,
                                 &repetition);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fch_fecencoding: Error in repetition!\n");
        exit(1);
    }


	DO_DUMP(DUMP_PHY_TX_FEC5_FCH_REPETITION_ID, 0, repetition,\
              fecpara->burst_len_repeated);

    /* Modulation */
    mod_data_r = fec_encoded_r;
    mod_data_i = fec_encoded_i;
    err_code = phy_dl_modulation(fecpara,
                                 repetition,
                                 mod_data_r,
                                 mod_data_i);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fch_fecencoding: Error in modulation!\n");
        exit(1);
    }


	DO_DUMP(DUMP_PHY_TX_FEC4_FCH_MOD_I_ID, 0, mod_data_r,\
              fecpara->burst_len_modulated);

	DO_DUMP(DUMP_PHY_TX_FEC4_FCH_MOD_Q_ID, 0, mod_data_i,\
              fecpara->burst_len_modulated);

    return SUCCESS_CODE; 
}


int phy_dl_fec_encoding(struct phy_dl_tx_syspara *para,
                        const struct phy_dl_slot *p_first_slot,
                        const unsigned char num_of_slots,
                        float *fec_encoded_r,
                        float *fec_encoded_i,
                        unsigned int *fec_len)
{
    unsigned char a_bit_in_byte[48*42*5*2]__attribute__((aligned(16)));
    unsigned char b_bit_in_byte[48*42*5*2]__attribute__((aligned(16)));
    unsigned char c_bit_in_byte[48*42*5*2]__attribute__((aligned(16)));
    unsigned char rand_seed[]__attribute__((aligned(16))) = {0,1,1,0,1,1,1,0,0,0,1,0,1,0,1}; 
    unsigned char *raw_data, *randomized_data,  *tailbiting, *interleaver, *repetition, *punctured_data; 
    float *mod_data_r, *mod_data_i; 
    int err_code; 

    struct phy_dl_fec_para fec_parameters;
    struct phy_dl_fec_para *fecpara;
    fecpara = &fec_parameters;

    //get the fec parameters from the global parameters
    fecpara->slots_num = num_of_slots;
    fecpara->repetition_code = (p_first_slot->repetition_coding_indication)? (p_first_slot->repetition_coding_indication*2): 1;
    fecpara->code_id = p_first_slot->code_id;

    //get the fec block size and parameters from the code_id
    set_fec_para(para, fecpara, fecpara->code_id);

    *fec_len = fecpara->fec_len;

    raw_data = p_first_slot->payload;

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              

		DO_DUMP(DUMP_PHY_TX_FEC10_MAC_IN_ID, 0, raw_data,\
                  fecpara->burst_len);

#ifdef _NO_PADDING_
    }
#endif                              


    /* Randomizer */
    randomized_data = c_bit_in_byte; 
    err_code = phy_dl_randomizer(fecpara,
                                 rand_seed,
                                 raw_data,
                                 randomized_data);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in randomizer!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              

DO_DUMP(DUMP_PHY_TX_FEC9_RANDOMIZER_ID, 0, randomized_data,\
                  fecpara->burst_len);

#ifdef _NO_PADDING_
    }
#endif                              

    /* CC encoder */
    tailbiting = a_bit_in_byte;
    err_code = phy_dl_encoder_cctailbiting(fecpara,
                                           randomized_data,
                                           tailbiting);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in cc encoder!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              

DO_DUMP(DUMP_PHY_TX_FEC8_TAILBITING_ID, 0, tailbiting,\
			 fecpara->burst_len_encoded);

#ifdef _NO_PADDING_
    }
#endif                              

    /* puncture */
    punctured_data = b_bit_in_byte;
    err_code = phy_dl_puncture(fecpara,
                               tailbiting,
                               &punctured_data);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in puncture!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              

DO_DUMP(DUMP_PHY_TX_FEC7_PUNCTURE_ID, 0,	punctured_data,\
				  fecpara->burst_len_punctured);


#ifdef _NO_PADDING_
    }
#endif                              

    /* Interleaver */
    interleaver = c_bit_in_byte;
    err_code = phy_dl_interleaver(fecpara,
                                  punctured_data,
                                  interleaver);
    if (err_code)
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in interleaver!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              


DO_DUMP(DUMP_PHY_TX_FEC6_INTERLEAVER_ID, 0, interleaver,\
		  fecpara->burst_len_punctured);


#ifdef _NO_PADDING_
    }
#endif                              

    /* Repetition */
    repetition = a_bit_in_byte;
    err_code = phy_dl_repetition(fecpara,
                                 interleaver,
                                 &repetition);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in repetition!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              


		DO_DUMP(DUMP_PHY_TX_FEC5_REPETITION_ID, 0, repetition,\
		  fecpara->burst_len_repeated);

#ifdef _NO_PADDING_
    }
#endif                              

    /* Modulation */
    mod_data_r = fec_encoded_r;
    mod_data_i = fec_encoded_i;
    err_code = phy_dl_modulation(fecpara,
                                 repetition,
                                 mod_data_r,
                                 mod_data_i);
    if (err_code) 
    {
        FLOG_ERROR("phy_dl_fec_encoding: Error in modulation!\n");
        return err_code;
    }

#ifdef _NO_PADDING_
    if(p_first_slot->block_id != 0) {
#endif                              

		DO_DUMP(DUMP_PHY_TX_FEC4_MOD_I_ID, 0, mod_data_r,\
				  fecpara->burst_len_modulated);
		
		DO_DUMP(DUMP_PHY_TX_FEC4_MOD_Q_ID, 0, mod_data_i,\
				  fecpara->burst_len_modulated);
		
#ifdef _NO_PADDING_
    }
#endif                              

    return SUCCESS_CODE; 
}

/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_DL_FEC

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif


