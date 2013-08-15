/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_dl_interface_data.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-Feb 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <string.h>
#include "adapter_bs_dl_interface_data.h"
#include "mac_interface_queue.h"
#include "queue_obj.h"

/*
 *only free the pointer to physical_frame data
 *because the pointer didn't allocate any data to it so don't free memory
 */

int initialize_ofdma_map(struct ofdma_map* p_ofdma_frame)
{
    int lopi = 0; 
    int lopj = 0;
    for(; lopi < MAX_SUBCHANNEL_NUM; lopi++)
    {
        lopj = 0;
        for(;lopj < MAX_SUBFRAME_SYMBOL_NUM; lopj++)
        {
            p_ofdma_frame->ele[lopi][lopj].payload= NULL;
            p_ofdma_frame->ele[lopi][lopj].len_in_bit = 0;
	    p_ofdma_frame->ele[lopi][lopj].offset_for_uchar = 0;
	    p_ofdma_frame->ele[lopi][lopj].count_in_bytes = 0;
	    p_ofdma_frame->ele[lopi][lopj].mc_type = -1;
	    p_ofdma_frame->ele[lopi][lopj].block_id = 0;
        }
    }
    return 0;
}

int free_ofdma_data(struct ofdma_map *p_ofdma_frame)
{
    int i = 0, j = 0;
    for(; i < p_ofdma_frame->subchannel_num; i++)
    {
        for(; j < p_ofdma_frame->slot_num;j++)
        {
	    
            p_ofdma_frame->ele[i][j].payload = NULL;
            p_ofdma_frame->ele[i][j].count_in_bytes = 0;
        }
    }
    return 0;
}


int adapter_malloc_physlotnode(struct phy_dl_slot** pp_slot)
{
    *pp_slot = (struct phy_dl_slot*)malloc(sizeof(struct phy_dl_slot));
    
    if(*pp_slot == NULL)
        return -1;
    memset(*pp_slot,0,sizeof(struct phy_dl_slot));
    (*pp_slot)->next = NULL;
    (*pp_slot)->payload = NULL;
    return 0;
}
int adapter_release_physlotnode(struct phy_dl_slot **pp_slotnode)
{
    if((*pp_slotnode) == NULL)
        return -1;
    free((*pp_slotnode)->payload);
    free((*pp_slotnode));
    *pp_slotnode = NULL;

    return 0;;
}

int adapter_malloc_physlotsymbol(struct phy_dl_slotsymbol **pp_slotsymbol)
{
  *pp_slotsymbol = (struct phy_dl_slotsymbol*)malloc(sizeof(struct phy_dl_slotsymbol));
  if(*pp_slotsymbol == NULL)
    return -1;
  memset(*pp_slotsymbol,0,sizeof(*pp_slotsymbol));
  (*pp_slotsymbol)->slot_header = NULL;
  return 0;
    
}


int  adapter_dl_deinit_physlotsymbol(struct phy_dl_slotsymbol **pphy)
{   
    if((*pphy) == NULL)
        return -1;

    struct phy_dl_slot *pphynode = NULL,*pprephynode = NULL;

    if((*pphy)->slot_header == NULL)
    {
        free(*pphy);
        *pphy = NULL;

        return 0;
    }
    pphynode = (*pphy)->slot_header;
    
    free((*pphy)->p_payload_buf);

    pphynode->payload = NULL;


    while(pphynode != NULL)
    {

        pprephynode = pphynode;

        pphynode = pphynode->next;

        free(pprephynode);
    }

    if ((*pphy)->dl_subframe_end_flag == 1)
    {
        if ((*pphy)->p_dts_info != NULL)
        {
            free((*pphy)->p_dts_info);
        }
    }

    free(*pphy);
    *pphy = NULL;
   return 0;
}

struct phy_dl_slotsymbol *  adapter_dl_init_physlotsymbol(const char *const p_queue_name)
{
    if(p_queue_name == NULL)
        return NULL;
    struct queue_obj *obj = NULL;
    struct phy_dl_slotsymbol *p_phy_slotsymbol;
    int iresult = dequeueobj((char *)p_queue_name,&obj);
    if(iresult == 0 )
    {
        p_phy_slotsymbol = obj->buf;
        obj->buf = NULL;
        free(obj);
        return p_phy_slotsymbol;
    }
    return NULL;
}
