/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: circq_array.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "circq_array.h"
#include "memmgmt.h"
#include "arq_types.h"
#include "debug.h"
#include "log.h"
#include "flog.h"
#include "mutex_macros.h"

// TO DO: Handling Q_FULL condition by increasing the allocation
//        as opposed to returning an error code


circq_array_t *create_circq_array (int array_size, int elem_size) {
	
	circq_array_t *circ_q;

	circ_q = (circq_array_t *)WiMAX_mac_malloc (sizeof(circq_array_t));
	assert (circ_q != NULL); // TO DO: Should assert be replaced by a return?

	*circ_q = (circq_array_t) EMPTY_QUEUE;

	circ_q->array = (char *)WiMAX_mac_malloc (array_size*elem_size);
	assert (circ_q->array != NULL);

	circ_q->array_size = array_size;
	circ_q->elem_size = elem_size;

	circ_q->head_idx = circ_q->tail_idx = 0;

	pthread_mutex_init (&circ_q->q_mutex, NULL);
	add_mutex_to_trace_record(&circ_q->q_mutex, "Circular Queue Mutex");
	pthread_cond_init (&circ_q->cond_is_empty, NULL);

	circ_q->grp_mutex = NULL;
	circ_q->grp_cond = NULL;

	return circ_q;
}

void destroy_circq_array (circq_array_t *circ_q) 
{
	assert (circ_q != NULL);
	if (circ_q->array != NULL) {
		WiMAX_mac_free(circ_q->array);
	}

	pthread_mutex_destroy (&circ_q->q_mutex);
	pthread_cond_destroy (&circ_q->cond_is_empty);

	if (circ_q != NULL) {
		WiMAX_mac_free (circ_q);
	}
}



op_status_t enqueue_circq_array (circq_array_t *circ_q, void *element, boolean is_locking, boolean blk_if_full) 
{
	int tail_offset;
	int next_tail_idx;

	FLOG_DEBUG("In enqueue_circq_array\n");

	
	// Detect queue full condition
	if ((circ_q->tail_idx+1)%circ_q->array_size == circ_q->head_idx) {
		FLOG_DEBUG( "Queue full\n");
		if (blk_if_full == true) {
			FLOG_DEBUG( "Enqueue: Trying to lock mutex...\n");
			pthread_mutex_lock (&circ_q->q_mutex);
			FLOG_DEBUG("Enqueue: Mutex locked and waiting for non-full queue...\n");
			if ((circ_q->tail_idx+1)%circ_q->array_size == circ_q->head_idx) {
				pthread_cond_wait (&circ_q->cond_is_empty, &circ_q->q_mutex);
			}
			FLOG_DEBUG("Enqueue unblocked...\n");
			pthread_mutex_unlock (&circ_q->q_mutex);
			FLOG_DEBUG("Unlocked cond mutex...\n");
		} else {
			return E_Q_FULL;
		}
	}

	FLOG_DEBUG("Queue not full\n");

	if (is_locking == true) {
		FLOG_DEBUG("Enqueue: Trying to lock mutex...\n");
		pthread_mutex_lock (&circ_q->q_mutex);
		FLOG_DEBUG("Enqueue: Mutex locked...\n");
	}

	next_tail_idx = (circ_q->tail_idx+1)%circ_q->array_size;

	tail_offset = (next_tail_idx)*circ_q->elem_size;
	assert (memcpy (&(circ_q->array[tail_offset]), element, circ_q->elem_size) != NULL);

	circ_q->tail_idx = next_tail_idx;


	if (is_locking == true) {
			pthread_mutex_unlock (&circ_q->q_mutex);
			FLOG_DEBUG("Enqueue: Mutex unlocked...\n");
	}


	if (circ_q->tail_idx == (circ_q->head_idx+1)%circ_q->array_size) { // If the queue was empty prior
	                                                                   // to this enqueue operation
																	   // No need to lock!!
		FLOG_DEBUG("Enqueue: Trying to lock mutex...\n");
		pthread_mutex_lock (&circ_q->q_mutex);
		FLOG_DEBUG("Enqueue: Mutex locked...\n");
		FLOG_DEBUG("Enqueue: Signaling...\n");
		pthread_cond_signal (&circ_q->cond_is_empty);
		pthread_mutex_unlock (&circ_q->q_mutex);
		FLOG_DEBUG("Enqueue: Mutex unlocked...\n");

		// Signal the group condition
		if (circ_q->grp_mutex != NULL) {
			pthread_mutex_lock (circ_q->grp_mutex);
			pthread_cond_signal(circ_q->grp_cond);
			pthread_mutex_unlock (circ_q->grp_mutex);
		}
	} 

	FLOG_DEBUG("Enqueue: head: %d tail: %d\n", circ_q->head_idx, circ_q->tail_idx);

	return SUCCESS;
}


