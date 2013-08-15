/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ss_scheduler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "ss_scheduler.h"
#include "arq_ifaces.h"
#include "arq_defines.h"
#include "debug.h"
#include "sdu_cid_queue.h"

int ss_scheduler(sdu_queue* dl_sdu_queue, ModulCodingType mcs_type, int num_allotted_slots, logical_dl_subframe_map *frame_map)
{
  int pdu_overhead = 0;
 
  short ii = 0, jj = 0;
  //int num_sdu = 0, max_sdu_size = 0, min_sdu_size = 0;
  int balance_bytes = 0, bytes_available = 0, total_bytes_needed = 0;

  sdu_cid_queue* sdu_cid_q;
  if (num_allotted_slots == 0)
  {
	FLOG_DEBUG("No slots allocated for this SS. No data scheduled\n");
	frame_map->num_bursts = 0;
	return 0;
  }
  frame_map->num_bursts = 1;
  logical_burst_map *lbm_ss = frame_map->burst_header;
  int num_allotted_bytes = num_allotted_slots*bits_per_car[mcs_type]*DL_DATA_CAR_PER_SLOT/8;
  if (param_UL_STC_MATRIX_TYPE == 1) // Matrix B, rate 2 code supported
            {num_allotted_bytes *= 2;}
  int dl_ss_bytes = 0;

  FLOG_DEBUG("Frame Number: %ld\n", get_current_frame_number());

  FLOG_DEBUG("Calling SS scheduler");

  // Management messages are high priority. Priority order: INIT_RNG_CID > Basic
  // CID > primary CID MMM > UGS > BE. For each ARQ enabled transport connection,
  // its retransmissions are honored before "not-sent" packets"
  logical_pdu_map *cur_pdu_map_for_ss=NULL;

  FLOG_DEBUG("Now allocate space for MMM on INIT_RNG CID");
  ss_dl_serve_sduQ_in_order(dl_sdu_queue, num_allotted_bytes, INIT_RNG_CID, INIT_RNG_CID, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);

  FLOG_DEBUG("Now allocate space for MMM on basic CID");
  ss_dl_serve_sduQ_in_order(dl_sdu_queue, num_allotted_bytes, BASIC_CID_MIN_VALUE, param_MAX_VALID_BASIC_CID, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);

  FLOG_DEBUG("Now allocate space for MMM on primary CID");
  ss_dl_serve_sduQ_in_order(dl_sdu_queue, num_allotted_bytes, PRIMARY_CID_MIN_VALUE, max_valid_primary_cid, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);

#ifdef ARQ_ENABLED
  FLOG_DEBUG("Now serve the UGS CIDs - ARQ ReTx packets.");
  // Traverse the list to find all ARQ enabled connections first. After their 
  // "waiting-for-retransmission" queues are served, pick up any new packets
  ss_serve_arqReTxQ_in_order(num_allotted_bytes, UGS_CID_MIN_VALUE, max_valid_ugs_cid, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);
#endif

  FLOG_DEBUG("Now serve the UGS CIDs, not sent packets");
  ss_dl_serve_sduQ_by_QoS(dl_sdu_queue, num_allotted_bytes, UGS_CID_MIN_VALUE, max_valid_ugs_cid, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);

  int fair_share, fsh_len = 0, psh_len = 0;
  u_int8_t is_frag;
  u_int8_t is_arq;

#ifdef ARQ_ENABLED
  FLOG_DEBUG("Now serving BE ARQ ReTX packets");
  int num_non_zero_BE_ReTx_queues = ARQ_num_conn_waiting_for_ReTX(BE_CID_MIN_VALUE, max_valid_be_cid);
  FLOG_DEBUG( "non zero BE ReTx queues: %d\n", num_non_zero_BE_ReTx_queues);

  ARQ_ReTX_Q_aggr_info arq_info;

  // Do only if there are any BE packets waiting for ReTX in ARQ queues, else skip
  if (num_non_zero_BE_ReTx_queues > 0)
    {
      fair_share = (num_allotted_bytes - dl_ss_bytes)/num_non_zero_BE_ReTx_queues;
      // because the above is truncated integer division, there might be some surplus slots
      balance_bytes = num_allotted_bytes - dl_ss_bytes - num_non_zero_BE_ReTx_queues * fair_share;
      FLOG_DEBUG( "Fair Share for ARQ ReTx: %d, Balance Bytes: %d\n", fair_share, balance_bytes);

      blocks_info_t blk_info;

      if (fair_share <= PDU_MIN_OVERHEAD)
	{
	  // If fair share < 1 slot, serve BE queues fully, in order
	  ss_serve_arqReTxQ_in_order(num_allotted_bytes, BE_CID_MIN_VALUE, max_valid_be_cid, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);
	}
      else
	{
	  // Now serve BE ARQ retrans pkts
	  for (ii = BE_CID_MIN_VALUE; ii <= max_valid_be_cid; ii++)
	    {
	      // get the basic attribute of this connection
              connection *conn = NULL;
	      get_connection(ii,&conn);
	      if (conn == NULL) {continue;}
	      is_frag_enabled(ii, &is_frag);
	      is_arq_enabled(ii, &is_arq);
	      int is_encrypted=0;
	      is_encrypt_enabled(ii,(u_int8_t*)&is_encrypted);
	      if (is_encrypted==1) pdu_overhead = PDU_OH_WITH_ENCRYPT; else pdu_overhead = PDU_MIN_OVERHEAD;
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

	      total_bytes_needed = estimate_arqReTx_PnF_overhead(ii, arq_info);
              FLOG_DEBUG( "BE ARQ CID: %d, Bytes in queue: %d, Total Bytes Needed: %d\n", ii, arq_info.num_bytes, total_bytes_needed);

	      int arq_blk_size = get_blk_size(ii);

	      // Number of slots needed to serve this CID completely		
	      FLOG_DEBUG( "BE ReTx CID: %d, existing dl_ss_bytes: %d\n", ii, dl_ss_bytes);

	      logical_packet* arq_retx_blk = NULL;
	      logical_pdu_map* pdu_map = NULL;
              logical_element *le=NULL, *prev_le=NULL;
	      int  first_time_flag = 0;
	      int ret_val;
 
	      if (total_bytes_needed <= fair_share + balance_bytes)
		{
		  balance_bytes = balance_bytes + fair_share - total_bytes_needed;
		  dl_ss_bytes += total_bytes_needed;
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
			  if (cur_pdu_map_for_ss != NULL)
			    {
			      // If there already are PDUs in the burst
			      cur_pdu_map_for_ss->next = pdu_map;
			      cur_pdu_map_for_ss = pdu_map;
			    }
			  else
			    {
			      // First PDU in the burst map
			      cur_pdu_map_for_ss = pdu_map;
			      lbm_ss->pdu_map_header = pdu_map;
			    }
			  pdu_map->cid = ii;
	
			  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
			  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
			  init_logical_packet(arq_retx_blk);
			  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
			  arq_retx_blk->cid = ii;
		
			  lbm_ss->pdu_num++;
			}
		      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (le == NULL)
		      {
			FLOG_FATAL("SS scheduler: Error allocating memory for logical element\n");
			return -1;
		      }
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
		      FLOG_DEBUG( "CID: %d, ARQ block %d of length %d, bsn: %d dequeued. \n", ii, jj, blk_info.size, blk_info.start_bsn);
		    }
		}
	      else
		{
		  FLOG_DEBUG( "BE ARQ , slots shortfall for CID: %d\n", ii);
		  bytes_available = num_allotted_bytes - dl_ss_bytes; 
		  dl_ss_bytes = num_allotted_bytes;
		  balance_bytes = 0;
		  // Super-safe and very wasteful dequeuing :-( TODO: Improve
		  while (bytes_available >= (arq_blk_size + pdu_overhead + psh_len))
		    {
		      FLOG_DEBUG( "BE ReTx CID: %d, bytes_available: %d \n", ii, bytes_available);
		      ret_val = ARQ_dequeue_ReTX_q(ii, &blk_info);
			if (ret_val != SUCCESS)
			{
			FLOG_DEBUG( "ARQ dequeue failed for CID: %d, BSN: %d\n", ii, blk_info.start_bsn);
			break;
			}
		  if(first_time_flag == 0)
		    {
			first_time_flag = 1;
		      // Construct the logical maps
		      pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		      init_pdu_map(pdu_map);
		      if (cur_pdu_map_for_ss != NULL)
			{
			  // If there already are PDUs in the burst
			  cur_pdu_map_for_ss->next = pdu_map;
			  cur_pdu_map_for_ss = pdu_map;
			}
		      else
			{
			  // First PDU in the burst map
			  cur_pdu_map_for_ss = pdu_map;
			  lbm_ss->pdu_map_header = pdu_map;
			}
		      pdu_map->cid = ii;

		      pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
		      arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
		      init_logical_packet(arq_retx_blk);
		      pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
		      arq_retx_blk->cid = ii;
	
		      lbm_ss->pdu_num++;
		    }
		      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (le == NULL)
		      {
			FLOG_FATAL("SS scheduler: Error allocating memory for logical element\n");
			return -1;
		      }
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
		      FLOG_DEBUG( "CID: %d, ARQ block %d of length %d dequeued. \n", ii, jj, blk_info.size);
		    } //end while 
		} //end else
	      FLOG_DEBUG( "BE ARQ ReTx CID: %d, dl_ss_bytes:%d ARQ reTx map size: %d, balance_bytes :%d \n",
		     ii, 
		     dl_ss_bytes,
		     arq_retx_blk->length, balance_bytes
		     );
	    } // end for(ii ..)
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
      fair_share = (num_allotted_bytes - dl_ss_bytes)/num_be_cids_with_sdu_data;

      // because the above is truncated integer division, there might be some surplus slots
      balance_bytes = num_allotted_bytes - dl_ss_bytes - num_be_cids_with_sdu_data * fair_share;
      FLOG_DEBUG( "Fair Share for BE not-sent: %d, Balance Slots: %d, num_be_cids_wsdu_data: %d\n", fair_share, balance_bytes, num_be_cids_with_sdu_data);

      int max_bytes_allowed = 0, max_blocks_allowed = 0, data_bytes_needed = 0, pdu_size = 0;

      if (fair_share <= PDU_MIN_OVERHEAD)
	{
	  // If fair share < 1 slot, serve BE queues fully, in order
	  ss_dl_serve_sduQ_in_order(dl_sdu_queue, num_allotted_bytes, BE_CID_MIN_VALUE, max_valid_be_cid, &dl_ss_bytes, lbm_ss, &cur_pdu_map_for_ss);
	}
      else
	{
	  for (ii = BE_CID_MIN_VALUE; ii <= max_valid_be_cid; ii++)
	    {
              connection *conn = NULL;
	      get_connection(ii,&conn);
	      if (conn == NULL) {continue;}
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
		  FLOG_DEBUG( "BE not-sent packets. CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
		}
	      // Estimate how many bytes are needed to send all the data
	      // (overhead included)
	      assert(data_bytes_needed >= 0);
	      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);
	
	      FLOG_DEBUG( "BE not-sent packets. CID: %d, data_bytes_needed: %d, total_bytes_needed: %d, dl_ss_bytes: %d\n", ii, data_bytes_needed, total_bytes_needed, dl_ss_bytes);
	     // printf("BE not-sent packets. CID: %d, data_bytes_needed: %d, total_bytes_needed: %d, dl_ss_bytes: %d\n", ii, data_bytes_needed, total_bytes_needed, dl_ss_bytes);

	      if (total_bytes_needed <= fair_share + balance_bytes)
		{
		  balance_bytes = balance_bytes + fair_share - total_bytes_needed;
		  dl_ss_bytes += total_bytes_needed;
		}
	      else
		{
		  bytes_available = fair_share + balance_bytes; 
		  
		  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
		  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	
		  if (data_bytes_needed == 0)
		    {
		      balance_bytes += fair_share;
		    }
		  else
		    {
		      balance_bytes = 0;
		      dl_ss_bytes += bytes_available;
	    	    }
		  FLOG_DEBUG( "dlscheduler: CID: %d, bytes_available:%d dl_ss_bytes:%d estimated_data_bytes_from_total:%d \n",
			 ii, 
			 bytes_available,
			 dl_ss_bytes,
			 data_bytes_needed 
			 );
		}
	      if (data_bytes_needed != 0)
		{
		  // Construct the logical maps
		  logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
		  init_pdu_map(pdu_map);
		  if (cur_pdu_map_for_ss != NULL)
		    {
		      // If there already are PDUs in the burst
		      cur_pdu_map_for_ss->next = pdu_map;
		      cur_pdu_map_for_ss = pdu_map;
		    }
		  else
		    {
		      // First PDU in the burst map
		      cur_pdu_map_for_ss = pdu_map;
		      lbm_ss->pdu_map_header = pdu_map;
		    }
		  pdu_map->cid = ii;

		  pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
	          if (pdu_map->transport_sdu_map == NULL)
	          {
		    FLOG_FATAL("SS scheduler: Error allocating memory for Transport SDU map\n");
		    return -1;
	          }
		  pdu_map->transport_sdu_map->cid = ii;
  
		  lbm_ss->pdu_num++;
		  pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
		}
	    } //end for (ii=BE_CID_MIN_VALUE;...)
	} //end else
    } // end if

  // In principle it is possible for some slots to still be free and some
  // queues to have backlog. Serve the UGS and BE queues fully in order, 
  // but can't do this yet as SDUs are not dequeued
  // ss_dl_serve_sduQ_in_order(UGS_CID_MIN_VALUE, max_valid_ugs_cid, dl_ss_bytes);
  // ss_dl_serve_sduQ_in_order(BE_CID_MIN_VALUE, max_valid_be_cid, dl_ss_bytes);

  lbm_ss->burst_bytes_num = dl_ss_bytes;
  lbm_ss->map_burst_index = 0;
  lbm_ss->next = NULL;
  return 0;
}

