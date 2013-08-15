/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ulmap_queue.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 18-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef MAC_ADAPTER_ULMAP_QUEUE_H_
#define MAC_ADAPTER_ULMAP_QUEUE_H_


#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include "ulmap.h"

struct frame_ul_map_msg
{
    ul_map_msg     *p_map_msg;
    int            iframenum;

    struct frame_ul_map_msg *next;
};



/*for special use, when the ul_map_msg enqueue to array, the ul_map_msg of source pointer will be set NULL
 *
 */
int enqueue_ul_map_msg_by_framenum(int framenum,ul_map_msg **p_ul_map_msg);

int dequeue_ul_map_msg_by_framenum(int iframenum,ul_map_msg **p_ul_map_msg);

/*function: get ul_map_msg data struct from array by frame number, 
 *then release resource for that slot node;
 */
int remove_ul_map_msg_by_framenum(int iframenum,ul_map_msg **p_ul_map_msg);


#endif
