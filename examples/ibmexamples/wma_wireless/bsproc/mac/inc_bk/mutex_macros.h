/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mutex_macros.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifdef LOG_METRICS
#ifndef _MUTEX_MACROS_H_
#define _MUTEX_MACROS_H_

#include <sys/syscall.h>
#include "log.h"
#include "util.h"
/*
#define pthread_mutex_lock(la)  LOG_EVENT_lock_trying(readtsc(), la);\
								pthread_mutex_lock(la);\
								LOG_EVENT_lock_acquired(readtsc(), la);

#define pthread_mutex_unlock(la) LOG_EVENT_lock_released(readtsc(), la);\
								 pthread_mutex_unlock(la);

*/
#endif  // _MUTEX_MACROS_H_
#endif