int ss_serve_arqReTxQ_in_order(int num_allotted_bytes , int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  short ii = 0, jj = 0;
  int total_bytes_needed = 0;
  int bytes_available = 0;

  ARQ_ReTX_Q_aggr_info arq_info;
  blocks_info_t blk_info;

  u_int8_t is_frag;
  u_int8_t is_arq;
  int fsh_len = 0, psh_len = 0;

  int pdu_overhead =0;

  for (ii = cid_range_start; (ii <= cid_range_end) & (*dl_ss_bytes < num_allotted_bytes); ii++)
  {
      connection* conn = NULL;
      get_connection(ii,&conn);
      if( conn == NULL) continue;
      int is_encrypted=0;
      is_encrypt_enabled(ii,(u_int8_t*)&is_encrypted);
      if (is_encrypted==1) pdu_overhead = PDU_OH_WITH_ENCRYPT; else pdu_overhead = PDU_MIN_OVERHEAD;
      // get the basic attribute of this connection
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

      // initialize subheader lengths according to connection attributes
      init_subheader_len(ii, is_frag, is_arq, &psh_len, &fsh_len);

      total_bytes_needed = estimate_arqReTx_PnF_overhead(ii, arq_info);

      FLOG_DEBUG( "ARQ ReTx CID: %d, Bytes in queue: %d, Total Bytes Needed: %d\n", ii, arq_info.num_bytes, total_bytes_needed);

      logical_packet* arq_retx_blk;
      logical_pdu_map* pdu_map;
      logical_element *le=NULL, *prev_le=NULL;
      int arq_blk_size = get_blk_size(ii);
      int ret_val, first_time_flag = 0;

      if (total_bytes_needed >= num_allotted_bytes - *dl_ss_bytes)
	{
	  bytes_available = num_allotted_bytes - *dl_ss_bytes; 
	  (*dl_ss_bytes) = num_allotted_bytes;
	  FLOG_DEBUG( "In serve_arq_in_order, CID: %d, arq_blk_size: %d \n", ii, arq_blk_size);
	  // Super-safe and very wasteful dequeuing :-( TODO: Improve
	  while (bytes_available >= (arq_blk_size + pdu_overhead + psh_len))
	    {
	      FLOG_DEBUG( "In serve_arq_in_order, CID: %d, bytes_available: %d \n", ii, bytes_available);
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
	      if (*cur_pdu_map_for_ss != NULL)
		{
		  // If there already are PDUs in the burst
		  (*cur_pdu_map_for_ss)->next = pdu_map;
		  (*cur_pdu_map_for_ss) = pdu_map;
		}
	      else
		{
		  // First PDU in the burst map
		  (*cur_pdu_map_for_ss) = pdu_map;
		  lbm_ss->pdu_map_header = pdu_map;
		}
	      pdu_map->cid = ii;

	      pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
	      arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
	      init_logical_packet(arq_retx_blk);
	      pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
	      arq_retx_blk->cid = ii;
	
	      lbm_ss->pdu_num++;
	    }
	      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (le == NULL)
		      {
			FLOG_FATAL("SS scheduler: Error allocating memory for logical element\n");
			return -1;
		      }
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
	      FLOG_DEBUG( "CID: %d, ARQ block of length %d dequeued. \n", ii, blk_info.size);
	    }
	}
      else
	{
	  (*dl_ss_bytes) = (*dl_ss_bytes) + total_bytes_needed;
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
		  if ((*cur_pdu_map_for_ss) != NULL)
		    {
		      // If there already are PDUs in the burst
		      (*cur_pdu_map_for_ss)->next = pdu_map;
		      (*cur_pdu_map_for_ss) = pdu_map;
		    }
		  else
		    {
		      // First PDU in the burst map
		      (*cur_pdu_map_for_ss) = pdu_map;
		      lbm_ss->pdu_map_header = pdu_map;
		    }
		  pdu_map->cid = ii;

		  pdu_map->arq_sdu_map = (arq_retrans_sdu_map*)mac_malloc(sizeof(arq_retrans_sdu_map));
		  arq_retx_blk = (logical_packet*)mac_malloc(sizeof(logical_packet));
		  init_logical_packet(arq_retx_blk);
		  pdu_map->arq_sdu_map->arq_retransmit_block = arq_retx_blk;
		  arq_retx_blk->cid = ii;
	
		  lbm_ss->pdu_num++;
		}
	      le = (logical_element*)mac_malloc(sizeof(logical_element));
		      if (le == NULL)
		      {
			FLOG_FATAL("SS scheduler: Error allocating memory for logical element\n");
			return -1;
		      }
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
	      FLOG_DEBUG( "CID: %d, ARQ block %d of length %d dequeued. \n", ii, jj, blk_info.size);
	    }
	}
      FLOG_DEBUG( "dlscheduler: ARQ ReTx CID: %d, dl_ss_bytes:%d ARQ reTx map size :%d \n",
	     ii, 
	     *dl_ss_bytes,
	     arq_retx_blk->length
	     );
    }

  return 0;
} 

