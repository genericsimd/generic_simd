/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: init_maps.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "init_maps.h"
#include "memmgmt.h"
#include "debug.h"

int init_pdu_map(logical_pdu_map* pm)
{
  if (pm == NULL)
  {
    FLOG_FATAL("Error allocating memory for PDU map.\n");
    return -1;
  }
  pm->cid = 0;
  pm->sdu_num = 0;
  pm->transport_sdu_map = NULL;
  pm->mac_msg_map = NULL;
  pm->arq_sdu_map = NULL;  
  pm->next = NULL;
  return 0;
}

int init_logical_packet(logical_packet* lp)
{
  lp->cid = BROADCAST_CID;
  lp->length = 0;
  lp->element_head = NULL;
  lp->prev = NULL;
  lp->next = NULL;
  return 0;
}

int init_burst_map(logical_burst_map* lbm)
{
  lbm->map_burst_index = 0;
  lbm->pdu_num = 0;
  lbm->burst_bytes_num = 0; // added for the ease of padding
  lbm->pdu_map_header = NULL;
  lbm->next = NULL;
  return 0;
}

int dl_subframe_map_init(logical_dl_subframe_map* frame_map)
{

  frame_map->num_bursts=0;

#ifndef SS_TX //need initialize FCH, ULMAP, DLMAP etc only when BS is transmitting
  // Allocate  mandatory portions of the frame and initialize them
  frame_map->fch=(dl_subframe_prefix*)mac_malloc(sizeof(dl_subframe_prefix));

  if(!frame_map->fch)
    {
      FLOG_FATAL("Error allocating memory for frame_map->fch");
      return -1;
    }
  else
    {
      FLOG_DEBUG("Allocated memory for FCH in frame_map.");
    }

  frame_map->dl_map=(dl_map_msg*)mac_malloc(sizeof(dl_map_msg));
  if(!frame_map->dl_map)
    {
      FLOG_FATAL("Error allocating memory for DLMAP in frame_map");
      return -1;
    }
  else
    {
      FLOG_DEBUG("Allocated memory for DLMAP in frame_map.");
    }
  init_dlmap(frame_map->dl_map);

  frame_map->ul_map=(ul_map_msg*)mac_malloc(sizeof(ul_map_msg));
  if(!frame_map->ul_map)
    {
      FLOG_FATAL("Error allocating memory for ULMAP in frame_map");
      return -1;
    }
  else
    {
      FLOG_DEBUG("Allocated memory for ULMAP in frame_map.");
    }
  init_ulmap(frame_map->ul_map);
#else
  frame_map->fch = NULL;
  frame_map->ul_map = NULL;
  frame_map->dl_map = NULL;
#endif

  frame_map->burst_header=(logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
  if(!frame_map->burst_header)
    {
      FLOG_FATAL("Error allocating memory for burst_header in frame_map");
      return -1;
    }
  else
    {
      FLOG_DEBUG("Allocated memory for burst_header in frame_map.");
    }
  init_burst_map(frame_map->burst_header);

  // UCD and DCD messages aren't sent every frame. Set to NULL here,
  // will be allocated only if it has to go in the current frame
  frame_map->dcd=NULL;
  frame_map->ucd=NULL;

  return 0;
}

int init_dlmap(dl_map_msg *dl_map)
{
  // This function initializes the UL Map message 
  dl_map->manage_msg_type=2;

  dl_map->frame_duration_code = FRAME_DURATION_CODE;
  dl_map->frame_number=0;

  // This count will be generated and maintained by the DCD builder module
  dl_map->dcd_count=0;
  dl_map->bs_id=0;

  // not clear which OFDMA symbols: DL/UL/total. Clarify with CRL 
  dl_map->ofdma_symbols_num = 0;
  dl_map->ie_head=NULL;

  return 0;
}

int init_ulmap(ul_map_msg *ul_map)
{
  // This function initializes the UL Map message 
  ul_map->manage_msg_type=3;

  // Uplink Channel ID. Not used currently
  ul_map->rsv=0;

  // This count will be generated and maintained by the UCD builder module
  ul_map->ucd_count=0;

  // Unit is number of Physical Slots(PS) since the start of DL frame. 
  // For OFDMA one PS=4/Fs (Sec 10.3.4.2)
  // Min value of allocation start time is 10 OFDMA symbols
  // Ref: Sec 10.3.4.1 of standard for OFDMA
  ul_map->alloc_start_time=ALLOCATION_START_TIME;

  ul_map->ulmap_ie_num=0;
  ul_map->ie=NULL;

  return 0;
}

int init_ulmap_ie(ul_map_ie *ie)
{
  ie->ie_index = 0;
  ie->cid = 0;
  ie->uiuc = 0;
  ie->uiuc_extend_ie = NULL;
  ie->uiuc_12_ie = NULL;
  ie->uiuc_13_ie = NULL;
  ie->uiuc_14_ie = NULL;
  ie->uiuc_15_ie = NULL;
  ie->uiuc_0_ie = NULL;
  ie->uiuc_other_ie = NULL;
  ie->next = NULL; 
  return 0;
}
