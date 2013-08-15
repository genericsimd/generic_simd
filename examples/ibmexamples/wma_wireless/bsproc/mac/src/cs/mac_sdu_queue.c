/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: mac_sdu_queue.c

Change Activity:

Date    	Description of Change        	By
----------------------------------------------------------
01-Oct-2008 	     Created		   Malolan Chetlur

----------------------------------------------------------
*/

#include "mac_sdu_queue.h"
#include "CS.h"
#include "memmgmt.h"
#include "sdu_cid_queue.h"
#include "mac_hash.h"
#include "debug.h"
#include "flog.h"

sdu_queue* dl_sdu_queue; // the MAC SDU for the downlink, 
extern int param_DL_CBR_PACKET_SIZE;

//intialize the mac sdu_queue DS
void mac_sdu_queue_init() {
  FLOG_DEBUG("Calling mac_sdu_queue_init ...");
  size_t num_bytes=sizeof(sdu_queue);
  dl_sdu_queue = (sdu_queue*)mac_malloc(num_bytes);

  //allocate array of sdu_cid_queue pointers for MAX_CIDS cid queues
  dl_sdu_queue->sdu_cid_q = (sdu_cid_queue**)mac_malloc(MAX_CIDS*(sizeof(sdu_cid_queue*)));
  dl_sdu_queue->overall_sdu_size=0;
  dl_sdu_queue->num_cids=0;
  dl_sdu_queue->num_ss=0;
  dl_sdu_queue->num_be_cids_with_sdu_data=0;
}


// dequeue the num_bytes from sdu queue for cid
// and return list of logical_packet
int dequeue_sduq(sdu_queue* sdu_q, int cid, size_t num_bytes, logical_packet** sdu_list) {
  FLOG_DEBUG("Calling dequeue_transport_sdu_queue ...");
  sdu_cid_queue* mac_sdu_cid_q=NULL;
  
  //cid queue is present; at this cid queue must be there
  if(ht_is_value_present(cid)) {
    //get mac_sdu_cid_queue
    int cid_q_indx=ht_get_key(cid);
    mac_sdu_cid_q=sdu_q->sdu_cid_q[cid_q_indx];

    //Adjust the total sdu size among all transport cids
    sdu_q->overall_sdu_size -=num_bytes; // Could be dirty for sometime
    return dequeue_sdu_cid_queue(mac_sdu_cid_q, cid, num_bytes, sdu_list);
  }
  return 0;
}

// enqueue the num_bytes into sdu_cid_queue for the cid
int enqueue_transport_sdu_queue(sdu_queue* sdu_q, int cid, size_t num_bytes, void* physical_sdu) {
  FLOG_DEBUG("Calling enqueue_transport_sdu_queue ...");
  sdu_cid_queue* mac_sdu_cid_q=NULL;

  //a sdu_cid_queue for cid is available
  if(ht_is_value_present(cid)) {
    //get mac_sdu_cid_queue
    int cid_q_indx=ht_get_key(cid);
    mac_sdu_cid_q=sdu_q->sdu_cid_q[cid_q_indx];
  }
  else { 
    //create sdu cid queue
    mac_sdu_cid_q=mac_sdu_cid_queue_init(cid);
    mac_sdu_cid_q->sdu_cid_aggr_info->cid = cid;

    //add it to the correct index
    int cid_q_indx=ht_get_key(cid);
    sdu_q->sdu_cid_q[cid_q_indx] = mac_sdu_cid_q;
    ht_add_value(cid);
  }

  //Adjust the total sdu size among all transport cids
  //Dirty write
  sdu_q->overall_sdu_size += num_bytes;

  //enqueue to mac_sdu_cid_queue
  return enqueue_sdu_cid_queue(mac_sdu_cid_q, cid, num_bytes, physical_sdu);
}

void mac_sdu_queue_finalize() {
  FLOG_DEBUG("Calling mac_sdu_queue_finalize() ...");
  sleep(1);
  mac_free(MAX_CIDS*sizeof(sdu_cid_queue*), dl_sdu_queue->sdu_cid_q);
  mac_free(sizeof(sdu_queue), dl_sdu_queue);

}

BOOL is_cid_in_sdu_queue(int cid) {
  return FALSE;
}

