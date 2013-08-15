/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: arq_ul_mgmt_msg.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <string.h>
#include <assert.h>
#include "arq_ds.h"
#include "arq_ifaces.h"
#include "arq_conn.h"
#include "arq_defines.h"
#include "arq_const.h"
#include "ll.h"
#include "debug.h"
#include "mac_sdu_queue.h"
#include "mac_connection.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"
#include "mutex_macros.h"
#include "mac_ul_frag_queue.h"

extern ARQ_globals globals;
extern sdu_queue *dl_sdu_queue;
extern int timespec_cmp (void *, void *);

/*
void *arq_process_ul_mgmt_msgs(void *msg_ptr) 
{
	int	ret_val;
	//void *msg_ptr;

	create_init_trace_buffer(2048, "arq_process_ul_mgmt_msgs");

	ARQ_mgmt_msg_t arq_msg;
	do {
		unsigned char msg_code;
		ret_val = dequeue_circq_array(arq_ul_mac_mgmt_msg_q, (void *)&arq_msg, false, true);	

		if ( true == arq_msg.is_payload_only) {
			msg_code = ARQ_FEEDBACK_MSG;
		} else {
			msg_code = ((unsigned char *)(arq_msg.message))[0];
		}

		switch (msg_code) {
			case ARQ_FEEDBACK_MSG:
					parse_and_process_feedback_msg((ARQ_feedback_message *)(arq_msg.message));
					break;

			case ARQ_RESET_MSG:
					process_reset_msg((ARQ_reset_message *)(arq_msg.message));
					break;

			case ARQ_DISCARD_MSG:
					process_discard_msg((ARQ_discard_message *)(arq_msg.message));

			default:
					TRACE1(2, "arq_process_ul_mgmt_msgs: Unknown message with code %d received\n", (unsigned char)msg_code);
					break;
		}
		WiMAX_mac_free (arq_msg.message);
	} while(1);

}

*/
void parse_and_process_feedback_msg (ARQ_feedback_message *fb_msg)
{
	assert (ARQ_FEEDBACK_MSG == fb_msg->mgmt_msg_type);

	int msg_offset = 1;
	ARQ_feedback_ie *fb_ie;

	FLOG_DEBUG("Received ARQ_FEEDBACK message\n");

	do {
		fb_ie = (ARQ_feedback_ie *)(((unsigned char *)fb_msg)+msg_offset);
	
		short cid = fb_ie->cid;
		dl_connection_t *dl_conn = find_dl_connection(cid);
		assert (dl_conn != NULL);
		short head_bsn = fb_ie->bsn;
		short next_bsn = head_bsn;
		short thrd_idx = dl_conn->timer_thrd_idx;
		short tmp_idx;int flag =0;
		short remove_idx;
		if (dl_conn->arq_sync_loss_timer != NULL)
		{
			heap_delete(timespec_cmp,arq_dl_timers[thrd_idx].timer_list, &(dl_conn->arq_sync_loss_timer));
			dl_conn->arq_sync_loss_timer = NULL;
		}
		struct timeval mytime; gettimeofday(&mytime, NULL);
		insert_dl_timer (dl_conn, ARQ_SYNC_LOSS_TIMEOUT, 0, ARQ_SYNC_LOSS_TIMEOUT_DEF, mytime, &(dl_conn->arq_sync_loss_timer));
		

		pthread_mutex_lock(&(dl_conn->block_buffer.arq_wnd_lock));

		FLOG_DEBUG("ARQ_FEEDBACK: CONN ID: %d ack type: %d BSN in message: %d current window start: %d\n", 
							cid, fb_ie->ack_type, head_bsn, dl_conn->arq_tx_window_start);
#ifdef FEEDBACK_TEST
		FLOG_INFO("ARQ_FEEDBACK: CONN ID: %d ack type: %d BSN in message: %d current window start: %d\n", 
							cid, fb_ie->ack_type, head_bsn, dl_conn->arq_tx_window_start);
#endif
		if (ARQ_SELECTIVE_ACK != fb_ie->ack_type) {
			if ((mod((head_bsn-dl_conn->arq_tx_window_start), ARQ_BSN_MODULUS)) < dl_conn->ARQ_TX_WINDOW_SIZE &&
							head_bsn != dl_conn->arq_tx_window_start) {
				short bsn;
				for (bsn = dl_conn->arq_tx_window_start; bsn != head_bsn; bsn = (bsn+1)%ARQ_BSN_MODULUS) {
					short blk_idx = bsn%dl_conn->ARQ_TX_WINDOW_SIZE;
					if (dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DONE && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_NOT_SENT && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DISCARDED){
					dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_DONE;
					// Remove the block from waiting for retransmission queue (if previously added)
					//unlink_and_discard_element_ll (dl_conn->retrans_q, bsn);
					ARQ_delete_ReTX_q(dl_conn,bsn,dl_conn->block_buffer.block_buffer[blk_idx].size);
					if (dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer != NULL) {
						//delete_node_delta_list(arq_dl_timers.timer_list,
											   //&(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, 
											   &(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
					}
					dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer = NULL;
					if (dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer != NULL){
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer));
					}
					dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer = NULL;
					// Free block
					flag = 0;
					if (dl_conn->block_buffer.block_buffer[blk_idx].type == NO_FRAGMENTATION)
					{
						remove_idx = blk_idx;
						flag = 0;
					}
					else
					{
						tmp_idx = blk_idx;
						while (dl_conn->block_buffer.block_buffer[tmp_idx].type != FIRST_FRAGMENT)
						{
							tmp_idx = (tmp_idx - 1)%dl_conn->ARQ_TX_WINDOW_SIZE;
						}
						remove_idx = tmp_idx;
						while (dl_conn->block_buffer.block_buffer[tmp_idx].type != LAST_FRAGMENT)
						{	
							if (dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DONE) 
							{
								flag = 1;
								//printf("Breaking at indx %d which has state %d\n",tmp_idx,dl_conn->block_buffer.block_buffer[tmp_idx].state); break;
							}
							tmp_idx = (tmp_idx+1)%dl_conn->ARQ_TX_WINDOW_SIZE;
						}
						//Accounting for LAST_FRAGMENT block too
						if (flag == 0 && (dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DONE || dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DISCARDED)) 
						{
							flag = 1;
							//printf("Breaking at indx %d which has state %d\n",tmp_idx,dl_conn->block_buffer.block_buffer[tmp_idx].state);
						}
					}
					//printf("Flag %d remove_idx %d blk_idx %d Type %d\n",flag, remove_idx, blk_idx, dl_conn->block_buffer.block_buffer[blk_idx].type);	
					if (flag == 0 && dl_conn->block_buffer.block_buffer[remove_idx].data != NULL && dl_conn->block_buffer.block_buffer[remove_idx].state != ARQ_DISCARDED)  mac_sdu_free (dl_conn->block_buffer.block_buffer[remove_idx].data, dl_conn->block_buffer.block_buffer[remove_idx].size,
								  dl_conn->block_buffer.block_buffer[remove_idx].type);
					if (flag == 0)
					{
						if (dl_conn->block_buffer.block_buffer[remove_idx].type != NO_FRAGMENTATION)
						{
							while (dl_conn->block_buffer.block_buffer[remove_idx].type != LAST_FRAGMENT)
							{
								dl_conn->block_buffer.block_buffer[remove_idx].data = NULL;
								dl_conn->block_buffer.block_buffer[remove_idx].state = ARQ_NOT_SENT;
								remove_idx = (remove_idx + 1)%dl_conn->ARQ_TX_WINDOW_SIZE;
							}
						}
						else
						{
							dl_conn->block_buffer.block_buffer[remove_idx].data = NULL;
							dl_conn->block_buffer.block_buffer[remove_idx].state = ARQ_NOT_SENT;

						}
					}
				FLOG_DEBUG("ARQ_FEEDBACK: CONN ID: %d tx window start advanced to %d\n", cid, head_bsn);
				//dl_conn->arq_tx_window_start = head_bsn;
				//dl_conn->block_buffer.arq_tx_wnd_start_idx = head_bsn;
				}
			}
					bsn = head_bsn;			
					short blk_idx = bsn%dl_conn->ARQ_TX_WINDOW_SIZE;
					if (dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DONE && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_NOT_SENT && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DISCARDED){
					dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_DONE;
					// Remove the block from waiting for retransmission queue (if previously added)
					//unlink_and_discard_element_ll (dl_conn->retrans_q, bsn);
					ARQ_delete_ReTX_q(dl_conn,bsn,dl_conn->block_buffer.block_buffer[blk_idx].size);
					if (dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer != NULL) {
						//delete_node_delta_list(arq_dl_timers.timer_list,
											   //&(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, 
											   &(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
					}
					dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer = NULL;
					if (dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer != NULL){
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer));
					}
					dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer = NULL;
					// Free block
					flag = 0;
					if (dl_conn->block_buffer.block_buffer[blk_idx].type == NO_FRAGMENTATION)
					{
						remove_idx = blk_idx;
						flag = 0;
					}
					else
					{
						tmp_idx = blk_idx;
						while (dl_conn->block_buffer.block_buffer[tmp_idx].type != FIRST_FRAGMENT)
						{
							tmp_idx = (tmp_idx - 1)%dl_conn->ARQ_TX_WINDOW_SIZE;
						}
						//printf("First Frag is %d\n",tmp_idx);	
						remove_idx = tmp_idx;	
						while (dl_conn->block_buffer.block_buffer[tmp_idx].type != LAST_FRAGMENT)
						{	
							if (dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DONE) 
							{
								flag = 1;
								break;
							}
							tmp_idx = (tmp_idx+1)%dl_conn->ARQ_TX_WINDOW_SIZE;
						}
						//Taking Last fragment into account.
						if (flag == 0 && (dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DONE || dl_conn->block_buffer.block_buffer[tmp_idx].state != ARQ_DISCARDED)) flag = 1;
					}
					//printf("Flag %d remove_idx %d blk_idx %d Type %d\n",flag, remove_idx, blk_idx, dl_conn->block_buffer.block_buffer[blk_idx].type);	
						
					if (flag == 0 && dl_conn->block_buffer.block_buffer[remove_idx].data != NULL && dl_conn->block_buffer.block_buffer[remove_idx].state != ARQ_DISCARDED)  mac_sdu_free (dl_conn->block_buffer.block_buffer[remove_idx].data, dl_conn->block_buffer.block_buffer[remove_idx].size,
								  dl_conn->block_buffer.block_buffer[remove_idx].type);
					if (flag == 0)
					{
						if (dl_conn->block_buffer.block_buffer[remove_idx].type != NO_FRAGMENTATION)
						{
							while (dl_conn->block_buffer.block_buffer[remove_idx].type != LAST_FRAGMENT)
							{
								dl_conn->block_buffer.block_buffer[remove_idx].data = NULL;
								dl_conn->block_buffer.block_buffer[remove_idx].state = ARQ_NOT_SENT;
								remove_idx = (remove_idx + 1)%dl_conn->ARQ_TX_WINDOW_SIZE;
							}
						}
						else
						{
							dl_conn->block_buffer.block_buffer[remove_idx].data = NULL;
							dl_conn->block_buffer.block_buffer[remove_idx].state = ARQ_NOT_SENT;

						}
					}

				FLOG_DEBUG("ARQ_FEEDBACK: CONN ID: %d tx window start advanced to %d\n", cid, head_bsn);
				dl_conn->arq_tx_window_start = head_bsn+1;
				dl_conn->block_buffer.arq_tx_wnd_start_idx = head_bsn+1;
				}





}

			if (ARQ_CUM_BLK_SEQ_SEL_ACK == fb_ie->ack_type) {
				next_bsn = (head_bsn+1)%ARQ_BSN_MODULUS;
			}
		}
		pthread_mutex_unlock(&(dl_conn->block_buffer.arq_wnd_lock));

		msg_offset += 4;

		if (ARQ_CUMULATIVE_ACK != fb_ie->ack_type) {
			if ((mod((head_bsn-dl_conn->arq_tx_window_start), ARQ_BSN_MODULUS)) < dl_conn->ARQ_TX_WINDOW_SIZE &&
							head_bsn != dl_conn->arq_tx_window_start){ 
			int ack_map_idx;
			for (ack_map_idx = 0; ack_map_idx <= fb_ie->no_ack_maps; ack_map_idx++) {
				if (ARQ_CUM_BLK_SEQ_SEL_ACK != fb_ie->ack_type) {
					int byte, offset;
					for (byte = 0; byte < 2; byte++) {
						unsigned char bitmap = *(((unsigned char *)fb_msg)+msg_offset);
						FLOG_DEBUG("ARQ_FEEDBACK: ack map: %d byte index: %d byte: %d\n",
											ack_map_idx, byte, (short)bitmap);
						for (offset = 0; offset < 8; offset++) {
							short blk_idx = next_bsn%dl_conn->ARQ_TX_WINDOW_SIZE;
					if (dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DONE && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_NOT_SENT && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DISCARDED){
					dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_DONE;

							if (((bitmap >> (7-offset)) & 0x01) == 0x01) {
								FLOG_DEBUG("ARQ_FEEDBACK: bsn: %d acked\n", next_bsn);
								// Remove the block from waiting for retransmission queue (if previously added)
								//unlink_and_discard_element_ll (dl_conn->retrans_q, next_bsn);
								ARQ_delete_ReTX_q(dl_conn,next_bsn,dl_conn->block_buffer.block_buffer[blk_idx].size);

								if (dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer != NULL) {
									heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,
														   &(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
								//printf("Deleitng Retry timer for BSN %d\n",next_bsn);
								}
								dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer = NULL;
								if (dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer != NULL){
									heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,
														   &(dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer));
								}
								dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer = NULL;
									
								dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_NOT_SENT;
							}
							next_bsn = (next_bsn+1)%ARQ_BSN_MODULUS;
						}}
						msg_offset++;
					}
				} else {
					unsigned char byte1 = *(((unsigned char *)fb_msg)+msg_offset);
					unsigned char byte2 = *(((unsigned char *)fb_msg)+msg_offset+1);
					unsigned short bitmap;

					//memcpy (&bitmap, (((unsigned char *)fb_msg)+msg_offset), sizeof(short));
					bitmap = (byte1 << 8) | byte2;
					FLOG_DEBUG("ARQ_FEEDBACK: byte: %d bitmap: %d\n",
										(unsigned short)byte1, bitmap);
					
					boolean block_status[3];
					short block_len[3];

					if ((bitmap & 0x4000) == 0x4000) {
						block_status[0] = true;	
					} else {
						block_status[0] = false;
					}

					if ((bitmap & 0x2000) == 0x2000) {
						block_status[1] = true;	
					} else {
						block_status[1] = false;
					}
					if ((byte1 & 0x80) == 0x80) {
						if ((bitmap & 0x1000) == 0x1000) {
							block_status[2] = true;	
						} else {
							block_status[2] = false;
						}
					}
					FLOG_DEBUG("ARQ_FEEDBACK: block_status[0]: %d, block_status[1]: %d, block_status[2]: %d\n",
											 block_status[0], block_status[1], block_status[2]);

					int blk_seq_idx_limit;
					if ((byte1 & 0x80) == 0x80) {
						block_len[0] = ((bitmap & 0x0F00) >> 8);
						block_len[1] = ((bitmap & 0x00F0) >> 4);
						block_len[2] = (bitmap & 0x000F);
						blk_seq_idx_limit = 3;
					} else {
						block_len[0] = ((bitmap & 0x1F80) >> 7);
						block_len[1] = ((bitmap & 0x007E) >> 1);
						blk_seq_idx_limit = 2;
					}
					FLOG_DEBUG("ARQ_FEEDBACK: block_len[0]: %d, block_len[1]: %d, block_len[2]: %d\n",
											 block_len[0], block_len[1], block_len[2]);

					int blk_seq_idx;
					for (blk_seq_idx = 0; blk_seq_idx <blk_seq_idx_limit; blk_seq_idx++) {
						if (block_status[blk_seq_idx] == true) {
							int sub_idx;
							for (sub_idx = 0; sub_idx < block_len[blk_seq_idx]; sub_idx++) {
									
								int blk_idx = next_bsn%dl_conn->ARQ_TX_WINDOW_SIZE;
					if (dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DONE && dl_conn->block_buffer.block_buffer[blk_idx].state != ARQ_DISCARDED){
					dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_DONE;
								//unlink_and_discard_element_ll (dl_conn->retrans_q, next_bsn);
								ARQ_delete_ReTX_q(dl_conn,next_bsn,dl_conn->block_buffer.block_buffer[blk_idx].size);

								if (dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer != NULL) {
									heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,
														   &(dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer));
								}
								dl_conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer = NULL;
								if (dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer != NULL) {
									heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,
														   &(dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer));
								}
								dl_conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer = NULL;
								dl_conn->block_buffer.block_buffer[blk_idx].state = ARQ_NOT_SENT;
								next_bsn = (next_bsn+1)%ARQ_BSN_MODULUS;	
							}}
						} else {
							next_bsn = (next_bsn+block_len[blk_seq_idx])%ARQ_BSN_MODULUS;
						}
					}
					FLOG_DEBUG("ARQ_FEEDBACK: next_bsn incremented to %d\n", next_bsn);
					msg_offset += 2;
				}
			}
			}
		}
	} while(fb_ie->last != 1); 
}

