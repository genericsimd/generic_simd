/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_scheduler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "dl_exp_params.h"
#include "ul_scheduler.h"
#include "debug.h"
#include "mac.h"

#include "sdu_cid_queue.h"

// UL scheduler will be called only when BS transmit chain is active
extern volatile int ulmap_ccc;
extern sll_fifo_q *ranging_scheduler_q;
extern int num_bytes_in_dlslotsymbol;
#include <pthread.h>
extern pthread_mutex_t bs_ss_list_mutex;

#ifdef ROUND_ROBIN
  int ul_last_allocated_cid = UL_UGS_CID_MIN_VALUE - 1;
#endif

int ul_scheduler(br_queue** br_q_list, amc_info* amc_info_header, int num_ul_data_slots, int num_ul_symbols, int num_ss, ul_map_msg* ul_map)
{
	int ii, slots_needed = 0, num_ext_ie = 0;
	long int bytes_allocated = 0, bytes_needed = 0;
	SchedulingType q_index;

	br_queue* br_q;
	bw_req_el* br_el;
	int fair_share=0, ss_index=0, time_since_last_alloc = 0 ;

	ModulCodingType dl_mcs_type = param_DL_MCS;
	ModulCodingType ul_mcs_type = param_UL_MCS;

	//serve_fifo(br_q, ul_ss_allocation, &num_ul_data_slots);

	serviceflow *sflow;
	int ugs_info_index = 0;

	// array of slots allocated to each SS
	// A consistent mapping of SS index to mac address is used by all modules
	int* ul_ss_allocation = (int*)malloc(num_ss*sizeof(int));
	memset(ul_ss_allocation, 0, num_ss*sizeof(int));
	int len = 0;
	q_container *q_cntr = NULL;
	ranging_adjust_ie *p_list_head = NULL, *p_ie_tmp = NULL; 


#ifdef ROUND_ROBIN
  //for roundrobin
	int cid_offset=0;
	int current_cid=0;
	int tem_cid=0;
#endif
	// CDMA alloc IE sending logic - BS gets a list of corrections for diff SS.
	// Of that, the CDMA codes that need no corrections (along w/ RNG_RSP that 
	// has success status) are enq'ed into the ranging_scheduler_q. The UL 
	// scheduler deq's these rng_adjust_ie's one by one and reserves space 
	// for a CDMA alloc IE for them, till all SSs are served or it runs out of
	// space. Thse IEs are passed as a list to ulmap_builder for building the IE
	// This is done before serving any UL transport CIDs. The premise is that 
	// Network entry is more important. If there isn't enough space to serve all
	// users, DSA requests can be denied later, but SS should be able to join a BS
	while (num_ul_data_slots >= CDMA_ALLOC_SIZE)  
	{
		if (sll_fifo_q_dequeue(ranging_scheduler_q, &q_cntr, (size_t*)&len) == -1)
		{
			break;
		}
		switch(q_cntr->data_type)
		{
			case RANGING_ADJUST:
				num_ext_ie++;
				num_ul_data_slots -= CDMA_ALLOC_SIZE;
				if (p_list_head == NULL)
				{
					p_list_head = q_cntr->data;
					p_ie_tmp = p_list_head;
				}
				else
				{
					p_ie_tmp->p_next = q_cntr->data;
					p_ie_tmp = p_ie_tmp->p_next;
					p_ie_tmp->p_next = NULL;
				}
			break;
			default:
				FLOG_ERROR("Error: Unrecognized message in ranging_scheduler_q\n");
		}
		free(q_cntr);
		q_cntr = NULL;
	}

	/* individual polling for entried MS in case of dsa request, bandwidth request...etc */
#ifdef DSX_ENABLE
	bs_ss_info      *ss_info;
	static int      last_polled_ss_index = 0;

	last_polled_ss_index++;
	if (last_polled_ss_index >= num_ss)
	{
		last_polled_ss_index = 0;
	}

	ss_info = get_ssinfo_from_ssindex(last_polled_ss_index);
	if ((ss_info != NULL) && (ul_ss_allocation[last_polled_ss_index] == 0) && (num_ul_data_slots >= CDMA_ALLOC_SIZE))
	{
		ul_ss_allocation[last_polled_ss_index] = CDMA_ALLOC_SIZE;
		num_ul_data_slots -= CDMA_ALLOC_SIZE;
	}
#endif

#ifdef BR_ENABLE
	connection	*connection;
	/* service basic cid */
	for (ii = BASIC_CID_MIN_VALUE; (ii <= param_MAX_VALID_BASIC_CID) & (num_ul_data_slots > 0); ii++)
	{
		connection = find_connection(ii);
		if ((connection != NULL) && (connection->min_reserved_traffic_rate > 0))
		{
			get_ss_mcs(connection->owner->ss_index, &dl_mcs_type, &ul_mcs_type);
			bytes_needed =  connection->min_reserved_traffic_rate / 1000;
			slots_needed = ceil((float)(bytes_needed * 8)/(bits_per_car[ul_mcs_type] * UL_DATA_CAR_PER_SLOT));
			ul_ss_allocation[connection->owner->ss_index] = slots_needed;
			num_ul_data_slots -= slots_needed;
		}
	}

	/* service primary cid */
	for (ii = PRIMARY_CID_MIN_VALUE; (ii <= max_valid_primary_cid) & (num_ul_data_slots > 0); ii++)
	{
		connection = find_connection(ii);
		if ((connection != NULL) && (connection->min_reserved_traffic_rate > 0))
		{
			get_ss_mcs(connection->owner->ss_index, &dl_mcs_type, &ul_mcs_type);
			bytes_needed =  connection->min_reserved_traffic_rate / 1000;
			slots_needed = ceil((float)(bytes_needed * 8)/(bits_per_car[ul_mcs_type] * UL_DATA_CAR_PER_SLOT));
			ul_ss_allocation[connection->owner->ss_index] = slots_needed;
			num_ul_data_slots -= slots_needed;
		}
	}
#endif

  // Serve all UGS connections first
  // IMP: currently assuming that the number of UGS connections in the UL and DL are the same (paired)
  // UGS bandwidth is allocated under this assumption to the CID of the SS corresponding to the DL UGS CIDs
  // If they are different, FIXME. Maybe take a parameter for num UL UGS CIDs and the starting CID
  
#ifdef ROUND_ROBIN
//   if(ul_last_allocated_cid>0)
   {
	    cid_offset=ul_last_allocated_cid-UL_UGS_CID_MIN_VALUE+1;
      // FLOG_INFO("cid_offeset is %d, last allocated cid is %d,UL_UGS_CID_MIN_VALUE is %d\n",cid_offset,ul_last_allocated_cid,UL_UGS_CID_MIN_VALUE);
  //    ul_last_allocated_cid=0;
   }

#endif
  
  for (ii = UL_UGS_CID_MIN_VALUE; (ii <= max_valid_ul_ugs_cid) & (num_ul_data_slots > 0); ii++)
  {
  #ifdef ROUND_ROBIN
        current_cid=ii+cid_offset;
        //if(ii==UL_UGS_CID_MIN_VALUE)
        //{
        //    FLOG_INFO("@@@@@@@@first enter,schedule start cid is %d, offset is %d\n",current_cid,cid_offset);
        //}
	      if(current_cid>max_valid_ul_ugs_cid)
        {
    	      current_cid = current_cid-max_valid_ul_ugs_cid+UL_UGS_CID_MIN_VALUE-1;
        }

	      tem_cid=ii;
	      ii=current_cid;
  #endif

	    pthread_mutex_lock(&bs_ss_list_mutex);
      get_ss_index(ii, &ss_index);
      get_service_flow(ii, &sflow); 
	    pthread_mutex_unlock(&bs_ss_list_mutex);
	    if (sflow == NULL)
          {
          #ifdef ROUND_ROBIN
              ii=tem_cid;
          #endif
              FLOG_ERROR("sflow == NULL\n");
              continue;
          }

      ugs_info_index = ii - UL_UGS_CID_MIN_VALUE;

//	  time_since_last_alloc = (frame_number - ul_ugs_cid_info[ugs_info_index].last_dequeued_frame_number) * frame_duration[FRAME_DURATION_CODE];
/*
      if (ul_ugs_cid_info[ugs_info_index].last_dequeued_frame_number < 0)
      {
          time_since_last_alloc = sflow->unsolicited_grant_interval;
          ul_ugs_cid_info[ugs_info_index].overall_deficit = 0;
      }else
      {
//          time_since_last_alloc = (get_current_frame_number() - ul_ugs_cid_info[ugs_info_index].last_dequeued_frame_number) * frame_duration[FRAME_DURATION_CODE];

          time_since_last_alloc = sflow->unsolicited_grant_interval;
          ul_ugs_cid_info[ugs_info_index].overall_deficit = 0;
      }

	  // time_since_last_alloc & ug_interval are in in millisec
	  if ((time_since_last_alloc < sflow->unsolicited_grant_interval) && (ul_ugs_cid_info[ugs_info_index].overall_deficit <= 0))
	  {
		FLOG_ERROR("for UL cid: %d, %d < %d, contg\n", ii, time_since_last_alloc, sflow->unsolicited_grant_interval);
		continue;
	  }
*/
      // In Service flow QoS parameters, units of traffic rates are bytes per sec

      time_since_last_alloc = frame_duration[FRAME_DURATION_CODE];

      bytes_needed =  time_since_last_alloc * sflow->min_reserved_traffic_rate/1000;

//      bytes_needed +=  ul_ugs_cid_info[ugs_info_index].overall_deficit;
      // mcs_type = get_mcs(ss_index);

	  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);

      slots_needed=ceil((float)(bytes_needed * 8)/(bits_per_car[ul_mcs_type]*UL_DATA_CAR_PER_SLOT));
      #ifdef ROUND_ROBIN
      //FLOG_INFO("current CID is %d,min reserved traffic rate is %d ,slots_needed is %d,num_ul_data_slots is %d\n",ii,sflow->min_reserved_traffic_rate,slots_needed,num_ul_data_slots);
      #endif
      if (slots_needed < num_ul_data_slots)
	    {
	  // request completely served
	        ul_ss_allocation[ss_index] += slots_needed;
	        num_ul_data_slots -= slots_needed;
          ul_ugs_cid_info[ugs_info_index].overall_deficit = 0;
          
          #ifdef ROUND_ROBIN
//          if(slots_needed>4)
          {
             ul_last_allocated_cid=ii;
          //   FLOG_INFO("for loop true,current cid is %d\n",ii);
          }
          #endif
	    } 
      else
	    {
	    	  #ifdef ROUND_ROBIN
	    	  if(num_ul_data_slots > 20)
          {
             ul_last_allocated_cid=ii;
            //  FLOG_INFO("for loop false,current cid is %d\n",ii);
          }
	    	  #endif
	        ul_ss_allocation[ss_index] += num_ul_data_slots;
	        bytes_allocated = (num_ul_data_slots * bits_per_car[ul_mcs_type] * UL_DATA_CAR_PER_SLOT/8);
	        num_ul_data_slots = 0;
	        ul_ugs_cid_info[ugs_info_index].overall_deficit = bytes_needed - bytes_allocated;
	    }
	        #ifdef ROUND_ROBIN
	           ii=tem_cid;
	        #endif
          ul_ugs_cid_info[ugs_info_index].last_dequeued_frame_number = get_current_frame_number();
      }

  // Right now, only traffic types UGS and BE are supported
  // Serve BE requests if all slots aren't already exhausted
  if(num_ul_data_slots>0)
    {
      br_q=br_q_list[q_index=SERVICE_BE];
      if(br_q->br_num==0)
	{
	  FLOG_DEBUG("No BE packets to schedule. Returning\n");
	}
      else
	{
	  // Lock the queue before dequeuing
	  pthread_mutex_lock(&(br_q->qmutex));
	  // If any connection's need is less than the fair share, allocate
	  // as much as it needs and return the balance to the available pool
	  do{
	    // Split the available slots between all BE connections equally 
	    fair_share=num_ul_data_slots/br_q->br_num;

	    // If number of available slots is so small that the fair share is
	    // less than one slot, just serve the BE bandwidth requests in order
	    if (fair_share==0)
	      {
		serve_fifo(br_q, ul_ss_allocation, &num_ul_data_slots);
	      }
	    else
	      {
		br_el=br_q->head;
		
		for (ii=0;ii<br_q->br_num;ii++)
		  {
		    get_ss_index(br_el->cid, &ss_index);

		    // TODO: This will be replaced by a method that returns the   
		    // UL/DL MCS scheme given the ss mac addr or SS 
		    // mcs_type = get_mcs(ss_index);
			get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);

		    slots_needed=ceil((float)(br_el->bandwidth_requested * 8)/(bits_per_car[ul_mcs_type]*UL_DATA_CAR_PER_SLOT));

		    if(slots_needed<fair_share)
		      {
			ul_ss_allocation[ss_index]+=slots_needed;
			num_ul_data_slots-=slots_needed;
			br_q->total_br -= br_el->bandwidth_requested;
			br_el->bandwidth_requested = 0;
		      }
		    else
		      {
			ul_ss_allocation[ss_index]+=fair_share;
			int bytes_allocated = floor(fair_share* bits_per_car[ul_mcs_type] * UL_DATA_CAR_PER_SLOT/8);
                        br_el->bandwidth_requested -=  bytes_allocated;
                        br_q->total_br -= bytes_allocated;
			num_ul_data_slots -= fair_share;
		      }
		    br_el=br_el->next;
		  }

		// To avoid the overhead of acquiring lock many times and 
		// dequeuing one at a time, we have a function cleanup_br_queue
		// which acquires the lock once and dequeues all with br=0
		cleanup_br_queue(br_q);

	      }
	  } while ((num_ul_data_slots>0) && (br_q->br_num != 0));
	  pthread_mutex_unlock(&(br_q->qmutex));
	}
  }
  // Use the allocation given by ul_scheduler to construct the ULMAP
  ul_map_builder(ul_ss_allocation, num_ul_symbols, num_ss, ul_map, p_list_head);
  free(ul_ss_allocation);
  ul_ss_allocation = NULL;
  return 0;
}

