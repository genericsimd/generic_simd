/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_net_frame.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include "adapter_net_frame.h"
struct net_frame * get_new_net_frame()
{
    struct net_frame *p_net = malloc(sizeof(struct net_frame));
    if(p_net != NULL)
    {
        memset(p_net,0,sizeof(struct net_frame));
        return p_net;
    }
    return NULL;
}

int free_net_frame(struct net_frame *p_netframe)
{
    struct net_node *p_node_iter = NULL,*p_next = NULL;
    p_node_iter = p_netframe->p_net_node_data;
    while(p_node_iter != NULL)
    {
        p_next = p_node_iter->next;
        free(p_node_iter->p_payload);
        free(p_node_iter);
        p_node_iter = p_next;
    }
    return 0; 
}
