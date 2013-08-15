/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: dll_ordered_list.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Apr.2009       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _DLL_ORDERED_LIST_H_
#define _DLL_ORDERED_LIST_H_

#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include "q_container.h"
#include "sll_fifo_q.h"

//Doubly lined list ordered list
//  |tail|elem|elem|elem|...|elem|head|
//  |kyn| >= |kyn-1| >= |ky n-2| >= ... >= |head key|
//Ordered removal from head

typedef struct dllorderedlist dll_ordered_list;
struct dllorderedlist {
  q_container* head;
  q_container* tail;
  q_container* last_visited;
  q_aggr_info* q_info;
  pthread_mutex_t qmutex;
  pthread_cond_t checkq;
  int head_chgd;
};

// initialize the queue
extern int dll_ordered_list_init(dll_ordered_list** dol);


extern int insert_dll_ordered_list(dll_ordered_list* timer_ev_q, unsigned long long key, void* data, size_t len, int* num_elems);

extern int remove_dll_ordered_list(dll_ordered_list* timer_ev_q, unsigned long long key, void** data, int* num_elems);

extern int pop_head(dll_ordered_list* dol, void** data, int* num_elems);

extern int peek_head(dll_ordered_list* dol, void** data, int* num_elems);

extern int dll_ordered_list_cleanup(dll_ordered_list* dol);
#endif

