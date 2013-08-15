/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: queue_obj.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jav 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef   _QUEUE_OBJ_H_
#define   _QUEUE_OBJ_H_
//
// Allocated for each receiver posted
//
//typedef struct _queue_obj   queue_obj;
struct queue_obj
{
    void             *buf;          // Data queue for data
    int              buflen;        // Length of queue or number of bytes contained in queue


    struct queue_obj      *next;   // Used to maintain a linked list of queues
};

//
// Function: GetqueueObj
// 
// Description:
//    Allocate a queue_obj. Each receive posted by a receive thread allocates
//    one of these. After the recv is successful, the queue_obj is queued for
//    sending by the send thread. Again, lookaside lists may be used to increase
//    performance.
//
struct queue_obj *getqueueobj(int buflen);


//
// Description:
//    Frees a socket object along with any queued queue objects.
//
void freequeueobj(struct queue_obj *obj);

#endif
