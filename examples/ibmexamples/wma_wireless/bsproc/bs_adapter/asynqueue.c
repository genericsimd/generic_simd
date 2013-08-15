/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: asynqueue.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 10-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "asynqueue.h"
#include "types.h"



// 
// Description:
//    Allocate a struct buffer_obj. Each receive posted by a receive thread allocates
//    one of these. After the recv is successful, the struct buffer_obj is queued for
//    sending by the send thread. Again, lookaside lists may be used to increase
//    performance.
//
struct buffer_obj *getbufferobj(int buflen)
{
    struct buffer_obj *newobj=NULL;

    // Allocate the object
    newobj = (struct buffer_obj *)malloc(sizeof(struct buffer_obj));
    if (newobj == NULL)
    {
        
        fprintf(stderr, "getbufferobj: malloc failed\n");
        return NULL;
    }
    // Allocate the buffer
    newobj->buf = (unsigned char *)malloc(sizeof(unsigned char)*buflen);
    if (newobj->buf == NULL)
    {
        fprintf(stderr, "getbufferobj: HeapAlloc failed\n");
        return NULL;
    }
    newobj->buflen = buflen;

    newobj->addrlen = sizeof(newobj->addr);

    return newobj;
}


// 
// Description:
//    Free the buffer object.
//
void freebufferobj(struct buffer_obj *obj)
{
    free(obj->buf);
    free(obj);
}




void enqueuebufferobjwithmutex(struct buffer_obj ** headpointer, struct buffer_obj ** tailpointer,struct buffer_obj *obj, int athead, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    if(1)//while(pthread_mutex_trylock(mutex) != EBUSY)
    {
        if (*headpointer == NULL)
        {
        // Queue is empty
            *headpointer = *tailpointer = obj;
        }
        else if (athead == 0)
        {
            // Put new object at the end 
	    (*tailpointer)->next = obj;
            (*tailpointer) = obj;
        }
        else
        {
            // Put new object at the head
	    //(*headpointer)->next = obj;
            obj->next = (*headpointer);
	    (*headpointer) = obj;
        }
    }
    pthread_mutex_unlock(mutex);
}




struct buffer_obj *dequeuebufferobjwithmutex(struct buffer_obj ** headpointer,struct buffer_obj ** tailpointer,pthread_mutex_t *mutex)
{
     struct buffer_obj *ret=NULL;
     pthread_mutex_lock(mutex);
     if(1)//while(pthread_mutex_trylock(mutex) != EBUSY)
     {
       

        if ((*tailpointer) != NULL)
        {
            // Queue is non empty
            ret = *headpointer;

            *headpointer = (*headpointer)->next;
            if ((*tailpointer) == ret)
            {
                // Item is the only item in the queue
                *tailpointer = NULL;
		*headpointer = NULL;
            }
        }
     }
     
     pthread_mutex_unlock(mutex);
     return ret;
}
