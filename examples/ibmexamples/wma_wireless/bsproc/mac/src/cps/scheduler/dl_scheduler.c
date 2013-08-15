/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_scheduler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "dl_scheduler.h"
#include "arq_ifaces.h"
#include "arq_defines.h"
#include "debug.h"
#include "mac.h"

#include "sdu_cid_queue.h"

#define MIN_SLOTS_ALLOCATED 10
#define MIN_PACKET_FRACTION 0
extern int param_DL_CBR_PACKET_SIZE;
extern int num_dl_symbols;
extern int num_bytes_in_dlslotsymbol;
static int num_dl_subchannels;

int alloc_dl_scheduler_var()
{
  dl_ss_slots=(int*)malloc((MAX_NUM_SS + 1)*sizeof(int));
  dl_ss_bytes=(int*)malloc(MAX_NUM_SS*sizeof(int));

  // Also allocate an array of pointers, where each element 
  // contains the pointer for a specific SS. This is under the
  // assumption in the current setup of one burst per SS
  lbm_ss = (logical_burst_map**)malloc(MAX_NUM_SS*sizeof(logical_burst_map*));

  if (!lbm_ss)
    {
      FLOG_FATAL("dl_scheduler.c: Error allocating memory for logical burst map array. Exiting");
      return -1;
    }
  else
    {
      FLOG_DEBUG("Allocated memory for logical burst map array");
    }

  cur_pdu_map_for_ss = (logical_pdu_map**)malloc(MAX_NUM_SS*sizeof(logical_pdu_map*));

  return 0;
}

int free_dl_scheduler_var()
{
  free(lbm_ss);
  free(cur_pdu_map_for_ss);
  free(dl_ss_slots);
  free(dl_ss_bytes);
  return 0;
}

int dl_scheduler(sdu_queue* dl_sdu_queue, amc_info* amc_info_header, int num_dl_slots, logical_dl_subframe_map* frame_map, int* dlmap_length,int num_dl_subch)
{
  int is_encrypted;
  int pdu_overhead=0;
  short ii = 0, jj = 0;
  //int num_dlmap_bits = 0;
  int num_available_dl_slots, ss_index = 0;
  //int num_sdu = 0, max_sdu_size = 0, min_sdu_size = 0;
  int extra_dlmap_slots, balance_slots, bytes_available = 0;
  int max_bytes_allowed = 0, max_blocks_allowed = 0, data_bytes_needed = 0, pdu_size = 0, data_slots_available = 0, temp = 0;
  int fair_share, fsh_len = 0, psh_len = 0;
  u_int8_t is_frag;
  u_int8_t is_arq;

  // DL-MAP has at least one DL-MAP IE
  int num_dlmap_ie = 0;
  int num_broadcast_bytes = 0, num_broadcast_slots = 0;
  sdu_cid_queue* sdu_cid_q;
#ifdef FOAK_TEMP_FIX
  // 1st slotsymbol is reserved for the FCH + DLMAP, no other bursts sent there
  num_dl_subchannels = num_dl_subch;
  num_available_dl_slots = floor((num_dl_symbols - DL_SYMBOLS_PER_SLOT)/DL_SYMBOLS_PER_SLOT)*floor(num_dl_subch/DL_SUBCHANNEL_PER_SLOT);
#else
  num_available_dl_slots = num_dl_slots;
#endif
  memset(dl_ss_slots, 0, (MAX_NUM_SS + 1)*sizeof(int));
  memset(dl_ss_bytes, 0, MAX_NUM_SS*sizeof(int));
  int burst_index = 0;

  for(ii = 0; ii < NUM_SS; ii++)
    {
      lbm_ss[ii]=NULL;
      cur_pdu_map_for_ss[ii]=NULL;
    }
  // Burst map for the broadcast mesgs (excluding ULMAP)
  logical_burst_map *cur_lbm = frame_map->burst_header;

  cur_lbm->map_burst_index = burst_index;
  cur_lbm->pdu_num = 0;
  cur_lbm->next = NULL;
  logical_pdu_map* pdu_map = NULL;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;

  FLOG_DEBUG("Frame Number: %ld\n", get_current_frame_number());

  FLOG_DEBUG("Calling DL scheduler");

  // Management messages are high priority.
  // Priority order: broadcast MMM(ULMAP, DLMAP, UCD, DCD etc on BROADCAST_CID)> 
  // MMM on INIT_RNG_CID (also a broadcast CID) > basic CID >
  // primary CID MMM > UGS > BE. For each ARQ enabled transport connection, 
  // its retransmissions are honored before "not-sent" packets"

  // 1st: Broadcast MMM
  // Get the CID queue pointer for the broadcast queue
  sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, BROADCAST_CID);
  if ((sdu_cid_q == NULL) ||(sdu_cid_q->sdu_num==0))
    {
      // Broadcast message queue is empty
      num_broadcast_bytes = ulmap_len_in_bytes;
      FLOG_DEBUG("Broadcast MMM queue is empty");
#ifdef FOAK_TEMP_FIX
	  if (num_broadcast_bytes > num_bytes_in_dlslotsymbol)
	  {
		FLOG_ERROR("ULMAP size exceeds one slotsymbol. Aborting\n");
		assert(0);
	  }
      // The broadcast burst itself will contain the DLMAP and ULMAP, which are 
      // sent separately in the logical_dl_subframe. No other broadcast messages dequeued
      cur_lbm->pdu_map_header = NULL;
#endif
    }
  else
    {
      // All the other broadcast messages can be clubbed in a burst and allocated 
      // Since broadcast messages can't be fragmented or packed, the number of
      // PDUs = number of SDUs, hence the header and CRC overhead is known 
      data_bytes_needed = sdu_cid_q->sdu_cid_aggr_info->overall_bytes;
	  num_broadcast_bytes = ulmap_len_in_bytes + data_bytes_needed + \
		sdu_cid_q->sdu_num * PDU_MIN_OVERHEAD;
#ifdef FOAK_TEMP_FIX
	  if (num_broadcast_bytes > num_bytes_in_dlslotsymbol)
	  {
		// Can't accommodate all the broadcast messages. check how many can fit.
		data_bytes_needed = estimate_data_bytes_from_total(BROADCAST_CID, sdu_cid_q, num_bytes_in_dlslotsymbol - ulmap_len_in_bytes);
		num_broadcast_bytes = num_bytes_in_dlslotsymbol;
	  }
#endif
	  if (data_bytes_needed == 0)
	  {
		cur_lbm->pdu_map_header = NULL;
	  }
	  else
	  {
		pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		init_pdu_map(pdu_map);
		pdu_map->cid = BROADCAST_CID;

		cur_lbm->pdu_map_header = pdu_map;
		cur_lbm->pdu_num++;
      
		pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
		pdu_map->transport_sdu_map->num_bytes =  data_bytes_needed;
		pdu_map->transport_sdu_map->cid = BROADCAST_CID;
		pdu_map->next = NULL;
	  }
    }

