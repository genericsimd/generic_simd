/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: binheap.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
//#define DDEBUG
//#define TRACE_LEVEL 3

#include <stdlib.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <assert.h>
#include <limits.h>
#include <memmgmt.h>
#include "binheap.h"
#include "arq_ds.h" // To be removed  with the special print function
#include "debug.h"
#include "log.h"
#include "mutex_macros.h"
#include "perf_log.h"
#include "flog.h"

extern int print_timer_entry(void *a);
extern int timespec_cmp(void*, void*);
void heap_node_print (bin_heap_node_t *node);


bin_heap_t *create_init_bin_heap(void)
{
	bin_heap_t *h;

	h = (bin_heap_t *)WiMAX_mac_calloc(1, sizeof(bin_heap_t));
	assert(h != NULL);

	pthread_mutex_init (&h->bh_mutex, NULL);
	add_mutex_to_trace_record(&h->bh_mutex, "Binary heap mutex");
	h->nxt_lev_st = 1;

	return h;
}

void heap_print (bin_heap_t *heap) 
{
	FLOG_DEBUG("heap_print: num_nodes: %d\n", heap->num_nodes);
	//printf("heap_print: num_nodes: %d\n", heap->num_nodes);
	//heap_node_print(heap->root);
}

void __heap_decrease(heap_prio_t higher_prio, struct bin_heap *heap, struct bin_heap_node *node)
{
	if (NULL == node || node->value == NULL) {
		return;
	}

	assert (node->back != NULL);

	if (NULL == node->back) {
		return;
	}

	struct bin_heap_node *parent;
	void *tmp;
	struct bin_heap_node **sav;

	parent = node->parent;
	while (parent && higher_prio(node->value, parent->value)) {
		/* Swap values in node and parent */
		tmp = node->value;
		node->value = parent->value;
		parent->value = tmp;
		assert(parent->back != NULL);
		if (parent->back)
			*(parent->back) = node;
		*(node->back)   = parent;
		sav        = parent->back;
		parent->back    = node->back;
		node->back      = sav;
		node   = parent;
		parent = node->parent;
	}
}

op_status_t __heap_increase(heap_prio_t higher_prio, struct bin_heap* heap,
				 struct bin_heap_node* node)
{
	if (NULL == node) {
		return E_NULL;
	}
	/* node's value is increased, so its priority is decreased, 
	we need to update its position */

	while (node->left != NULL || node->right != NULL) {
		struct bin_heap_node *cand;
		if (NULL == node->right || higher_prio(node->left->value, node->right->value)) {
			cand = node->left;	
		} else {
			cand = node->right;
		}
		/* bubble down if necessary */
		if (higher_prio(cand->value, node->value)) {
			void *tmp;
			struct bin_heap_node **sav;
			/* swap values of node and cand */
			tmp = node->value;
			node->value = cand->value;
			cand->value = tmp;
			/* swap back pointers */
			assert (cand->back != NULL);
			if (cand->back) {
				*(cand->back) = node;
			}
			assert (node->back != NULL);
			*(node->back) = cand;
			sav = cand->back;
			cand->back = node->back;
			node->back = sav;
		} else {
			// Node already has higher priority than all both its
			// children, so stop
			break;
		}
		node = cand;
	}

	return SUCCESS;
}

struct bin_heap_node *__heap_find_node_from_num (struct bin_heap* heap, int node_idx, int node_level)
{
#define LEFT  0
#define RIGHT 1
#define MASK_TYPE unsigned char

	struct bin_heap_node *node;
	unsigned char dir_mask[] = {0, 0, 0, 0};
	unsigned char bit_mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	
	assert (node_idx <= heap->num_nodes);

	if (node_idx > heap->num_nodes) {
		return NULL;
	}

	FLOG_DEBUG( "__heap_find_node_from_num: node_idx: %d node_level: %d\n", node_idx, node_level);

	int mask_bits = sizeof(MASK_TYPE)*8;
	int level = node_level-1;
	int curr = node_idx;
	int parent;
	unsigned char dir;

