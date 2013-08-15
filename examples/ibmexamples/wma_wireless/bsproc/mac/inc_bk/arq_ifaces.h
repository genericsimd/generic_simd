/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_ifaces.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ARQ_IFACES_H_
#define _ARQ_IFACES_H
#include "arq_types.h"
#include "arq_ds.h"
#include "arq_conn.h"
#include "logical_packet.h"
#include "mac_ul_frag_queue.h"

extern op_status_t ARQ_enqueue_tx_blocks_q (int cid, blocks_info_t *element);
extern op_status_t ARQ_set_tx_next_bsn (short cid, int next_bsn);
extern int ARQ_get_tx_window_start (short cid);
extern int ARQ_get_tx_window_size (short cid);
extern int ARQ_get_rx_window_start (short cid);
extern int ARQ_get_rx_window_size (short cid);
extern int ARQ_get_tx_block_size (short cid);
extern int ARQ_get_rx_block_size (short cid);

extern void ARQ_init(void);
void ARQ_shutdown();
extern op_status_t ARQ_uplink_conn_init(short, short, short);
extern op_status_t ARQ_downlink_conn_init(short, short, short, float, boolean);
extern op_status_t notify_received_blocks (short, int, int);


extern ARQ_ReTX_Q_aggr_info ARQ_get_ReTX_queue_aggr_info(short cid);
extern void ARQ_cond_dequeue_ReTX_q(short cid, size_t num_bytes, size_t num_blocks, logical_packet **block_seq); 
extern op_status_t ARQ_dequeue_ReTX_q (short, blocks_info_t *);
extern void ARQ_enqueue_ReTX_q_unique(dl_connection_t *conn, long int key, blocks_info_t *bl_info);
extern op_status_t ARQ_delete_ReTX_q(dl_connection_t *conn, short bsn, size_t size);
int ARQ_num_conn_waiting_for_ReTX(short, short);
void ARQ_enqueue_mgmt_msg(void *, size_t, boolean);

extern void parse_and_process_feedback_msg (ARQ_feedback_message *fb_msg);
extern void process_reset_msg (ARQ_reset_message *rst_msg,frag_queue *fragq);
extern void process_discard_msg (ARQ_discard_message *disc_msg, frag_queue *fragq);
#endif  //_ARQ_IFACES_H_