op_status_t dequeue_circq_array (circq_array_t *circ_q, void *element, boolean is_locking, boolean blk_if_empty)
{
	int head_offset;
	int next_head_idx;

	FLOG_DEBUG("In dequeue_circq_array\n");


	// Detect empty queue condition
	if (circ_q->head_idx == circ_q->tail_idx) {
		FLOG_DEBUG("Queue empty\n");
		if (blk_if_empty == true) {
			FLOG_DEBUG("Dequeue: Trying to lock mutex...\n");
			pthread_mutex_lock (&circ_q->q_mutex);
			FLOG_DEBUG("Dequeue: Mutex locked and waiting for non-empty queue...\n");
			if (circ_q->head_idx == circ_q->tail_idx) {
				pthread_cond_wait (&circ_q->cond_is_empty, &circ_q->q_mutex);
			}
			FLOG_DEBUG("Dequeue: Cond mutex unlocked...\n");
			pthread_mutex_unlock (&circ_q->q_mutex);
		} else {
			return E_Q_EMPTY;
		}
	}

	if (is_locking == true) {
		FLOG_DEBUG("Dequeue: Trying to lock mutex...\n");
		pthread_mutex_lock (&circ_q->q_mutex);
		FLOG_DEBUG("Dequeue: Mutex locked...\n");
	}


	next_head_idx = (circ_q->head_idx+1)%circ_q->array_size;	

	head_offset = next_head_idx*circ_q->elem_size;
	//*element = circ_q->array+head_offset;
	assert (memcpy (element, circ_q->array+head_offset, circ_q->elem_size) != NULL);

	circ_q->head_idx = next_head_idx;


	if (is_locking == true) {
		pthread_mutex_unlock (&circ_q->q_mutex);
		FLOG_DEBUG("Dequeue: Mutex unlocked...\n");
	}

	if ((circ_q->tail_idx+2)%circ_q->array_size == circ_q->head_idx) { // If the Q were full before this dequeue
		FLOG_DEBUG("Dequeue: Trying to lock mutex...\n");
		pthread_mutex_lock (&circ_q->q_mutex);
		FLOG_DEBUG("Dequeue: Mutex locked...\n");
		TRACE(2, "Dequeue: Signaling...\n");
		pthread_cond_signal (&circ_q->cond_is_empty);
		pthread_mutex_unlock (&circ_q->q_mutex);
		FLOG_DEBUG("Dequeue: Mutex unlocked...\n");
		fflush (stdout);
	}  

	return SUCCESS;
}

boolean is_empty_circq_array (circq_array_t *circ_q)
{
	assert (circ_q != NULL);

	if (circ_q->head_idx == circ_q->tail_idx) {
		return true;
	} else {
		return false;
	}

	// return true; // This statement will not be reached
}

void set_grp_cond_and_mutex_circq_array (circq_array_t *circ_q, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
	assert (circ_q != NULL);
	assert (mutex != NULL);
	assert (cond != NULL);

	circ_q->grp_mutex = mutex;
	circ_q->grp_cond = cond;
}