#ifdef FOAK_TEMP_FIX
  if (num_broadcast_bytes < num_bytes_in_dlslotsymbol - PDU_MIN_OVERHEAD)
  {
#endif
  // Now account for any MMMs that might be there on the Initial Ranging CID
  sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, INIT_RNG_CID);
  if ((sdu_cid_q != NULL) && (sdu_cid_q->sdu_num != 0))
    {
      // All the ranging broadcast messages can be clubbed in this burst and allocated 
      // Since broadcast messages can't be fragmented or packed, the number of
      // PDUs = number of SDUs, hence the header and CRC overhead is known 
      data_bytes_needed = sdu_cid_q->sdu_cid_aggr_info->overall_bytes;
#ifdef FOAK_TEMP_FIX
	  if (num_broadcast_bytes + data_bytes_needed + sdu_cid_q->sdu_num * PDU_MIN_OVERHEAD > num_bytes_in_dlslotsymbol)
	  {
		// Can't accommodate all the INIT_RNG cid messages. check how many can fit.
		data_bytes_needed = estimate_data_bytes_from_total(INIT_RNG_CID, sdu_cid_q, num_bytes_in_dlslotsymbol - num_broadcast_bytes);
		num_broadcast_bytes = num_bytes_in_dlslotsymbol;		
	  }
	  else
	  {
		num_broadcast_bytes += data_bytes_needed + sdu_cid_q->sdu_num * PDU_MIN_OVERHEAD;
	  }
#else
	  num_broadcast_bytes += data_bytes_needed + sdu_cid_q->sdu_num * PDU_MIN_OVERHEAD;
#endif

      pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
      init_pdu_map(pdu_map);
      pdu_map->cid = INIT_RNG_CID;

      cur_lbm->pdu_num++;
      
      pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
      pdu_map->transport_sdu_map->num_bytes =  data_bytes_needed;
      pdu_map->transport_sdu_map->cid = INIT_RNG_CID;
	  if (cur_lbm->pdu_map_header == NULL)
	  {
		cur_lbm->pdu_map_header = pdu_map;
	  }
	  else
	  {
		cur_lbm->pdu_map_header->next = pdu_map;
	  }
	  pdu_map->next = NULL;
    }
#ifdef FOAK_TEMP_FIX
  }
#endif
  num_broadcast_slots = ceil((float)num_broadcast_bytes * 8/BROADCAST_DBPS);

  // The last index in the allocation vector is for broadcast slots
  dl_ss_slots[NUM_SS] = num_broadcast_slots;
#ifdef FOAK_TEMP_FIX
  num_dlmap_ie += 1;
#else
  num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
#endif

  num_available_dl_slots = num_dl_slots - (num_broadcast_slots + \
					   calc_dlmap_slots(num_dlmap_ie));

  // Data region for these broadcast messages can be allocated last, once the 
  // length of DLMAP is known, since it follows DLMAP. Check for the worst case here 
  if(num_available_dl_slots < 0)
    {
      FLOG_ERROR("Not enough slots to send broadcast management messages.\n");
      return -1;
    }

  // Actual number of bytes in the broadcast burst additionally depends on the 
  // number of bytes taken up by the DLMAP. known only at the end of dl_scheduler processing
  // burst_bytes_num is updated there 
  cur_lbm->burst_bytes_num = num_broadcast_bytes;
  burst_index++;

  int sum_used_slots = num_broadcast_slots;
  int num_slots_temp = 0, total_bytes_needed = 0;

  FLOG_DEBUG("Available slots: %d. Now allocate space for MMM on basic CID\n", num_available_dl_slots);
  dl_serve_sduQ_in_order(dl_sdu_queue, num_dl_slots, BASIC_CID_MIN_VALUE, param_MAX_VALID_BASIC_CID, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);

  FLOG_DEBUG("Available slots: %d. Now allocate space for MMM on primary CID\n", num_available_dl_slots);
  dl_serve_sduQ_in_order(dl_sdu_queue, num_dl_slots, PRIMARY_CID_MIN_VALUE, max_valid_primary_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);

#ifdef ARQ_ENABLED
  FLOG_DEBUG("Now serve the UGS CIDs - ARQ ReTx packets.");
  // Traverse the list to find all ARQ enabled connections first. After their 
  // "waiting-for-retransmission" queues are served, pick up any new packets
  serve_arqReTxQ_in_order(num_dl_slots, UGS_CID_MIN_VALUE, max_valid_ugs_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);
#endif

  FLOG_DEBUG("Available slots: %d. Now serve the UGS CIDs, not sent packets\n", num_available_dl_slots);
  dl_serve_sduQ_by_QoS(dl_sdu_queue, num_dl_slots, UGS_CID_MIN_VALUE, max_valid_ugs_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);