	while (curr > 1 /* && level > 0*/) {
		int idx, offset;

		//TRACE3(4, "curr: %d level: %d parent: %d\n", curr, level, parent);
		assert (level > 0);
		parent = curr/2;
		dir = (parent*2 < curr) ? RIGHT : LEFT;
		
		idx = level/mask_bits;
		offset = level%mask_bits;

		if (RIGHT == dir) {
			dir_mask[idx] |= bit_mask[offset];
		}
		curr = parent;
		level--;
	}

	// Find the node using the path constructed
	// above
	node = heap->root;
	for (int i = 1; i < node_level; i++) {
		int idx, offset;
		
		idx = i/mask_bits;
		offset = i%mask_bits;

		dir = (dir_mask[idx] & bit_mask[offset]) >> (mask_bits-1-offset);
		//TRACE4(4, "level: %d idx: %d offset: %d dir: %d\n", i, idx, offset, dir);
		assert (NULL != node);
		if (i <= node_level-1) {
			if (LEFT == dir) {
				// Take the left branch
				node = node->left;
			} else if (RIGHT == dir) {
				node = node->right;
			} else {
				assert (0);
			}
		} 
	}
	return node;
}

op_status_t __heap_extract_top_value(heap_prio_t higher_prio, struct bin_heap *heap, void **node_value)
{
	struct bin_heap_node *final;

	if (heap->num_nodes == 0) {
		return E_Q_EMPTY;
	}

	final = __heap_find_node_from_num(heap, heap->num_nodes, heap->num_levels);

	assert (final != NULL);	

	*node_value = heap->root->value;
	// Copy contents of the last node to root
	heap->root->value = final->value;
	heap->root->back = final->back;
	assert(heap->root->back);
	if (heap->root->back) {
		*(heap->root->back) = heap->root;
	}

	// Update parent's node child pointer
	if (final->parent != NULL) {
		if (heap->num_nodes%2 == 0) {
			final->parent->left = NULL;
		} else {
			final->parent->right = NULL;
		}
	} else {
		assert (final == heap->root);
		heap->root = NULL;
	}
	// delete final
	WiMAX_mac_free (final);

	heap->num_nodes--;
	if (heap->num_nodes < heap->nxt_lev_st/2) {
		heap->num_levels--;
		heap->nxt_lev_st = heap->nxt_lev_st/2;
		//TRACE3 (4, "__heap_extract_top_value: num_nodes: %d num_levels: %d next level starts at: %d\n", 
							//heap->num_levels, heap->num_nodes, heap->nxt_lev_st);
	}

	__heap_increase (higher_prio, heap, heap->root);
	return SUCCESS;
}


op_status_t heap_increase(heap_prio_t higher_prio, heap_copy_value_t copy, struct bin_heap *heap, 
							 struct bin_heap_node* node, void *new_value)
{
	if (NULL == node) {
		return SUCCESS;
	}

	if (node->back == NULL) {
		return E_FAILED;
	}

	op_status_t ret_val;
	FLOG_DEBUG("In heap_increase...\n");
	pthread_mutex_lock (&heap->bh_mutex);

	heap_node_print(heap->root);
	// Make sure that the new value is larger than the current value
	assert (higher_prio (node->value, new_value));
	// Copy the new value
	copy (node->value, new_value);	
	
	ret_val = __heap_increase (higher_prio, heap, node);
	heap_print(heap);

	heap_node_print(heap->root);
	pthread_mutex_unlock (&heap->bh_mutex);
	FLOG_DEBUG("End of  heap_increase...\n");

	return (ret_val);
}

