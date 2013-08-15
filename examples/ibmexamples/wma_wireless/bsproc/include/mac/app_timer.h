/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: app_timer.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Apr.2009       Created										Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _APP_TIMER_H_
#define _APP_TIMER_H_

#include <stdlib.h>
#include "sll_fifo_q.h"
#include "thread_sync.h"

#define WAITING_TIME_BTW_SUCCESSIVE_MSGS 3000000LL
#define TIMEOUT_INTERVAL_2ENTITY_MSGS 3000000LL
#define TIMEOUT_INTERVAL_3ENTITY_MSGS 7000000LL

typedef int (*timeout_action) (void* args);

typedef struct {
  int timer_id;
  unsigned long long absolute_time;
  u_int8_t expired;
  u_int8_t deleted;
  u_int32_t flags;
  timeout_action action;
  sll_fifo_q* event_queue;
  size_t len;
  void* application_data;
  void* timer_specific_data;
} app_timer;

//This function is used to create timers
// abstime -- timer expiry in microseconds 
// f_ptr --- function to call upon expiry of timer; should be non-blocking code
// ev_q -- queue to enqueue data upon expiry
//         either f_ptr or ev_q are valid but not both
// data  -- used by caller to decide on application state and actions
// to performon timer expiry
// flags  -- timer configurable flags
// returns 0 on success

	
extern int app_timer_add(long long int abstime,
		 	 timeout_action f_ptr,
			 sll_fifo_q* ev_q,
			 void* data,
			 size_t len,
			 u_int32_t flags, 
			 void** timer_id,void* timer_specific_data);

extern void* timer_handler(void* timer_list);

extern int app_timer_delete(void* timer_id);

extern int app_timer_init();

#endif //_APP_TIMER_H_
