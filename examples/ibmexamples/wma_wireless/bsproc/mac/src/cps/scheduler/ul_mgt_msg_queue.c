/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_mgt_msg_queue.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <assert.h>
#include "debug.h"
#include "ul_mgt_msg_queue.h"
#include "mme_test.h"
#include "memmgmt.h"
#include "mac_message.h"
#include "mac_unit_test_normal.h"
#include "mac_bs_ranging_utm.h"
#include "mac_ss_ranging_utm.h"

extern sll_fifo_q *ranging_q;
extern sll_fifo_q *bs_ranging_q;
extern sll_fifo_q *bs_ranging_test_q;

mgt_msg_queue* ul_mgt_msg_queue_init() {
  int i;
  mgt_msg_queue* mgt_q = (mgt_msg_queue*)mac_malloc(MAX_MMM_CLASS*sizeof(mgt_msg_queue));

  if(!mgt_q){
      FLOG_FATAL("Can't mac_malloc for creating queue in ul_mgt_msg_queue_init()! \n");
      return NULL;
  }

  memset(mgt_q, 0, MAX_MMM_CLASS*sizeof(mgt_msg_queue));

  for (i=0; i<MAX_MMM_CLASS; i++ ){
      if (pthread_mutex_init(&(mgt_q[i].qmutex),NULL)){
         FLOG_FATAL("Error initializing qmutex! \n");
      }
      if (pthread_cond_init(&(mgt_q[i].notempty),NULL)){
         FLOG_FATAL("Error initializing condition variable notempty! \n");
      }
  }
  ul_msg_queue = mgt_q;
  return(mgt_q);
}

int free_ul_mm_queue(mgt_msg_queue **p_mgt_q)
{
	mgt_msg_queue *mgt_q = *p_mgt_q;
	mgt_msg *msg = NULL;
	int i = 0;
	for (i=0; i<MAX_MMM_CLASS; i++ )
	{
		while(mgt_q[i].length != 0)
		{
			msg = dequeue_ul_mmq_nowait(&(mgt_q[i]));
			if (msg == NULL) {break;}
			free(msg->data);
			free(msg);
		}
		// Signal the function that might be waiting on this empty q to unblock
		// Assumption: there can only be one such function
		// pthread_cond_signal(&(mgt_q[i].notempty));
		pthread_cond_destroy(&(mgt_q[i].notempty));
		pthread_mutex_destroy(&(mgt_q[i].qmutex));
	}

	free(mgt_q);
	*p_mgt_q = NULL;
	return 0;
}

int enqueue_specific_queue(mgt_msg_queue* mgt_queue, int frame_num, int cid, int msg_type, int length, void* data){
  mgt_msg *msg= (mgt_msg*)mac_malloc(sizeof(mgt_msg));
  if(!msg){
      FLOG_FATAL("Can't malloc! \n");
  }
  
  msg->cid = cid;
  msg->msg_type = msg_type; 
  msg->length = length;
  msg->data = data;
  msg->rcv_frame_num = frame_num;
  
  pthread_mutex_lock(&(mgt_queue->qmutex));
  //DEBUG("enqueue : got lock \n");

  msg->next=NULL;
  if (mgt_queue->tail!=NULL){
    mgt_queue->tail->next=msg;
    msg->prev = mgt_queue->tail;  // update the prev of new node
  }
  mgt_queue->tail=msg;
  if (mgt_queue->head==NULL)
    mgt_queue->head=msg;
  (mgt_queue->length)++;


  pthread_cond_signal(&(mgt_queue->notempty));
  //DEBUG("enqueue : signalled not-empty \n");

  pthread_mutex_unlock(&(mgt_queue->qmutex));
  //DEBUG("enqueue : released lock \n");

  return 0;
}

  // IMPORTANT: enqueue() does not copy the data - just duplicates the pointer