struct bin_heap_node *heap_insert_value(heap_prio_t higher_prio, struct bin_heap* heap,
			       void* value, struct bin_heap_node **new_node)
{
	FLOG_DEBUG("In heap_insert_value...\n");

	print_timer_entry(value);

	struct bin_heap_node *node = WiMAX_mac_malloc(sizeof (struct bin_heap_node));

	assert (node != NULL);
	assert (heap->num_nodes < UINT_MAX);
	node->parent = NULL;
	node->left = node->right = NULL;
	node->value = value;
	node->back = new_node;

	pthread_mutex_lock(&(heap->bh_mutex));
	heap_print(heap);
	heap_node_print(heap->root);

	*new_node = node;

	// Increase the number of nodes, and 
	// levels, if needed
	heap->num_nodes++;
	if (heap->num_nodes == heap->nxt_lev_st) {
		heap->num_levels++;
		// Update the node index that starts
		// the next level
		heap->nxt_lev_st *= 2; 
		//TRACE3 (4, "heap_insert_value: num_nodes: %d num_levels: %d next level starts at: %d\n", 
							//heap->num_nodes, heap->num_levels, heap->nxt_lev_st);
	}

	// Find the parent node
	int parent = heap->num_nodes/2;
	struct bin_heap_node *pnode;

	pnode = __heap_find_node_from_num(heap, parent, heap->num_levels-1);

	if (NULL != pnode) {
		if (parent*2 < heap->num_nodes) {
			pnode->right = node;
		} else {
			pnode->left = node;
		}
		node->parent = pnode;
	} else {
		assert (heap->root == NULL);
		heap->root = node;
	}
	__heap_decrease (higher_prio, heap, node);

	heap_print(heap);
	heap_node_print(heap->root);
	pthread_mutex_unlock(&(heap->bh_mutex));
	FLOG_DEBUG("End of heap_insert_value...\n");
	return node;
}
 
op_status_t heap_peek_value (struct bin_heap *heap, void **value)
{
	op_status_t ret_val;

	FLOG_DEBUG("In heap_peek_value\n");
	pthread_mutex_lock (&(heap->bh_mutex));
	heap_print(heap);
	//heap_node_print(heap->root);
	if (heap->root) {
		*value = heap->root->value;
		ret_val = SUCCESS;
	} else {
		ret_val = E_Q_EMPTY;
	}	
	pthread_mutex_unlock (&(heap->bh_mutex));
	FLOG_DEBUG("In End of heap_peek_value\n");

	return ret_val;
	
}

op_status_t heap_extract_top_value(heap_prio_t higher_prio, struct bin_heap *heap, void **node_value)
{
	FLOG_DEBUG("In heap_extract_top_value...\n");
	if (heap->num_nodes == 0) {
		return E_Q_EMPTY;
	}

	op_status_t ret_val;

	pthread_mutex_lock(&heap->bh_mutex);
	heap_print(heap);
	heap_node_print(heap->root);
	ret_val = __heap_extract_top_value (higher_prio, heap, node_value);
	print_timer_entry(*node_value);
	heap_print(heap);
	heap_node_print(heap->root);
	pthread_mutex_unlock(&heap->bh_mutex);

	FLOG_DEBUG("End of heap_extract_top_value\n");
	return ret_val;
}

op_status_t heap_extract_top_value_cond(heap_prio_t higher_prio, struct bin_heap *heap, void *ref_value, void **node_value)
{
	op_status_t ret_val = SUCCESS;
	FLOG_DEBUG("In heap_extract_top_value_cond...\n");
	pthread_mutex_lock (&heap->bh_mutex);
	heap_print(heap);
	heap_node_print(heap->root);

	if (heap->num_nodes > 0) {
		if (!higher_prio (ref_value, heap->root->value)) {
			__heap_extract_top_value (higher_prio, heap, node_value);
			print_timer_entry(*node_value);
		} else {
			*node_value = heap->root->value;
			ret_val = E_COND_NOT_MET;
		}
	} else {
		ret_val = E_Q_EMPTY;
	}
	heap_print(heap);
	heap_node_print(heap->root);
	pthread_mutex_unlock (&heap->bh_mutex);
	FLOG_DEBUG("End of heap_extract_top_value_cond\n");

	return ret_val;
}