#ifdef ARQ_ENABLED
  FLOG_DEBUG("Now serving BE ARQ ReTX packets");
  int num_non_zero_BE_ReTx_queues = ARQ_num_conn_waiting_for_ReTX(BE_CID_MIN_VALUE, max_valid_be_cid);
  FLOG_DEBUG("non zero BE ReTx queues: %d, num_available_dl_slots: %d\n", num_non_zero_BE_ReTx_queues, num_available_dl_slots);

  ARQ_ReTX_Q_aggr_info arq_info;

  // Do only if there are any BE packets waiting for ReTX in ARQ queues, else skip
  if (num_non_zero_BE_ReTx_queues > 0)
    {
      fair_share = num_available_dl_slots/num_non_zero_BE_ReTx_queues;
      // because the above is truncated integer division, there might be some surplus slots
      balance_slots = num_available_dl_slots - num_non_zero_BE_ReTx_queues * fair_share;
      FLOG_DEBUG("Fair Share for ARQ ReTx: %d, Balance Slots: %d\n", fair_share, balance_slots);

      int temp;
      blocks_info_t blk_info;

      if (fair_share < MIN_SLOTS_ALLOCATED)
	{
	  // If fair share < 1 slot, serve BE queues fully, in order
	  serve_arqReTxQ_in_order(num_dl_slots, BE_CID_MIN_VALUE, max_valid_be_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);
	}
      else
	{
	  // Now serve BE ARQ retrans pkts
	  for (ii = BE_CID_MIN_VALUE; ii <= max_valid_be_cid; ii++)
	    {
             connection *conn = NULL;
	     get_connection(ii,&conn);
	     if (conn == NULL) {continue;}
	     is_encrypt_enabled(ii,(u_int8_t*)&is_encrypted);
	     if (is_encrypted==1) pdu_overhead = PDU_OH_WITH_ENCRYPT;
	     else pdu_overhead = PDU_MIN_OVERHEAD;
	      // get the basic attribute of this connection
	      is_frag_enabled(ii, &is_frag);
	      is_arq_enabled(ii, &is_arq);

	      if (!is_arq)
		{
		  continue;
		}
	      arq_info = ARQ_get_ReTX_queue_aggr_info(ii);
	      if (arq_info.num_bytes==0)
		{
		  // No ARQ blocks waiting for retransmission for this CID. 
		  continue;
		}

	      // initialize subheader lengths according to connection attributes
	      init_subheader_len(ii, is_frag, is_arq, &psh_len, &fsh_len);
	      get_ss_index(ii, &ss_index);

	      total_bytes_needed = estimate_arqReTx_PnF_overhead(ii, arq_info);
              FLOG_DEBUG("BE ARQ CID: %d, Bytes in queue: %d, Total Bytes Needed: %d\n", ii, arq_info.num_bytes, total_bytes_needed);

	      total_bytes_needed += dl_ss_bytes[ss_index]; 
	      extra_dlmap_slots = 0;

	      // Currently to maintain generality, assume separate DLMAP IEs for each SS.
	      // Each time a new SS is served, new dlmap IE(s) generated
	      if (dl_ss_slots[ss_index]==0)
		{
		  extra_dlmap_slots = calc_dlmap_slots(num_dlmap_ie + NUM_MAX_DLMAP_IE_PER_SS)-calc_dlmap_slots(num_dlmap_ie);
		  // If no allocation so far, start a new DLMAP IE
		  // Check if the number of available slots is more than the overhead 
		  // added by a new DLMAP-IE
		  if (fair_share + balance_slots < extra_dlmap_slots)
		    {
		      // The number of additional slots needed for DLMAP IE are more than fair_share. Return the fair_share to the pool of balance slots
		      balance_slots += fair_share;
		      // Continue searching. Can allocate rest of the slots to SS for which 	 
		      // DLMAP-IE already allocated
		      continue;
		    }
		  else		
		    {
		      num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
		      lbm_ss[ss_index] = (logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
		      init_burst_map(lbm_ss[ss_index]);
		    }
		}
	      int arq_blk_size = get_blk_size(ii);

		  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);
          int bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
          if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
            {bytes_per_slot *= 2;}

	      // TODO: This will be replaced by a method that returns the   
	      // UL/DL MCS scheme given the ss mac addr or SS 
	      // mcs_type = get_mcs(ss_index);

	      // Number of slots needed to serve this CID completely		
	      num_slots_temp = ceil((float)total_bytes_needed/bytes_per_slot);
	      FLOG_DEBUG("BE ReTx CID: %d, num_slots_temp: %d, existing dl_ss_slots[%d]: %d, existing dl_ss_bytes[.]: %d\n", ii, num_slots_temp, ss_index, dl_ss_slots[ss_index], dl_ss_bytes[ss_index]);

	      sum_used_slots -= dl_ss_slots[ss_index];

	      logical_packet* arq_retx_blk;
	      logical_pdu_map* pdu_map;
              logical_element *le=NULL, *prev_le=NULL;
	      int  first_time_flag = 0;
	      int ret_val;
 
	      if (num_slots_temp + extra_dlmap_slots <= dl_ss_slots[ss_index] + fair_share + balance_slots)
		{
		  balance_slots += (dl_ss_slots[ss_index] + fair_share - num_slots_temp - extra_dlmap_slots);
		  dl_ss_slots[ss_index] = num_slots_temp;
		  dl_ss_bytes[ss_index] = total_bytes_needed;
		  for (jj=0; jj<arq_info.num_blocks; jj++)
		    {
		      ret_val = ARQ_dequeue_ReTX_q(ii, &blk_info);
		      if (ret_val != SUCCESS){
			FLOG_ERROR("ARQ dequeue failed for CID: %d, BSN: %d\n", ii, blk_info.start_bsn);
			continue;}
		      if (jj == 0)
			{
			  // Construct the logical maps
			  pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
			  init_pdu_map(pdu_map);
			  if (cur_pdu_map_for_ss[ss_index] != NULL)
			    {
			      // If there already are PDUs in the burst
			      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
			      cur_pdu_map_for_ss[ss_index] = pdu_map;
			    }
			  else
			    {
			      // First PDU in the burst map
			      cur_pdu_map_for_ss[ss_index] = pdu_map;
			      lbm_ss[ss_index]->pdu_map_header = pdu_map;
			    }
			  pdu_map->cid = ii;

			  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
			  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
			  init_logical_packet(arq_retx_blk);
			  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
			  arq_retx_blk->cid = ii;
	
			  lbm_ss[ss_index]->pdu_num++;
			}
		      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (prev_le == NULL)
			{
			  arq_retx_blk->element_head = le;
			}
		      else
			{
			  prev_le->next = le;
			}
		      prev_le = le;
		      le->type = ARQ_BLOCK;
		      le->blk_type = blk_info.btype;
		      le->data = (u_char*)blk_info.data;
		      le->length = blk_info.size;
		      le->start_bsn = blk_info.start_bsn;
		      le->next = NULL;
		      arq_retx_blk->length += blk_info.size;
		      FLOG_DEBUG("CID: %d, ARQ block %d of length %d, bsn: %d dequeued. \n", ii, jj, blk_info.size, blk_info.start_bsn);
		    }
		}
	      else
		{
		  FLOG_DEBUG("BE ARQ , slots shortfall for CID: %d\n", ii);
		  dl_ss_slots[ss_index] += (fair_share + balance_slots - extra_dlmap_slots);
		  temp = ceil((float)dl_ss_slots[ss_index]*bytes_per_slot);
		  bytes_available = max(temp - dl_ss_bytes[ss_index], 0); 
		  dl_ss_bytes[ss_index] = temp;
		  balance_slots = 0;
		  // Super-safe and very wasteful dequeuing :-( TODO: Improve
		  while (bytes_available >= (arq_blk_size + pdu_overhead + psh_len))
		    {
		      FLOG_DEBUG("BE ReTx CID: %d, bytes_available: %d \n", ii, bytes_available);
		      ret_val = ARQ_dequeue_ReTX_q(ii, &blk_info);
		      if (ret_val != SUCCESS)
			{
			  FLOG_DEBUG("ARQ dequeue failed for CID: %d, BSN: %d\n", ii, blk_info.start_bsn);
			  break;
			}
		      if(first_time_flag == 0)
			{
			  first_time_flag = 1;
			  // Construct the logical maps
			  pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
			  init_pdu_map(pdu_map);
			  if (cur_pdu_map_for_ss[ss_index] != NULL)
			    {
			      // If there already are PDUs in the burst
			      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
			      cur_pdu_map_for_ss[ss_index] = pdu_map;
			    }
			  else
			    {
			      // First PDU in the burst map
			      cur_pdu_map_for_ss[ss_index] = pdu_map;
			      lbm_ss[ss_index]->pdu_map_header = pdu_map;
			    }
			  pdu_map->cid = ii;

			  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
			  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
			  init_logical_packet(arq_retx_blk);
			  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
			  arq_retx_blk->cid = ii;
	
			  lbm_ss[ss_index]->pdu_num++;
			}
		      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (prev_le == NULL)
			{
			  arq_retx_blk->element_head = le;
			}
		      else
			{
			  prev_le->next = le;
			}
		      prev_le = le;
		      le->type = ARQ_BLOCK;
		      le->blk_type = blk_info.btype;
		      le->data = (u_char*)blk_info.data;
		      le->length = blk_info.size;
		      le->start_bsn = blk_info.start_bsn;
		      le->next = NULL;
		      arq_retx_blk->length += blk_info.size;
		      bytes_available = bytes_available - blk_info.size - pdu_overhead - psh_len;
		      FLOG_DEBUG("CID: %d, ARQ block %d of length %d dequeued. \n", ii, jj, blk_info.size);
		    } //end while 
		} //end else
	      FLOG_DEBUG("BE ARQ ReTx CID: %d, dl_ss_slots[ssindex]:%d dl_ss_bytes[ss_index]:%d ARQ reTx map size: %d, balance_slots :%d \n",
		     ii, 
		     dl_ss_slots[ss_index],
		     dl_ss_bytes[ss_index],
		     arq_retx_blk->length, balance_slots
		     );
	      sum_used_slots += dl_ss_slots[ss_index];
	    } // end for(ii ..)
          num_available_dl_slots = balance_slots;
	} // end else 
    } // end if num_non_zero_BE_ReTx_queues
