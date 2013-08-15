/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.   


   File Name: phy_dl_tx.h

   Function: Declare the functions to form DL frame in transmitter.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_DL_TX_H__
#define __PHY_DL_TX_H__

#include "phy_dl_tx_interface.h"
#include "adapter_bs_dl_interface_data.h"

//structure for FEC coding and decoding
struct phy_dl_fec_para
{
    u_int32_t slots_num;             /* slots number */
    u_int32_t bits_slotsymbol;       /* [2880] */
    u_int32_t blockjslot;            /* [10] */
    u_int32_t blocksizejslot;        /* [288] */
    u_int32_t blocksizeceilslot;     /* [0] */
    u_int32_t blocksizefloorslot;    /* [0] */

    u_int32_t burst_len;
    u_int32_t burst_len_encoded;     /* For Convolutional coding it should be 2* bits_slotsymbol, for CTC, it should be 3*bits_slotsymbol [5760] */
    u_int32_t burst_len_punctured;   /* Puncture  length, this is the lenth for the  interleaving [5760] */
    u_int32_t burst_len_repeated;    /* Repeated  length, this is the final lenth for the  modulation and  the following following [5760] */
    u_int32_t burst_len_modulated;   /* modulated length */
    u_int32_t fec_len;                

    u_int32_t repetition_code;       /* Table 320 - RepetitionIndication*2 */
    u_int32_t code_id;               /*modulation, rate */
    u_int32_t ncpc;                  /* The number of coded bits per subcarrier, 2, 4, or 6 for QPSK, 16-QAM or 64-QAM [4] */
};


struct phy_dl_rru_symbol *phy_dl_init_rrusymbol(const struct phy_dl_tx_syspara *const para,
                                                struct phy_dl_slotsymbol *p_slot_symbol);
                                          
int32_t phy_dl_tx_div1(struct phy_dl_tx_syspara *para,
                       const u_int32_t in_que_id,
                       const u_int32_t out_que_id);

int32_t phy_dl_tx_div2(struct phy_dl_tx_syspara *para,
                       const u_int32_t in_que_id1,
                       const u_int32_t in_que_id2,
                       const u_int32_t out_que_id1,
                       const u_int32_t out_que_id2);

int32_t phy_dl_tx_single(struct phy_dl_tx_syspara *para,
                         const struct phy_dl_slotsymbol *p_slotsymbol,
                         struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1);

int32_t phy_dl_tx_cdd (struct phy_dl_tx_syspara *para,
                       const struct phy_dl_slotsymbol *p_slotsymbol,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2);


int32_t phy_dl_tx_stca(struct phy_dl_tx_syspara *para,
                       const struct phy_dl_slotsymbol *p_slotsymbol,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2);

int32_t phy_dl_tx_stcb(struct phy_dl_tx_syspara *para,
                       const struct phy_dl_slotsymbol *p_slotsymbol_1,
                       const struct phy_dl_slotsymbol *p_slotsymbol_2,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                       struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2);

#endif //__PHY_DL_TX_H__