int ss_dl_serve_sduQ_in_order(sdu_queue* dl_sdu_queue, int num_allotted_bytes , int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  int ii = 0, total_bytes_needed, bytes_available = 0;
  int max_bytes_allowed = 0, max_blocks_allowed = 0, data_bytes_needed = 0;
  //int num_sdu = 0, min_sdu_size = 0, max_sdu_size = 0;
  sdu_cid_queue* sdu_cid_q;

  for (ii = cid_range_start; (ii <= cid_range_end) & (*dl_ss_bytes < num_allotted_bytes); ii++)
    {
      // Get the queue pointer for CID queue
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
	  FLOG_DEBUG( "CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
	}
      if (data_bytes_needed==0) continue;

      // Fragmentation & Packing case not accounted for. Can't predict overhead
      // without knowing exact MMM sizes. Assume worst case overhead here, i.e.
      // assume each MMM is in its own PDU. (one MMM being fragmented across 
      // multiple PDUs is not a likely case, ignored here)
      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);

      FLOG_DEBUG( "CID: %d, Bytes in queue: %d, Data Bytes Needed: %d, Total Bytes Needed: %d\n", ii, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, data_bytes_needed, total_bytes_needed);

      // If available bytes are more than the requirement, do a partial allocation 
      if (total_bytes_needed > (num_allotted_bytes - *dl_ss_bytes))
	{
	  bytes_available = num_allotted_bytes - *dl_ss_bytes;
		  
	  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
	  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	  *dl_ss_bytes = num_allotted_bytes;
	  
	  FLOG_DEBUG( "dlscheduler: CID: %d, bytes_available:%d dl_ss_bytes:%d estimated_data_bytes_from_total:%d \n",
		 ii, 
		 bytes_available,
		 *dl_ss_bytes,
		 data_bytes_needed 
		 );
	}
      else
	{
	  (*dl_ss_bytes) = (*dl_ss_bytes) + total_bytes_needed;
	}
      if (data_bytes_needed != 0)
	{
	  // Construct the logical maps
	  logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
	  init_pdu_map(pdu_map);
	  if ((*cur_pdu_map_for_ss) != NULL)
	    {
	      // If there already are PDUs in the burst
	      (*cur_pdu_map_for_ss)->next = pdu_map;
	      (*cur_pdu_map_for_ss) = pdu_map;
	    }
	  else
	    {
	      // First PDU in the burst map
	      (*cur_pdu_map_for_ss) = pdu_map;
	      lbm_ss->pdu_map_header = pdu_map;
	    }
	  pdu_map->cid = ii;
	  pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
          if (pdu_map->transport_sdu_map == NULL)
          {
	    FLOG_FATAL("SS scheduler: Error allocating memory for Transport SDU map\n");
	    return -1;
          }
	  pdu_map->transport_sdu_map->cid = ii;
	  pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
	  assert(data_bytes_needed>=0);
	  lbm_ss->pdu_num++;
	} 
    }
  return 0;
}

