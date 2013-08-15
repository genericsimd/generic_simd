/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: dll_ordered_list.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Apr.2009       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "dll_ordered_list.h"
#include "memmgmt.h"
#include "debug.h"

int dll_ordered_list_init(dll_ordered_list** dol) {
  if(app_malloc((void**)dol, sizeof(dll_ordered_list))) {
    FLOG_FATAL("Error Allocating Memory dll_ordered_list\n");
    exit(-1);
  }
  if(app_malloc((void**)&((*dol)->q_info), sizeof(q_aggr_info))) {
    FLOG_FATAL("Error Allocating Memory dll_ordered_list(q_aggr_info)\n");
    exit(-1);
  }
  (*dol)->head=NULL;
  (*dol)->tail=NULL;
  (*dol)->last_visited=NULL;
  (*dol)->head_chgd=0;
  (*dol)->q_info->num_elems=0;

  if(pthread_mutex_init(&((*dol)->qmutex), NULL)) {
    FLOG_FATAL("Error Initializing dll_ordered_list mutex \n");
  }

  if(pthread_cond_init(&((*dol)->checkq), NULL)) {
    FLOG_FATAL("Error Initializing dll_ordered_list checkq conditional variable \n");
  }
  return 0;
}


int insert_dll_ordered_list(dll_ordered_list* dol, unsigned long long key, void* data, size_t len, int* num_elems) {
  q_container* q_cntr=NULL;
  if(app_malloc((void**)&q_cntr, sizeof(q_container))) {
    return -1;
  }
  
  assert(q_cntr!=NULL);

  q_cntr->data=data;
  q_cntr->len=len;
  q_cntr->next=NULL;
  q_cntr->prev=NULL;
  q_cntr->key=key;

  pthread_mutex_lock(&(dol->qmutex));
  //If the list is empty add this rightway
  if(dol->q_info->num_elems==0) {
    dol->head=q_cntr;
    dol->tail=q_cntr;
    dol->last_visited=q_cntr;
    dol->head_chgd=1;
  }
  else {
    assert(dol->last_visited!=NULL);
    q_container* curr=dol->last_visited;
    if(key<dol->last_visited->key) {
      // walk towards head
      // use prev
      while((curr!=NULL) &&(key<curr->key)) {
	curr= curr->prev;
      }
      //we are here 
      //either curr is NULL
      //or key>=curr->key
      if(curr==NULL) {
	// make the new node head
	q_cntr->next=dol->head;
	dol->head->prev=q_cntr;
	dol->head=q_cntr;
	dol->last_visited=q_cntr;
	dol->head_chgd=1;
      }
      else {
	// insert approriately
	assert(curr!=NULL);
	q_cntr->prev=curr;
	q_cntr->next=curr->next;
	curr->next=q_cntr;
	q_cntr->next->prev=q_cntr;
      }
    }
    else if(key>dol->last_visited->key) {
      // walk towards tail
      // use next
      while((curr!=NULL) &&(key>curr->key)) {
	curr= curr->next;
      }
      //we are here 
      //either curr is NULL
      //or key<=curr->key
      if(curr==NULL) {
	// make the new node tail
	q_cntr->prev=dol->tail;
	dol->tail->next=q_cntr;
	dol->tail=q_cntr;
	dol->last_visited=q_cntr;
      }
      else {
	// insert approriately
	assert(curr!=NULL);
	q_cntr->next=curr;
	q_cntr->prev=curr->prev;
	curr->prev=q_cntr;
	q_cntr->prev->next=q_cntr;
      }
    }
    else { //key==dol->last_visited->key
      // can insert right away
      q_cntr->prev=dol->last_visited;
      q_cntr->next=dol->last_visited->next;
      dol->last_visited->next=q_cntr;
      if(q_cntr->next!=NULL) {
	q_cntr->next->prev=q_cntr;	
      }
    }
  }
  dol->q_info->num_elems +=1;
  dol->last_visited=q_cntr;
  *num_elems=dol->q_info->num_elems;
  if(dol->head_chgd==1) {
    pthread_cond_signal(&(dol->checkq));
  }
  pthread_mutex_unlock(&(dol->qmutex));
  return 0;
}