//peek the sdu cid queue and for requested num_bytes
// determine -- how many num_sdus are needed and their max and min size
int peek_sdu_queue(sdu_queue* sdu_q, int cid, int num_bytes, int* num_sdu, int* min_sdu_size, int* max_sdu_size) {

  FLOG_DEBUG("Calling peek_sdu_queue ...");
  sdu_cid_queue* mac_sdu_cid_q=NULL;

  //a sdu_cid_queue for cid is available
  if(ht_is_value_present(cid)) {
    //get mac_sdu_cid_queue
    int cid_q_indx=ht_get_key(cid);
    mac_sdu_cid_q=sdu_q->sdu_cid_q[cid_q_indx];
    return peek_cid_queue(mac_sdu_cid_q, cid, num_bytes, num_sdu, min_sdu_size, max_sdu_size);
  }
  else { 
    //There is no sdu cid queue for this cid
    *num_sdu=0;
    *min_sdu_size=0;
    *max_sdu_size=0;
  }

  return 0;
}

sdu_cid_queue* get_sdu_cid_queue(sdu_queue* sdu_q, int cid)
{
  sdu_cid_queue* mac_sdu_cid_q=NULL;
  if(ht_is_value_present(cid)) {
  //get mac_sdu_cid_queue
  int cid_q_indx=ht_get_key(cid);
  mac_sdu_cid_q=sdu_q->sdu_cid_q[cid_q_indx];
  }

  return mac_sdu_cid_q;
}


int initialize_sduq(sdu_queue** sduq, u_int8_t is_dl){
    (*sduq) = (sdu_queue *) malloc(sizeof(sdu_queue));
    // (*sduq)->sdu_cid_q = NULL;
    (*sduq)->overall_sdu_size = 0;
    (*sduq)->num_cids = 0;
    int i;

    if (! is_dl)
    {
        ul_sdu_queue = (*sduq);
        ul_sdu_queue->sdu_cid_q = (sdu_cid_queue**)mac_malloc(MAX_CIDS*(sizeof(sdu_cid_queue*)));
        for (i=0; i<MAX_CIDS; i++)
        {
            ul_sdu_queue->sdu_cid_q[i] = NULL;
        }
    }
    return 0;
}

int get_sduq(sdu_queue** sduq, u_int8_t is_dl){

    if (is_dl)
    {
        (*sduq) = dl_sdu_queue;
    }
    else
    {
        (*sduq) = ul_sdu_queue;
    }
    return 0;
}

/* int dequeue_sduq(sdu_queue* sduq, int cid, int bytes_num, logical_packet** sdulist){ */
/*     int left_bytes = bytes_num; */
/*     sdu_cid_queue* sq; */
/*     logical_packet* sdu; */
/*     logical_packet* pre_sdu; */
/*     logical_packet* fragment; */
/*     logical_element* frag_le; */
/*     sq = sduq->sdu_cid_q; */

/*     while (sq) */
/*     { */
/*         // find the proper sdu cid queue */
/*         if (sq->sdu_cid_aggr_info->cid == cid) */
/*         { */
/*             sdu = sq->head; */
/*             pre_sdu = sdu; */
/*             (*sdulist) = sq->head; */
/*             while (left_bytes > 0) */
/*             { */
/*                 if (sdu->length <= left_bytes) */
/*                 { */
/*                     // calculate the remainder of the left bytes */
/*                     if (sdu->element_head->type == MAC_SDU_FRAG) */
/*                     { */
/*                         sdu->element_head->blk_type = LAST_FRAGMENT;                 */
/*                     } */
                    
/*                     left_bytes -= sdu->length; */
/*                     pre_sdu = sdu; */
/*                     sdu = sdu->next; */
/*                     sq->head = sdu; */
/*                     sq->sdu_num--; */
/*                 } */
/*                 else  */
/*                 { */
/*                     // left a fragments */

                    
/*                     fragment = (logical_packet *) malloc(sizeof(logical_packet)); */
/*                     frag_le = (logical_element *) malloc(sizeof(logical_element)); */
/*                     memcpy(fragment, sdu, sizeof(logical_packet)); */
/*                     memcpy(frag_le, sdu->element_head, sizeof(logical_element)); */
/*                     fragment->element_head = frag_le; */
/*                     // the residue fragments */
/*                     fragment->length = sdu->length - left_bytes; */
/*                     fragment->element_head->length = fragment->length; */
/*                     fragment->element_head->data += left_bytes; */
                    
