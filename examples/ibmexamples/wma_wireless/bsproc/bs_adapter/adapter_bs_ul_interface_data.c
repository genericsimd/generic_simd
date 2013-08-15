/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_interface_data.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include "adapter_bs_ul_interface_data.h"
#include "flog.h"
struct ul_slot_node * get_ul_slot_node()
{
    struct ul_slot_node *p_slot = (struct ul_slot_node*)malloc(sizeof(struct ul_slot_node));
    if(p_slot == NULL)
    {
        FLOG_ERROR("allocate data have failed\n");
        return NULL;
    }
    memset(p_slot,0,sizeof(struct ul_slot_node));
    return p_slot;

}
int    free_ul_slot_link(struct ul_slot_node *p_slot_node)
{
    if(p_slot_node == NULL)
        return -1;
    struct ul_slot_node *p_iternode = p_slot_node,*p_record_node = NULL;
    while(p_iternode != NULL)
    {
         p_record_node = p_iternode->next;
         free(p_iternode->p_payload);
         free(p_iternode);
         p_iternode = p_record_node;
    }
    p_slot_node = NULL;
    return 0;
}


struct ul_net_frame*  get_new_ul_net_frame()
{
    struct ul_net_frame *p_ul_net_frame = (struct ul_net_frame *) malloc(sizeof(struct ul_net_frame));
    if(p_ul_net_frame == NULL)
        return NULL;
    else
    {
      p_ul_net_frame->p_net_node_data = NULL;
    }
    return p_ul_net_frame;
}
int free_ul_net_frame(struct ul_net_frame *p_ul_net_frame)
{
    struct net_node* p_current_node= NULL,*p_next_node = NULL;
    if(p_ul_net_frame == NULL)
      return -1;
    
    p_current_node = p_ul_net_frame->p_net_node_data;
    while(p_current_node != NULL)
      {
	p_next_node = p_current_node->next;
	free(p_current_node);
	p_current_node = p_next_node;
      }
    free(p_ul_net_frame);
    return 0;
}