#endif

  int num_be_cids_with_sdu_data = 0, cid_q_indx;
  for (ii = BE_CID_MIN_VALUE; ii <= max_valid_be_cid; ii++)
    {
      connection *conn = NULL;
      get_connection(ii,&conn);
      if (conn == NULL) {continue;}
      if(ht_is_value_present(ii)) 
	{
  	  cid_q_indx=ht_get_key(ii);
	  if(dl_sdu_queue->sdu_cid_q[cid_q_indx]->sdu_cid_aggr_info->overall_bytes > 0)
	    {
	      num_be_cids_with_sdu_data ++;
	    }
  	}
    }

  FLOG_DEBUG("Now serving BE not-sent pkts");
  // Calculate the number of BE CID's that have any data to send

  // Do only if there are any BE CIDs with packets to send
  if (num_be_cids_with_sdu_data > 0)
    {
      fair_share = num_available_dl_slots/num_be_cids_with_sdu_data;

      // because the above is truncated integer division, there might be some surplus slots
      balance_slots = num_available_dl_slots - num_be_cids_with_sdu_data * fair_share;
      FLOG_DEBUG("Fair Share for BE not-sent: %d, Balance Slots: %d, num_be_cids_wsdu_data: %d\n", fair_share, balance_slots, num_be_cids_with_sdu_data);

      if (fair_share < MIN_SLOTS_ALLOCATED) 
	{
	  // If fair share < 1 slot, serve BE queues fully, in order
	  dl_serve_sduQ_in_order(dl_sdu_queue, num_dl_slots, BE_CID_MIN_VALUE, max_valid_be_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes, lbm_ss, cur_pdu_map_for_ss);
	}
      else
	{
	  for (ii = BE_CID_MIN_VALUE; ii <= max_valid_be_cid; ii++)
	    {
              connection *conn = NULL;
	      get_connection(ii,&conn);
	      if (conn == NULL) {continue;}
	      get_ss_index(ii, &ss_index);
              is_frag_enabled(ii, &is_frag);
	
	
	      pdu_size = get_mac_pdu_size(ii);

	      sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, ii);
	      if ((sdu_cid_q == NULL) ||(sdu_cid_q->sdu_num==0))
		{
		  // This CID queue is empty. Go to next
		  continue;
		}

	      data_bytes_needed = sdu_cid_q->sdu_cid_aggr_info->overall_bytes;
	      if(is_conn_arq_enabled(ii))
		{
		  int arq_tx_win_start = ARQ_get_tx_window_start(ii);
		  // If the connection is ARQ enabled, allocation shouldn't exceed Tx window
		  max_blocks_allowed = mod(arq_tx_win_start + \
					   ARQ_get_tx_window_size(ii) - sdu_cid_q->sdu_cid_aggr_info->next_bsn, \
					   ARQ_BSN_MODULUS);
		  max_bytes_allowed = max_blocks_allowed * get_blk_size(ii);
		  data_bytes_needed = min(data_bytes_needed, max_bytes_allowed);
		  FLOG_DEBUG("BE not-sent packets. CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
		}
	      // Estimate how many bytes are needed to send all the data
	      // (overhead included)
	      assert(data_bytes_needed >= 0);
	      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);
	
	      FLOG_DEBUG("BE not-sent packets. CID: %d, data_bytes_needed: %d, total_bytes_needed: %d, dl_ss_bytes[ss_index]: %d\n", ii, data_bytes_needed, total_bytes_needed, dl_ss_bytes[ss_index]);
	      total_bytes_needed += dl_ss_bytes[ss_index]; 
	      extra_dlmap_slots = 0;
	
	      // Currently to maintain generality, assume separate DLMAP IEs for each SS.
	      // Each time a new SS is served, new dlmap IE(s) generated
	      if (dl_ss_slots[ss_index]==0)
		{
		  extra_dlmap_slots = calc_dlmap_slots(num_dlmap_ie + NUM_MAX_DLMAP_IE_PER_SS)-calc_dlmap_slots(num_dlmap_ie);
		  // If no allocation so far, start a new DLMAP IE
		  // Check if the number of available slots is more than the overhead 
		  // added by a new DLMAP-IE
		  if (fair_share + balance_slots < extra_dlmap_slots)
		    {
		      // The number of additional slots needed for DLMAP IE are more than fair_share. Return the fair_share to the pool of balance slots
		      balance_slots += fair_share;
		      // Continue searching. Can allocate rest of the slots to SS for which 	 
		      // DLMAP-IE already allocated
		      continue;
		    }
		  else		
		    {
		      num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
		      lbm_ss[ss_index] = (logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
		      init_burst_map(lbm_ss[ss_index]);
		    }
		}
	
	      // TODO: This will be replaced by a method that returns the   
	      // UL/DL MCS scheme given the ss mac addr or SS 
          //mcs_type = get_mcs(ss_index);
		  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);

          int bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
          if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
            {bytes_per_slot *= 2;}
	
	      // Number of slots needed to serve this CID completely
	      num_slots_temp = ceil((float)total_bytes_needed /bytes_per_slot);
	      sum_used_slots -= dl_ss_slots[ss_index];

	      FLOG_DEBUG("dlscheduler: CID: %d, num_slots_temp:%d extra_dlmap_slots:%d dl_ss_slots[ssindex]:%d fairshare:%d balance_slots:%d \n",
		     ii, num_slots_temp,
		     extra_dlmap_slots,
		     dl_ss_slots[ss_index],
		     fair_share,
		     balance_slots
		     );
	      if (num_slots_temp + extra_dlmap_slots <= dl_ss_slots[ss_index] + fair_share + balance_slots)
		{
		  balance_slots += (dl_ss_slots[ss_index] + fair_share - num_slots_temp - extra_dlmap_slots);
		  dl_ss_slots[ss_index] = num_slots_temp;
		  dl_ss_bytes[ss_index] = total_bytes_needed;
		}
	      else
		{
		  data_slots_available = (dl_ss_slots[ss_index] + fair_share + balance_slots - extra_dlmap_slots);
		  temp = ceil((float)data_slots_available*bytes_per_slot);
		  bytes_available = max(temp - dl_ss_bytes[ss_index], 0); 
		  
		  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
		  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	
		  if (data_bytes_needed < param_DL_CBR_PACKET_SIZE*MIN_PACKET_FRACTION)
		    {
		      // This is to avoid excessive fragmentation
		      // If the bytes that can be accomodated is smaller than a threshold, don't allocate
		      data_bytes_needed = 0;
		      balance_slots += fair_share - extra_dlmap_slots;
		    }
		  else
		    {
		      dl_ss_slots[ss_index] += (fair_share + balance_slots - extra_dlmap_slots);
		      balance_slots = 0;
		      dl_ss_bytes[ss_index] = ceil((float)dl_ss_slots[ss_index] * bytes_per_slot);
	    	    }
		  FLOG_DEBUG("dlscheduler: CID: %d, data_slots_available:%d bytes_available:%d dl_ss_slots[ssindex]:%d dl_ss_bytes[ss_index]:%d estimated_data_bytes_from_total:%d \n",
			 ii, data_slots_available,
			 bytes_available,
			 dl_ss_slots[ss_index],
			 dl_ss_bytes[ss_index],
			 data_bytes_needed 
			 );
		}
	      sum_used_slots += dl_ss_slots[ss_index];
	      if (data_bytes_needed != 0)
		{
		  // Construct the logical maps
		  logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		  init_pdu_map(pdu_map);
		  if (cur_pdu_map_for_ss[ss_index] != NULL)
		    {
		      // If there already are PDUs in the burst
		      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		    }
		  else
		    {
		      // First PDU in the burst map
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		      lbm_ss[ss_index]->pdu_map_header = pdu_map;
		    }
		  pdu_map->cid = ii;

		  pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
		  pdu_map->transport_sdu_map->cid = ii;
  
		  lbm_ss[ss_index]->pdu_num++;
		  pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
		}
	    } //end for (ii=BE_CID_MIN_VALUE;...)
          num_available_dl_slots = balance_slots;
	} //end else
    } // end if

  // In principle it is possible for some slots to still be free and some
  // queues to have backlog. Serve the UGS and BE queues fully in order, 
  // but can't do this yet as SDUs are not dequeued
  // dl_serve_sduQ_in_order(UGS_CID_MIN_VALUE, max_valid_ugs_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes);
  // dl_serve_sduQ_in_order(BE_CID_MIN_VALUE, max_valid_be_cid, &num_available_dl_slots, &num_dlmap_ie, &sum_used_slots, dl_ss_slots, dl_ss_bytes);

  int sum_slots_chk = 0, sum_bytes_chk = 0;

  int bytes_per_slot = 0;

  FLOG_DEBUG("Done with all scheduling. Now building the maps.");
  // Link up all the bursts with non-zero length to form a list
  for (ii = NUM_SS - 1; ii >= 0; ii--)
    {
      sum_slots_chk += dl_ss_slots[ii];
      sum_bytes_chk += dl_ss_bytes[ii];
      if (lbm_ss[ii] != NULL) 
	{
	  if (lbm_ss[ii]->pdu_num != 0)
	    {
	      cur_lbm->next = lbm_ss[ii];
	      cur_lbm = lbm_ss[ii];
              cur_lbm->map_burst_index = burst_index;
	      burst_index++;
		  
		  get_ss_mcs(ii, &dl_mcs_type, &ul_mcs_type);
		  bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
		  if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
		  {
		    	bytes_per_slot *= 2;
		  }

	      cur_lbm->burst_bytes_num = ceil((float)dl_ss_slots[ii] * bytes_per_slot);

	      assert(dl_ss_slots[ii]>=0);
	    }
	}
    }
   //printf("burst bytes num is %d\n",sum_bytes_chk);
  dl_map_builder(dl_ss_slots, amc_info_header, frame_map->dl_map, dlmap_length, num_dl_subch);
  
  // Update the number of bytes in the broadcast burst after accounting for the DLMAP bytes.
  // Broadcast burst is the first in the frame map
  frame_map->num_bursts = burst_index;

  FLOG_DEBUG("Num_dl_slots: %d, dlmap_len_in_slots: %d, Num_bursts: %d, Sum_used_slots: %d, sum_slots_chk: %d, sum_bytes_chk: %d\n", num_dl_slots, *dlmap_length, burst_index, sum_used_slots, sum_slots_chk, sum_bytes_chk);
  return 0;
}
#ifdef FOAK_TEMP_FIX
int calc_dlmap_slots(int num_dlmap_ie)
{
  // In FOAK, the ULMAP + broadcast burst starts from the slotsymbol after 
  // those occupied by the FCH+DLMAP burst. if there are any free slots in the
  // slotsymbols after the end of DLMAP, they'll be unused. e.g. if FCH + DLMAP
  // takes 1st slotsymbol, ULMAP burst will start from the 2nd slotsymbol. If 
  // FCH + DLMAP occupy 1.5 slotsymbols, ULMAP burst will start from the 3rd 
  // slotsymbol. This is due to PHY constraint that ULMAP burst shouldn't exceed
  // one slotsymbol and it should be described by one DLMAP-IE. To Remove later.
  // Because of this, calc_dlmap_slots rounds of slots needed by DLMAP in FOAK
  // to the number of slots in integer slotsymbols
  int   num_dlmap_bits = 0, num_dlmap_slots = 0, num_dlmap_slots_rounded = 0;
  int num_slots_per_sym = floor(num_dl_subchannels/DL_SUBCHANNEL_PER_SLOT);

  // mimo is not supported in FOAK
  num_dlmap_bits = PDU_MIN_OVERHEAD*8 + (DL_FIXED_FIELD_LENGTH + CID_SWITCH_IE_LEN + num_dlmap_ie * DLMAP_IE_LENGTH);
  num_dlmap_slots = ceil((float)num_dlmap_bits/BROADCAST_DBPS) + NUM_FCH_SLOTS;
  num_dlmap_slots_rounded = ceil((float)num_dlmap_slots/num_slots_per_sym)*num_slots_per_sym;
  return num_dlmap_slots_rounded;
}
#else
int calc_dlmap_slots(int num_dlmap_ie)
{
  // TODO: This function is inefficient currently. Repeats computation even though param_DL_STC_MATRIX_TYPE is same for all SS
  // Ideally could do once and pass the parameter for this current assumption. Later when diff STC capabilities for diff SS need
  // to be supported, structure will have to change
  int   num_dlmap_bits = 0, ie_length = 0;
  switch(param_DL_STC_MATRIX_TYPE)
  {
    case -1:
        ie_length = DLMAP_IE_LENGTH;
    break;
    case 0: // rate 1 Matrix A code
        num_dlmap_bits = STC_ZONE_IE_LEN;
        ie_length = MIMO_DL_BASIC_IE_COMMON_LEN + 8 * ceil((float)(NUM_ASSIGN_LEN + REGION_ATTRI_SIZE + LAYER_ATTRI_SIZE)/8); 
    break;
    case 1: //rate 2 Matrix B code
        num_dlmap_bits = STC_ZONE_IE_LEN;
        // The divide and multiply by 8 is to round length to nearest byte
        ie_length = MIMO_DL_BASIC_IE_COMMON_LEN + 8 * ceil((float)(NUM_ASSIGN_LEN + REGION_ATTRI_SIZE + 2*LAYER_ATTRI_SIZE)/8); 
    break;
    default:
        FLOG_ERROR("In dl_scheduler: Unsupported DL STC Matrxi type\n");
  }
  num_dlmap_bits = num_dlmap_bits + PDU_MIN_OVERHEAD*8 + (DL_FIXED_FIELD_LENGTH + CID_SWITCH_IE_LEN + num_dlmap_ie * ie_length);

  return ceil((float)num_dlmap_bits/BROADCAST_DBPS) + NUM_FCH_SLOTS;
}
#endif

