/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: thd_utils.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef __THD_UTILS_H_
#define __THD_UTILS_H_

#include <pthread.h>

#define SET_PRI 2
#define MAX_PRI 1
#define MIN_PRI 0

int set_thread_pri(pthread_attr_t * thread_attr, int policy, int flag, int priority);
int set_affinity(pthread_attr_t * thread_attr, int cpu_idx);

#endif /* __GPP_UTILS_H_ */