int enqueue_ul_mgt_msg(mgt_msg_queue* mgt_queue, int frame_num, int cid, int msg_type, int length, void* data){

  // firstly, copy the payload to a new memory region
  u_int8_t* msg_payload = NULL;
  logical_packet* lp;
  logical_element* le;
  logical_element* next_le;
  int offset = 0;
#ifndef SS_TX
#ifndef SS_RX
   int type=0;  
#endif
#endif
  //mgt_msg* msg;
  int index;
#ifdef SS_TX
#ifdef SS_RX
  int kk=0;
  ucd_msg *ucd = NULL;
  dcd_msg *dcd = NULL;
#endif
#endif
  ul_map_msg *ul_map = NULL;
  msg_payload = (u_int8_t* )mac_malloc(length);
  if (msg_type == FRAG_MANAGEMENT_MESSAGE){
      lp = (logical_packet*) data;
      le = lp->element_head;
 
      while (le){
          next_le = le->next;
          memcpy(msg_payload+offset, le->data, le->length);
          offset += le->length;
          free(le->data);
          free(le);
          le = next_le;
      }
      free(lp);
      msg_type = (u_int8_t)(msg_payload[0]);
  }
  else 
  {
      memcpy (msg_payload, data, length);
      if (msg_type == UNFRAG_MANAGEMENT_MESSAGE)
     {
         msg_type = msg_payload[0];    
     }
  }
  FLOG_DEBUG("\nEnqueuing UL mgmt msg for cid: %d, msg_type: %d", cid, msg_type);
#ifdef SS_TX
#ifdef SS_RX
  rng_req_msg *rng_req = NULL;
#endif
#endif
  switch(msg_type){
      case UCD:
	  // Parse and store the UCD only in the SS uplink. If received at BS, discard 
	  #ifdef SS_RX
	  ucd = (ucd_msg*)mac_malloc(sizeof(ucd_msg));
	  parse_ucd(msg_payload, length, ucd);
	  FLOG_DEBUG("\nFinished parsing UCD");
	  set_ucd_msg(ucd);
	  #endif

	  /****** Begin Test ********
	  ucd_msg *stored_ucd = ucd_msg_query(ucd->configuration_change_count);
	  printf("\nNow printing UCD 1 (parsed):");
	  print_ucd_msg(ucd);
	  printf("\nNow printing UCD 2 (stored):");
	  print_ucd_msg(stored_ucd);
	  if(err_code = compare_ucd (ucd, stored_ucd))
		{
		printf("Error: %d. The stored and parsed UCDs don't match.\n", err_code);
		return -1;
		}
	  ****** End Test ********/
		  free(msg_payload);
          break;
      case DCD:
	  // Parse and store the DCD only in the SS uplink. If received at BS, discard 
	  #ifdef SS_RX
	  dcd = (dcd_msg*)mac_malloc(sizeof(dcd_msg));
	  parse_dcd(msg_payload, length, dcd);
	  FLOG_DEBUG("\nFinished parsing DCD");
	  set_dcd_msg(dcd);
	  #endif

	  /****** Begin Test ********
	  dcd_msg *stored_dcd = dcd_msg_query(dcd->configuration_change_count);
	  printf("\nNow printing DCD 1 (parsed):");
	  print_dcd_msg(dcd);
	  printf("\nNow printing DCD 2 (stored):");
	  print_dcd_msg(stored_dcd);
	  if(err_code = compare_dcd (dcd, stored_dcd))
		{
		printf("Error %d: The stored and parsed DCDs don't match.\n", err_code);
		return -1;
		}
	  ****** End Test ********/
		  free(msg_payload);
          break;
      case DLMAP:
          // DLMAP will be processed by PHY, not used by MAC. Discard.
		  free(msg_payload);	
	  break;
      case ULMAP:
	  // Parse and store the ULMAP only in the SS uplink. If received at BS, discard 
	  #ifdef SS_RX
		ul_map = (ul_map_msg*)mac_malloc(sizeof(ul_map_msg));
		parse_ul_map_msg(msg_payload, length, ul_map);
		FLOG_DEBUG("\nFinished parsing ULMAP");
		set_ul_map_msg(ul_map, frame_num);
		FLOG_DEBUG("\nFinished saving ULMAP");

		// At the SS, the rcvd ULMAP controls the timing of UL (TX) subframe
		// Currently this logic is not the best: ul_map is stored in a list 
		// (above) and also passed in a queue (below). But actually this queue
		// is only used for synching the UL tx subframe, the ul_map is retrieved
		// from the list only. this is for uniformity between the integration
		// and MAC unit test codes. It is possible to pass a null ULMAP in the 
		// queue below for integration_test code, only frame_num is used.
		#ifdef INTEGRATION_TEST
        enqueue_specific_queue(&mgt_queue[ULMAP_MMM_INDEX], frame_num, cid, msg_type, length, ul_map);
		#endif
	  #else
	  #ifdef RANGING_TEST
		ul_map = (ul_map_msg*)mac_malloc(sizeof(ul_map_msg));
		parse_ul_map_msg(msg_payload, length, ul_map);
		//printf("Printing ULMAP at the output of the parser in the UL:\n");
		//print_ulmap(ul_map);
		free_ulmap(ul_map);
		free(ul_map);
		ul_map = NULL;
	  #endif
	  #endif 

	  /****** Begin Test ********
	  ul_map_msg *stored_ulmap_msg;
	  get_ul_map_msg(&stored_ulmap_msg, frame_num);
	  printf("\nNow printing ULMAP 1 (parsed):");
	  print_ulmap(ul_map);
	  printf("\nNow printing ULMAP 2 (stored):");
	  print_ulmap(stored_ulmap_msg);
	  if(compare_ul_map (ul_map, stored_ulmap_msg))
		{
		printf("Error: The stored and parsed ULMAPs don't match.\n");
		return -1;
		}
	  **** End Test *****/
		  free(msg_payload);	
          break;
//#endif
      case RNG_REQ:
		#ifndef SS_RX
		if (cid == INIT_RNG_CID)
			type = RNG_REQ_IR_CID;
		else
			type = RNG_REQ;
		sll_fifo_q_enqueue(bs_ranging_q, msg_payload, length, type);
		#else
			#ifdef RANGING_TEST
				parse_rng_req(msg_payload, length, &rng_req);
				if (rng_req->ss_mac.length != 6)
				{
					// This is not a valid SS MAC ID (6 bytes)
					FLOG_ERROR("ERROR: Invalid SS MAC ID in RNG_REQ. Ignoring\n");
				}
				else
				{
					FLOG_DEBUG("Received RNG_REQ with MAC ID: ");
					for (kk = 0; kk < 6; kk++)
						FLOG_DEBUG("%d.", rng_req->ss_mac.value[kk]);
				}
				free(rng_req);
				rng_req = NULL;
			#else
			FLOG_ERROR("Error: RNG_REQ message received in SS receive. Discarding\n");
			#endif
			free(msg_payload);
		#endif
		break;
      case RNG_RSP:
	    // Enqueue the RNG_RSP message into the ss ranging_q only if this is SS 
		// side processing, i.e. SS_TX and SS_RX are defined, otherwise mac_ss_ranging 
		// will not function properly. Discard at the BS_RX
		#ifdef SS_TX
			#ifdef SS_RX
				sll_fifo_q_enqueue(ranging_q, msg_payload, length, RNG_RSP);
			#endif
		#else
			#ifdef RANGING_TEST
				// Enqueue to the BS ranging Test module
				sll_fifo_q_enqueue(bs_ranging_test_q, msg_payload, length, RNG_RSP);
			#endif
		#endif
		break;
      case REG_REQ:
      case REGN_RSP:
      case SBC_REQ:
      case SBC_RSP:
          index = NETWORK_ENTRY_MMM_INDEX;
          enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;
      case PKM_REQ:
      case PKM_RSP:
#ifdef PKM_TEST
	  printf("Received PKM Message in UL_MGT_queue\n");
#endif
	  index = PKM_MMM_INDEX;
	  enqueue_specific_queue(&mgt_queue[index],frame_num,cid,msg_type, length, msg_payload);
	  break;
      case DSA_REQ:
      case DSA_RSP:
      case DSA_ACK:
      case DSC_REQ:
      case DSC_RSP:
      case DSC_ACK:
      case DSD_REQ:
      case DSD_RSP:
	  case DSX_RVD:
          index = DS_MMM_INDEX;
		  FLOG_DEBUG("Rxd in the UL, DS message type: %d\n",msg_type );
          enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;

      case PHY_CHANNEL_REPORT_HEADER:
      case CQICH_ALLOCATION_REQUEST_HEADER:
      case FEEDBACK_HEADER:
      case MIMO_CHANNEL_FEEDBACK_HEADER:
      case MIMO_MODE_FEEDBACK_EXTENDED_SUBHEADER:
      case REP_REQ:
      case REP_RSP:
      case BR_CINR_REPORT_HEADER:
          index =  CHANNEL_REPORT_MMM_INDEX;
          enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;
          
       case UL_TX_POWER_REPORT_EXTENDED_SUBHEADER:
       case MOB_SLP_REQ:
       case MOB_SLP_RSP:
       case PCM_REQ:
       case PCM_RSP:
       case BR_UL_TX_POWER_REPORT_HEADER:
       case  BR_UL_SLEEP_CONTROL_HEADER:
          index =  POWER_CONTROL_MMM_INDEX;
          enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;

       case MOB_BSHO_REQ:
       case MOB_MSHO_REQ:
       case MOB_BSHO_RSP:
       case MOB_HO_IND:
           index =  HO_MMM_INDEX;
          enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;
/*
       //case ARQ_FEEDBACK:
       //case ARQ_DISCARD:
       //case ARQ_RESET:
	case ARQ_FEEDBACK_IE:
           index =  ARQ_MMM_INDEX;
	   ARQ_feedback_message *tmp = (ARQ_feedback_message*)msg_payload;
	  //printf("feedback enqueued. type %d\n",tmp->mgmt_msg_type);
	   if (tmp->mgmt_msg_type == ARQ_FEEDBACK) parse_and_process_feedback_msg(tmp);
	   else if (tmp->mgmt_msg_type == ARQ_DISCARD)process_discard_msg((ARQ_discard_message*)tmp,fragq1);
	   else if (tmp->mgmt_msg_type == ARQ_RESET) process_reset_msg((ARQ_reset_message*)tmp);
	   else printf("Unknown ARQ Mgmt Message\n");
          //enqueue_specific_queue(&mgt_queue[index], frame_num, cid, msg_type, length, msg_payload);
          break;
  */        
       default:
          FLOG_DEBUG("uplink management message type not supported \n");

  }
  
  return 0;
}