int ss_dl_serve_sduQ_by_QoS(sdu_queue* dl_sdu_queue, int num_allotted_bytes, int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss)
{
  int ii = 0, total_bytes_needed, bytes_available = 0;
  int data_bytes_needed = 0;
  int  max_bytes_allowed = 0, max_blocks_allowed = 0;
  int time_since_last_alloc = 0;
  //connection* conn;
  serviceflow* sflow;
  sdu_cid_queue* sdu_cid_q;

  for (ii = cid_range_start; (ii <= cid_range_end) & (*dl_ss_bytes < num_allotted_bytes); ii++)
    {
      connection* conn = NULL;
      get_connection(ii,&conn);
      if( conn == NULL) continue;
      // Get the queue pointer for CID queue
      sdu_cid_q = get_sdu_cid_queue(dl_sdu_queue, ii);
      if ((sdu_cid_q == NULL) || (sdu_cid_q->sdu_num==0))
	{
	  // This CID queue is empty. Go to next
	  continue;
	}
      get_connection(ii, &conn);
      sflow = conn->sf;

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
		continue;
	  }
      // In Service flow QoS parameters, units of traffic rates are bytes per sec
      data_bytes_needed = time_since_last_alloc * sflow->min_reserved_traffic_rate/1000;

      data_bytes_needed += sdu_cid_q->sdu_cid_aggr_info->overall_deficit;

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
	  FLOG_DEBUG( "CID: %d, Max Blocks allowed: %d, Bytes in queue: %d, Next BSN: %d, ARQ Tx Win Start: %d\n", ii, max_blocks_allowed, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, sdu_cid_q->sdu_cid_aggr_info->next_bsn, arq_tx_win_start);
	}

      if (data_bytes_needed==0) continue;

      // Can't predict overhead accurately
      // without knowing exact SDU sizes. Assume worst case overhead here, 
      total_bytes_needed = estimate_sdu_PnF_overhead(ii, sdu_cid_q, data_bytes_needed);
      FLOG_DEBUG( "CID: %d, Last Dequeued Frame num: %ld, Deficit: %d, Bytes in queue: %d, Data Bytes Needed: %d, Total Bytes Needed: %d\n", ii, sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number, sdu_cid_q->sdu_cid_aggr_info->overall_deficit, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, data_bytes_needed, total_bytes_needed);
     // printf("CID: %d, Last Dequeued Frame num: %ld, Deficit: %d, Bytes in queue: %d, Data Bytes Needed: %d, Total Bytes Needed: %d\n", ii, sdu_cid_q->sdu_cid_aggr_info->last_dequeued_frame_number, sdu_cid_q->sdu_cid_aggr_info->overall_deficit, sdu_cid_q->sdu_cid_aggr_info->overall_bytes, data_bytes_needed, total_bytes_needed);

      if (total_bytes_needed > num_allotted_bytes - *dl_ss_bytes) 
	{
	  // If you serve partially, it might not be enough to carry any MMM. 
	  // Ideally should peek at MMM size and allocate
	  bytes_available = num_allotted_bytes - *dl_ss_bytes;
		  
	  // Calculate how many bytes can be accommodated in the available space, after accounting for overhead
	  data_bytes_needed = estimate_data_bytes_from_total(ii, sdu_cid_q, bytes_available);
	  *dl_ss_bytes = num_allotted_bytes;
	  FLOG_DEBUG( "dlscheduler: CID: %d, bytes_available:%d dl_ss_bytes:%d estimated_data_bytes_from_total:%d \n",
		 ii, 
		 bytes_available,
		 *dl_ss_bytes,
		 data_bytes_needed 
		 );
	}
      else
	{
	  (*dl_ss_bytes) = (*dl_ss_bytes) + total_bytes_needed;
	}
      if (data_bytes_needed != 0)
	{
	  // Construct the logical maps
	  logical_pdu_map* pdu_map = (logical_pdu_map*)mac_malloc(sizeof(logical_pdu_map));
	  init_pdu_map(pdu_map);
	  if ((*cur_pdu_map_for_ss) != NULL)
	    {
	      // If there already are PDUs in the burst
	      (*cur_pdu_map_for_ss)->next = pdu_map;
	      (*cur_pdu_map_for_ss) = pdu_map;
	    }
	  else
	    {
	      // First PDU in the burst map
	      (*cur_pdu_map_for_ss) = pdu_map;
	      lbm_ss->pdu_map_header = pdu_map;
	    }
	  pdu_map->cid = ii;

	  pdu_map->transport_sdu_map = (transport_sdu_map*)mac_malloc(sizeof(transport_sdu_map));
          if (pdu_map->transport_sdu_map == NULL)
          {
	    FLOG_FATAL("SS scheduler: Error allocating memory for Transport SDU map\n");
	    return -1;
          }
	  pdu_map->transport_sdu_map->cid = ii;
  
	  lbm_ss->pdu_num++;
	  pdu_map->transport_sdu_map->num_bytes = data_bytes_needed;
	}
    }
  return 0;
}