int serve_arqReTxQ_in_order(int num_dl_slots , int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  int pdu_overhead;
  short ii = 0, jj = 0;
  int ss_index = 0, total_bytes_needed = 0, bytes_needed = 0, num_slots_temp;
  int bytes_available = 0;

  int num_available_dl_slots = *num_av_dl_slots_ptr;
  int num_dlmap_ie = *num_dlmap_ie_ptr;
  int sum_used_slots = *sum_slots_ptr;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;

  ARQ_ReTX_Q_aggr_info arq_info;
  blocks_info_t blk_info;

  u_int8_t is_frag;
  u_int8_t is_arq;
  int fsh_len = 0, psh_len = 0;

  for (ii = cid_range_start; (ii <= cid_range_end) & (num_available_dl_slots>0); ii++)
    {
      // get the basic attribute of this connection
      connection *conn = NULL;
      get_connection(ii,&conn);
      if (conn == NULL) {continue;}
      is_frag_enabled(ii, &is_frag);
      is_arq_enabled(ii, &is_arq);

      if (!is_arq) 
	{
	  // ARQ Not Enabled for this CID. 
	  continue;
	}

      arq_info = ARQ_get_ReTX_queue_aggr_info(ii);

      if (arq_info.num_bytes==0)
	{
	  // No ARQ blocks waiting for retransmission for this CID. 
	  continue;
	}
      // Get the queue pointer for CID queue
      get_ss_index(ii, &ss_index);
      // initialize subheader lengths according to connection attributes
      init_subheader_len(ii, is_frag, is_arq, &psh_len, &fsh_len);

      bytes_needed = estimate_arqReTx_PnF_overhead(ii, arq_info);

      FLOG_DEBUG("ARQ ReTx CID: %d, Bytes in queue: %d, Total Bytes Needed: %d\n", ii, arq_info.num_bytes, bytes_needed);
      total_bytes_needed = bytes_needed + dl_ss_bytes[ss_index]; 
      // Currently to maintain generality, assume separate DLMAP IEs for each SS.
      // Each time a new SS is served, new dlmap IE(s) generated
      if (dl_ss_slots[ss_index]==0)
	{
	  // If no allocation so far, start a new DLMAP IE
	  // Check if the number of available slots is more than the overhead 
	  // added by a new DLMAP-IE
	  if (num_available_dl_slots < (calc_dlmap_slots(num_dlmap_ie + NUM_MAX_DLMAP_IE_PER_SS)-calc_dlmap_slots(num_dlmap_ie)))
	    {
	      // Continue searching. Can allocate rest of the slots to SS for which 	 // DLMAP-IE already allocated
	      continue;
	    }
	  else		
	    {
	      num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
	      lbm_ss[ss_index] = (logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
	      init_burst_map(lbm_ss[ss_index]);
	      //	      cur_pdu_map_for_ss[ss_index] = lbm_ss[ss_index]->pdu_map_header;
	    }
	}

      // TODO: This will be replaced by a method that returns the   
      // UL/DL MCS scheme given the ss mac addr or SS 
		get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);

        int bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
        if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
		{
			bytes_per_slot *= 2;
		}
      // Number of slots needed to serve this CID completely		
      num_slots_temp = ceil((float)total_bytes_needed/bytes_per_slot );
      
      sum_used_slots -= dl_ss_slots[ss_index];
      num_available_dl_slots = num_dl_slots - sum_used_slots - \
		num_slots_temp - calc_dlmap_slots(num_dlmap_ie);

      logical_packet* arq_retx_blk;
      logical_pdu_map* pdu_map;
      logical_element *le=NULL, *prev_le=NULL;
      int arq_blk_size = get_blk_size(ii);
      int temp, ret_val, first_time_flag = 0;

      if (num_available_dl_slots < 0)
	{
	  FLOG_DEBUG("In serve_ARQ_ReTxQ_in_order, slots shortfall for CID: %d", ii);
	  dl_ss_slots[ss_index]=max(num_slots_temp+num_available_dl_slots, 0);
	  num_available_dl_slots = 0;
	  temp = ceil((float)dl_ss_slots[ss_index]* bytes_per_slot);
	  bytes_available = max(temp - dl_ss_bytes[ss_index], 0); 
	  dl_ss_bytes[ss_index] = temp;
	  FLOG_DEBUG("In serve_arq_in_order, CID: %d, arq_blk_size: %d \n", ii, arq_blk_size);
	  // Super-safe and very wasteful dequeuing :-( TODO: Improve
	  while (bytes_available >= (arq_blk_size + pdu_overhead + psh_len))
	    {
	      FLOG_DEBUG("In serve_arq_in_order, CID: %d, bytes_available: %d \n", ii, bytes_available);
	      ret_val = ARQ_dequeue_ReTX_q(ii, &blk_info);
	      if (ret_val != SUCCESS){
		FLOG_ERROR("ARQ dequeue failed for CID: %d, BSN: %d\n", ii, blk_info.start_bsn);
		break;}
	      if(first_time_flag == 0)
		{
		  first_time_flag = 1;
		  // Construct the logical maps
		  pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		  init_pdu_map(pdu_map);
		  if (cur_pdu_map_for_ss[ss_index] != NULL)
		    {
		      // If there already are PDUs in the burst
		      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		    }
		  else
		    {
		      // First PDU in the burst map
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		      lbm_ss[ss_index]->pdu_map_header = pdu_map;
		    }
		  pdu_map->cid = ii;

		  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
		  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
		  init_logical_packet(arq_retx_blk);
		  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
		  arq_retx_blk->cid = ii;
	
		  lbm_ss[ss_index]->pdu_num++;
		}
	      le = (logical_element*)mac_malloc(sizeof(logical_element));
	      if (prev_le == NULL)
		{
		  arq_retx_blk->element_head = le;
		}
	      else
		{
		  prev_le->next = le;
		}
	      prev_le = le;
	      le->type = ARQ_BLOCK;
	      le->blk_type = blk_info.btype;
	      le->data = (u_char*)blk_info.data;
	      le->length = blk_info.size;
	      le->start_bsn = blk_info.start_bsn;
	      le->next = NULL;
	      arq_retx_blk->length += blk_info.size;
	      bytes_available = bytes_available - blk_info.size - pdu_overhead - psh_len;
	      FLOG_DEBUG("CID: %d, ARQ block of length %d dequeued. \n", ii, blk_info.size);
	    }
	}
      else
	{
	  dl_ss_slots[ss_index] = num_slots_temp;
	  dl_ss_bytes[ss_index] = total_bytes_needed;
	  // All the blocks to be dequeued
	  for (jj=0; jj<arq_info.num_blocks; jj++)
	    {
	      ret_val = ARQ_dequeue_ReTX_q(ii, &blk_info);
	      if (ret_val != SUCCESS){
		FLOG_ERROR("ARQ dequeue failed for CID: %d, BSN: %d\n", ii, blk_info.start_bsn);
		continue;}
	      if (jj==0)
		{
		  // Construct the logical maps
		  pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		  init_pdu_map(pdu_map);
		  if (cur_pdu_map_for_ss[ss_index] != NULL)
		    {
		      // If there already are PDUs in the burst
		      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		    }
		  else
		    {
		      // First PDU in the burst map
		      cur_pdu_map_for_ss[ss_index] = pdu_map;
		      lbm_ss[ss_index]->pdu_map_header = pdu_map;
		    }
		  pdu_map->cid = ii;

		  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
		  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
		  init_logical_packet(arq_retx_blk);
		  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
		  arq_retx_blk->cid = ii;
	
		  lbm_ss[ss_index]->pdu_num++;
		}
	      le = (logical_element*)mac_malloc(sizeof(logical_element));
	      if (prev_le == NULL)
		{
		  arq_retx_blk->element_head = le;
		}
	      else
		{
		  prev_le->next = le;
		}
	      prev_le = le;
	      le->type = ARQ_BLOCK;
	      le->blk_type = blk_info.btype;
	      le->data = (u_char*)blk_info.data;
	      le->length = blk_info.size;
	      le->start_bsn = blk_info.start_bsn;
	      le->next = NULL;
	      arq_retx_blk->length += blk_info.size;
	      FLOG_DEBUG("CID: %d, ARQ block %d of length %d dequeued. \n", ii, jj, blk_info.size);
	    }
	}
      sum_used_slots += dl_ss_slots[ss_index];	
      FLOG_DEBUG("dlscheduler: ARQ ReTx CID: %d, dl_ss_slots[ssindex]:%d dl_ss_bytes[ss_index]:%d ARQ reTx map size :%d \n",
	     ii, 
	     dl_ss_slots[ss_index],
	     dl_ss_bytes[ss_index],
	     arq_retx_blk->length
	     );
    }
  // Update values in the pointers
  *num_av_dl_slots_ptr = num_available_dl_slots;
  *num_dlmap_ie_ptr = num_dlmap_ie;
  *sum_slots_ptr = sum_used_slots;

  return 0;
} 

