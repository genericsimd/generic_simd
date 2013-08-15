/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_cs_consumer.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "ul_cs_consumer.h"

void* ul_cs(void* arg)
{
    struct timeval before_execution_time;
    struct timeval after_execution_time;
    double before_execution_time_in_us;
    double after_execution_time_in_us;
    double next_execution_time_in_us;
    double dl_duration;
    double time_diff;
    int first_frame = 1;

    sdu_queue* ul_sduq;


    dl_duration = CONSUME_INTERVAL * 1000;
    // first sleep the given period of time;
    usleep(dl_duration);

    get_sduq(&ul_sduq, 0);
    
    while(can_sync_continue())
    {
        // waiting for some signal
        // pthread_mutex_lock(scheduler_call_lock);
        // pthread_cond_wait(scheduler_call, scheduler_call_lock);
      
        gettimeofday (&before_execution_time, NULL);
        before_execution_time_in_us = before_execution_time.tv_sec * 1000000 + before_execution_time.tv_usec;
        // printf("current time is %lf \n",  before_execution_time_in_us);
        if (first_frame){
            next_execution_time_in_us = before_execution_time_in_us + dl_duration;
            first_frame = 0;
        }
        else
        {
            next_execution_time_in_us += dl_duration;
        }

		//pthread_rwlock_rdlock(&conn_info_rw_lock);
        // now try to access the uplink sdu queue
        dequeue_ul_sduq(ul_sduq);
        //pthread_rwlock_unlock(&conn_info_rw_lock);


        gettimeofday (&after_execution_time, NULL);
        after_execution_time_in_us = after_execution_time.tv_sec * 1000000 + after_execution_time.tv_usec;
        time_diff = next_execution_time_in_us - after_execution_time_in_us;

        if (time_diff > 0)
        {
            usleep(time_diff);
        }


    }

    return NULL;
}