//currently assumes single instance of key found
int remove_dll_ordered_list(dll_ordered_list* dol, unsigned long long key, void** data, int* num_elems) {
  q_container* q_cntr=NULL;

  pthread_mutex_lock(&(dol->qmutex));
  q_cntr=dol->last_visited;
  //If the list is empty add this rightway
  if(dol->q_info->num_elems==0) {
    *data=NULL;
  }
  else {
    assert(dol->last_visited!=NULL);
    q_container* curr=dol->last_visited;
    if(key<dol->last_visited->key) {
      // walk towards head
      // use prev
      while((curr!=NULL) &&(key<curr->key)) {
	curr= curr->prev;
      }
      //we are here 
      //either curr is NULL
      //or key>=curr->key
      if(curr==NULL) {
	//data not found
	*data=NULL;
      }
      else {
	// insert approriately
	assert(curr!=NULL);
	if(curr->key==key) {
	  if(curr->prev!=NULL) {
	    curr->prev->next=curr->next;
	    dol->last_visited=curr->prev;
	  }
	  if(curr->next!=NULL) {
	    curr->next->prev=curr->prev;
	    dol->last_visited=curr->next;
	  }
	  dol->q_info->num_elems--;
	}
	else {
	  //data not found
	  *data=NULL;
	}
      }
    }
    else if(key>dol->last_visited->key) {
      // walk towards tail
      // use next
      while((curr!=NULL) &&(key>curr->key)) {
	curr= curr->next;
      }
      //we are here 
      //either curr is NULL
      //or key<=curr->key
      if(curr==NULL) {
	//data not found
	*data=NULL;
      }
      else {
	if(curr->key==key) {
	  if(curr->prev!=NULL) {
	    curr->prev->next=curr->next;
	    dol->last_visited=curr->prev;
	  }
	  if(curr->next!=NULL) {
	    curr->next->prev=curr->prev;
	    dol->last_visited=curr->next;
	  }
	  dol->q_info->num_elems--;
	}
	else {
	  //data not found
	  *data=NULL;
	}
      }
    }
    else { //key==dol->last_visited->key
      curr=dol->last_visited;
      if(curr->prev!=NULL) {
	curr->prev->next=curr->next;
	dol->last_visited=curr->prev;
      }
      if(curr->next!=NULL) {
	curr->next->prev=curr->prev;
	dol->last_visited=curr->next;
      }
      dol->q_info->num_elems--;
    }
  }
  *num_elems=dol->q_info->num_elems;
  pthread_mutex_unlock(&(dol->qmutex));
  return 0;
}

int pop_head(dll_ordered_list* dol, void** data, int* num_elems) {
  q_container* q_cntr=NULL;
  *data=NULL;
  *num_elems=0;
  pthread_mutex_lock(&(dol->qmutex));
  if(dol->q_info->num_elems>0) {
    assert(dol->head!=NULL);
    q_cntr=dol->head;
    dol->head=dol->head->next;
    if(dol->head!=NULL) {
      dol->head->prev=NULL;
    }
    dol->q_info->num_elems--;
    if(dol->last_visited==q_cntr) {
      dol->last_visited=dol->head;
    }
    *num_elems=dol->q_info->num_elems;
    (*data)=q_cntr->data;
    //free the q_cntr
    app_free(q_cntr, sizeof(q_container));
  }
  pthread_mutex_unlock(&(dol->qmutex));
  return 0;
}

int peek_head(dll_ordered_list* dol, void** data, int* num_elems) {

  (*data)=NULL;
  *num_elems=0;

  pthread_mutex_lock(&(dol->qmutex));
  if(dol->q_info->num_elems>0) {
    assert(dol->head!=NULL);
    (*data)=dol->head->data;
    *num_elems=dol->q_info->num_elems;
  }
  pthread_mutex_unlock(&(dol->qmutex));
  return 0;
}

int dll_ordered_list_cleanup(dll_ordered_list* dol) {
  //cleaup the dol
  pthread_mutex_lock(&(dol->qmutex));
  q_container* q_cntr=dol->head;
  while(q_cntr!=NULL) {
    q_container* tmp_cntr=q_cntr;
    q_cntr=q_cntr->next;
    app_free(tmp_cntr->data, tmp_cntr->len);
    //free the q_cntr
    app_free(tmp_cntr, sizeof(q_container));
  }
  dol->head_chgd = 1;
  dol->q_info->num_elems = 0;
  dol->head = NULL;
  pthread_mutex_unlock(&(dol->qmutex));
  return 0;
}