int serve_fifo(br_queue *br_q, int* ul_ss_allocation, int* num_ul_data_slots)
{
  int ii, slots_needed=0,ss_index=0, num_el_to_dequeue=0;
  int num_ul_ds = *num_ul_data_slots;
  long int bytes_allocated=0;
  BOOL flag=TRUE;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;

  bw_req_el* br_el=br_q->head;

  for (ii=0;(ii<br_q->br_num) && (flag==TRUE);ii++)
    {
      get_ss_index(br_el->cid, &ss_index);
      
      // TODO: This will be replaced by a method that returns the   
      // UL/DL MCS scheme given the ss mac addr or SS 
      // mcs_type = get_mcs(ss_index);

	  get_ss_mcs(ss_index, &dl_mcs_type, &ul_mcs_type);
      slots_needed=ceil((float)(br_el->bandwidth_requested * 8)/(bits_per_car[ul_mcs_type]*UL_DATA_CAR_PER_SLOT));

      if (slots_needed < num_ul_ds)
	{
	  // request completely served
	  ul_ss_allocation[ss_index]+=slots_needed;
	  num_ul_ds -= slots_needed;
	  num_el_to_dequeue++;
	  br_el=br_el->next;
	}
      else
	{
	  // This BR can't be served totally. Give a partial allocation and 
	  // update bandwidth_requested for this element
	  bytes_allocated=(num_ul_ds*bits_per_car[ul_mcs_type]*UL_DATA_CAR_PER_SLOT/8);
	  
	  // Acquire lock before modifying values in queue
	  pthread_mutex_lock(&(br_q->qmutex));
	  br_el->bandwidth_requested -= bytes_allocated;
	  br_q->total_br -= bytes_allocated;

	  // Release lock
	  pthread_mutex_unlock(&(br_q->qmutex));

	  // This BR was partially served with all the remaining slots
	  ul_ss_allocation[ss_index] += num_ul_ds;
	  num_ul_ds=0;
	  flag=FALSE;
	}
    }

  // dequeue all served elements in one go to avoid function calling & locking
  // overheads
  dequeue_br_queue(br_q, num_el_to_dequeue);
  *num_ul_data_slots=num_ul_ds;

  return 0;
}

