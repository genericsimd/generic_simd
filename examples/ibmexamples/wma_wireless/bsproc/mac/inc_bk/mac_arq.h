/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_arq.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_ARQ_H__
#define __MAC_ARQ_H__

#include <stdlib.h>
#include "string.h"
#include "mac_config.h"
#include "arq_const.h"

typedef struct arqstate arq_state;

struct arqstate {
    u_int16_t cid;
    u_int16_t arq_window_size;
    u_int16_t transmitter_delay;
    u_int16_t receiver_delay;
    u_int16_t arq_block_lifetime;
    u_int16_t arq_sync_loss_timeout;
    u_int8_t is_order_preserved;
    u_int16_t arq_rx_purge_timeout;
    u_int16_t receiver_arq_ack_processing_time;
	
    double arq_retrans_time;
    u_int32_t ack_seq;
    u_int8_t   ack_counter;
    u_int32_t arq_curr_seq;
    u_int32_t rx_highest_bsn;
    u_int32_t rx_window_start;
    int arq_block_size;
    arq_state* next;
};

/**
 * notify_received_blocks - uplink cps thread notify the arq engine about the arrival of arq blocks
 * @cid: the connection that this arq block belong to
 * @start_bsn: the start block number
 * @end_bsn: the end block number
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */

int initialize_arq(int cid, arq_state** arq);


#endif

