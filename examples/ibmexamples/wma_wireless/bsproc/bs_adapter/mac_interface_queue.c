/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_interface_queue.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_interface_queue.h"
#include <string.h>
#include   <unistd.h>  
#include   <sys/time.h>

//#define _DEBUG      0
static struct asynqueuenode            *g_queuehead;
static struct asynqueuenode            *g_queueend;


void printcount_num()
{
  //printf("the current queue num is %d\n",g_queuenum );
}

void setqueuemaxnum(char *pheadname,long lnum)
{
    struct asynqueuenode *ploopnode = g_queuehead;
    while(ploopnode != NULL)
    {
	if(strcmp(ploopnode->linkname,pheadname) == 0)
	  break;
    }
    ploopnode->queuemaxnum = lnum;
}

void initialrunqueue(char *pheadname)
{

    
    if(pheadname == NULL)
        return;
    struct asynqueuenode *headnode = (struct asynqueuenode*)malloc(sizeof(struct asynqueuenode));
    if(headnode == NULL)
        return;
    pthread_mutexattr_t        mattr;
    pthread_mutexattr_init(&mattr);
    //pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_RECURSIVE_NP);
    headnode->notifymutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    headnode->notify = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

    pthread_mutex_init(headnode->notifymutex,&mattr);

    pthread_cond_init(headnode->notify,NULL);
    
    headnode->headlink = NULL;
    headnode->taillink = NULL;

    headnode->queuemaxnum = 100;
    headnode->queuenum = 0;

    strcpy(headnode->linkname,pheadname);

    if(g_queuehead ==NULL && g_queueend == NULL)
      {
	g_queuehead = g_queueend = headnode;
	return ;
      }
    if(g_queueend != NULL)
      g_queueend->next = headnode;
    g_queueend = headnode;
}

void destoryqueue()
{
    struct asynqueuenode *ploopnode = g_queuehead;
    struct asynqueuenode *pfreenode = NULL;
    while(ploopnode != NULL)
    {
        pthread_mutex_destroy(ploopnode->notifymutex);
        pfreenode = ploopnode;
        ploopnode =  ploopnode->next;
        free(pfreenode);
    }
}





int  enqueueobj(const char  * const queuename,struct  queue_obj  *obj, int athead)
{
    if(obj == NULL || queuename == NULL)
        return -1;
    struct asynqueuenode *ploopnode = g_queuehead;
    while(ploopnode != NULL)
    {
      if(strcmp(ploopnode->linkname,queuename) == 0)
          break;
      ploopnode = ploopnode->next;
    }
    if(ploopnode == NULL)
      return -1;
    while(1)
    {
      
      if(ploopnode->queuemaxnum <= ploopnode->queuenum)
        usleep(100);
      else
        break;
    }
    pthread_mutex_lock(ploopnode->notifymutex);
    
    ploopnode->queuenum++;
    
    if (ploopnode->headlink == NULL &&ploopnode->taillink == NULL)
    {
        // Queue is empty
        ploopnode->headlink = ploopnode->taillink = obj;
    }
    else if (athead == 0)
    {
            // Put new object at the end 
        ploopnode->taillink->next = obj;
        ploopnode->taillink = obj;
    }
    else
    {
            // Put new object at the head
        obj->next = ploopnode->headlink;
        //ploopnode->headlink->next = obj;
        ploopnode->headlink = obj;
    }
    
    pthread_mutex_unlock(ploopnode->notifymutex);
    pthread_cond_signal(ploopnode->notify);
    return 0;
}

int dequeueobj(const char *const queuename,struct queue_obj **pp_obj)
{
    if(queuename == NULL)
        return -1;
    struct asynqueuenode *ploopnode = g_queuehead;
    while(ploopnode != NULL)
    {
      if(strcmp(ploopnode->linkname,queuename) == 0)
          break;
      ploopnode = ploopnode->next;
    }
    if(ploopnode == NULL)
      return -1;
    pthread_mutex_lock(ploopnode->notifymutex);
    //pthread_cond_wait(&notify,&notifymutex);

    
    //pthread_mutex_lock(&notifymutex);
    struct queue_obj *ret=NULL;

    if (ploopnode->taillink != NULL && ploopnode->headlink != NULL)
    {
            // Queue is non empty

#ifdef _DEBUG
      printf("<enter if statment> the queue contain number is %d \n",ploopnode->queuenum);
#endif
        ret = ploopnode->headlink;
        ploopnode->headlink = ploopnode->headlink->next;
        if (ploopnode->taillink == ret)
        {
                // Item is the only item in the queue
            ploopnode->taillink = NULL;
            ploopnode->headlink = NULL;
        }
    }
    else
    {
#ifdef _DEBUG
      printf("<enter else statment> the queue contain number is %d \n",ploopnode->queuenum);
      printf("the header pointer is %p,tailer pointer is %p\n",ploopnode->headlink,ploopnode->taillink);
#endif
        pthread_cond_wait(ploopnode->notify,ploopnode->notifymutex);
	if(ploopnode->headlink != NULL)
        {
            ret = ploopnode->headlink;
            ploopnode->headlink = ploopnode->headlink->next;
            if (ploopnode->taillink == ret)
            {
                // Item is the only item in the queue
                ploopnode->taillink = NULL;
                ploopnode->headlink = NULL;
            }
        }
	else
	  {
             pthread_mutex_unlock(ploopnode->notifymutex);
             return -1;
	  }
    }
    ploopnode->queuenum--;
    pthread_mutex_unlock(ploopnode->notifymutex);
    *pp_obj = ret;
    return 0;    
}


