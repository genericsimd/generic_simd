
/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_map_test.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 25-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_bs_ul_map_test.h"
 
int adapter_print_frame_map(struct  ul_frame_ie *p_frame_ie)
{
    struct block_data_ie *p_block_ie = NULL;
    struct slot_symbol_ie  *p_slot_ie = p_frame_ie->p_slot_header;
    struct union_burst_ie *p_burst_ie = p_frame_ie->p_burst_header;
    printf("total buffer %p\n",p_frame_ie->p_total_buffer_out);
    while(p_slot_ie != NULL)
    {
        p_block_ie = p_slot_ie->p_block_header;
        printf("/*************************/\n");
        printf("slot_offset %d slot_num %d\n",p_slot_ie->slot_offset,p_slot_ie->slot_number);
        while(p_block_ie != NULL)
        {
            printf("uiuc = %d, n_cid = %d\n",p_block_ie->uiuc,p_block_ie->n_cid);
            printf("sym_off = %d, sub_off = %d, sym_num = %d, sub_num = %d\n",p_block_ie->ofdma_symbol_offset,p_block_ie->subchannel_offset,p_block_ie->ofdma_symbol_num,    p_block_ie->subchannel_num);
            printf("code_ind = %d,cod_type = %d,code_id = %d,burst_id = %d,is_used = %d\n",p_block_ie->repetition_coding_indication,p_block_ie->coding_type,p_block_ie->code_id,p_block_ie->burst_id,p_block_ie->is_used);
            p_block_ie = p_block_ie->next;
        }
        p_slot_ie = p_slot_ie->next;
    }
    while(p_burst_ie != NULL)
    {
        printf("code_id  %d burst pointer %p,burst len %d\n",p_burst_ie->code_id,p_burst_ie->p_buf_out,p_burst_ie->buf_out_len);
        p_burst_ie = p_burst_ie->next;
    }
   
    return 0;
}
