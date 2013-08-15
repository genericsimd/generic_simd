/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_frame_transform_net.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_phy_data.h"
#include "mac_shift_bits.h"
#include "mac_assistfunc.h"
#include "adapter_net_frame.h"
#include "adapter_frame_transform_net.h"

int transform_phyframe_netframe(physical_subframe *p_phy_frame,struct net_frame **p_netframe)
{
    *p_netframe = get_new_net_frame();
    struct net_node *p_net_node = NULL,*p_net_iter = NULL;
    phy_burst *p_dataunit = p_phy_frame->burst_header;
    (*p_netframe)->frame_num = p_phy_frame->frame_num;
    
    while(p_dataunit != NULL)
    {
        p_net_node = malloc(sizeof(struct net_node));
        p_net_node->p_payload = malloc(sizeof(unsigned char ) * p_dataunit->length);
        memcpy(p_net_node->p_payload,p_dataunit->burst_payload,p_dataunit->length * sizeof(unsigned char));
	p_net_node->ibytelength = p_dataunit->length;
	if((*p_netframe)->p_net_node_data == NULL)
	{
            (*p_netframe)->p_net_node_data = p_net_node;
        }
        if(p_net_iter != NULL)
            p_net_iter->next = p_net_node;
        p_net_iter = p_net_node;	
        p_dataunit = p_dataunit->next;
    }
    return 0;
}