void heap_delete(heap_prio_t higher_prio, struct bin_heap* heap, struct bin_heap_node** NODE)
{
	FLOG_DEBUG("In heap_delete\n");
	pthread_mutex_lock (&(heap->bh_mutex));	
	struct bin_heap_node *node = *NODE;
	heap_node_print(heap->root);


	if (NULL != node) {
		// Copy the contents of final node to NODE and free final
		// Find final node
		struct bin_heap_node *final;
		final = __heap_find_node_from_num (heap, heap->num_nodes, heap->num_levels);
		if (final == NULL) 
		{	pthread_mutex_unlock(&(heap->bh_mutex));
			FLOG_WARNING("Unexpected. Heap_delete called for a node that is not found in heap. Num nodes %d num levels %d\n",heap->num_nodes, heap->num_levels);
			return;
		}
		// Copy the contents of final to NODE
		print_timer_entry(node->value);
		node->value = final->value;
		node->back = final->back;
		assert (node->back != NULL);
		if (node->back) {
			*(node->back) = node;
		}

		// Update parent node's child pointer
		if (final->parent != NULL) {
			if (heap->num_nodes%2 == 0) {
				final->parent->left = NULL;
			} else {
				final->parent->right = NULL;
			}
		} else {
			assert (final == heap->root);
			heap->root = NULL;
		}
		// delete final
		WiMAX_mac_free (final);

		*NODE = NULL;

		heap->num_nodes--;
		if (heap->num_nodes < heap->nxt_lev_st/2) {
			heap->num_levels--;
			heap->nxt_lev_st = heap->nxt_lev_st/2;
			//TRACE3 (4, "heap_delete: num_nodes: %d num_levels: %d next level starts at: %d\n", 
							//heap->num_levels, heap->num_nodes, heap->nxt_lev_st);
		}
		if (node != final) {
			__heap_decrease(higher_prio, heap, node);
			__heap_increase(higher_prio, heap, node);
		}

	}
	heap_print(heap);
	heap_node_print(heap->root);

	pthread_mutex_unlock (&(heap->bh_mutex));	
	FLOG_DEBUG("End of heap_delete\n");
}


void heap_clear (bin_heap_t *heap, bin_heap_node_t *node)
{
	if (node) {
		heap_clear (heap, node->left);
		heap_clear (heap, node->right);
		if (node != NULL) heap_delete (timespec_cmp, heap, &node);
		node = NULL;
	}
}

void destroy_bin_heap (bin_heap_t *heap) {
	assert (heap != NULL);

	// TO DO: Add code to delete all nodes
	//heap_clear (heap, heap->root);
	while (heap->root != NULL) {
		bin_heap_node_t *node = heap->root;
		if (node != NULL)heap_delete (timespec_cmp, heap, &(node));
		node = NULL;
	}

	pthread_mutex_destroy (&heap->bh_mutex);

	if (heap != NULL) {
		WiMAX_mac_free (heap);
	}
}

void heap_node_print (bin_heap_node_t *node) 
{
#ifdef DDEBUG
	if (node) {
		FLOG_DEBUG("Heap Print Begin\n");
		FLOG_DEBUG("Heap Print End\n");
	}
	return;
	if (node) {
		FLOG_DEBUG("heap_node_print: node: %p parent: %p left: %p: right: %p value:%p\n",
						node, node->parent, node->left, node->right, node->value);
		print_timer_entry(node->value);
		heap_node_print(node->left);
		heap_node_print(node->right);
	}
#endif
}

/*****
extern int timespec_cmp(void *, void *);
void heap_node_print_special (bin_heap_node_t *node, void *root_value) 
{
	static int heap_errors = 0;
	if (node) {
		printf("heap_node_print: node: %p parent: %p left: %p: right: %p value:%p ",
						node, node->parent, node->left, node->right, node->value);
		timer_entry_t *te = (timer_entry_t *)(node->value);
		printf("  value: CONN: %d BSN: %d timer type: %d fireat %ld sec %ld nsec\n", 
				te->cid, te->bsn, te->timer_type, te->fireat.tv_sec, te->fireat.tv_nsec);
		if (timespec_cmp(node->value, root_value)) {
			printf("HEAP ERROR: %d %ld %ld\n", ++heap_errors, te->fireat.tv_sec, te->fireat.tv_nsec);
		}
		assert(!timespec_cmp(node->value, root_value));
		heap_node_print_special(node->left, root_value);
		heap_node_print_special(node->right, root_value);
	}
}
*******/
