/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: ll.c

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
#include "ll.h"
#include "debug.h"
#include "memmgmt.h"
#include "arq_defines.h"
#include "arq_const.h"
#include "log.h"
#include "flog.h"
#include "mutex_macros.h"


// Memory is not allocated to ll. ll should have been
// created either statically or dynamically prior
// to a call to this function.
linked_list_t *create_init_linked_list (size_t el_size)
{
	linked_list_t *ll;
	ll_node_t *dummy_node;

	
	ll = (linked_list_t *)WiMAX_mac_calloc(1, sizeof(linked_list_t));
	assert (ll != NULL);	

	*ll = (linked_list_t) EMPTY_LIST;

	ll->el_size = el_size;

	pthread_mutex_init (&ll->ll_mutex, NULL);
	add_mutex_to_trace_record(&ll->ll_mutex, "Linked List Mutex");
	pthread_cond_init (&ll->cond_is_empty, NULL);

	dummy_node = (ll_node_t *)WiMAX_mac_calloc(1, sizeof(ll_node_t)+ll->el_size);
	assert (dummy_node != NULL);
	FLOG_DEBUG("LL ALLOCED: %p\n", dummy_node);

	ll->head = ll->tail = dummy_node;

	return ll;
}

ll_node_t *enqueue_linked_list (linked_list_t *ll, long int key,  void *data, boolean is_locking)
{
	ll_node_t *next_node;

	assert (ll != NULL);

	next_node = (ll_node_t *)WiMAX_mac_calloc(1, sizeof(ll_node_t)+ll->el_size);
	assert (next_node != NULL);
	FLOG_DEBUG("LL ALLOCED: %p\n", next_node);

	next_node->key = key;

	assert (data != NULL);
	memcpy (next_node->data, data, ll->el_size);

	if (is_locking == true) {
		pthread_mutex_lock (&ll->ll_mutex);
	}
	next_node->prev = ll->tail;
	next_node->next = NULL;
	ll->tail->next = next_node;
	ll->tail = next_node;

	if (is_locking == true) {
		pthread_mutex_unlock (&ll->ll_mutex);
	}

	return next_node;
}

ll_node_t *enqueue_unique_linked_list (linked_list_t *ll, long int key,  void *data)
{
	ll_node_t *next_node = NULL;

	assert (ll != NULL);
	pthread_mutex_lock (&ll->ll_mutex);

	//if (ll->head != ll->tail) {
		ll_node_t *node;

		node = ll->head->next;

		while (node != NULL && key != node->key) {
			node = node->next;
		}

		if (node == NULL) { // A node with key does not exist, so enqueue
			next_node = (ll_node_t *)WiMAX_mac_calloc(1, sizeof(ll_node_t)+ll->el_size);
			assert (next_node != NULL);
			FLOG_DEBUG("LL ALLOCED: %p\n", next_node);

			next_node->key = key;

			assert (data != NULL);
			memcpy (next_node->data, data, ll->el_size);

			next_node->prev = ll->tail;
			next_node->next = NULL;
			ll->tail->next = next_node;
			ll->tail = next_node;
		}
	//}

	pthread_mutex_unlock (&ll->ll_mutex);

	return next_node;
}



ll_node_t *enqueue_arq_wnd_order_linked_list (linked_list_t *ll, long int key,  void *data, short arq_wnd_start)
{
	ll_node_t *next_node = NULL;

	assert (ll != NULL);
	pthread_mutex_lock (&ll->ll_mutex);

	//if (ll->head != ll->tail) {
		ll_node_t *node;

		node = ll->head->next;

		while (node != NULL && mod (key-arq_wnd_start, ARQ_BSN_MODULUS) >
						   mod (node->key-arq_wnd_start, ARQ_BSN_MODULUS)) {
			node = node->next;
		}

		if (NULL == node || node->key != key) {
			next_node = (ll_node_t *)WiMAX_mac_calloc(1, sizeof(ll_node_t)+ll->el_size);
			assert (next_node != NULL);
			FLOG_DEBUG("LL ALLOCED: %p\n", next_node);
			next_node->key = key;

			assert (data != NULL);
			memcpy (next_node->data, data, ll->el_size);

			if (NULL == node ) { // Enqueue as the last node

				next_node->prev = ll->tail;
				next_node->next = NULL;
				ll->tail->next = next_node;
				ll->tail = next_node;
			}
			else {
				node->prev->next = next_node;
				next_node->prev = node->prev;
				next_node->next = node;
				node->prev = next_node;
			}
		}
	//}
	pthread_mutex_unlock (&ll->ll_mutex);

	return next_node;
}


op_status_t dequeue_linked_list (linked_list_t *ll, long int *key,  void *data, boolean is_locking)
{
	ll_node_t *head_node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	if (is_locking == true) {
		pthread_mutex_lock (&ll->ll_mutex);
	}

	if (ll->head == ll->tail) {
		ret_val = E_Q_EMPTY;
	} else {
		head_node = ll->head->next;
		*key = head_node->key;
		memcpy (data, head_node->data, ll->el_size);
		head_node->prev = NULL;

		head_node = ll->head;
		ll->head = ll->head->next;
		FLOG_DEBUG("LL About to FREE: %p\n", head_node);
		WiMAX_mac_free (head_node);
		FLOG_DEBUG("LL FREED: %p\n", head_node);
	}
	if (is_locking == true) {
		pthread_mutex_unlock (&ll->ll_mutex);
	}

	return ret_val;
}


