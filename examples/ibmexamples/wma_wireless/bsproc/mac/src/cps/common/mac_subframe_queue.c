/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_subframe_queue.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


//  Function:  the downlink formed mac layer physical subframe are put into this queue waiting to be transmitted
//  the uplink received mac layer physical subframe are put into this queue waiting to be send to the cps processing

#include "mac_subframe_queue.h"

int initialize_subframe_queue(subframe_queue** subframeq, u_int8_t is_dl)
{
    (*subframeq) = (subframe_queue *) malloc(sizeof(subframe_queue));
    memset((*subframeq), 0, sizeof(subframe_queue));
   
    //intialize the mutex locks and conditional variables
    if(pthread_mutex_init(&((*subframeq)->qmutex), NULL)) 
    {
        FLOG_FATAL("initialize_subframe_queue(): Error while initializing mutex_lock...");
    }
    if(pthread_cond_init(&((*subframeq)->notempty), NULL)) 
    {
        FLOG_FATAL("initialize_subframe_queue(): Error while initializing mutex_conditional variable...");
    }
    if (is_dl)
    {
        dl_subframe_queue = (*subframeq);
    }
    else
    {
        ul_subframe_queue = (*subframeq);
    }
    return 0;
}

int initialize_subframe(physical_subframe** phy_subframe)
{
    (*phy_subframe) = (physical_subframe *) malloc(sizeof(physical_subframe));
    memset((*phy_subframe), 0, sizeof(physical_subframe));
    return 0;
}

int dequeue_subframe(subframe_queue* subframeq, physical_subframe** phy_subframe)
{
	pthread_cleanup_push((void*)pthread_mutex_unlock, (void *) &(subframeq->qmutex));
    pthread_mutex_lock(&(subframeq->qmutex));  
    while( subframeq->overall_frame_num == 0 ) {
        pthread_cond_wait(&(subframeq->notempty), &(subframeq->qmutex));
    } 
    //if (subframeq->subframe_head == NULL)
    //{
    //    (*phy_subframe) = NULL;
    //}
    //else
    //{
        if (subframeq->subframe_head == subframeq->subframe_tail)
        {
            (*phy_subframe) = subframeq->subframe_head;
            subframeq->subframe_head = NULL;
            subframeq->subframe_tail = NULL;
        }
        else
        {
                (*phy_subframe) = subframeq->subframe_head;
                subframeq->subframe_head = subframeq->subframe_head->next;
                (*phy_subframe)->next = NULL;
        }
        subframeq->overall_frame_num--;
    //}
    pthread_mutex_unlock(&(subframeq->qmutex));
	pthread_cleanup_pop(0);
    return 0;
}

int enqueue_subframe(subframe_queue* subframeq, physical_subframe* phy_subframe)
{
    pthread_mutex_lock(&(subframeq->qmutex)); 
    if (subframeq->subframe_tail == NULL)
    {
        subframeq->subframe_head = phy_subframe;
        subframeq->subframe_tail = phy_subframe;
    }
    else 
    {
        subframeq->subframe_tail->next = phy_subframe;
        subframeq->subframe_tail = phy_subframe;
     }
    phy_subframe->next = NULL;
    subframeq->overall_frame_num++;
    pthread_cond_signal(&(subframeq->notempty));
    pthread_mutex_unlock(&(subframeq->qmutex));
    return 0;
}

int release_subframe(physical_subframe* phy_subframe)
{
    phy_burst* pb;
    phy_burst* next_pb;
    // release subframe
    pb = phy_subframe->burst_header;
    while (pb)
    {
        next_pb = pb->next;
        // begin to free burst
        free(pb->burst_payload);
        free(pb);
        pb = next_pb;
    }

    if (phy_subframe->fch_dl_map != NULL)
    {
        free (phy_subframe->fch_dl_map);
    }

    if (phy_subframe->raw_ul_map != NULL)
    {
        free (phy_subframe->raw_ul_map);
        phy_subframe->raw_ul_map = NULL;
    }

    free(phy_subframe);    
    phy_subframe = NULL;
    return 0;
}

int get_subframe_queue(u_int8_t is_dl, subframe_queue** subframeq){
    
    if (is_dl)
    {
        (*subframeq) = dl_subframe_queue;
    }
    else
    {
        (*subframeq) = ul_subframe_queue;
    }    
    return 0;
}

int release_subframe_queue(subframe_queue* subframeq, u_int8_t is_dl)
{
    physical_subframe* ps;
    physical_subframe* next_ps;
    phy_burst* pb;
    phy_burst* next_pb;

    pthread_mutex_lock(&(subframeq->qmutex));

    ps = subframeq->subframe_head;

    while (ps)
    {
        next_ps = ps->next;

        // release subframe
        pb = ps->burst_header;
        while (pb)
        {
            next_pb = pb->next;
            // begin to free burst
            free(pb->burst_payload);
            pb->burst_payload = NULL;
            free(pb);
            pb = next_pb;
        }
        free(ps);
        ps = next_ps;
    }
    subframeq->overall_frame_num = 0;
    pthread_mutex_unlock(&(subframeq->qmutex));
    pthread_mutex_destroy(&subframeq->qmutex);
    pthread_cond_destroy(&subframeq->notempty);
    free(subframeq);
    subframeq = NULL;

    if (is_dl)
    {
        dl_subframe_queue = NULL;
    }
    else
    {
        ul_subframe_queue = NULL;
    }
    return 0;
}