int dl_serve_sduQ_in_order(sdu_queue* dl_sdu_queue, int num_dl_slots , int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  int ii = 0, total_bytes_needed, num_slots_temp, temp, bytes_available = 0, ss_index = 0;
  int num_available_dl_slots = *num_av_dl_slots_ptr;
  int num_dlmap_ie = *num_dlmap_ie_ptr;
  int sum_used_slots = *sum_slots_ptr;
  int max_bytes_allowed = 0, max_blocks_allowed = 0, data_bytes_needed = 0;
  //int num_sdu = 0, min_sdu_size = 0, max_sdu_size = 0;
  sdu_cid_queue* sdu_cid_q;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;

  for (ii = cid_range_start; (ii <= cid_range_end) & (num_available_dl_slots>0); ii++)
    {
      // Get the queue pointer for CID queue
      sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, ii);
      if ((sdu_cid_q == NULL) ||(sdu_cid_q->sdu_num==0))
	{
	  // This CID queue is empty. Go to next
	  continue;
	}
      get_ss_index(ii, &ss_index);

      data_bytes_needed = sdu_cid_q->sdu_cid_aggr_info->overall_bytes;

      if(is_conn_arq_enabled(ii))
	{
	  int arq_tx_win_start = ARQ_get_tx_window_start(ii);
	  // If the connection is ARQ enabled, allocation shouldn't exceed Tx window
	  max_blocks_allowed = mod(arq_tx_win_start + \
				   ARQ_get_tx_window_size(ii) - sdu_cid_q->sdu_cid_aggr_info->next_bsn, \
				   ARQ_BSN_MODULUS);
	  max_bytes_allowed = max_blocks_allowed * get_blk_size(ii);
	  data_bytes_needed = min(data_bytes_needed, max_bytes_allowed);
	  FLOG_DEBUG("CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
	}
	
      if (ii> PRIMARY_CID_MAX_VALUE && data_bytes_needed < param_DL_CBR_PACKET_SIZE * MIN_PACKET_FRACTION) continue;

      // Fragmentation & Packing case not accounted for. Can't predict overhead
      // without knowing exact MMM sizes. Assume worst case overhead here, i.e.
      // assume each MMM is in its own PDU. (one MMM being fragmented across 
      // multiple PDUs is not a likely case, ignored here)
      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);

      total_bytes_needed += dl_ss_bytes[ss_index]; 
      // Currently to maintain generality, assume separate DLMAP IEs for each SS. Each time a new SS  
      // is served, new dlmap IE(s) generated
      if (dl_ss_slots[ss_index]==0)
	{
	  // If no allocation so far, start a new DLMAP IE
	  // But check if the number of available slots is more than the 
	  // overhead added by a new DLMAP-IE
	  if (num_available_dl_slots < (calc_dlmap_slots(num_dlmap_ie + NUM_MAX_DLMAP_IE_PER_SS)-calc_dlmap_slots(num_dlmap_ie)))
	    {
	      // Continue searching. Can allocate rest of the slots to SS for 
	      // which DLMAP-IE already allocated
	      continue;
	    }
	  else		
	    {
	      num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
	      lbm_ss[ss_index] = (logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
	      init_burst_map(lbm_ss[ss_index]);
	      //cur_pdu_map_for_ss[ss_index] = lbm_ss[ss_index]->pdu_map_header;
	    }
	}

      // TODO: This will be replaced by a method that returns the   
      // UL/DL MCS scheme given the ss mac addr or SS 
		  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);
          int bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
          if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
            {bytes_per_slot *= 2;}
      // Number of slots needed to serve this CID completely		
      num_slots_temp = ceil((float)total_bytes_needed /bytes_per_slot);

      sum_used_slots -= dl_ss_slots[ss_index];      
      num_available_dl_slots = num_dl_slots - sum_used_slots - \
		num_slots_temp - calc_dlmap_slots(num_dlmap_ie);

      FLOG_DEBUG("CID: %d, Bytes in queue: %d, Data Bytes Needed: %d, Total Bytes Needed: %d\n", ii, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, data_bytes_needed, total_bytes_needed);

      if (num_available_dl_slots < 0)
	{

	  dl_ss_slots[ss_index]=max(num_slots_temp+num_available_dl_slots, 0);
	  temp = ceil((float)dl_ss_slots[ss_index]*bytes_per_slot);
	  bytes_available = max(temp - dl_ss_bytes[ss_index], 0); 
		  
	  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
	  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	  dl_ss_bytes[ss_index] = ceil((float)dl_ss_slots[ss_index]*bytes_per_slot);
	  
	  num_available_dl_slots = 0;
	  FLOG_DEBUG("dlscheduler: CID: %d, bytes_available:%d dl_ss_slots[ssindex]:%d dl_ss_bytes[ss_index]:%d estimated_data_bytes_from_total:%d \n",
		 ii, 
		 bytes_available,
		 dl_ss_slots[ss_index],
		 dl_ss_bytes[ss_index],
		 data_bytes_needed 
		 );
	}
      else
	{
	  dl_ss_slots[ss_index] = num_slots_temp;
	  dl_ss_bytes[ss_index] = total_bytes_needed;
	}
      if (data_bytes_needed != 0)
	{
	  // Construct the logical maps
	  logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
	  init_pdu_map(pdu_map);
	  if (cur_pdu_map_for_ss[ss_index] != NULL)
	    {
	      // If there already are PDUs in the burst
	      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
	      cur_pdu_map_for_ss[ss_index] = pdu_map;
	    }
	  else
	    {
	      // First PDU in the burst map
	      cur_pdu_map_for_ss[ss_index] = pdu_map;
	      lbm_ss[ss_index]->pdu_map_header = pdu_map;
	    }
	  pdu_map->cid = ii;
	      pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
	      pdu_map->transport_sdu_map->cid = ii;
	      pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
	      assert(data_bytes_needed>=0);
	  lbm_ss[ss_index]->pdu_num++;
		FLOG_DEBUG("#### [%d]: %d\n", ii, data_bytes_needed);
	} 

      // Update sum with the new allocation. The old value was subtracted above
      sum_used_slots += dl_ss_slots[ss_index];
    }

  // Update values in the pointers
  *num_av_dl_slots_ptr = num_available_dl_slots;
  *num_dlmap_ie_ptr = num_dlmap_ie;
  *sum_slots_ptr = sum_used_slots;
  return 0;
}