void process_reset_msg (ARQ_reset_message *rst_msg,frag_queue* fragq)
{
	assert (ARQ_RESET_MSG == rst_msg->mgmt_msg_type);
	FLOG_DEBUG("PROCESS_RESET_MSG: Received Reset Message of type %d and direction %d\n",rst_msg->type,rst_msg->direction);
	short cid = rst_msg->cid;

	FLOG_DEBUG("Received ARQ_RESET message for CONN: %d\n", cid);

	if ((rst_msg->type == 0x00 && rst_msg->direction == 0x0) || (rst_msg->type == 0x01 && rst_msg->direction == 0x1)) {
		ul_connection_t *ul_conn = find_ul_connection(cid);

		assert(ul_conn != NULL);
		if (ul_conn != NULL) {
			short bsn;
			int thrd_idx = ul_conn->timer_thrd_idx;
			pthread_mutex_lock(&(ul_conn->ul_block_buffer.arq_wnd_lock));

			for (bsn = 0; bsn < ul_conn->ARQ_RX_WINDOW_SIZE ; bsn++) {
				if (ul_conn->ul_block_buffer.ul_block_buffer[bsn].purge_timeout_timer != NULL) {
					heap_delete (timespec_cmp, arq_ul_timers[thrd_idx].timer_list,&(ul_conn->ul_block_buffer.ul_block_buffer[bsn].purge_timeout_timer));
					}
					ul_conn->ul_block_buffer.ul_block_buffer[bsn].purge_timeout_timer = NULL;
				ul_conn->ul_block_buffer.ul_block_buffer[bsn].state = ARQ_BLK_NOT_RECEIVED;
			}

			ul_conn->is_reset = true;
			ul_conn->arq_rx_window_start = 0;
			ul_conn->arq_rx_highest_bsn = 0;
			ul_conn->arq_last_cum_ack = 0;
			
			if (rst_msg->type == 0x00)
			{
				ARQ_reset_message* reset_msg = (ARQ_reset_message *)mac_sdu_malloc(sizeof(ARQ_reset_message), 5);
				reset_msg->mgmt_msg_type = ARQ_RESET_MSG;
				reset_msg->cid = ul_conn->cid;
				reset_msg->type = 0x01;
				reset_msg->direction = rst_msg->direction;

			// Post the message to the MAC management message queue
				int basic_cid = ul_conn->cid-1024;
				int ret_val;
				ret_val = get_basic_cid(ul_conn->cid, &basic_cid);

				if (0 == ret_val) {
						enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, sizeof(ARQ_reset_message), reset_msg);
				}
			}
			pthread_mutex_unlock(&(ul_conn->ul_block_buffer.arq_wnd_lock));
		}
		reset_arq_block (fragq, cid);
	} else if ((rst_msg->type == 0x01 && rst_msg->direction == 0x0) || (rst_msg->type == 0x00 && rst_msg->direction == 0x1)) {
		dl_connection_t *dl_conn = find_dl_connection(cid);
		assert (dl_conn != NULL);
		 dl_conn->arq_tx_window_start = 0;
		 dl_conn->arq_tx_next_bsn = 0;
		//pthread_mutex_lock(&(dl_conn->block_buffer.arq_wnd_lock));
		int thrd_idx = dl_conn->timer_thrd_idx;
		if (dl_conn->arq_sync_loss_timer != NULL)
		{
			//Need to nullify the reset timer here.
			heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,&(dl_conn->arq_sync_loss_timer));
		}
		dl_conn->arq_sync_loss_timer = NULL;
			for (int bsn = 0; bsn < dl_conn->ARQ_TX_WINDOW_SIZE ; bsn++) {
				if (dl_conn->block_buffer.block_buffer[bsn].retry_timeout_timer != NULL) {
					heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,&(dl_conn->block_buffer.block_buffer[bsn].retry_timeout_timer));
					}
				dl_conn->block_buffer.block_buffer[bsn].retry_timeout_timer = NULL;
				if (dl_conn->block_buffer.block_buffer[bsn].block_lifetime_timer != NULL) {
					heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list,&(dl_conn->block_buffer.block_buffer[bsn].block_lifetime_timer));
					}
				dl_conn->block_buffer.block_buffer[bsn].block_lifetime_timer = NULL;
			}
		dl_conn->arq_sync_loss_timer = NULL;		
			if (rst_msg->type == 0x00)
			{
				ARQ_reset_message* reset_msg = (ARQ_reset_message *)mac_sdu_malloc(sizeof(ARQ_reset_message), 5);
				reset_msg->mgmt_msg_type = ARQ_RESET_MSG;
				reset_msg->cid = dl_conn->cid;
				reset_msg->type = 0x01;
				reset_msg->direction = rst_msg->direction;

			// Post the message to the MAC management message queue
				int basic_cid = dl_conn->cid-1024;
				int ret_val;

				ret_val = get_basic_cid(dl_conn->cid, &basic_cid);

				if (0 == ret_val) {
					enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, sizeof(ARQ_reset_message), reset_msg);
				}
			}
		//	pthread_mutex_unlock(&(dl_conn->block_buffer.arq_wnd_lock));
			reset_arq_block (fragq, cid);
	}
}