int initialize_ulmap_list_for_ss()
{
  int frame_no = 0;
  int ul_ss_allocation[2]={50, 0};
  ulmap_ccc = 0;
  for (frame_no = 0; frame_no < NUM_ULMAP_STORED; frame_no++)
  {
    ul_map_msg *ulmap = (ul_map_msg*)mac_malloc(sizeof(ul_map_msg));
    // TODO: num_ul_symbols currently hardcoded for WNC test setup. make configurable later
	ul_map_builder(ul_ss_allocation, 15, 1, ulmap, NULL);
    set_ul_map_msg(ulmap, frame_no);
  }
  return 0;
}

// This function helps in testing/debugging the ULMAP
int print_ulmap(ul_map_msg *ul_map)
{
    ul_map_ie *ie = ul_map->ie;
    mimo_ul_basic_ie *mubi;
    printf("Starting print_ulmap for frame number %ld: UCD CCC: %d, Number of ULMAP IEs: %d\n", get_current_frame_number(), ul_map->ucd_count, ul_map->ulmap_ie_num);
    printf("Now traversing the ULMAP-IEs:\n");
    while (ie != NULL)
    {
        printf("IE index: %d, CID: %d, UIUC: %d\n", ie->ie_index, ie->cid, ie->uiuc);

        // These IEs are not supported yet
        if (ie->uiuc_13_ie != NULL || ie->uiuc_15_ie != NULL || ie->uiuc_0_ie)
        {
            printf("Error: Unexpected IE\n");
            return -1;
        }
		if(ie->uiuc == 12)
		{
			if(ie->uiuc_12_ie != NULL)
			{
				printf("OFDMA symbol offset: %d, Subchannel offset: %d, Num OFDMA symbols: %d, Num subchannels: %d, Ranging method: %d, Ranging Indicator: %d\n", \
			 	ie->uiuc_12_ie->ofdma_symbol_offset, ie->uiuc_12_ie->subchannel_offset, \
			 	ie->uiuc_12_ie->ofdma_symbol_num, ie->uiuc_12_ie->subchannel_num, \
			 	ie->uiuc_12_ie->ranging_method, ie->uiuc_12_ie->dedicated_ranging_indicator);
			}
			else
			{
				printf("In print_ulmap: Ranging IE (UIUC = 12) is NULL\n");
			}
		}
		else if (ie->uiuc == 14)
		{
			if(ie->uiuc_14_ie != NULL)
			{
				printf("CDMA_alloc_IE: Duration: %d, UIUC: %d, RCI:%d, frame_num_index: %d, Ranging Code: %d, Ranging symbol: %d, Ranging subchannel: %d, BWreq: %d\n", ie->uiuc_14_ie->duration, ie->uiuc_14_ie->uiuc, ie->uiuc_14_ie->repetition_coding_indication, ie->uiuc_14_ie->frame_num_index, ie->uiuc_14_ie->ranging_code, ie->uiuc_14_ie->ranging_symbol, ie->uiuc_14_ie->ranging_subchannel, ie->uiuc_14_ie->bw_request_mandatory);
			}
			else
			{
				printf("In print_ulmap: CDMA alloc IE (UIUC = 14) is NULL\n");
			}
		}
        else if (ie->uiuc == 11)
        {
            if(ie->uiuc_other_ie != NULL) 
            {
                printf("Error: Normal IE present with UIUC=11");
                return -1;
            }
            printf("Extended UIUC: %d, Length: %d\n", ie->uiuc_extend_ie->extended_uiuc, ie->uiuc_extend_ie->length);
            if(ie->uiuc_extend_ie->unspecified_data != NULL)
            {
                switch(ie->uiuc_extend_ie->extended_uiuc)
                {
                // TODO: Need to add more case blocks for other types of extd UIUC IDs
                case 9: // MIMO_UL_BASIC_IE
                    mubi = ie->uiuc_extend_ie->unspecified_data;
                    printf("Number of assigned regions: %d, ", mubi->num_assign);
                        assigned_burst_attri *aba = mubi->assigned_burst_header;
                    while(aba != NULL)
                    {
                        printf("CSM: %d, Duration: %d, CID: %d, UIUC: %d\n", aba->collaborative_sm_indication, aba->duration, aba->cid, aba->uiuc);
                        aba = aba->next;
                        
                    }
                    break;
                default:
                    printf("Can't print extended UIUC IE. Unknown type: %d\n", ie->uiuc_extend_ie->extended_uiuc);
                    return -1;
                }
            }
        }
        else
        {
            if(ie->uiuc_extend_ie != NULL) 
            {
                printf("Error: UIUC 11 IE present with UIUC=1 to 10");
                return -1;
            }
            if (ie->uiuc_other_ie != NULL)
            {
                printf("Duration: %d, RCI: %d, Slot offset: %d\n", ie->uiuc_other_ie->duration, ie->uiuc_other_ie->repetition_coding_indication, ie->uiuc_other_ie->slot_offset);
            }
        }
        ie = ie->next;
    }
    return 0;
}

