/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_transform_mac.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_bs_ul_transform_mac.h"
#include "mac_shift_bits.h"
#include "mac.h"

int adapter_transform_physical_frame(struct  ul_frame_ie *p_frame, physical_subframe **phy_frame, void ** pr_result)
{
    struct union_burst_ie  *p_union_iter;
    phy_burst *p_burst_cur = NULL, *p_burst_iter = NULL;
    *phy_frame = (physical_subframe*) malloc(sizeof(physical_subframe));
    memset(*phy_frame,0,sizeof(physical_subframe));
    (*phy_frame)->frame_num = p_frame->frame_num;
    if (p_frame->pscan_resflag == 1)
    {
        (*phy_frame)->interference_info = p_frame->dts_info;

        (*pr_result) = p_frame->p_total_buffer_out;

        return 0;
    }

    p_union_iter = p_frame->p_burst_header;
    while(p_union_iter!= NULL)
    {
        p_burst_cur = (phy_burst *)malloc(sizeof(phy_burst));
        memset(p_burst_cur,0,sizeof(phy_burst));
        p_burst_cur->length = p_union_iter->buf_out_len / 8 + 1;
        p_burst_cur->burst_payload = malloc(sizeof(char ) * p_burst_cur->length);
        p_burst_cur->cid = p_union_iter->cid;
#ifdef AMC_ENABLE
	p_burst_cur->cinr = p_union_iter->cinr;
#endif

        p_burst_cur->burst_len = p_union_iter->buf_out_len/8;
        bits_to_byte(p_union_iter->p_buf_out,p_burst_cur->length * 8,p_burst_cur->burst_payload,0,0);
        if(p_burst_iter == NULL)
        {
            (*phy_frame)->burst_header = p_burst_cur;
        }
        else
            p_burst_iter->next = p_burst_cur;
        p_burst_iter = p_burst_cur;
             
        p_union_iter = p_union_iter->next;
    }

    (*pr_result) = NULL;
    (*phy_frame)->interference_info = NULL;

    return 0;
}


