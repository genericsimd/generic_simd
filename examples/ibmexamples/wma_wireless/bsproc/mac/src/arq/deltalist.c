/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: deltalist.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include "deltalist.h"
#include "debug.h"
#include "arq_types.h"
#include "arq_ds.h"
#include "arq_defines.h"
#include "memmgmt.h"
#include "flog.h"

delta_list_t *create_init_delta_list (void)
{
	delta_list_t *dl;
	delta_list_node_t *dummy_node;
	
	dl = (delta_list_t *)WiMAX_mac_calloc(1, sizeof(delta_list_t));
	assert (dl != NULL);	

	*dl = (delta_list_t) EMPTY_DELTA_LIST;

	pthread_mutex_init (&dl->dl_mutex, NULL);
	pthread_cond_init (&dl->cond_is_empty, NULL);

	dummy_node = (delta_list_node_t *)WiMAX_mac_calloc(1, sizeof(delta_list_node_t));
	assert (dummy_node != NULL);

	dl->head = dl->tail = dummy_node;

	return dl;
}

// Insert a new timer whose expiry offset from time "curr_time" is specified in te. ref_time denotes the absolute expiry time of the timer 
// currently at the head of the list.
delta_list_node_t *insert_delta_list (delta_list_t *dl, timer_entry_t *te, const struct timeval ref_time, 
										const struct timeval curr_time, struct timeval *new_ref_time, delta_list_node_t **new_node)
{
	delta_list_node_t *next_node, *curr_node;
	struct timespec delta;
	struct timespec ref_ts;
	struct timespec curr_ts;

	assert (dl != NULL);

	//*new_node = NULL;
	next_node = (delta_list_node_t *)WiMAX_mac_calloc(1, sizeof(delta_list_node_t));
	assert (next_node != NULL);

	TIMEVAL_TO_TIMESPEC(&ref_time, &ref_ts);
	TIMEVAL_TO_TIMESPEC(&curr_time, &curr_ts);

	struct timespec ts; // Absolute time for the new timer to be inserted
	struct timespec head_delta;
	boolean insert_at_head = false;

	timeradd_ts(&curr_ts, &(te->delta), &ts);

	FLOG_DEBUG("insert_delta_list: curr time ts: sec -- %ld usec -- %ld delta: sec: %ld usec: %ld\n", 
												curr_time.tv_sec, curr_time.tv_usec, te->delta.tv_sec, te->delta.tv_nsec);
	FLOG_DEBUG("insert_delta_list: ref time ts: sec -- %ld usec -- %ld\n", ref_time.tv_sec, ref_time.tv_usec);
	FLOG_DEBUG("insert_delta_list: New timer's time ts: sec -- %ld usec -- %ld\n", ts.tv_sec, ts.tv_nsec/1000);

	FLOG_DEBUG("insert_delta_list: about to lock mutex\n");
	pthread_mutex_lock (&dl->dl_mutex);
	FLOG_DEBUG("insert_delta_list: mutex lock acquired\n");

	if (dl->head != dl->tail) { 
		// If curr_time is later than head_time (i.e., ref_time)
		if (timercmp_ts(&ref_ts, &curr_ts, <)) {
		struct timespec timediff;

		timersub_ts(&curr_ts, &ref_ts, &timediff);
		timeradd_ts(&(te->delta), &timediff, &(te->delta));
		} else {
			if (timercmp_ts(&ts, &ref_ts, <)) {
				insert_at_head = true;
				timersub_ts(&ref_ts, &ts, &head_delta);
			} else { //if (dl->head != dl->tail)
				timersub_ts(&ts, &ref_ts, &(te->delta));
			}
		}
	}

	if (insert_at_head == true || dl->head == dl->tail) {
		if (dl->head == dl->tail) {
			dl->tail = next_node;
		} else {
			dl->head->next->prev = next_node;
		}
		next_node->timer_entry = (*te);
		next_node->timer_entry.delta.tv_sec = next_node->timer_entry.delta.tv_nsec = 0;
		next_node->next = dl->head->next;
		next_node->prev = dl->head;
		dl->head->next = next_node;
		TIMESPEC_TO_TIMEVAL (new_ref_time, &ts);

		FLOG_DEBUG("insert_delta_list: Inserting to an empty timer list or at the head\n");
		FLOG_DEBUG("insert_delta_list: delta: sec -- %ld nsec -- %ld new ref time: sec -- %ld nsec -- %ld\n", next_node->timer_entry.delta.tv_sec, 
																					   next_node->timer_entry.delta.tv_nsec,
																					   new_ref_time->tv_sec, new_ref_time->tv_usec);
		
	} else {
		struct timespec new_node_delta = te->delta;

		FLOG_DEBUG("insert_delta_list: new timer's total delta: sec %ld nsec %ld\n", new_node_delta.tv_sec, new_node_delta.tv_nsec);

		delta.tv_sec = delta.tv_nsec = 0;
		for (curr_node = dl->head->next->next; curr_node != NULL; curr_node = curr_node->next) {
			timeradd_ts (&delta, &(curr_node->timer_entry.delta), &delta);
			if (timercmp_ts(&delta, &(te->delta), >)) {
				break;
			}
			timersub_ts (&new_node_delta, &(curr_node->timer_entry.delta), &new_node_delta);
		}	

		next_node->timer_entry = *te;
		next_node->timer_entry.delta = new_node_delta;
		FLOG_DEBUG("insert_delta_list: new timer's delta in the list: sec %ld nsec %ld\n", new_node_delta.tv_sec, new_node_delta.tv_nsec);

		if (curr_node != NULL) {
			timersub_ts(&(curr_node->timer_entry.delta), &new_node_delta, &(curr_node->timer_entry.delta));
			next_node->prev = curr_node->prev;
			next_node->next = curr_node;
			curr_node->prev->next = next_node;
			curr_node->prev = next_node;
		} else {
			next_node->prev = dl->tail;
			dl->tail->next = next_node;
			next_node->next = NULL;
			dl->tail = next_node;
		}
		*new_ref_time = ref_time;
	}

	assert (dl->head != dl->tail);
	assert (next_node != dl->head);

	*new_node = next_node;
	next_node->cross_ref_ptr = new_node;
	pthread_mutex_unlock (&dl->dl_mutex);
	FLOG_DEBUG("insert_delta_list: mutex unlocked\n");

	return next_node;
}