/*                     fragment->element_head->type = MAC_SDU_FRAG; */

/*                     if (sdu->element_head->type == MAC_SDU) */
/*                     { */
/*                         sdu->element_head->blk_type = FIRST_FRAGMENT; */
/*                     } */
/*                     else if (sdu->element_head->type == MAC_SDU_FRAG) */
/*                     { */
/*                         sdu->element_head->blk_type = CONTINUING_FRAGMENT; */
/*                     } */
                    
/*                     sq->head = fragment; */

/*                     // process the original sdu */
/*                     sdu->length = left_bytes; */
/*                     pre_sdu = sdu; */
/*                     sdu->element_head->length = left_bytes; */
                    
/*                     sdu->element_head->type = MAC_SDU_FRAG; */
/*                     left_bytes = 0; */
/*                     break; */
/*                 } */
/*             } */
/*             pre_sdu->next = NULL; */
/*             if (sq->sdu_num == 0) */
/*             { */
/*                 sq->head = NULL; */
/*                 sq->tail = NULL; */
/*             } */
/*             break; */
/*         } */
/*         else */
/*         { */
/*             sq = sq->next; */
/*         } */

/*     } */
    
/*     return 0; */
/* } */

int enqueue_sduq(sdu_queue* sduq, int cid, logical_packet* sdu){
    sdu_cid_queue* sq;
    u_int8_t is_found;
    is_found = 0;

    sq = sduq->sdu_cid_q[cid];

    sdu->next = NULL;

    sdu->element_head->type = MAC_SDU;

    sdu->element_head->blk_type = NO_FRAGMENTATION;
    if (!sq) 
    {
	add_sdu_cid_queue(sduq,cid);
        sq = sduq->sdu_cid_q[cid];
        if (sq)
	{
#ifdef PRINT_DATA		
        	char* file_name=(char*)malloc(8);
        	sprintf(file_name,"%d.ul", cid);
        	sq->ul_fp = fopen(file_name, "w");
        	if (!sq->ul_fp)
        	{
          		FLOG_ERROR("Error opening file %s\n", file_name);
        	}
        	else
        	{
          	printf("Opened file %s\n", file_name);
        	}
		free(file_name);
#endif		
		
	}
    }        
    pthread_mutex_lock(&sq->qmutex);
        if (sq->sdu_cid_aggr_info->cid == cid)
        {
    	//    pthread_mutex_lock(&(sq->qmutex)); 
            if (sq->tail == NULL)
            {
                sq->head = sdu;
                sq->tail = sdu;
            }
            else
            {
                sq->tail->next = sdu;
                sq->tail = sdu;
            }
            sq->sdu_num++;
            is_found = 1;
         //   pthread_mutex_unlock(&(sq->qmutex));
            //break;
        }


#ifdef PRINT_DATA	
    // Print the SDU bytes
    logical_element* le = sdu->element_head;
    u_char *data;
    int ii = 0;
    while (le != NULL)
    {
	data = le->data;
	for (ii = 0; ii < le->length; ii++)
	{
	fprintf(sq->ul_fp,"%d\n", (*data));
	data++;
	}
	le = le->next;
    }
#endif	
    pthread_mutex_unlock(&sq->qmutex);
    return 0;
}

int add_sdu_cid_queue(sdu_queue* sduq, int cid){
    //sdu_cid_queue* sq;
    //sdu_cid_queue* prev_sq;
    sdu_cid_queue* sdu_cidq;
    //u_int8_t is_insert;
    sdu_cidq = (sdu_cid_queue *) malloc(sizeof(sdu_cid_queue));
    memset(sdu_cidq, 0, sizeof(sdu_cid_queue));
    sdu_cidq->sdu_cid_aggr_info = (sdu_cid_overall_info *) malloc(sizeof(sdu_cid_overall_info));
    memset(sdu_cidq->sdu_cid_aggr_info, 0, sizeof(sdu_cid_overall_info));
    sdu_cidq->sdu_cid_aggr_info->cid = cid;
    if(pthread_mutex_init(&(sdu_cidq->qmutex), NULL)) 
    {
        FLOG_DEBUG("add_sdu_cid_queue(): Error while initializing mutex_lock...");
    }
    
    sduq->sdu_cid_q[cid] = sdu_cidq; 

    return 0;
}