void process_discard_msg (ARQ_discard_message *disc_msg,frag_queue* fragq)
{
	
	assert (ARQ_DISCARD_MSG == disc_msg->mgmt_msg_type);

	// Should we lock? -- looks like it

	short disc_bsn = disc_msg->bsn;
	short cid = disc_msg->cid;
	ul_connection_t *ul_conn = find_ul_connection(cid);

	FLOG_DEBUG("Received ARQ_DISCARD message for CONN: %d with bsn: %d\n", cid, disc_bsn);
#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
	FLOG_INFO("Received ARQ_DISCARD message for CONN: %d with bsn: %d\n", cid, disc_bsn);
#endif

	assert(ul_conn != NULL);


	if (NULL != ul_conn) {
	
		short bsn;

		pthread_mutex_lock(&(ul_conn->ul_block_buffer.arq_wnd_lock));
		// if disc_bsn in not in the receiver's sliding window, assume
		// that the feedback for this block was lost and not received at
		// the transmitter, or that the purge timer for a later block
		// expired and the window was updated here and so send a cumulative ack

		if (mod(disc_bsn-ul_conn->arq_rx_window_start, ARQ_BSN_MODULUS) >= ul_conn->ARQ_RX_WINDOW_SIZE) {
			size_t ack_msg_size = sizeof(ARQ_feedback_message)+sizeof(ARQ_feedback_ie);
			ARQ_feedback_message *ack_msg = (ARQ_feedback_message *)mac_sdu_malloc( ack_msg_size,5);
			ack_msg->mgmt_msg_type = ARQ_FEEDBACK_MSG;
			ARQ_feedback_ie *fb_ie = (ARQ_feedback_ie*)(ack_msg->acks);
			fb_ie->cid = ul_conn->cid;
			fb_ie->ack_type = ARQ_CUMULATIVE_ACK;
			fb_ie->bsn = disc_bsn;
			fb_ie->last = 1;  

			// Post the message into the MAC dl management message queue
			int basic_cid = ul_conn->cid-1024;
			int ret_val;

#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
			FLOG_INFO("Sending Discard Feedback\n");
#endif
			ret_val = get_basic_cid(ul_conn->cid, &basic_cid);
			if (0 == ret_val) {

				enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, ack_msg_size, ack_msg);
			}
			 discard_arq_block (fragq, cid, ul_conn->arq_rx_window_start, disc_bsn);
		} else {
			short arq_wnd_start = ul_conn->arq_rx_window_start;
			for (bsn = arq_wnd_start; bsn != disc_bsn; bsn++) {
				short blk_idx = bsn%ul_conn->ARQ_RX_WINDOW_SIZE;

				if (ul_conn->ul_block_buffer.ul_block_buffer[blk_idx].state != ARQ_BLK_NOT_RECEIVED) {
					if (ul_conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer != NULL) {
						int thrd_idx = ul_conn->timer_thrd_idx;
						heap_delete (timespec_cmp, arq_ul_timers[thrd_idx].timer_list, 
											   &(ul_conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer));
					}
					ul_conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer = NULL;
					continue;
				}
				ul_conn->ul_block_buffer.ul_block_buffer[blk_idx].state = ARQ_BLK_RECEIVED;
			}
			// advance the window
			ul_conn->arq_rx_window_start = (disc_bsn+1)%ARQ_BSN_MODULUS;
			// Call CRL's function
			 discard_arq_block (fragq, cid, arq_wnd_start, disc_bsn);
		}

#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
		FLOG_INFO("After discard, The new window start for Conn %d is %d\n",ul_conn->cid, ul_conn->arq_rx_window_start);
#endif
		pthread_mutex_unlock(&(ul_conn->ul_block_buffer.arq_wnd_lock));

	}
}
