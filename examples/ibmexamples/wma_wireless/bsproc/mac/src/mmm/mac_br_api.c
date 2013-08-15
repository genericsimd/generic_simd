/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2012

   All Rights Reserved.

   File Name: mac_br_api.c

   Change Activity:

   Date                      Description of Change                   By
   -----------      ---------------------       --------
   03-Jun.2012      Created                                     Xianwei. Yi
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "ul_mgt_msg_queue.h"
#include "ranging_mm.h"
#include "mac_br_api.h"
#include "mac_serviceflow.h"
#include "mac_connection.h"

#ifdef BR_ENABLE
extern sll_fifo_q *ranging_scheduler_q;
extern dts_info int_info;

extern int get_total_ul_rate(void);
extern int get_total_dl_rate(void);
extern int get_total_ul_slot(void);
extern int get_total_dl_slot(void);

sll_fifo_q *mac_br_q = NULL;

#define BITS_PER_BYTE (8U)
/*
  * handle_br_req - handle bandwidth request
  * @br_req: bandwidth request
  *
  * The API is used to handle bandwidth request. It determine whether the bandwidth can be
  * supported.
  *
  * Return:
  *     0 if successful
  *     otherwise if error happened
  */
int handle_br_req(struct br_req *br_req)
{
    int                 num_ul_slots;
    int                 num_dl_slots;
    float               dl_capacity;
    float               ul_capacity;
#if 0
    int                 total_rsvd_rate;
#endif
    struct service_flow *sf_node;
    connection          *connection;
    int                 added_bw_rate;
    int 		ss_index;

    assert(br_req != NULL);

    if ((br_req->type != BR_INCREMENTAL_HEADER) && (br_req->type != BR_AGGREGATE_HEADER))
    {
	FLOG_INFO("wrong br type %d\n", br_req->type);
        return -1;
    }

    dl_capacity = 0;
    ul_capacity = 0;
    num_dl_slots = 0;
    num_ul_slots = 0;
    calculate_phy_capacity(1, NULL, &dl_capacity, &ul_capacity, \
        &num_dl_slots, &num_ul_slots, NUM_DL_SUBCHANNELS - int_info.num_dl_interference, \
        NUM_UL_SUBCHANNELS - int_info.num_ul_interference);

    pthread_rwlock_wrlock(&conn_info_rw_lock);
    connection = find_connection(br_req->cid);
    
    if (connection == NULL)
    {
        pthread_rwlock_unlock(&conn_info_rw_lock);
        return -1;
    }
    ss_index = connection->owner->ss_index;

    sf_node = connection->sf;
    if (sf_node != NULL)
    {  
        switch (br_req->type)
        {
            case BR_INCREMENTAL_HEADER:
		FLOG_INFO("incremental br\n");
                added_bw_rate = br_req->bw_len;
                break;
            case BR_AGGREGATE_HEADER:
		FLOG_INFO("aggregate br\n");
                added_bw_rate = br_req->bw_len - sf_node->min_reserved_traffic_rate;
                break;
            default:
		FLOG_INFO("unknown br\n");
                break;
        }
        
        if (sf_node->sf_direction == UL)
        {
#if 0
	    if (added_bw_rate > 0)
	    {
	    	ModulCodingType ul_type;
	    	ModulCodingType dl_type;

		get_ss_mcs(ss_index, &dl_type, &ul_type);
		int added_slot_num = ceil(((float)(added_bw_rate * 8)) / (bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT));
		int total_ul_slot = get_total_ul_slot();

		num_ul_slots = num_ul_slots * (1000 / frame_duration[FRAME_DURATION_CODE]);
            	/* check if the new total capacity exceeds the UL subframe cap */
            	if (added_slot_num + total_ul_slot > num_ul_slots)
            	{
                	FLOG_INFO("added_slot_num = %d, total_ul_slot = %d, num_ul_slots = %d, not supported, will add %d slots\n", \
                                added_slot_num, total_ul_slot, num_ul_slots, (num_ul_slots - total_ul_slot));

			added_bw_rate = ((num_ul_slots - total_ul_slot) * (bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT)) / 8;
            	}
	    }
#endif
        }
        else
        {
	    if (added_bw_rate > 0)
            {
                ModulCodingType ul_type;
                ModulCodingType dl_type;

                get_ss_mcs(ss_index, &dl_type, &ul_type);
                int added_slot_num = ceil(((float)(added_bw_rate * 8)) / (bits_per_car[dl_type] * DL_DATA_CAR_PER_SLOT));
                int total_dl_slot = get_total_dl_slot();
		
		num_dl_slots = num_dl_slots * (1000 / frame_duration[FRAME_DURATION_CODE]);
                /* check if the new total capacity exceeds the UL subframe cap */
                if (added_slot_num + total_dl_slot > num_dl_slots)
                {
                        FLOG_INFO("added_slot_num = %d, total_dl_slot = %d, num_dl_slots = %d, not supported, will add %d slots\n", \
                                	added_slot_num, total_dl_slot, num_dl_slots, (num_dl_slots - total_dl_slot));
			added_bw_rate = ((num_dl_slots - total_dl_slot) * (bits_per_car[dl_type] * DL_DATA_CAR_PER_SLOT)) / 8;
                }
            }
        }

	FLOG_INFO("cid = %d, original bandwidth = %d\n", sf_node->cid, sf_node->min_reserved_traffic_rate);
        sf_node->min_reserved_traffic_rate += added_bw_rate;
	FLOG_INFO("cid = %d, now bandwidth = %d\n", sf_node->cid, sf_node->min_reserved_traffic_rate);
    }
    else
    {
        switch (br_req->type)
        {
            case BR_INCREMENTAL_HEADER:
                added_bw_rate = br_req->bw_len;
                break;
            case BR_AGGREGATE_HEADER:
                added_bw_rate = br_req->bw_len - connection->min_reserved_traffic_rate;
                break;
            default:
                break;
        }
	if (added_bw_rate > 0)
        {
		ModulCodingType ul_type;
                ModulCodingType dl_type;

                get_ss_mcs(ss_index, &dl_type, &ul_type);
                int added_slot_num = ceil(((float)(added_bw_rate * 8)) / (bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT));
                int total_ul_slot = get_total_ul_slot();

		num_ul_slots = num_ul_slots * (1000 / frame_duration[FRAME_DURATION_CODE]);
                /* check if the new total capacity exceeds the UL subframe cap */
                if (added_slot_num + total_ul_slot > num_ul_slots)
                {
                        FLOG_INFO("added_slot_num = %d, total_ul_slot = %d, num_ul_slots = %d, not supported, will add %d slots\n", \
                                added_slot_num, total_ul_slot, num_ul_slots, (num_ul_slots - total_ul_slot));

			added_bw_rate = ((num_ul_slots - total_ul_slot) * (bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT)) / 8;
                }
	}

        connection->min_reserved_traffic_rate += added_bw_rate;
    }
    pthread_rwlock_unlock(&conn_info_rw_lock);

    return 0;
}

