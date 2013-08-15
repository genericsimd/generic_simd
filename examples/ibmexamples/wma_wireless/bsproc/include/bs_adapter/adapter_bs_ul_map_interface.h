/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_map_interface.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef ADAPTER_BS_UL_MAP_INTERFACE_H_
#define ADAPTER_BS_UL_MAP_INTERFACE_H_

//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////MAC->PHY Interface /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////
//UL_BS 
//PHY -> MAC Interface for PHY Receiver output
////////////////////////////////////////////
#include <stdint.h>
#include "mac.h"
#include "adapter_config.h"

struct block_data_ie
{
    u_int32_t      uiuc;
    u_int16_t      *cid;
    u_int8_t       n_cid;
    u_int8_t       code_id;
    u_int8_t       ofdma_symbol_offset;
    u_int8_t       subchannel_offset;
    u_int8_t       ofdma_symbol_num;	
    u_int8_t       subchannel_num;
    u_int8_t       repetition_coding_indication;
    u_int8_t       coding_type;          //0 CC 1 CTC 
    u_int8_t       mimo_mode;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
    int8_t         burst_id;          //this id is for identify burst from one with another
    u_int8_t       is_used;
    u_int16_t      len;               //for data length for bit_in_byte
    u_int8_t       *p_data_buffer;
    u_int32_t       data_offset;
    u_int32_t       pilot_offset;
    struct block_data_ie  *next;
};
struct slot_symbol_ie
{
    u_int8_t       slot_offset;
    u_int8_t       slot_number;
    struct slot_symbol_ie *next;
    struct block_data_ie  *p_block_header;
};

struct union_burst_ie
{
    u_int32_t       data_offset;
    u_int32_t       pilot_offset;
    u_int32_t       buf_in_len;
    u_int32_t       buf_out_len;
    u_int8_t        burst_id;
    u_int8_t        code_id;
    u_int32_t       uiuc;  	
    u_int32_t       slots_num;
    u_int32_t       repetition_coding_indication;
    u_int32_t       coding_type;          //0 CC 1 CTC 
    u_int32_t       mimo_mode;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
    u_int32_t       coding_rate;
    u_int32_t       cid;
    int             cinr;
    u_int32_t       burst_len;
    void            *p_buf_out;
    struct          union_burst_ie   *next;
};

struct  ul_frame_ie
{
    struct  slot_symbol_ie  *p_slot_header;
    struct  union_burst_ie  *p_burst_header;
    void                    *p_total_buffer_out;
    int                     pscan_resflag;
    int                     frame_num;
    void * dts_info;
};

int parse_ul_map(u_int8_t *p_buf,int len,struct  ul_frame_ie **pp_frame);
//int free_adapter_frame(struct  ul_frame_ie **pp_frame);
int free_adapter_frame(void **p);
#endif
