/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_stream_transform_netframe.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include "adapter_stream_transform_netframe.h"
int stream_transform_netframe(void *p_buffer,int *pflag,struct net_frame **p_burstframe)
{
  // find 
    int ipacksize = 0;
    
    memcpy(&ipacksize,p_buffer,DL_DATASTARTPOS);
    int  icopypos = DL_DATASTARTPOS;
    struct net_frame *p_netframe = get_new_net_frame();
    struct net_node    *p_net_node = NULL,*p_recordnode = NULL;
    //struct ul_slot_node *p_burstframe = (struct ul_slot_node*)malloc(sizeof(struct ul_slot_node));
    
    memcpy(&p_netframe->frame_num,p_buffer+icopypos,sizeof(p_netframe->frame_num));
    icopypos += sizeof(p_netframe->frame_num);

    memcpy(&p_netframe->operator_version,p_buffer+icopypos,sizeof(p_netframe->operator_version));
    icopypos += sizeof(p_netframe->operator_version);
    while(icopypos < ipacksize)
    {
        p_net_node = (struct net_node*)malloc(sizeof(struct net_node));;
        
	if(p_net_node == NULL)
	    return -1;
        memset(p_net_node,0,sizeof(struct net_node));
        memcpy(&p_net_node->ibytelength,p_buffer+icopypos,sizeof(p_net_node->ibytelength));
        icopypos += sizeof(p_net_node->ibytelength);
        memcpy(&p_net_node->ibitlength,p_buffer+icopypos,sizeof(p_net_node->ibitlength));
        icopypos += sizeof(p_net_node->ibitlength);
        memcpy(&p_net_node->ibitoffset,p_buffer+icopypos,sizeof(p_net_node->ibitoffset));
        icopypos += sizeof(p_net_node->ibitoffset);
        p_net_node->p_payload = (u_int8_t*)malloc(sizeof(u_int8_t) * p_net_node->ibytelength);
        memcpy(p_net_node->p_payload,p_buffer+icopypos,p_net_node->ibytelength);
        icopypos += p_net_node->ibytelength;
        if(p_netframe->p_net_node_data == NULL)
	  {
	    p_netframe->p_net_node_data = p_net_node;
	    p_recordnode = p_net_node;
	    continue;
	  }
	  p_recordnode->next = p_net_node;
	  p_recordnode = p_net_node;

    }
   
    (*p_burstframe) = p_netframe;
    *pflag = 0; return 0;
}
