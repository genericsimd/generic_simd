/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: queue_obj.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 28-Jav 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue_obj.h"


// 
// Description:
//    Allocate a queue_obj. Each receive posted by a receive thread allocates
//    one of these. After the recv is successful, the queue_obj is queued for
//    sending by the send thread. Again, lookaside lists may be used to increase
//    performance.
//
struct queue_obj *getqueueobj(int buflen)
{
    struct queue_obj *newobj = NULL;

    // Allocate the object
    newobj = (struct queue_obj *)malloc(sizeof(struct queue_obj));
    if (newobj == NULL)
    {
        
      //fprintf(stderr, "getqueueobj: malloc failed\n");
        printf("getqueueobj: malloc failed\n");
        return NULL;
    }
    // Allocate the queue
    newobj->buf = (unsigned char *)malloc(sizeof(unsigned char)*buflen);
    if (newobj->buf == NULL)
    {
        fprintf(stderr, "getqueueobj: HeapAlloc failed\n");
        return NULL;
    }
    newobj->buflen = buflen;

    return newobj;
}


// 
// Description:
//    Free the queue object.
//
void freequeueobj(struct queue_obj *obj)
{
    free(obj->buf);
    free(obj);
}

