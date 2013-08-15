/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: app_timer.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Apr.2009       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include "app_timer.h"
#include "dll_ordered_list.h"
#include "debug.h"
#include "util.h"
#include "memmgmt.h"

#define APP_TIMER_RESOLUTION 100LL

dll_ordered_list* g_timer_list=NULL;
int g_timer_counter=0;
pthread_mutex_t timer_mutex;
int execute_timer=1;
extern pthread_t ss_ir_thread, ss_pr_thread;

int app_timer_init() {

  dll_ordered_list_init(&g_timer_list);

  if(pthread_mutex_init(&timer_mutex, NULL)) {
    FLOG_FATAL("Error Initializing timer_mutex \n");
  }
  g_timer_counter=0;
  execute_timer=1;
  return 0;
}


int app_timer_add(long long int abstime,
		  timeout_action f_ptr,
		  sll_fifo_q* ev_q,
		  void* data,
		  size_t len,
		  u_int32_t flags, 
		  void** timer_id,void *timer_specific_data) {

  // Currently assumes the timout or other events are enqueue to ev_q 
  // timeout action function pointer not used

  app_timer* a_t=NULL;
  (*timer_id)=NULL;
  if(app_malloc((void**)&a_t, sizeof(app_timer))) {
    FLOG_FATAL("Error: Allocating app_timer\n");
    exit(-1);
  }

  //determine the ordered list for this thread
  dll_ordered_list* dol=g_timer_list; // let us use one global list for now

  // determine timer id
#ifdef INTEGRATION_TEST
  a_t->timer_id=(int)(long long int)((void*)a_t);
#else
  a_t->timer_id=(int)((void*)a_t);
#endif
  *timer_id=a_t;

  a_t->expired=0;
  a_t->deleted=0;
  a_t->flags=flags;
  a_t->action=f_ptr;
  a_t->event_queue=ev_q;
  a_t->application_data=data;
  a_t->len=len;
  a_t->timer_specific_data=timer_specific_data;
  // determine absolute time of timeout
  a_t->absolute_time=abstime;

  int num_elems=0;
  // insert it into to timer/event queue
  if(insert_dll_ordered_list(dol, a_t->absolute_time, a_t, sizeof(app_timer), &num_elems)) {
    FLOG_FATAL("Error: inserting a timer in dll ordered lsit\n");
    exit(-1);
  }
  FLOG_DEBUG("Completed app_timer_add timer id:%p absolute time:%lld abstime:%lld\n", (*timer_id), a_t->absolute_time, abstime);

  return 0;
}


int app_timer_delete(void* timer_id) 
{
	if(timer_id != NULL)
	{((app_timer*)timer_id)->deleted=1;}
	return 0;
}

void* timer_handler(void* timer_list) 
{
	dll_ordered_list* dol=(dll_ordered_list*)timer_list;
	assert(dol!=NULL);
	struct timespec ts={0};
	struct timeval tv={0};

#ifdef INTEGRATION_TEST
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }
#endif

	while(can_sync_continue()) 
	{
		int num_elems=0;
		app_timer* data=NULL;
		int rc=0;
		int min_timer=0;

		if(peek_head(dol, (void**)&data, &num_elems)) 
		{
			FLOG_ERROR("Unable to peek timer Q\n");
		}
		//remove all deleted timers
		while((data!=NULL)&&(data->deleted==1)) 
		{
			if(pop_head(dol, (void**)&data, &num_elems)) 
			{
				FLOG_ERROR("Unable to pop timer Q\n");
			}
			assert(data!=NULL);
			assert(data->deleted==1);
			//free the expired app_timer
			app_free(data, sizeof(app_timer));
			if(peek_head(dol, (void**)&data, &num_elems)) 
			{
				FLOG_ERROR("Unable to peek timer Q\n");
			}
		}

		pthread_mutex_lock(&(dol->qmutex));
		//if(dol is empty)
		// block and wait for 500 ms
		if(num_elems==0) 
		{
			gettimeofday(&tv, NULL);
			tv.tv_sec +=1;
			ts.tv_sec = tv.tv_sec;
			ts.tv_nsec = (tv.tv_usec) * 1000L;
			//wait for 1 sec if timer q is empty
			FLOG_DEBUG("Timer Q empty, wait for 1 sec\n");
		}
		else 
		{
			// wait on the earliest event
			//timed wait on earliest event from dol
			ts.tv_sec=data->absolute_time/ONE_MILLION_LL;
			ts.tv_nsec=(data->absolute_time%ONE_MILLION_LL)*1000;
			FLOG_DEBUG("Timer Q Not empty, timer_id:%p wait for:%lld\n", data->timer_id, data->absolute_time);
		}

		while ((dol->head_chgd==0)&&(rc==0)) 
		{
			FLOG_DEBUG("Timer About to enter timed wait, time:%lld\n", readtsc());
			rc=pthread_cond_timedwait( &(dol->checkq), &(dol->qmutex), &ts);
			FLOG_DEBUG("Timer Exited timed wait, time:%lld\n", readtsc());
		}
		if(dol->head_chgd) 
		{
			min_timer=1;
			dol->head_chgd=0;
		}
		pthread_mutex_unlock(&(dol->qmutex));
		//We are out of timed wait
		//dol head changed
		if(min_timer) 
		{
		  FLOG_DEBUG("Timer Q Head Changed\n");
		  continue;
		}
	
		//Remove all the expired timers
		if(rc==ETIMEDOUT) 
		{
			// timed wait expired
			//remove timers from dol and enqueue to appropriate q
			//also remove all timers within 100 micro second window
			if(peek_head(dol, (void**)&data, &num_elems)) 
			{
				FLOG_ERROR("Unable to peek timer Q\n");
			}
			unsigned long long expr_time=0;
			if(data!=NULL) 
			{
				expr_time=data->absolute_time;
			}

			//Remove all closely expiring timers (within 100 micro seconds
			while((data!=NULL) && (data->absolute_time <= expr_time+APP_TIMER_RESOLUTION)) 
			{
				if(pop_head(dol, (void**)&data, &num_elems)) 
				{
					FLOG_ERROR("Unable to pop timer Q\n");
				}
				assert(data!=NULL);
				if(data->deleted!=1) 
				{ //cosider only the non deleted timers
					FLOG_DEBUG("Timer: Adding to Event Q timer_id:%p expiry time:%lld current time:%lld\n",\
					data->timer_id, data->absolute_time, readtsc());

					// On timer expiry, either a message can be enqueued in the
					// event queue or a function can be called
					if((data->event_queue != NULL ) && (data->application_data != NULL))
					{
						sll_fifo_q_enqueue(data->event_queue, data->application_data, \
						data->len, 0);
					}
					if(data->action != NULL)
					{
						// Call the function. This shouldn't stall the main 
						// timer_handler thread, else spawn a separate thread?
						if (data->timer_specific_data == NULL)	(*(data->action))(NULL);
						else (*(data->action))(data->timer_specific_data);
					}
					FLOG_DEBUG("Timer: Added to Event Q timer_id:%p expiry time:%lld current time:%lld\n", \
					data->timer_id, data->absolute_time, readtsc());
				}
				//free the expired app_timer
				app_free(data, sizeof(app_timer));
				if(peek_head(dol, (void**)&data, &num_elems)) 
				{
					FLOG_ERROR("Unable to peek timer Q\n");
				}
			}
		}
	}
	return NULL;
}