// This function constructs the UL Map, given the allocation from ul_scheduler 
int ul_map_builder(int* ul_ss_allocation, int num_ul_symbols, int num_ss, ul_map_msg* ul_map, ranging_adjust_ie *p_list_head)
{
  int ii, ie_index = 0, basic_cid = 0;

  ModulCodingType dl_mcs_type = param_DL_MCS;
  ModulCodingType ul_mcs_type = param_UL_MCS;
  int uiuc_code = (int)(param_UL_MCS) + 1;

  ranging_adjust_ie *rng_ie_tmp = NULL;
  // Mgmt message type of ULMAP is 3
  ul_map->manage_msg_type = 3;

  // This count will be generated and maintained by the UCD builder module
  ul_map->ucd_count = ulmap_ccc;

  ul_map->num_ofdma_sym = num_ul_symbols;
  // The fixed fields defined above occupy 8 bytes
  // Plus the overhead of packing ULMAP (GMH, CRC) in a MAC PDU addressed to 
  // broadcast CID
  // IMPORTANT: This ulmap_len_in_bytes will be slightly larger because we 
  // ceil the ULMAP-IEs with fractional byte sizes to next higher bytes 
  ulmap_len_in_bytes = PDU_MIN_OVERHEAD + ceil((float)UL_FIXED_FIELD_LENGTH/8);

  ul_map->ulmap_ie_num=0;
  ul_map_ie *ie=NULL, *last_ulmap_ie=NULL;

  // Add a ULMAP-IE for ranging subchannel
  ie=(ul_map_ie*)mac_malloc(sizeof(ul_map_ie));
  init_ulmap_ie(ie);

  ie->cid = BROADCAST_CID;
  ie->ie_index = ie_index;
  ie_index++;
  ie->uiuc = RANGING_UIUC;

  ie->uiuc_12_ie = (uiuc12_ie*)mac_malloc(sizeof(uiuc12_ie));
  ie->uiuc_12_ie->ofdma_symbol_offset = 0;
  ie->uiuc_12_ie->subchannel_offset = 0;
  ie->uiuc_12_ie->ofdma_symbol_num = NUM_RANGING_SYMBOLS;
  ie->uiuc_12_ie->subchannel_num = NUM_RANGING_SUBCHANNELS;
  ie->uiuc_12_ie->ranging_method = 0;
  ie->uiuc_12_ie->dedicated_ranging_indicator = 0;

  ulmap_len_in_bytes += ceil((float) RANGING_ULMAP_IE_LENGTH/8);
  ul_map->ie = ie;
  last_ulmap_ie = ie;
  ul_map->ulmap_ie_num++;
 
  while (p_list_head != NULL)
  {
	ie=(ul_map_ie*)mac_malloc(sizeof(ul_map_ie));
	init_ulmap_ie(ie);
	last_ulmap_ie->next=ie;
    last_ulmap_ie = ie;
    ie->ie_index = ie_index;
	ie_index++;
	ie->cid = INIT_RNG_CID;
	ie->uiuc = CDMA_ALLOC_UIUC;
	ie->uiuc_14_ie = (cdma_alloc_ie*)mac_malloc(sizeof(cdma_alloc_ie));
    ie->uiuc_14_ie->duration = CDMA_ALLOC_SIZE;
    ie->uiuc_14_ie->uiuc = 1;
    ie->uiuc_14_ie->repetition_coding_indication = 0;
    ie->uiuc_14_ie->frame_num_index = p_list_head->frame_num_index;
    ie->uiuc_14_ie->ranging_code = p_list_head->ranging_code;
    ie->uiuc_14_ie->ranging_symbol = p_list_head->ranging_symbol;
    ie->uiuc_14_ie->ranging_subchannel = p_list_head->ranging_subchannel;
    ie->uiuc_14_ie->bw_request_mandatory = 0;

    ul_map->ulmap_ie_num++;
	ulmap_len_in_bytes += ceil((float)CDMA_ALLOC_IE_LEN_BITS/8);

	rng_ie_tmp = p_list_head;
	p_list_head = p_list_head->p_next;
	free(rng_ie_tmp);
  }
  // assume all special UIUCs - 0, 12-15 are processed first then all SS 
  // allocations are processed together

  // For UIUC 1 to 10
  for(ii=0;ii<num_ss;ii++)
    {
	  get_ss_mcs(ii, &dl_mcs_type, &ul_mcs_type);
	  uiuc_code = (int)ul_mcs_type + 1;

      if(ul_ss_allocation[ii]>0)
	{
	  // If this SS has any bandwidth allocated in the current UL subframe
	  // construct a ULMAP-IE for it
	  ie=(ul_map_ie*)mac_malloc(sizeof(ul_map_ie));
	  init_ulmap_ie(ie);

	  if(ul_map->ulmap_ie_num==0) {ul_map->ie=ie;}
	  else {last_ulmap_ie->next=ie;}

	  basic_cid = get_basic_cid_from_ss_index(ii);
	  ie->cid=basic_cid;
	  ie->ie_index = ie_index;
	  ie_index++;
      if (param_UL_STC_MATRIX_TYPE !=-1)
      {
        // MIMO enabled. Insert MIMO UL Basic IE. UIUC = 11 means extended 2 UIUC
    	ie->uiuc=11;
        ie->uiuc_extend_ie = (extended_uiuc_ie*)mac_malloc(sizeof(extended_uiuc_ie));
        ie->uiuc_extend_ie->extended_uiuc = MIMO_UL_BASIC_IE;

        // 4 bits for num of assign bursts (=1 per SS acc to current algo). CSM not supported yet
        // TODO: Although there is no mention of padding in the spec, since length of Ext2 UIUC IEs are 
        // given in bytes, padding should be needed
        ie->uiuc_extend_ie->length = ceil((float)(4 + BURST_ATTR_STC_LEN)/8);

        mimo_ul_basic_ie *mubi = (mimo_ul_basic_ie*)mac_malloc(sizeof(mimo_ul_basic_ie));
        ie->uiuc_extend_ie->unspecified_data = mubi;
        
        mubi->num_assign = 0; // In current algorithm only one burst per SS. May change after AMC is implemented

        assigned_burst_attri *burst_attr = (assigned_burst_attri*)mac_malloc(sizeof(assigned_burst_attri));
        mubi->assigned_burst_header = burst_attr;
        burst_attr->next = NULL; 
        //Collaborative SM is not supported yet
        burst_attr->collaborative_sm_indication = 0;

        burst_attr->cid = basic_cid;
	    // The UIUC value will be provided by AMC module. Currently, since
    	// we choose the same uplink burst profile for all CID's, 
		// set to param_UL_MCS + 1
        burst_attr->uiuc = uiuc_code;
        burst_attr->repetition_coding_indication = 0;

        if (param_UL_STC_MATRIX_TYPE == 0)
        {
            // If Matrix type A
            burst_attr->mimo_control = 0;
        }
        else
        {
            // If Matrix Type B
            burst_attr->mimo_control = 1;
        }
        burst_attr->duration = ul_ss_allocation[ii];

	    // 16(CID) +4(UIUC) + 4 (Ext2 UIUC) + 8(Length) = 32 bits = 4 bytes taken up by the ULMAP-IE and Ext2 IE headers
	    ulmap_len_in_bytes = ulmap_len_in_bytes + ie->uiuc_extend_ie->length + 4;

      }
      else
      {

	  // The UIUC value will be provided by AMC module. Currently, since
	  // we choose the same uplink burst profile for all CID's, use the
	  // value from settings file
	  ie->uiuc = uiuc_code;
	  ie->uiuc_other_ie = (other_uiuc_ie*)mac_malloc(sizeof(other_uiuc_ie));
	  ie->uiuc_other_ie->duration=ul_ss_allocation[ii];

	  // Currently assume no repetition coding. According to PHY feedback, can 
	  // use repetition coding but UL scheduler proportionately needs to
	  // allocate more resources
	  ie->uiuc_other_ie->repetition_coding_indication=0;
		
	  // Not used here - relevant only for AAS or AMC. Set to 0
	  ie->uiuc_other_ie->slot_offset = 0;
		
	  // Since data allocations have 16+4+10+2 = 32 bits = 4 bytes, 
	  // padding nibble is not needed for UIUC=1 to 10
	  ulmap_len_in_bytes += (ULMAP_IE_LENGTH/8);

      }
	  ul_map->ulmap_ie_num++;
#ifdef FOAK_TEMP_FIX
		// MIMO not supported in FOAK
	  if (ulmap_len_in_bytes + (ULMAP_IE_LENGTH/8)> num_bytes_in_dlslotsymbol)
	  {break;}
#endif
	  last_ulmap_ie=ie;
	}
    }
#ifdef DDEBUG
  if (ul_map!=NULL) print_ulmap(ul_map);
  else printf("ULMAP is NULL\n");
#endif
  return 0;
}