op_status_t delete_node_linked_list (linked_list_t *ll, ll_node_t *node)
{

	assert (ll != NULL);
	assert (node != NULL);

	pthread_mutex_lock (&ll->ll_mutex);

	assert (node != ll->head);

	if (node == ll->tail) {
		ll->tail = ll->tail->prev;
	}

	if (node->next != NULL) {
		node->next->prev = node->prev;
	}
	if (node->prev != NULL) {
		node->prev->next = node->next;
	}

	FLOG_DEBUG("LL About to FREE: %p\n", node);
	WiMAX_mac_free (node);
	FLOG_DEBUG("LL FREED: %p\n", node);

	pthread_mutex_unlock (&ll->ll_mutex);

	return SUCCESS;
}


op_status_t find_element_ll (linked_list_t *ll, long int key, void *data)
{
	ll_node_t *node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	pthread_mutex_lock (&ll->ll_mutex);

	if (ll->head == ll->tail) {
		ret_val = E_Q_EMPTY;
	} else {

		node = ll->head->next;

		while (node != NULL && key != node->key) {
			node = node->next;
		}

		if (node != NULL) {
			memcpy (data, node->data, ll->el_size);
		} else {
			ret_val =  E_LL_NOT_FOUND;
		}
	}

	pthread_mutex_unlock( &ll->ll_mutex);
	return ret_val;
}

op_status_t peek_ll (linked_list_t *ll, long int *key, void *data, boolean is_locking)
{
	ll_node_t *head_node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	if (is_locking == true) {
		pthread_mutex_lock (&ll->ll_mutex);
	}

	if (ll->head == ll->tail) {
		pthread_mutex_unlock (&ll->ll_mutex);
		ret_val = E_Q_EMPTY;
	} else {

		head_node = ll->head->next;
		*key = head_node->key;
		memcpy (data, head_node->data, ll->el_size);
	}

	if (is_locking == true) {
		pthread_mutex_unlock (&ll->ll_mutex);
	}

	return SUCCESS;
}

op_status_t update_head_ll (linked_list_t *ll, long int key, void *data, boolean is_locking)
{
	ll_node_t *head_node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	if (is_locking == true) {
		pthread_mutex_lock (&ll->ll_mutex);
	}

	if (ll->head == ll->tail) {
		ret_val = E_Q_EMPTY;
	} else {
		head_node = ll->head->next;
		head_node->key = key;

		assert (data != NULL);
		memcpy (head_node->data, data, ll->el_size);
	}

	if (is_locking == true) {
		pthread_mutex_unlock (&ll->ll_mutex);
	}

	return ret_val;
}

op_status_t unlink_element_ll (linked_list_t *ll, long int key, void *data)
{
	ll_node_t *node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	pthread_mutex_lock (&ll->ll_mutex);

	if (ll->head == ll->tail) {
		ret_val = E_Q_EMPTY;
	} else {
		node = ll->head->next;

		while (node != NULL && key != node->key) {
			node = node->next;
		}

		if (node != NULL) {
			if (node->next != NULL) {
				node->next->prev = node->prev;
			} else {
				ll->tail = node->prev;
			}
			memcpy(data, node->data, ll->el_size);
			node->prev->next = node->next;

			WiMAX_mac_free (node);
		} else {
			ret_val = E_LL_NOT_FOUND;
		}
	}
	pthread_mutex_unlock( &ll->ll_mutex);
	return ret_val;
}


op_status_t unlink_and_discard_element_ll (linked_list_t *ll, long int key)
{
	ll_node_t *node;
	op_status_t ret_val = SUCCESS;

	assert (ll != NULL);

	pthread_mutex_lock (&ll->ll_mutex);

	if (ll->head == ll->tail) {
		ret_val = E_Q_EMPTY;
	} else {
		node = ll->head->next;

		while (node != NULL && key != node->key) {
			node = node->next;
		}

		if (node != NULL) {
			if (node->next != NULL) {
				node->next->prev = node->prev;
			} else {
				ll->tail = node->prev;
			}
			node->prev->next = node->next;

			WiMAX_mac_free (node);
		} else {
			ret_val = E_LL_NOT_FOUND;
		}
	}
	pthread_mutex_unlock( &ll->ll_mutex);
	return ret_val;
}

boolean is_ll_empty (linked_list_t *ll)
{	
	if (NULL == ll) {
		return true;
	}

	boolean is_empty = false;
	pthread_mutex_lock (&ll->ll_mutex);

	if (ll->head == ll->tail) {
		is_empty = true;
	}

	pthread_mutex_unlock (&ll->ll_mutex);
	return is_empty;
}

// Currently, this function frees data contained in the nodes too,
// although memory for data is allocated externally
void delete_nodes_ll (linked_list_t *ll)
{
	ll_node_t *node;

	assert (ll != NULL);

	pthread_mutex_lock (&ll->ll_mutex);

	node = ll->head->next;

	while (node != NULL) {
		ll_node_t *next_node = node->next;
		WiMAX_mac_free (node);
		node = next_node;
	}

	ll->head->next = ll->head->prev = NULL;
	ll->tail = ll->head;

	pthread_mutex_unlock (&ll->ll_mutex);
}
		
void destroy_linked_list (linked_list_t *ll)
{
	assert (ll != NULL);

	delete_nodes_ll (ll);

	assert (ll->head != NULL);
	WiMAX_mac_free (ll->head);

	pthread_mutex_destroy (&(ll->ll_mutex));
	pthread_cond_destroy (&(ll->cond_is_empty));

	WiMAX_mac_free(ll);
}

