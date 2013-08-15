/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: asynqueue.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 10-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ASYNQUEUE_H_
#define _ASYNQUEUE_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "types.h"
//
// Function: GetBufferObj
// 
// Description:
//    Allocate a buffer_obj. Each receive posted by a receive thread allocates
//    one of these. After the recv is successful, the buffer_obj is queued for
//    sending by the send thread. Again, lookaside lists may be used to increase
//    performance.
//
struct buffer_obj *getbufferobj(int buflen);


//
// Description:
//    Frees a socket object along with any queued buffer objects.
//
void freebufferobj(struct buffer_obj *obj);



//
// Function: enqueuebufferobjwithmutex
//
// Description:
//   Queue up a  buffer .
//
void enqueuebufferobjwithmutex(struct buffer_obj ** headpointer,struct buffer_obj ** tailpointer,struct buffer_obj *obj, int athead, pthread_mutex_t *mutex);

// 
// Function: dequeuebufferobjwithmutex
//
// Description:
//    Remove a buffer_obj from the given  queue.
//
struct buffer_obj *dequeuebufferobjwithmutex(struct buffer_obj ** headpointer,struct buffer_obj ** tailpointer,pthread_mutex_t *mutex);


#endif
