/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_net_transform_frame.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 25-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_net_transform_frame.h"

int net_transform_frame(struct net_frame *p_netframe, physical_subframe **p_phyframe)
{
    
    struct net_node *p_net_iter = NULL;
    phy_burst *p_dataunit = NULL,*p_frame_iter = NULL;
    (*p_phyframe)->frame_num = p_netframe->frame_num;
    p_net_iter = p_netframe->p_net_node_data;
    while(p_net_iter != NULL)
    {
        p_dataunit = malloc(sizeof(phy_burst));	    
        p_dataunit->burst_payload = malloc(sizeof(unsigned char ) * p_net_iter->ibytelength);
        memcpy(p_dataunit->burst_payload, p_net_iter->p_payload,sizeof(unsigned char ) * p_net_iter->ibytelength);
	p_dataunit->length = p_net_iter->ibytelength;
	if((*p_phyframe)->burst_header == NULL)
	{
            (*p_phyframe)->burst_header = p_dataunit;
        }
        if(p_frame_iter != NULL)
            p_frame_iter->next = p_dataunit;
        p_frame_iter = p_dataunit;
        p_net_iter = p_net_iter->next;
    }
    return 0;
}
