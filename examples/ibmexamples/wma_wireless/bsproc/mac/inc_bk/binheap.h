/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: bin_heap.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _BINARY_HEAP
#define _BINARY_HEAP

#include <pthread.h>
#include "arq_types.h"

typedef struct bin_heap_node {
	struct bin_heap_node *parent;
	struct bin_heap_node *left;
	struct bin_heap_node *right;

	void *value;
	struct bin_heap_node **back;
} bin_heap_node_t;

typedef struct bin_heap {
	bin_heap_node_t *root;
	pthread_mutex_t bh_mutex;
	int num_nodes;
	int num_levels;
	int nxt_lev_st; // number of the node that begins the 
				    // next level
	
}bin_heap_t;

typedef int (*heap_prio_t)(void* a, void* b);
typedef int (*heap_copy_value_t)(void *a, void *b);

extern bin_heap_t *create_init_bin_heap(void);
extern bin_heap_node_t *heap_insert_value(heap_prio_t, struct bin_heap*, void*, struct bin_heap_node **);
extern op_status_t heap_extract_top_value(heap_prio_t, struct bin_heap *, void **);
extern op_status_t heap_extract_top_value_cond(heap_prio_t, struct bin_heap *, void *, void **);
extern void heap_delete(heap_prio_t, struct bin_heap*, struct bin_heap_node**);
extern op_status_t heap_increase(heap_prio_t, heap_copy_value_t, struct bin_heap*, struct bin_heap_node* node, void *); 
extern op_status_t heap_peek_value (struct bin_heap *, void **);
extern void destroy_bin_heap (struct bin_heap *heap);
#endif // _BINARY_HEAP
