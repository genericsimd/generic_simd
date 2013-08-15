/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_netframe_transform_stream.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include "adapter_netframe_transform_stream.h"
int netframe_transform_stream(struct net_frame *p_net_frame,void *pbuffer,int *p_size)
{
    
    //u_int8_t   *pbuffer = (u_int8_t*)malloc(sizeof(u_int8_t) * DL_NETFRAME_BUFFER_SIZE);

    int  icopypos = DL_DATASTARTPOS;
    struct net_node *p_currentframe = p_net_frame->p_net_node_data;
    unsigned char  *pdata = pbuffer;

    memcpy(pdata+DL_DATASTARTPOS,&p_net_frame->frame_num,sizeof(p_net_frame->frame_num));
    icopypos += sizeof(p_net_frame->frame_num);
    memcpy(pdata+icopypos,&p_net_frame->operator_version,sizeof(p_net_frame->operator_version));
    icopypos += sizeof(p_net_frame->operator_version);
    while(p_currentframe != NULL)
    {
        
        memcpy(pdata+icopypos,&p_currentframe->ibytelength,sizeof(p_currentframe->ibytelength));
        icopypos += sizeof(p_currentframe->ibytelength);

        memcpy(pdata+icopypos,&p_currentframe->ibitlength,sizeof(p_currentframe->ibitlength));
        icopypos += sizeof(p_currentframe->ibitlength);

        memcpy(pdata+icopypos,&p_currentframe->ibitoffset,sizeof(p_currentframe->ibitoffset));
        icopypos += sizeof(p_currentframe->ibitoffset);

        memcpy(pdata+icopypos,p_currentframe->p_payload,p_currentframe->ibytelength);
        icopypos += p_currentframe->ibytelength;
        p_currentframe = p_currentframe->next;
    }
    memcpy(pdata,&icopypos,sizeof(icopypos));
    *p_size = icopypos;
    return 0;
}