mgt_msg *dequeue_ul_mmq_nowait(mgt_msg_queue *mgt_queue){

  assert(mgt_queue != NULL);
  pthread_mutex_lock(&(mgt_queue->qmutex));
  FLOG_DEBUG("dequeue : got lock \n");
  mgt_msg *msg = NULL;

  if(mgt_queue->length != 0){

		msg=mgt_queue->head;
		if (mgt_queue->head!=NULL){
			if(mgt_queue->length == 1){   // when there is a single element in the q
				mgt_queue->tail = NULL;
			}
			mgt_queue->head=mgt_queue->head->next; //move head
			if(mgt_queue->head!=NULL){  // update the new head's prev to NULL
				mgt_queue->head->prev = NULL;
			}
			(mgt_queue->length)--; // decrease length
		}
	}
	pthread_mutex_unlock(&(mgt_queue->qmutex));
	FLOG_DEBUG("dequeue : released lock \n");
	return msg;
}

//IMPORTANT: dequeue() does not deallocate the memory used by the mgt_msg that is dequeued

mgt_msg *dequeue_ul_mgt_msg_queue(mgt_msg_queue *mgt_queue){

  assert(mgt_queue != NULL);
  mgt_msg *msg;
  pthread_cleanup_push((void*)pthread_mutex_unlock, (void*)&(mgt_queue->qmutex));
  pthread_mutex_lock(&(mgt_queue->qmutex));
  FLOG_DEBUG("dequeue : got lock \n");

  while(mgt_queue->length == 0){
    FLOG_DEBUG("dequeue : queue empty - waiting for an enqueue \n");
    pthread_cond_wait(&(mgt_queue->notempty), &(mgt_queue->qmutex));
    FLOG_DEBUG("dequeue : got the enqueue signal \n");
  }

  msg=mgt_queue->head;
  if (mgt_queue->head!=NULL){
    if(mgt_queue->length == 1){   // when there is a single element in the q
       mgt_queue->tail = NULL;
    }
    mgt_queue->head=mgt_queue->head->next; //move head
    if(mgt_queue->head!=NULL){  // update the new head's prev to NULL
      mgt_queue->head->prev = NULL;
    }
    (mgt_queue->length)--; // decrease length
  }

  //two prints for testing
/*     printQ_HtoT(mgt_queue); */
/*     printQ_TtoH(mgt_queue); */

  pthread_mutex_unlock(&(mgt_queue->qmutex));
  pthread_cleanup_pop(0);
  FLOG_DEBUG("dequeue : released lock \n");

  return msg;
}


// for testing
void printQ_HtoT(mgt_msg_queue *myQ1){
  
  printf("HtoT:  ");
  if(myQ1 == NULL)
    return;
  mgt_msg *cur = myQ1->head;
  int count = 0;
  while(cur != NULL){
    printf(" %d (%d) :",count, cur->rcv_frame_num);
    cur = cur->next;
    count++;
  }
  printf("\n");

}


// for testing
void printQ_TtoH(mgt_msg_queue *myQ1){

  printf("TtoH:  ");
  if(myQ1 == NULL)
    return;
  mgt_msg *cur = myQ1->tail;
  int count = 0;
  while(cur != NULL){
    printf(" %d (%d):",count, cur->rcv_frame_num);
    cur = cur->prev;
    count++;
  }
  printf("\n");

}

int get_ul_mgt_msg_queue(mgt_msg_queue**ul_msgq){
    *ul_msgq = ul_msg_queue;
    return 0;
}

