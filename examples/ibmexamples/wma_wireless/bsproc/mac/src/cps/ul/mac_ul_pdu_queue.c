/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_pdu_queue.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_ul_pdu_queue.h"

int initialize_pduq(pdu_queue** ul_pduq){
    (*ul_pduq) = (pdu_queue *) malloc(sizeof(pdu_queue));
    memset((*ul_pduq), 0, sizeof(pdu_queue));
    (*ul_pduq)->frame_num = 0;
    (*ul_pduq)->pdu_frame_q_head = NULL;
    (*ul_pduq)->pdu_frame_q_tail = NULL;
    (*ul_pduq)->next = NULL;
    return 0;
}

int initialize_pduframeq(pdu_frame_queue** ul_framepduq, int frame_no){
    (*ul_framepduq) = (pdu_frame_queue *) malloc(sizeof(pdu_frame_queue));
    memset((*ul_framepduq), 0, sizeof(pdu_frame_queue));
    (*ul_framepduq)->frame_no = frame_no;
    (*ul_framepduq)->num_cids = 0;
    (*ul_framepduq)->pdu_cid_q_head = NULL;
    (*ul_framepduq)->next = NULL;
    return 0;
}

int enqueue_pduq(pdu_queue*ul_pduq, pdu_frame_queue* ul_framepduq)
{
    if (ul_pduq->pdu_frame_q_tail == NULL)
    {
        ul_pduq->pdu_frame_q_tail = ul_framepduq;
        ul_pduq->pdu_frame_q_head = ul_framepduq;
    }
    else
    {
        ul_pduq->pdu_frame_q_tail->next = ul_framepduq;
        ul_pduq->pdu_frame_q_tail = ul_framepduq;
    }
    ul_pduq->frame_num++;
    return 0;
}

int initialize_pducidq(int cid, pdu_cid_queue** cidq){
    (*cidq) = (pdu_cid_queue *) malloc(sizeof (pdu_cid_queue));
    memset((*cidq), 0, sizeof (pdu_cid_queue));
    (*cidq)->cid = cid;
    (*cidq)->is_processed = 0;
    (*cidq)->pdu_num = 0;
    (*cidq)->head = NULL;
    (*cidq)->tail = NULL;
    (*cidq)->next = NULL;
    return 0;
}

int get_pducidq(pdu_frame_queue* pduframeq, int cid, pdu_cid_queue** ul_pducidq){
    pdu_cid_queue * cidq = NULL;

    pdu_cid_queue* cur_cidq = pduframeq->pdu_cid_q_head;
    pdu_cid_queue* prev_cidq = cur_cidq;
    while (cur_cidq)
    {
        if (cur_cidq->cid == cid){
            (*ul_pducidq) = cur_cidq;
            return 0;
        }
        prev_cidq = cur_cidq;
        cur_cidq = cur_cidq->next;
    }
    // there are no related cidq, so generate a new pducidqueue
    initialize_pducidq(cid, &(cidq));
    if (prev_cidq == NULL)
    {
        // it's the header, the first element in the frag_queue
        pduframeq->pdu_cid_q_head = cidq;
    }
    else 
    {
        prev_cidq->next = cidq;
    }
    pduframeq->num_cids++;
    (*ul_pducidq) = cidq;    
    return 0;
}

int enqueue_pducidq(pdu_cid_queue* ul_pducidq, logical_element* pdu){
    if (ul_pducidq == NULL)
    {
        return 1;
    }
    pdu->next = NULL;
    if (ul_pducidq->head == NULL)
    {
        // the first pdu
        ul_pducidq->head = pdu;
        ul_pducidq->tail = pdu;
        ul_pducidq->pdu_num++;
    }
    else 
    {
        ul_pducidq->tail->next = pdu;
        ul_pducidq->tail = pdu;
        ul_pducidq->pdu_num++;
    }
    return 0;
}

int dequeue_pduq(pdu_queue* ul_pduq_header, pdu_frame_queue** ul_pduframeq){
    
    if (ul_pduq_header->pdu_frame_q_head)
    {
        (*ul_pduframeq) = ul_pduq_header->pdu_frame_q_head;
        if (ul_pduq_header->pdu_frame_q_head == ul_pduq_header->pdu_frame_q_tail)
        {
            ul_pduq_header->pdu_frame_q_head = NULL;
            ul_pduq_header->pdu_frame_q_tail = NULL;
        }
        else
        {
            ul_pduq_header->pdu_frame_q_head = ul_pduq_header->pdu_frame_q_head->next;
            (*ul_pduframeq)->next = NULL;
        }
        ul_pduq_header->frame_num--;
    }
    
    return 0;
}

// remove the parsed logical element from this queue.
int release_pducidq(pdu_cid_queue* ul_pducidq)
{
    logical_element * le; 
    logical_element * pre_le;

    le = ul_pducidq->head;
    
    while (le)
    {
        pre_le = le;
        le = pre_le->next;    
        free(pre_le);
        pre_le = NULL;
    }

    ul_pducidq->head = NULL;
    return 0;
}

int release_pduframeq(pdu_frame_queue * pduframeq)
{
    pdu_cid_queue * pducidq;
    pdu_cid_queue * pre_pducidq;

    pducidq = pduframeq->pdu_cid_q_head;

    while (pducidq)
    {
        pre_pducidq = pducidq;
        pducidq = pre_pducidq->next;
        release_pducidq(pre_pducidq);
        free(pre_pducidq);
        pre_pducidq = NULL;
    }

    free(pduframeq);
    pduframeq = NULL;
    return 0;
}

// only release the pdu cid queue within this queue, not the payload
int release_pduq(pdu_queue* ul_pduq)
{
    pdu_frame_queue * pduframeq;
    pdu_frame_queue * pre_pduframeq;

    pduframeq = ul_pduq->pdu_frame_q_head;

    while (pduframeq)
    {
        pre_pduframeq = pduframeq;
        pduframeq = pre_pduframeq->next;
        release_pduframeq(pre_pduframeq);
    }

    free(ul_pduq);
    ul_pduq = NULL;
    
    return 0;
}