int del_sdu_cid_queue(sdu_queue* sduq, int cid){
    sdu_cid_queue* sq = NULL;
    //sdu_cid_queue* prev_sq;
    logical_packet* sdu;
    logical_packet* next_sdu;
    logical_element* le;
    logical_element* next_le;
    sq = sduq->sdu_cid_q[cid];
    
    if(sq)
    {
        if (sq->sdu_cid_aggr_info == NULL)
        {
            sduq->sdu_cid_q[cid] = NULL;
            return 0;
        }

        if (sq->sdu_cid_aggr_info->cid == cid)
        {
            // remove the logical packet within this sdu cid queue
           sdu = sq->head;
           while (sdu)
           {
               next_sdu = sdu->next;
               // begin to free sdu
               le = sdu->element_head;
               while (le)
               {
                    next_le = le->next;
		    mac_sdu_free((void*) le->data, le->length, le->blk_type);
                    free(le);
                    le = next_le;
               }
               free(sdu);
               sdu = next_sdu;
           }
           free(sq->sdu_cid_aggr_info);
           free(sq);
            
        }

    }
    sduq->sdu_cid_q[cid] = NULL;
    return 0;

}

int dequeue_ul_sduq(sdu_queue* sduq){
    sdu_cid_queue* sq;
    //sdu_cid_queue* next_sq;
    logical_packet* sdu;
    logical_packet* next_sdu;
    logical_element* le;
    logical_element* next_le;
    int i;

    
    for (i=0; i<MAX_CIDS; i++)
    {
        sq = sduq->sdu_cid_q[i];
        if (sq == NULL)
        {
            continue;
        }
        

        // release the sdu_cid_queue sq
        pthread_mutex_lock(&(sq->qmutex)); 
        sdu = sq->head;
        while (sdu)
        {
            next_sdu = sdu->next;
            // begin to free the element in the sdu
            le = sdu->element_head;
            while (le)
            {
                next_le = le->next;

                if (le->data != NULL)
                {
                    free(le->data);
                }
                free(le);
                le = next_le;
            }
            free(sdu);
            sdu = next_sdu;
        }

        sq->head = NULL;
        sq->sdu_num = 0;
        sq->tail = NULL;

        pthread_mutex_unlock(&(sq->qmutex));

        
    }


    return 0;
}


int release_sduq(sdu_queue* sduq, u_int8_t is_dl){
    sdu_cid_queue* sq;
    sdu_cid_queue* next_sq;
    logical_packet* sdu;
    logical_packet* next_sdu;
    logical_element* le;
    logical_element* next_le;
    int i;
    // sq = sduq->sdu_cid_q[0];
    for (i=0; i< MAX_CIDS; i++)
    {
        sq = sduq->sdu_cid_q[i];
        if (sq == NULL)
        {
            continue;
        }
        pthread_mutex_lock(&(sq->qmutex)); 
        next_sq = sq->next;
        // release the sdu_cid_queue sq
        sdu = sq->head;
        while (sdu)
        {
            next_sdu = sdu->next;
            // begin to free the element in the sdu
            le = sdu->element_head;
            while (le)
            {
                next_le = le->next;
                if (! is_dl)
                {
                    free(le->data);
                }
                free(le);
                le = next_le;
            }
            free(sdu);
            sdu = next_sdu;
        }
        free(sq->sdu_cid_aggr_info);
        pthread_mutex_unlock(&(sq->qmutex));
        pthread_mutex_destroy(&(sq->qmutex));
        free(sq);
        sduq->sdu_cid_q[i] = NULL;
        //sq = next_sq;
        
    }
    free(sduq->sdu_cid_q);
    if (ul_sdu_queue == sduq)
    {
        ul_sdu_queue = NULL;
    }

    free(sduq);
    return 0;
}
