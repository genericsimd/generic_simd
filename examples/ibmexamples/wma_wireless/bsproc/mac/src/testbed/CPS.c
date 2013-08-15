/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: CPS.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <string.h>
#include "mac.h"
#include "logical_packet.h"
#include "mac_sdu_queue.h"
#include "memmgmt.h"
#include "mac_hash.h"
#include "dl_exp_params.h"
#include "debug.h"

extern sdu_queue* dl_sdu_queue;
/*
void* CPS(void* param) {
  TRACE(10,"Starting MAC CPS layer threads ...");
  // Dequeue SDUs to mac sdu queue
  int i=0;
  sdu_cid_queue* cid_q=NULL;
  size_t total_bytes=0;
  for(i=0; i<100; ) {

    //get randomly a connection id
    int cid=rand()%10;

    //Schedule a mac pdu packet to be dequeued
    size_t num_bytes=rand()%1000 +1;
    size_t allowed_bytes=0;



    logical_packet* lp=NULL;
    int cid_q_indx=0;
    if(ht_is_value_present(cid)) {
      //dequeue it to the sdu queue
      cid_q_indx=ht_get_key(cid);
      if(dl_sdu_queue->sdu_cid_q[cid_q_indx]->sdu_num >0) {
	cid_q=dl_sdu_queue->sdu_cid_q[cid_q_indx];
	allowed_bytes=dequeue_transport_sdu_queue(dl_sdu_queue, cid, num_bytes, &lp);
      }
    }
    BOOL svcd=FALSE;
    char buf[1024];
    //deallocate the packet
    int len=0;
    while(lp!=NULL) {
      logical_packet* tmp=lp;
      lp=lp->next;
      sprintf(buf+len, " Element type: %d, Length:%d Blk type:%d start_bsn:%d num_bytes:%d allowed_bytes:%d cid_q->sdu_num:%d cid_q->head:%p cid_q->tail:%p lp->prev:%p, lp->next:%p \n",
	     tmp->element_head->type,
	     tmp->element_head->length,
	     tmp->element_head->blk_type,
	     tmp->element_head->start_bsn,
	      num_bytes,
	      allowed_bytes,
	      cid_q->sdu_num,
	      cid_q->head,
	      cid_q->tail,
	     tmp->prev,
	     tmp->next);
      len=strlen(buf);

      if(tmp->element_head->type == MAC_SDU) {
	//free payload memory pointed by logical element
	mac_free(tmp->element_head->length, tmp->element_head->data);

	//free memory used by logical element structure
	logical_element_finalize(tmp->element_head);

	//free memory used by logical packet structure
	logical_packet_finalize(tmp);
      }
      svcd=TRUE;
    }
    if(svcd==TRUE) {
      total_bytes +=allowed_bytes;
      printf("%s\n", buf);
      printf(" Servicing of packet # %d cid:%d  num_bytes:%d  \n", i, cid, num_bytes);
      i++;
    }
    //sleep for some time
    msleep(100);
  }
  printf("Total bytes served: %d\n", total_bytes);
  return NULL;
}
*/
