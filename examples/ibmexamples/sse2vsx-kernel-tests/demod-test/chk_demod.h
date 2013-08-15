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
#include <string.h>

#define ERROR_CODE -1
typedef unsigned int u_int32_t;
typedef unsigned char u_int8_t;

enum
{
    CC_QPSK12    = 0,
    CC_QPSK34    = 1,
    CC_QAM16_12  = 2,
    CC_QAM16_34  = 3,
    CC_QAM64_12  = 4,
    CC_QAM64_23  = 5,
    CC_QAM64_34  = 6,

    CTC_QPSK12   = 7,
    CTC_QPSK34   = 8,
    CTC_QAM16_12 = 9,
    CTC_QAM16_34 = 10,
    CTC_QAM16_56 = 11,
    CTC_QAM64_12 = 12,
    CTC_QAM64_23 = 13,
    CTC_QAM64_34 = 14,
    CTC_QAM64_56 = 15,

    CODEIDRESERVED = 16
}enum_code_id;


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
    u_int32_t       burst_len;
    void            *p_buf_out;
    struct          union_burst_ie   *next;
};

#endif