op_status_t dequeue_cond_delta_list (delta_list_t *dl, timer_entry_t *p_te, struct timespec ts)
{
	op_status_t ret_val = SUCCESS;

	assert (dl != NULL);

	FLOG_DEBUG("dequeue_cond_delta_list: about to lock mutex\n");
	pthread_mutex_lock (&(dl->dl_mutex));
	FLOG_DEBUG("dequeue_cond_delta_list: mutex lock acquired\n");

	if (dl->head == dl->tail) {
		ret_val = E_Q_EMPTY;
	} else if (timercmp_ts(&(dl->head->next->timer_entry.delta), &ts, >)) {
		ret_val = E_COND_NOT_MET;
	} else {
		delta_list_node_t *head_node;

		head_node = dl->head->next;
		*p_te = head_node->timer_entry;
		head_node->prev = NULL;
		*(head_node->cross_ref_ptr) = NULL;

		head_node = dl->head;
		dl->head = dl->head->next;
		head_node->next = NULL;

		WiMAX_mac_free (head_node);
	}

	pthread_mutex_unlock (&dl->dl_mutex);
	FLOG_DEBUG("dequeue_cond_delta_list: mutex unlocked\n");

	return ret_val;
}

op_status_t set_and_get_head_timer_val_delta_list (delta_list_t *dl, struct timespec set_delta, struct timespec *get_delta)
{
	op_status_t ret_val = SUCCESS;

	assert (dl != NULL);

	FLOG_DEBUG("set_and_get_timer_val_delta_list: about to lock mutex\n");
	pthread_mutex_lock (&(dl->dl_mutex));
	FLOG_DEBUG("set_and_get_timer_val_delta_list: mutex lock acquired\n");

	if (dl->head == dl->tail) {
		ret_val = E_Q_EMPTY;
	} else {
		struct timespec curr_delta;
		delta_list_node_t *head_node;

		head_node = dl->head->next;

		curr_delta = head_node->timer_entry.delta; 
		head_node->timer_entry.delta = set_delta;
		(*get_delta) = curr_delta;
	}

	pthread_mutex_unlock (&dl->dl_mutex);
	FLOG_DEBUG("set_and_get_timer_val_delta_list: mutex unlocked\n");

	return ret_val;
}

op_status_t delete_node_delta_list (delta_list_t *dl, delta_list_node_t **NODE)
{

	FLOG_DEBUG("delete_node_delta_list: about to lock mutex\n");
	pthread_mutex_lock (&dl->dl_mutex);
	FLOG_DEBUG"delete_node_delta_list: mutex lock acquired\n");

	delta_list_node_t *node = *NODE; // Don't move this line before the lock
								     // acquisition statement and invite
									 // TROUBLE

	assert (dl != NULL);
	//assert (node != NULL);
	//assert (node != dl->head);

	if (NULL != node && node != dl->head) {
		if (node->next != NULL) {
			node->next->prev = node->prev;
		}
		if (node->prev != NULL) {
			node->prev->next = node->next;
		}

		if (node == dl->tail) {
			dl->tail = dl->tail->prev;
		}
		WiMAX_mac_free (node);
		*NODE = NULL;
	}
	pthread_mutex_unlock (&dl->dl_mutex);
	FLOG_DEBUG("delete_node_delta_list: mutex unlocked\n");


	return SUCCESS;
}

void delete_nodes_dl (delta_list_t *dl)
{
	delta_list_node_t *node;

	assert (dl != NULL);

	FLOG_DEBUG("delete_nodes_dl: about to lock mutex\n");
	pthread_mutex_lock (&dl->dl_mutex);
	FLOG_DEBUG("delete_nodes_dl: mutex lock acquired\n");

	node = dl->head->next;

	while (node != NULL) {
		delta_list_node_t *next_node = node->next;
		WiMAX_mac_free (node);
		node = next_node;
	}

	dl->head->next = dl->head->prev = NULL;
	dl->tail = dl->head;
	pthread_mutex_unlock (&dl->dl_mutex);
	FLOG_DEBUG("delete_nodes_dl: mutex unlocked\n");
}
		
void destroy_delta_list (delta_list_t *dl)
{
	assert (dl != NULL);

	delete_nodes_dl (dl);

	assert (dl->head != NULL);
	WiMAX_mac_free (dl->head);

	pthread_mutex_destroy (&(dl->dl_mutex));
	pthread_cond_destroy (&(dl->cond_is_empty));

	WiMAX_mac_free(dl);
}
