/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_rx.h

   Function:  the declaration for the receiver PHY layer 

   Change Activity:

   Date             Description of Change                   By
   -------------    ---------------------                  ------------
   
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PHY_UL_RX_H_ 
#define _PHY_UL_RX_H_ 

#include "phy_ul_rx_interface.h"
#include "adapter_bs_ul_interface_data.h"

#define MAX_SLOTS_NUM (149)

struct phy_hook_ps_result
{
    float frm_num;
    float active_band[21];
    float dl_unused_subch;
    float ul_unused_subch;
    float power[1024];
    float noise_figure;
    float noise_maxhold;
}__attribute__((packed));

struct phy_hook_const_result
{
    float frm_num;
    float slot_num;
    float code_id;
    float burst_idx;
    float total_burst;
    float buf[MAX_SLOTS_NUM * 48 * 8];
}__attribute__((packed));

struct phy_hook_chan_quality_result
{
    float frm_num;
    float slots_num;
    float buf[MAX_SLOTS_NUM * 4];
}__attribute__((packed));


/* rx processing function */


int32_t phy_ul_deinit_rrusymbol(struct phy_ul_rru_symbol *p_rru_symbol);


int32_t phy_ul_rx_div1(struct phy_ul_rx_syspara *para,
                       const u_int32_t in_que_id,
                       const u_int32_t out_que_id);

int32_t phy_ul_rx_div2(struct phy_ul_rx_syspara *para,
                       const u_int32_t in_que_id1,
                       const u_int32_t in_que_id2,
                       const u_int32_t out_que_id1,
                       const u_int32_t out_que_id2);

int32_t phy_ul_rx_cdd(struct phy_ul_rx_syspara *para,
                      const u_int32_t in_que_id1,
                      const u_int32_t in_que_id2,
                      const u_int32_t out_que_id1);


int32_t phy_ul_rx_stca(struct phy_ul_rx_syspara *para,
                       const u_int32_t in_que_id1,
                       const u_int32_t in_que_id2,
                       const u_int32_t out_que_id1);

int32_t phy_ul_rx_stcb(struct phy_ul_rx_syspara *para,
                       const u_int32_t in_que_id1,
                       const u_int32_t in_que_id2,
                       const u_int32_t out_que_id1,
                       const u_int32_t out_que_id2); 
/*
int32_t parsing_data(struct phy_ul_rx_syspara *para,
                     u_int32_t unit_len,
		     u_int32_t *count,
                     float *input_r,
                     float *input_i,
                     float *output_r,
                     float *output_i);					   
*/

int frm_dump_channel_quality (int flag, FILE * f_p, int len, void * buf);

#endif  /* end of _PHY_UL_RX_H_  */