/*
  * mac_br_thread - mac bandwidth processing thread
  * @br_req: bandwidth request
  *
  * The API is the mac bandwidth processing thread.It process the incoming ranging code 
  * and bandwidth request
  *
  * Return:
  *     0 if successful
  *     otherwise if error happened
  */
void* mac_br_thread(void *args)
{
    q_container         *br_container;
    int                 ret;
    ranging_info        *p_rng_info;
    ranging_adjust_ie   *p_rng_adjust;
    int                 i;
    struct br_req       *br_req;
    connection          *connect;

    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        pthread_exit(NULL);
    }
    
    ret = sll_fifo_q_init(&mac_br_q);
    if (ret < 0)
    {
        pthread_exit(NULL);
    }

    pthread_cleanup_push((void *)release_sll_fifo_q, (void *)mac_br_q);

    while (1)
    {
        br_container = NULL;
        sll_fifo_q_dequeue_with_wait(mac_br_q, &br_container);
	FLOG_INFO("%s awaken, data_type = %d\n", __FUNCTION__, br_container->data_type);

        switch(br_container->data_type)
        {
            case RANGING_ADJUST:
                p_rng_info = (ranging_info *)br_container->data;
                for (i = 0; i < p_rng_info->num_ranging_adjust_ie; i++)
                {
                    /* delink one ranging_adjust node from the top of the list */
                    p_rng_adjust = p_rng_info->p_ranging_ie_list;
                    if (p_rng_adjust != NULL)
                    {
                        p_rng_info->p_ranging_ie_list = p_rng_adjust->p_next;
                        p_rng_adjust->p_next = NULL;

                        /* ul scheduler will add a cdma allocatioe ie for this */
                        sll_fifo_q_enqueue(ranging_scheduler_q, p_rng_adjust, sizeof(ranging_adjust_ie), RANGING_ADJUST);
                    }
                }
                break;
            case BR_REQ:
#if 1
                br_req = (struct br_req *)br_container->data;
                ret = handle_br_req(br_req);
                if (ret != 0)
                {
                    FLOG_DEBUG("can't serve bandwidth request for cid %d\n", br_req->cid);
                }
#endif
                
                break;
            default:
                break;
        }
        
        free(br_container->data);
        free(br_container);
    }
    pthread_cleanup_pop(0);
    pthread_exit(NULL);
}
#endif
