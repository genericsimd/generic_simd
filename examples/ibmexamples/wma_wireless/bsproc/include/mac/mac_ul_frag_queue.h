/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_frag_queue.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_UL_FRAG_QUEUE_H__
#define __MAC_UL_FRAG_QUEUE_H__

#include <time.h>
#include "mac_sdu_queue.h"
#include "ul_mgt_msg_queue.h"
#include "mac_arq.h"

//#define FRAG_CID_QUEUE_EXPIRATION_TIME 0
//#define ARQ_BLOCK_CID_QUEUE_EXPIRATION_TIME 0

typedef struct fragcidqueue frag_cid_queue; 
struct fragcidqueue{
    int frag_num;
    u_int16_t cid;
    int bytes_num;
    //time_t last_update_time;
    logical_element *head;
    logical_element *tail;
    frag_cid_queue* next;
};

typedef struct arqblockcidqueue arq_block_cid_queue;
struct arqblockcidqueue{
    int block_num;
    int start_block_index;
    u_int16_t cid;
    //time_t last_update_time;
    logical_element *arq_block[ARQ_BSN_MODULUS];
    arq_block_cid_queue* next;
};


typedef struct {
    frag_cid_queue* frag_cid_q_head;
    arq_block_cid_queue* arqfrag_cid_q_head;
    int num_cids;
}frag_queue;

int initialize_fragq(frag_queue** fragq);

int get_cid_fragq(frag_queue* fragq, int cid, frag_cid_queue** cidq);

int get_cid_arq_fragq(frag_queue* fragq, int cid, arq_block_cid_queue** cidq);

int enqueue_arq_fragq(frag_queue* fragq, unsigned int frame_num, int cid, logical_element * frag, u_int8_t is_order, int rx_win_start, int arq_win_size, int bsn_modulo, u_int8_t is_mgt_con, sdu_queue* sduq);

int enqueue_fragq(frag_queue* fragq, unsigned int frame_num, int cid, logical_element* frag, int bsn_modulo,u_int8_t is_mgt_con, sdu_queue* sduq);

//int release_expired_cid_fragq(frag_queue* fragq);

int release_fragq(frag_queue* frag_queue);
int release_fragcidq(frag_queue* fragq, int cid, u_int8_t is_arq);
int discard_arq_block(frag_queue* fragq, int cid, int start_bsn, int end_bsn);
int reset_arq_block_cidq(frag_queue* fragq, int cid);
int reset_arq_block(frag_queue* fragq, int cid);
#endif