#ifdef ROUND_ROBIN
  int last_allocated_CID=0;
#endif

#define MAX_INT_VALUE (2147483648)
int dl_serve_sduQ_by_QoS(sdu_queue* dl_sdu_queue, int num_dl_slots, int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  int ii = 0, total_bytes_needed, num_slots_temp, temp, bytes_available = 0, ss_index = 0;
  int num_available_dl_slots = *num_av_dl_slots_ptr;
  int num_dlmap_ie = *num_dlmap_ie_ptr;
  int sum_used_slots = *sum_slots_ptr;
  int data_bytes_needed = 0;
  int  max_bytes_allowed = 0, max_blocks_allowed = 0;
  int time_since_last_alloc = 0;
  //connection* conn;
  serviceflow* sflow;
  sdu_cid_queue* sdu_cid_q;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;

#ifdef ROUND_ROBIN
  int current_cid=0;
  int tem_cid=0;
  int cid_offset=0; 

  if(last_allocated_CID>0)
  {
    cid_offset= last_allocated_CID-cid_range_start+1; 
	//FLOG_INFO("last allocated CID is %d\n",last_allocated_CID);
  }
#endif

  for (ii = cid_range_start; (ii <= cid_range_end) & (num_available_dl_slots>0); ii++)
  {
      connection *conn = NULL;
      
#ifdef ROUND_ROBIN
      current_cid = ii+cid_offset;
	    if(current_cid>cid_range_end)
      {
		    current_cid = current_cid-cid_range_end+cid_range_start-1;
	    }
	    
      //if(ii==cid_range_start)
      //{
      //    FLOG_INFO("@@@@@@@loop start, schedule start from %d, offset %d\n",current_cid,cid_offset);  
      //}
      
      tem_cid=ii;
      ii=current_cid;
#endif
      get_connection(ii,&conn);
      if (conn == NULL) {continue;}
      // Get the queue pointer for CID queue
      sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, ii);
      if ((sdu_cid_q == NULL) || (sdu_cid_q->sdu_num==0))
	  {
		// This CID queue is empty. Go to next
		  #ifdef ROUND_ROBIN
		  ii=tem_cid;
		  #endif
		  continue;
	  }
      get_ss_index(ii, &ss_index);
      get_connection(ii, &conn);
      sflow = conn->sf;

//	  time_since_last_alloc = (frame_number - sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number) * frame_duration[FRAME_DURATION_CODE];

#if 0
      if (sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number < 0)
      {
          time_since_last_alloc = sflow->unsolicited_grant_interval;
          sdu_cid_q->sdu_cid_aggr_info->overall_deficit = 0;
      }else
      {
          time_since_last_alloc = (get_current_frame_number() - sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number) * frame_duration[FRAME_DURATION_CODE];
      }

	  // time_since_last_alloc & ug_interval are in in millisec
	  if ((time_since_last_alloc < sflow->unsolicited_grant_interval) && (sdu_cid_q->sdu_cid_aggr_info->overall_deficit <= 0))
	  {
		  FLOG_DEBUG("For CID: %d, %d < %d, continuing\n", ii, time_since_last_alloc, sflow->unsolicited_grant_interval);
		
		  continue;
	  }
      // In Service flow QoS parameters, units of traffic rates are bytes per sec
      data_bytes_needed = time_since_last_alloc * sflow->min_reserved_traffic_rate/1000;

      data_bytes_needed += sdu_cid_q->sdu_cid_aggr_info->overall_deficit;
#else
	int frame_num = get_current_frame_number();
        /* it is a wrong decision to make the global frame number a integer type, it shall be unsigned integer */
        if (frame_num >= sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number)
        {
            time_since_last_alloc = (frame_num - sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number);
        }
        else
        {
            time_since_last_alloc = MAX_INT_VALUE - sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number + frame_num;
        }
        
        if (time_since_last_alloc < 1)
        {
            time_since_last_alloc = 1;
        }
        else if (time_since_last_alloc > (1000 / frame_duration[FRAME_DURATION_CODE]))
        {
            time_since_last_alloc = 1000 / frame_duration[FRAME_DURATION_CODE];
        }
        else
        {
        }
        time_since_last_alloc *= frame_duration[FRAME_DURATION_CODE];
        int64_t byte_count = ((int64_t)time_since_last_alloc) * ((int64_t)sflow->min_reserved_traffic_rate) / ((int64_t)1000);
        data_bytes_needed = (int)byte_count;
#endif

      // If there are fewer bytes in SDU CID queue than number guaranteed by
      // the service class, allot for fewer
      data_bytes_needed = min(sdu_cid_q->sdu_cid_aggr_info->overall_bytes, \
			      data_bytes_needed);

      // In case, deficit is negative, e.g. in case more when bytes than thought
      // can be sent in over-provisioned bursts, don't allocate anything
      data_bytes_needed = max(data_bytes_needed, 0);

      if(is_conn_arq_enabled(ii))
	  {
		int arq_tx_win_start = ARQ_get_tx_window_start(ii);
		// If the connection is ARQ enabled, allocation shouldn't exceed Tx window
		max_blocks_allowed = mod(arq_tx_win_start + \
				   ARQ_get_tx_window_size(ii) - sdu_cid_q->sdu_cid_aggr_info->next_bsn, \
				   ARQ_BSN_MODULUS);
		max_bytes_allowed = max_blocks_allowed * get_blk_size(ii);
		data_bytes_needed = min(data_bytes_needed, max_bytes_allowed);
		FLOG_DEBUG("CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
	  }

      if (data_bytes_needed < param_DL_CBR_PACKET_SIZE * MIN_PACKET_FRACTION) continue;

      // Can't predict overhead accurately
      // without knowing exact SDU sizes. Assume worst case overhead here, 
      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);
      FLOG_DEBUG("CID: %d, Last Dequeued Frame num: %ld, Deficit: %d, Bytes in queue: %d, Data Bytes Needed: %d, Total Bytes Needed: %d\n", ii, sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number, sdu_cid_q->sdu_cid_aggr_info->overall_deficit, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, data_bytes_needed, total_bytes_needed);
      
      total_bytes_needed += dl_ss_bytes[ss_index]; 
      // Currently to maintain generality, assume separate DLMAP IEs for each SS. Each time a new SS  
      // is served, new dlmap IE(s) generated
      if (dl_ss_slots[ss_index]==0)
	  {
		// If no allocation so far, start a new DLMAP IE
		// But check if the number of available slots is more than the 
		// overhead added by a new DLMAP-IE
		if (num_available_dl_slots < (calc_dlmap_slots(num_dlmap_ie + NUM_MAX_DLMAP_IE_PER_SS)-calc_dlmap_slots(num_dlmap_ie)))
	    {
	      // Continue searching. Can allocate rest of the slots to SS for 
	      // which DLMAP-IE already allocated
	      continue;
	    }
		else		
	    {
	      num_dlmap_ie += NUM_MAX_DLMAP_IE_PER_SS;
	      lbm_ss[ss_index] = (logical_burst_map*)mac_malloc(sizeof(logical_burst_map));
	      init_burst_map(lbm_ss[ss_index]);
	    }
	  }

      // TODO: This will be replaced by a method that returns the   
      // UL/DL MCS scheme given the ss mac addr or SS 

	  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);
      int bytes_per_slot = bits_per_car[dl_mcs_type]*DL_DATA_CAR_PER_SLOT/8;
      if (param_DL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
         {bytes_per_slot *= 2;}

      // Number of slots needed to serve this CID completely		
      num_slots_temp = ceil((float)total_bytes_needed /bytes_per_slot);

      sum_used_slots -= dl_ss_slots[ss_index];      
      num_available_dl_slots = num_dl_slots - sum_used_slots - \
		num_slots_temp - calc_dlmap_slots(num_dlmap_ie);
	FLOG_DEBUG("1. dl_ss_slots[%d] = %d\n", ss_index, dl_ss_slots[ss_index]);
	FLOG_DEBUG("1. num_available_dl_slots = %d, num_dl_slots = %d, sum_used_slots = %d\n", \
				num_available_dl_slots, num_dl_slots, sum_used_slots);

      if (num_available_dl_slots < 0)
	  {

	  // If you serve partially, it might not be enough to carry any MMM. 
	  // Ideally should peek at MMM size and allocate
	  dl_ss_slots[ss_index]=max(num_slots_temp+num_available_dl_slots, 0);
	  temp = ceil((float)dl_ss_slots[ss_index]*bytes_per_slot);
	  bytes_available = max(temp - dl_ss_bytes[ss_index], 0); 
		  
	  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
	  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	  dl_ss_bytes[ss_index] = ceil((float)dl_ss_slots[ss_index]*bytes_per_slot);
	  num_available_dl_slots = 0;
	  FLOG_DEBUG("dlscheduler: CID: %d, bytes_available:%d dl_ss_slots[ssindex]:%d dl_ss_bytes[ss_index]:%d estimated_data_bytes_from_total:%d \n",
		 ii, 
		 bytes_available,
		 dl_ss_slots[ss_index],
		 dl_ss_bytes[ss_index],
		 data_bytes_needed 
		 );

	  }
      else
	  {
		dl_ss_slots[ss_index] = num_slots_temp;
		dl_ss_bytes[ss_index] = total_bytes_needed;
	  }
	FLOG_DEBUG("2. dl_ss_slots[%d] = %d\n", ss_index, dl_ss_slots[ss_index]);
      if (data_bytes_needed != 0)
	  {
		// Construct the logical maps
		logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		init_pdu_map(pdu_map);
		if (cur_pdu_map_for_ss[ss_index] != NULL)
	    {
	      // If there already are PDUs in the burst
	      cur_pdu_map_for_ss[ss_index]->next = pdu_map;
	      cur_pdu_map_for_ss[ss_index] = pdu_map;
	    }
		else
	    {
	      // First PDU in the burst map
	      cur_pdu_map_for_ss[ss_index] = pdu_map;
	      lbm_ss[ss_index]->pdu_map_header = pdu_map;
	    }
		pdu_map->cid = ii;

		pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
		pdu_map->transport_sdu_map->cid = ii;
  
		lbm_ss[ss_index]->pdu_num++;
		pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
		FLOG_DEBUG("#### [%d]: %d\n", ii, data_bytes_needed);
		#ifdef ROUND_ROBIN
		   last_allocated_CID =ii;
		#endif
	  }
      // Update sum with the new allocation. The old value was subtracted above
      sum_used_slots += dl_ss_slots[ss_index];
      #ifdef ROUND_ROBIN
		   ii=tem_cid;
		  #endif
    }

  // Update values in the pointers
  *num_av_dl_slots_ptr = num_available_dl_slots;
  *num_dlmap_ie_ptr = num_dlmap_ie;
  *sum_slots_ptr = sum_used_slots;
  return 0;
}