// This function will be used by the SS side
int get_ss_allocation(long int frame_num, int basic_cid, short *uiuc, short *config_count, int *num_allotted_slots)
{
  ul_map_msg *ulmap = NULL;
  ul_map_ie *ulmap_ie = NULL;

  // Get the ULMAP corresponding to the current frame number
  get_ul_map_msg(&ulmap, frame_num);
  if (!ulmap)
    {
      FLOG_ERROR("No ULMAP found for this frame number, returning ..");
      return -1;
    }
  (*config_count) = ulmap->ucd_count;
  ulmap_ie = ulmap->ie;
  while(ulmap_ie)
    {
      if (ulmap_ie->cid == basic_cid)
      {
		*uiuc = ulmap_ie->uiuc;
        *num_allotted_slots = ulmap_ie->uiuc_other_ie->duration;
      }
      ulmap_ie = ulmap_ie->next;
    }
  // If a ULMAP IE corresponding to this basic CID was not found, no allocation for this SS in the current frame
  return 0;
}


int dump_ulmap(FILE *fp_ulmap, ul_map_msg *ul_map)
{
    ul_map_ie *ie = ul_map->ie;
    mimo_ul_basic_ie *mubi;
//    static FILE *fp_ulmap;
//    static long loop_count = 0;
//    if ( loop_count ++ == 0)
//    {
//        fp_ulmap = fopen("ul_map.out", "w+");
//    }

    fprintf(fp_ulmap,"\n----------Start Ulmap, FrameNumber:%ld: UCD CCC:%d, UlMapIeNumber:%d-----------\n", get_current_frame_number(), ul_map->ucd_count, ul_map->ulmap_ie_num);
   //fprintf(fp_ulmap,"Now traversing the ULMAP-IEs:\n");
    while (ie != NULL)
    {
        fprintf(fp_ulmap,"IE index:%d, CID:%d, UIUC:%d\n", ie->ie_index, ie->cid, ie->uiuc);

        // These IEs are not supported yet
        if (ie->uiuc_13_ie != NULL || ie->uiuc_15_ie != NULL || ie->uiuc_0_ie)
        {
            fprintf(fp_ulmap,"Error: Unexpected IE\n");
            return -1;
        }
        if(ie->uiuc == 12)
        {
            if(ie->uiuc_12_ie != NULL)
            {
                fprintf(fp_ulmap,"SymbolOffset:%d, SubChanOffset:%d, SymbolNum:%d, SubChanNum:%d, RangingMethod:%d, RangingIndicator:%d\n", \
                                ie->uiuc_12_ie->ofdma_symbol_offset, ie->uiuc_12_ie->subchannel_offset, \
                                ie->uiuc_12_ie->ofdma_symbol_num, ie->uiuc_12_ie->subchannel_num, \
                                ie->uiuc_12_ie->ranging_method, ie->uiuc_12_ie->dedicated_ranging_indicator);
            }
            else
            {
                fprintf(fp_ulmap,"In print_ulmap: Ranging IE (UIUC = 12) is NULL\n");
            }
        }
        else if (ie->uiuc == 14)
        {
            if(ie->uiuc_14_ie != NULL)
            {
                fprintf(fp_ulmap,"CDMAIE: Duration:%d, UIUC:%d, RCI:%d, FrameNum:%d, RangingCode:%d, RangingSymbol:%d, RangingSubchannel: %d, BWreq: %d\n",\
                        ie->uiuc_14_ie->duration, ie->uiuc_14_ie->uiuc, ie->uiuc_14_ie->repetition_coding_indication, ie->uiuc_14_ie->frame_num_index, \
                        ie->uiuc_14_ie->ranging_code, ie->uiuc_14_ie->ranging_symbol, ie->uiuc_14_ie->ranging_subchannel, ie->uiuc_14_ie->bw_request_mandatory);
            }
            else
            {
                fprintf(fp_ulmap,"In print_ulmap: CDMA alloc IE (UIUC = 14) is NULL\n");
            }
        }
        else if (ie->uiuc == 11)
        {
            if(ie->uiuc_other_ie != NULL)
            {
                fprintf(fp_ulmap,"Error: Normal IE present with UIUC=11");
                return -1;
            }

            fprintf(fp_ulmap,"Extended UIUC: %d, Length: %d\n", ie->uiuc_extend_ie->extended_uiuc, ie->uiuc_extend_ie->length);
            if(ie->uiuc_extend_ie->unspecified_data != NULL)
            {
                switch(ie->uiuc_extend_ie->extended_uiuc)
                {
                     // TODO: Need to add more case blocks for other types of extd UIUC IDs
                    case 9: // MIMO_UL_BASIC_IE
                        mubi = ie->uiuc_extend_ie->unspecified_data;
                        fprintf(fp_ulmap,"Number of assigned regions: %d, ", mubi->num_assign);
                        assigned_burst_attri *aba = mubi->assigned_burst_header;
                        while(aba != NULL)
                        {
                            fprintf(fp_ulmap,"CSM: %d, Duration: %d, CID: %d, UIUC: %d\n", aba->collaborative_sm_indication, aba->duration, aba->cid, aba->uiuc);
                            aba = aba->next;
                        }
                        break;
                    default:
                        fprintf(fp_ulmap,"Can't print extended UIUC IE. Unknown type: %d\n", ie->uiuc_extend_ie->extended_uiuc);









                    return -1;
                }
            }
        }
        else
        {
            if(ie->uiuc_extend_ie != NULL)
            {
                fprintf(fp_ulmap,"Error: UIUC 11 IE present with UIUC=1 to 10");
                return -1;
            }
            if (ie->uiuc_other_ie != NULL)
            {
                fprintf(fp_ulmap,"Duration: %d, RCI: %d, Slot offset: %d\n",\
                        ie->uiuc_other_ie->duration, ie->uiuc_other_ie->repetition_coding_indication, ie->uiuc_other_ie->slot_offset);
            }
        }
        ie = ie->next;
    }
    return 0;
}
