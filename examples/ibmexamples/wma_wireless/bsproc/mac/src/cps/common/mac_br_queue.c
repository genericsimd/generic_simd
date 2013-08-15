/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_br_queue.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------	--------
   3-Aug.2008       Created                                 	Chen Lin
   4-Jun.2012	Modified for bandwidth request	Xianwei.Yi
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_br_queue.h"
#include "assert.h"

int initialize_br_queue(ul_br_queue** brq){
    (*brq) = (ul_br_queue*) malloc(sizeof(ul_br_queue));
    memset((*brq), 0, sizeof(ul_br_queue));
    brqueue = (*brq);
    //pthread_mutex_init(&((*brq)->brq_mutex), NULL);
    return 0;
}

int get_br_queue(ul_br_queue** brq){
    (*brq) = brqueue;
    return 0;
}

int initialize_br_cid_queue(br_cid_queue** br_cidq, int cid){
    (*br_cidq) = (br_cid_queue*) malloc(sizeof(br_cid_queue));
    memset((*br_cidq), 0, sizeof(br_cid_queue));
    (*br_cidq)->cid = cid;
    pthread_mutex_init(&((*br_cidq)->brcidq_mutex), NULL);
    return 0;
}

int get_br_cid_queue(ul_br_queue* brq, int cid, br_cid_queue** br_cidq){
    br_cid_queue* next_cidq;
    br_cid_queue* cidq;
    next_cidq = brq->brcidq_head;
    while (next_cidq)
    {
        cidq = next_cidq;
        next_cidq = cidq->next;
        if (cidq->cid == cid)
        {
            (*br_cidq) = cidq;
            return 0;
        }
    }
    (*br_cidq) = NULL;
    return 0;
}

#ifndef BR_ENABLE
int enqueue_br_queue(ul_br_queue* brq, unsigned int frame_num, int cid, int msg_type, int length, void* data){
#ifndef SS_RX // enqueue only in the BS receive chain. If a BW req is received at the SS, discard
    br_cid_queue* brcidq = NULL;
    u_char* payload = NULL;
    mgt_msg* mmsg;
    // obtain the br cid queue
    get_br_cid_queue(brq, cid, &brcidq);
    if (brcidq == NULL)
    {
        //pthread_mutex_lock(&(brq->brq_mutex));
        initialize_br_cid_queue(&brcidq, cid);
        if (brq->brcidq_head == NULL )
        {
            brq->brcidq_head = brcidq;
        }
        else
        {
            brq->brcidq_tail->next = brcidq;
        }
        brq->brcidq_tail = brcidq;
        brq->num_cids++;
        //pthread_mutex_unlock(&(brq->brq_mutex));
    }
    
    // construct the structured br management message
    mmsg = (mgt_msg*) malloc(sizeof(mgt_msg));
    memset(mmsg, 0, sizeof(mgt_msg));
    mmsg->cid = cid;
    mmsg->length = length;
    mmsg->msg_type = msg_type;
    mmsg->rcv_frame_num = frame_num;
    payload = (void*) malloc(sizeof(length));
    memcpy(payload, data, length);
    mmsg->data = payload;
   
    // insert it into the br_cid_queue
    pthread_mutex_lock(&(brcidq->brcidq_mutex));
    if (brcidq->br_msg_head == NULL)
    {
        brcidq->br_msg_head = mmsg;
    }
    else
    {
        brcidq->br_msg_tail->next = mmsg;
    }
    brcidq->br_msg_tail = mmsg;
    pthread_mutex_unlock(&(brcidq->brcidq_mutex));
#endif
    return 0;
}
#else
#include "mac_br_api.h"
extern sll_fifo_q *mac_br_q;
/*
  * enqueue_br_queue - parse the bandwidth request and post it to br thread
  * @brq: bandwidth request queue, not used currently 
  * @frame_num: frame number
  * @cid: connection id
  * @length: mac management header length
  * @data: pointer to the bandwith reqeust packet header
  *
  * The API is used to parse the bandwith request and post it to the bandwith request handler thread
  *
  * Return:
  *		always 0
  */
