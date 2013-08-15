/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: free_maps.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "free_maps.h"


// The memory for ULMAP IE is dynamically allocated per frame as needed
// needs to be freed after the frame's lifetime
int free_ulmap(ul_map_msg* ul_map)
{
  ul_map_ie* temp;
  mimo_ul_basic_ie *mubi;
  while(ul_map->ie!=NULL)
    {
      temp=ul_map->ie;
      ul_map->ie=ul_map->ie->next;
      // Need to add similar logic for other special UIUC IE's
      if(temp->uiuc_other_ie != NULL)
	  {
	    mac_free(sizeof(other_uiuc_ie), temp->uiuc_other_ie);
	  }
      if(temp->uiuc_12_ie != NULL)
	{
	  mac_free(sizeof(uiuc12_ie), temp->uiuc_12_ie);
	}      
	  if(temp->uiuc_14_ie != NULL)
	  {
		free(temp->uiuc_14_ie);
	  }
      // Free the Extd UIUC 2 IE, corresponding to UIUC = 11;
      if(temp->uiuc_extend_ie != NULL)
      {
        if(temp->uiuc_extend_ie->unspecified_data != NULL)
        {
            switch(temp->uiuc_extend_ie->extended_uiuc)
            {
                // TODO: Need to add more case blocks for other types of extd UIUC IDs
                case MIMO_UL_BASIC_IE:
                    mubi = temp->uiuc_extend_ie->unspecified_data;
                    while(mubi->assigned_burst_header != NULL)
                    {
                        assigned_burst_attri *aba = mubi->assigned_burst_header;
                        mubi->assigned_burst_header = aba->next;
                        free(aba);
                    }
                    free(mubi);
                    break;
                default:
                    FLOG_ERROR("Can't free extended UIUC IE. Unknown type\n");
            }
        }
        free(temp->uiuc_extend_ie);
      }
      mac_free(sizeof(ul_map_ie), temp);
    }
  return 0;
}

// The memory for DLMAP IE is dynamically allocated per frame as needed
// needs to be freed after the frame's lifetime
int free_dlmap(dl_map_msg* dl_map)
{
  dl_map_ie* temp;
  mimo_dl_basic_ie *mubi;
  while(dl_map->ie_head!=NULL)
    {
      temp=dl_map->ie_head;
      dl_map->ie_head=dl_map->ie_head->next;

      if(temp->normal_ie != NULL)
	{
	  if (temp->normal_ie->cid != NULL)
	    {
	      free(temp->normal_ie->cid);
	    }
	  mac_free(sizeof(normal_diuc_ie), temp->normal_ie);
	}
      if(temp->extended_ie != NULL)
	{
        if(temp->extended_ie->unspecified_data != NULL)
        {
                if (temp->diuc == 15)
                {
            switch(temp->extended_ie->extended_diuc)
            {
                // Need to add more case blocks for other types of extd DIUC IDs
                case STC_ZONE_IE:
                    free(temp->extended_ie->unspecified_data);
                    break;
                        default:
                            FLOG_ERROR("Can't free extended DIUC IE. Unknown type\n");
                    } // end switch
                }
                else if (temp->diuc == 14) // Ext2 DIUC IE
                {
                    switch(temp->extended_ie->extended_diuc)
                    {
                case MIMO_DL_BASIC_IE:
                    mubi = temp->extended_ie->unspecified_data;
                    while (mubi->region_header != NULL)
                    {
                        region_attri *ra = mubi->region_header;
                        mubi->region_header = ra->next;
                        // Need to add similar logic for user_header and ms_header, and maybe RCID IE
                        while(ra->layer_header != NULL)
                        {
                            layer_attri *la = ra->layer_header;
                            ra->layer_header = la->next;
                            free(la);
                        }
                        free(ra);
                    }
                    free(mubi);
                    break;
                default:
                            FLOG_ERROR("Can't free Ext2 DIUC IE, Unknown type.\n");
                            break;
                    }
            }
        }
        
	  free(temp->extended_ie);
	}
      mac_free(sizeof(dl_map_ie), temp);
    }
  return 0;
}

int free_dl_subframe_map(logical_dl_subframe_map* frame_map)
{
  mac_free(sizeof(logical_burst_map), frame_map->burst_header);

  mac_free(sizeof(dl_subframe_prefix), frame_map->fch);
  mac_free(sizeof(logical_dl_subframe_map), frame_map);
  return 0;
}

int free_logical_packet(logical_packet* lp)
{
  logical_packet* temp_lp;
  logical_element* temp_le;
  while (lp != NULL)
    {
      temp_lp = lp;
      lp = lp->next;
      while (temp_lp->element_head != NULL)
	{
	  temp_le = temp_lp->element_head;
	  temp_lp->element_head = temp_le->next;
	  mac_free(sizeof(logical_element), temp_le);
	}
      mac_free(sizeof(logical_packet), temp_lp);
    }
  return 0;
}

int free_logical_burst_map(logical_burst_map* lbm)
{
  logical_burst_map* temp_lbm;
  logical_pdu_map* temp_lpm;
  while (lbm != NULL)
    {
      temp_lbm = lbm;
      lbm=lbm->next;
      while (temp_lbm->pdu_map_header != NULL)
	{
	  temp_lpm = temp_lbm->pdu_map_header;
	  temp_lbm->pdu_map_header = temp_lpm->next;
 	  if (temp_lpm->transport_sdu_map != NULL)
	    {
	      mac_free(sizeof(transport_sdu_map), temp_lpm->transport_sdu_map);
	    }	  
	  if (temp_lpm->mac_msg_map != NULL)
	    {
	      free_logical_packet(temp_lpm->mac_msg_map->mac_mgmt_msg_sdu);
	      mac_free(sizeof(mgmtmsg_sdu_map), temp_lpm->mac_msg_map);
	    }
	  if (temp_lpm->arq_sdu_map != NULL)
	    {
	      free_logical_packet(temp_lpm->arq_sdu_map->arq_retransmit_block);
	      mac_free(sizeof(arq_retrans_sdu_map), temp_lpm->arq_sdu_map);
	    }
	  mac_free(sizeof(logical_pdu_map), temp_lpm);
	}
      mac_free(sizeof(logical_burst_map), temp_lbm);
    }
  return 0;
}

