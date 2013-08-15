/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_interface_queue.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _MAC_INTERFACE_QUEUE_H_
#define _MAC_INTERFACE_QUEUE_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include "queue_obj.h"

struct asynqueuenode
{
    char                     linkname[64];
    pthread_mutex_t          *notifymutex;
    pthread_cond_t           *notify;

    struct queue_obj               *headlink;
    struct queue_obj               *taillink;

    long                     queuenum;
    long                     queuemaxnum;

    struct asynqueuenode            *next;
};



void initialrunqueue(char *pheadname);

void destoryqueue();



void setqueuemaxnum(char *pqueuename,long lnum);


//
// Function: enqueueobj
//
// Description:
//   Queue up a  queue .
//
//void enqueueobj(queue_obj ** headpointer, queue_obj ** tailpointer, queue_obj *obj, int athead);

// 
// Function: dequeueobj
//
// Description:
//    Remove a queue_obj from the given  queue.
//
//queue_obj *dequeueobj(queue_obj ** headpointer, queue_obj ** tailpointer);

//
//Function enqueueobj
//find name of queue then enqueue the node 
//
int  enqueueobj(const char * const queuename, struct queue_obj *obj, int athead);

int  dequeueobj(const char * const queuename,struct queue_obj **pp_obj);

//
//Function: print count of node in queue 
//just for test
//
void printcount_num();

#endif