int enqueue_br_queue(ul_br_queue* brq, unsigned int frame_num, int cid, int msg_type, int length, void* data)
{
	struct br_req		*br_req;
	u_int8_t		*ptr;
	
	assert(brq != NULL);
	assert(data != NULL);

	br_req = (struct br_req *)malloc(sizeof(struct br_req));
	if (br_req != NULL)
	{
		ptr = (u_int8_t *)data;
		br_req->cid = cid;
		br_req->type = msg_type;
		br_req->bw_len = ((((u_int32_t)(*ptr & 0x7)) << 16U) | \
							(((u_int32_t)(*(ptr + 1))) << 8U) | \
							((u_int32_t)(*(ptr + 2))));
		sll_fifo_q_enqueue(mac_br_q, (void *)br_req, sizeof(struct br_req), BR_REQ);
	}

	return 0;
}
#endif

int dequeue_br_cid_queue(ul_br_queue* brq, int cid, mgt_msg** br_msgs){
    br_cid_queue* brcidq = NULL;
    // obtain the br cid queue
    get_br_cid_queue(brq, cid, &brcidq);
    if (brcidq)
    {
        pthread_mutex_lock(&(brcidq->brcidq_mutex));
        (*br_msgs) = brcidq->br_msg_head;
        brcidq->br_msg_head = NULL;
        brcidq->br_msg_tail = NULL;
        pthread_mutex_unlock(&(brcidq->brcidq_mutex));
    }
    return 0;
}

int release_br_queue(ul_br_queue* brq){
    br_cid_queue* next_cidq;
    br_cid_queue* pre_cidq;
    br_cid_queue* cidq;
    mgt_msg* mmsg;
    mgt_msg* pre_mmsg;
    next_cidq = brq->brcidq_head;
    pre_cidq = NULL;
    while (next_cidq)
    {
        cidq = next_cidq;

        // save the next cidq
        next_cidq = cidq->next;
        pre_cidq = cidq;
        mmsg = cidq->br_msg_head;
        while (mmsg)
        {
             pre_mmsg = mmsg;
             mmsg = pre_mmsg->next;
             free(pre_mmsg->data);
             pre_mmsg->data = NULL;
             free(pre_mmsg);
             pre_mmsg = NULL;
        }
        pthread_mutex_destroy(&(cidq->brcidq_mutex));
        free(cidq);
        brq->num_cids--;
    }
    free(brq);
    //pthread_mutex_destroy(&(brq->brq_mutex));
    brq = NULL;
    brqueue = NULL;
    return 0;
}

int release_br_cid_queue(ul_br_queue* brq, int cid){
    br_cid_queue* next_cidq;
    br_cid_queue* pre_cidq;
    br_cid_queue* cidq;
    next_cidq = brq->brcidq_head;
    mgt_msg* mmsg;
    mgt_msg* pre_mmsg;
    pre_cidq = NULL;
    //pthread_mutex_lock(&(brq->brq_mutex));
    while (next_cidq)
    {
        cidq = next_cidq;
        next_cidq = cidq->next;
        if (cidq->cid == cid)
        {
            // begin to del this cidq
            if (pre_cidq)
            {
                pre_cidq->next = cidq->next;
            }
            else
            {
               brq->brcidq_head = cidq->next;
            }

            mmsg = cidq->br_msg_head;
            while (mmsg)
            {
                 pre_mmsg = mmsg;
                 mmsg = pre_mmsg->next;
                 free(pre_mmsg->data);
                 pre_mmsg->data = NULL;
                 free(pre_mmsg);
                 pre_mmsg = NULL;
            }
            pthread_mutex_destroy(&(cidq->brcidq_mutex));
            free(cidq);
            cidq = NULL;
            brq->num_cids--;
            break;
        }

            pre_cidq = cidq;
    }
    //pthread_mutex_lock(&(brq->brq_mutex));
    return 0;
}
