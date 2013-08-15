/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: thd_utils.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


/** OS */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//#define _GNU_SOURCE
#include <pthread.h>

/** Application */
#include "thd_util.h"
#include "flog.h"

int set_thread_pri(pthread_attr_t * thread_attr, int policy, int flag, int priority)
{
    int max_priority;
    int min_priority;

    struct sched_param schedule_param;
    int ret;

    if ((policy != SCHED_FIFO) && (policy != SCHED_RR) && (policy != SCHED_OTHER))
    {
        FLOG_ERROR("Wrong schedule policy set\n");
        return 1;
    }

    if ( thread_attr == NULL )
    {
        FLOG_ERROR("NULL thread Attr\n");
        return 1;
    }

    /* make sure thread priority in allowed range */

    max_priority = sched_get_priority_max (policy);
    min_priority = sched_get_priority_min (policy);

    if (flag == SET_PRI)
    {
        if (priority > max_priority)
        {
                priority = max_priority;
        }
        else if (priority < min_priority)
        {
                priority = min_priority;
        }

    }else if (flag == MAX_PRI)
    {
        priority = max_priority;

    }else if (flag == MIN_PRI)
    {
        priority = min_priority;
    }else
    {
        FLOG_ERROR("Wrong priority FLAG set\n");
        return 1;
    }

    /* set schedule policy */

    ret = pthread_attr_setschedpolicy (thread_attr, policy);

    if (ret != 0)
    {
        FLOG_ERROR ("error in pthread_attr_setschedpolicy\n");
        return 1;
    }

    /* set schedule priority */
    schedule_param.sched_priority = priority;
    ret = pthread_attr_setschedparam (thread_attr, &schedule_param);

    if (ret != 0)
    {
        FLOG_ERROR ("error in pthread_setschedparam\n");
        return 1;
    }

    ret = pthread_attr_setinheritsched (thread_attr, PTHREAD_EXPLICIT_SCHED);

    if (ret != 0)
    {
        FLOG_ERROR ("error in pthread_attr_setinheritsched()\n");
        return 1;
    }

    return 0;
}

/*
int set_affinity(pthread_attr_t * thread_attr, int cpu_idx)
{
    cpu_set_t cpuset;
    int ret = 0;

    __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
    __CPU_SET_S(cpu_idx, sizeof (cpu_set_t), &cpuset);
    ret = pthread_attr_setaffinity_np(thread_attr, sizeof(cpu_set_t), &cpuset);

    if (ret != 0)
    {
        FLOG_ERROR ("Set CPU affinity error\n");
        return 1;
    }

    return 0;

}
*/

