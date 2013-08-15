/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_frag_queue.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_ul_frag_queue.h"
#include "queue_util.h"

int initialize_fragq(frag_queue** fragq){
    (*fragq) = (frag_queue *) malloc(sizeof(frag_queue));
    memset((*fragq), 0, sizeof(frag_queue));
    return 0; 
}

int initialize_cid_fragq (frag_cid_queue ** cidq, u_int16_t cid)
{
    (*cidq) = (frag_cid_queue *) malloc(sizeof(frag_cid_queue));
    memset((*cidq), 0, sizeof(frag_cid_queue));
    (*cidq)->cid = cid;
    return 0;
}

int initialize_cid_arqblockq(arq_block_cid_queue** cidq, u_int16_t cid){
    int i;
    (*cidq) = (arq_block_cid_queue *) malloc(sizeof(arq_block_cid_queue));
    memset((*cidq), 0, sizeof(arq_block_cid_queue));
    (*cidq)->cid = cid;
    (*cidq)->block_num = 0;
    (*cidq)->next = NULL;
    (*cidq)->start_block_index = -1;
    for (i = 0; i < ARQ_BSN_MODULUS; i++)
    {
        (*cidq)->arq_block[i] = NULL;
    }
    return 0;
}
/*
int release_expired_cid_fragq(frag_queue* fragq){
    frag_cid_queue * cidq;
    frag_cid_queue * pre_cidq;
    frag_cid_queue * next_cidq;
    arq_block_cid_queue * arq_cidq;
    arq_block_cid_queue * arq_pre_cidq;
    arq_block_cid_queue * arq_next_cidq;

    time_t cur_time;
    int expire_duration;
    logical_element * le;
    logical_element * pre_le;
    logical_element ** arq_le;
    int i;
    if (fragq)
    {
        next_cidq = fragq->frag_cid_q_head;
        pre_cidq = NULL;
        cur_time = time(NULL);
        expire_duration = FRAG_CID_QUEUE_EXPIRATION_TIME;
        while (next_cidq)
        {
            cidq = next_cidq;
            if (cur_time > (cidq->last_update_time + expire_duration))
            {
                // begin to del this cidq
                if (pre_cidq)
                {
                    pre_cidq->next = cidq->next;
                }
                else
                {
                   fragq->frag_cid_q_head = cidq->next; 
                }
                // save the next cidq
                next_cidq = cidq->next;
                le = cidq->head;
                while (le)
                {
                    pre_le = le;
                    le = pre_le->next;
                    free(pre_le);
                }
                free(cidq);
            }
            else 
            {
               pre_cidq = cidq;
               next_cidq = cidq->next; 
            }
        }
        // begin to process the arq block cid queue
        arq_next_cidq = fragq->arqfrag_cid_q_head;
        arq_pre_cidq = NULL;
        expire_duration = ARQ_BLOCK_CID_QUEUE_EXPIRATION_TIME;
        while (arq_next_cidq)
        {
            arq_cidq = arq_next_cidq;
            if (cur_time > (arq_cidq->last_update_time + expire_duration))
            {
                // begin to del this cidq
                if (arq_pre_cidq)
                {
                    arq_pre_cidq->next = arq_cidq->next;
                }
                else
                {
                   fragq->arqfrag_cid_q_head = arq_cidq->next;
                }
                // save the next cidq
                arq_next_cidq = arq_cidq->next;
                arq_le = arq_cidq->arq_block;
                for (i = 0; i < ARQ_BSN_MODULUS; i++)
                {
                    if (arq_le[i])
                    {
                        free(arq_le[i]);
                    }
                }
                free(arq_cidq);
            }
            else
            {
               arq_pre_cidq = arq_cidq;
               arq_next_cidq = arq_cidq->next;
            }
        }
 
    }
    return 0;
}
*/

int reset_arq_block(frag_queue* fragq, int cid)
{
    int i;
    arq_block_cid_queue* arq_cidq;
    get_cid_arq_fragq(fragq, cid, &arq_cidq);
    for (i = 0; i < ARQ_BSN_MODULUS; i++)
    {
        if (arq_cidq->arq_block[i] != NULL)
        {
            free(arq_cidq->arq_block[i]);
            arq_cidq->arq_block[i] = NULL;
        }
        
    }
    return 0;
}

int discard_arq_block(frag_queue* fragq, int cid, int start_bsn, int end_bsn)
{
    int i;
    arq_block_cid_queue* arq_cidq;
    get_cid_arq_fragq(fragq, cid, &arq_cidq);
    int bsn_module = ARQ_BSN_MODULUS;
    if (end_bsn >= start_bsn)
    {
        for (i = start_bsn; i <= end_bsn; i++)
        {
            if (arq_cidq->arq_block[i])
            {
			if (arq_cidq->arq_block[i]->data)
			{
				 free(arq_cidq->arq_block[i]->data);
				arq_cidq->arq_block[i]->data = NULL;
			}
                	free(arq_cidq->arq_block[i]);
                	arq_cidq->arq_block[i] = NULL;
            }
        }
    }
    else
    {
        for (i = start_bsn; i < bsn_module; i++)
        {
		if (arq_cidq->arq_block[i])
		{
			if (arq_cidq->arq_block[i]->data)
			{
				free(arq_cidq->arq_block[i]->data);
				arq_cidq->arq_block[i]->data = NULL;
			}
                	free(arq_cidq->arq_block[i]);
                	arq_cidq->arq_block[i] = NULL;
		}
        }
        for (i = 0; i <= end_bsn; i++)
        {
		if (arq_cidq->arq_block[i])
		{
			if (arq_cidq->arq_block[i]->data)
			{
				free(arq_cidq->arq_block[i]->data);
				arq_cidq->arq_block[i]->data = NULL;
			}
                	free(arq_cidq->arq_block[i]);
                	arq_cidq->arq_block[i] = NULL;
		}
        }
        
    }
    return 0;
}

int release_fragcidq(frag_queue* fragq, int cid, u_int8_t is_arq)
{
    frag_cid_queue * cidq;
    frag_cid_queue * pre_cidq;
    frag_cid_queue * next_cidq;
    logical_element * le;
    logical_element * pre_le;
    arq_block_cid_queue * arq_cidq;
    arq_block_cid_queue * arq_pre_cidq;
    arq_block_cid_queue * arq_next_cidq;
    logical_element ** arq_le;
    int i;

    if (is_arq)
    {
        arq_next_cidq = fragq->arqfrag_cid_q_head;
        arq_pre_cidq = NULL;
        while (arq_next_cidq)
        {
            arq_cidq = arq_next_cidq;
            arq_next_cidq = arq_cidq->next;
            if (arq_cidq->cid == cid)
            {
                // begin to del this cidq
                if (arq_pre_cidq)
                {
                    arq_pre_cidq->next = arq_cidq->next;
                }
                else
                {
                    fragq->arqfrag_cid_q_head = arq_cidq->next;
                }

                arq_le = arq_cidq->arq_block;
                for (i = 0; i < ARQ_BSN_MODULUS; i++)
                {
                    if (arq_le[i])
                    {
                        free(arq_le[i]->data);
                        arq_le[i]->data = NULL;
                        free(arq_le[i]);
                        arq_le[i] = NULL;
                    }
                }
                free(arq_cidq);
                arq_cidq = NULL;
                break;
            }
            // save the next cidq
            arq_pre_cidq = arq_cidq;
        }

    }
    else
    {
        next_cidq = fragq->frag_cid_q_head;
        pre_cidq = NULL;
        while (next_cidq)
        {
            cidq = next_cidq;
            next_cidq = cidq->next;
            if (cidq->cid == cid)
            {
                // begin to del this cidq
                if (pre_cidq)
                {
                    pre_cidq->next = cidq->next;
                }
                else
                {
                   fragq->frag_cid_q_head = cidq->next;
                }

                le = cidq->head;
                while (le)
                {
                     pre_le = le;
                     le = pre_le->next;
                     free(pre_le->data);
                     pre_le->data = NULL;
                     free(pre_le);
                     pre_le = NULL;
                }
                free(cidq);
                cidq = NULL;
                break;
            }

            pre_cidq = cidq;
        }

    }
    return 0;
}


int release_fragq(frag_queue* fragq){
    frag_cid_queue * cidq;
    frag_cid_queue * pre_cidq;
    frag_cid_queue * next_cidq;
    logical_element * le;
    logical_element * pre_le;
    arq_block_cid_queue * arq_cidq;
    arq_block_cid_queue * arq_next_cidq;
    logical_element ** arq_le;
    int i;
    if (fragq)
    {
        next_cidq = fragq->frag_cid_q_head;
        pre_cidq = NULL;
        while (next_cidq)
        {
            cidq = next_cidq;

            // save the next cidq
            next_cidq = cidq->next;
            pre_cidq = cidq;
            le = cidq->head;
            while (le)
            {
                 pre_le = le;
                 le = pre_le->next;
                 free(pre_le->data);
                 pre_le->data = NULL;
                 free(pre_le);
                 pre_le = NULL;
            }
            free(cidq);
        }
        // begin to release the arq block queue
        arq_next_cidq = fragq->arqfrag_cid_q_head;

        while (arq_next_cidq)
        {
            arq_cidq = arq_next_cidq;
            // save the next cidq
            arq_next_cidq = arq_cidq->next;
            arq_le = arq_cidq->arq_block;
            for (i = 0; i < ARQ_BSN_MODULUS; i++)
            {
                if (arq_le[i])
                {
                    free(arq_le[i]->data);
                    arq_le[i]->data = NULL;
                    free(arq_le[i]);
                    arq_le[i] = NULL;
                }
            }
            free(arq_cidq);
            arq_cidq = NULL;
        }

        free(fragq);
        fragq = NULL;

    }

    return 0;
}

int get_cid_fragq(frag_queue* fragq, int cid, frag_cid_queue** cidq){
    
    frag_cid_queue* cur_cidq = fragq->frag_cid_q_head;
    frag_cid_queue* prev_cidq = cur_cidq;
    while (cur_cidq)
    {
        if (cur_cidq->cid == cid)
        {
            (*cidq) = cur_cidq;
            return 0;
        }
        prev_cidq = cur_cidq;
        cur_cidq = cur_cidq->next;
    }
    // there are no related cidq 
    initialize_cid_fragq(cidq, cid);
    if (prev_cidq == NULL)
    {
        // it's the header, the first element in the frag_queue
        fragq->frag_cid_q_head = (*cidq);
    }
    else 
    {
        prev_cidq->next = (*cidq);
    }
    
    return 0;
}

int get_cid_arq_fragq(frag_queue* fragq, int cid, arq_block_cid_queue** cidq){
    
    arq_block_cid_queue* cur_cidq = fragq->arqfrag_cid_q_head;
    arq_block_cid_queue* prev_cidq = cur_cidq;
    while (cur_cidq)
    {
        if (cur_cidq->cid == cid)
        {
            (*cidq) = cur_cidq;
            return 0;
        }
        prev_cidq = cur_cidq;
        cur_cidq = cur_cidq->next;
    }
    // there are no related cidq 
    initialize_cid_arqblockq(cidq, cid);
    if (prev_cidq == NULL)
    {
        // it's the header, the first element in the frag_queue
        fragq->arqfrag_cid_q_head = (*cidq);
    }
    else 
    {
        prev_cidq->next = (*cidq);
    }
    
    return 0;
}

int enqueue_arq_fragq(frag_queue * fragq, unsigned int frame_num, int cid, logical_element * frag, u_int8_t is_order, int rx_win_start, int arq_win_size, int bsn_modulo, u_int8_t is_mgt_con, sdu_queue* sduq){
    arq_block_cid_queue* cidq;
    u_char* payload;
    logical_packet* sdupkt;
    int current_bsn;
    int next_bsn;
    int before_bsn;
    int payload_len;
    int end_bsn;
    u_int8_t is_before_complete, is_next_complete;
    u_int8_t found_start;

    int frag_msg_type = FRAG_MANAGEMENT_MESSAGE;

    struct queue_msg cs_msg;

    cs_msg.my_type = csforw_en_id;

    is_before_complete = 0;
    is_next_complete = 0;  
    // get the frag queue related with this cid
    get_cid_arq_fragq(fragq, cid, &cidq);

    // memory copy the payload of logical element
    payload = (u_char *) malloc(frag->length);
    memcpy(payload, frag->data, frag->length);
    frag->data = payload;
    current_bsn = frag->start_bsn;
    end_bsn = (rx_win_start + arq_win_size) % bsn_modulo;
    
    if (frag->blk_type == FIRST_FRAGMENT)
    {
        // if this is the first unreceived arq block, check whether it could be connect with the following arq block to become a sdu
        cidq->arq_block[current_bsn] = frag;
        next_bsn = (current_bsn+1) % bsn_modulo;
        payload_len = frag->length;
        if (cidq->arq_block[next_bsn])
        {
            // link the arq blocks
            (cidq->arq_block[current_bsn])->next = cidq->arq_block[next_bsn];
        }
        if ((is_order && current_bsn == rx_win_start) || (!is_order))
        {
            // this is the block that the window wait for, so it also should be the first sdu that could be delivered in order
            // otherwise, if the deliver is not in order, check whether a complete sdu has been assembled

            while (cidq->arq_block[next_bsn])
            {
                payload_len += (cidq->arq_block[next_bsn])->length;
                if ((cidq->arq_block[next_bsn])->blk_type == LAST_FRAGMENT)
                {
                    // find a complete sdu, now move it to the sduq
                    is_next_complete = 1;
                    sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
                    memset(sdupkt, 0, sizeof(logical_packet));
                    sdupkt->cid = cid;
                    sdupkt->element_head = cidq->arq_block[current_bsn];
                    sdupkt->length = payload_len;
                    if (is_mgt_con)
                    {
                        enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, (void *) sdupkt);
                    }
                    else
                    {
                        enqueue_sduq(sduq, cid, sdupkt);

                        cs_msg.len = cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                    // remove these arq block from the array
                    while (current_bsn != next_bsn)
                    {
                        cidq->arq_block[current_bsn] = NULL;
                        current_bsn = (current_bsn +1) % bsn_modulo;
                    }
                    cidq->arq_block[current_bsn] = NULL;
                    
                }
                else 
                {
                    next_bsn = (next_bsn +1) % bsn_modulo;
                    if (next_bsn == end_bsn)
                    {
                        break;
                    }
                }
            }


        }

    }
    else if (frag->blk_type == NO_FRAGMENTATION)
    {
        // this sdu is smaller than an arq block 
        cidq->arq_block[current_bsn] = frag;
        next_bsn = current_bsn;
        payload_len = frag->length;
          
        if ((is_order && current_bsn == rx_win_start) || (!is_order))
        {
            // find a complete sdu, now move it to the sduq
            is_next_complete = 1;
            (cidq->arq_block[current_bsn])->next = NULL;
            sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
            memset(sdupkt, 0, sizeof(logical_packet));
            sdupkt->cid = cid;
            sdupkt->element_head = cidq->arq_block[current_bsn];
            sdupkt->length = payload_len;
            if (is_mgt_con)
            {
                enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
            }
            else
            {
                enqueue_sduq(sduq, cid, sdupkt);

                cs_msg.len = cid;

                if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                     sizeof(struct queue_msg)) == -1)
                {
                    FLOG_ERROR("enqueue error\n");
                }

            }
            // remove these arq block from the array
            cidq->arq_block[current_bsn] = NULL;
        }
    }
    else if (frag->blk_type == LAST_FRAGMENT)
    {
        // if this is the last unreceived arq block, check whether it could be connect with the previous arq block to become a sdu
        cidq->arq_block[current_bsn] = frag;
        (cidq->arq_block[current_bsn])->next = NULL;
        next_bsn = current_bsn;
        before_bsn = (current_bsn -1 + bsn_modulo) % bsn_modulo;
        payload_len = frag->length;
        if (cidq->arq_block[before_bsn])
        {
            // link the arq blocks
            (cidq->arq_block[before_bsn])->next = cidq->arq_block[current_bsn];
        }
        if ((is_order && current_bsn == rx_win_start) || (!is_order))
        {
            // this is the block that the window wait for, so it also should be the first sdu that could be delivered in order
            // otherwise, if the deliver is not in order, check whether a complete sdu has been assembled

            while (cidq->arq_block[before_bsn])
            {
                payload_len += (cidq->arq_block[before_bsn])->length;
                if ((cidq->arq_block[before_bsn])->blk_type == FIRST_FRAGMENT)
                {
                    // find a complete sdu, now move it to the sduq
                    is_before_complete = 1;
                    sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
                    memset(sdupkt, 0, sizeof(logical_packet));
                    sdupkt->cid = cid;
                    sdupkt->element_head = cidq->arq_block[before_bsn];
                    sdupkt->length = payload_len;
                    if (is_mgt_con)
                    {
                        enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
                    }
                    else
                    {
                        enqueue_sduq(sduq, cid, sdupkt);

                        cs_msg.len = cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                    // remove these arq block from the array
                    while (current_bsn != before_bsn)
                    {
                        cidq->arq_block[before_bsn] = NULL;
                        before_bsn = (before_bsn +1) % bsn_modulo;
                    }
                    cidq->arq_block[current_bsn] = NULL;
                    
                }
                else 
                {   
                    before_bsn = (before_bsn -1 + bsn_modulo) % bsn_modulo;
                    if (before_bsn == ((rx_win_start + arq_win_size)%bsn_modulo))
                    {
                        break;
                    }
                }
            }

        }


    }
    else if (frag->blk_type == CONTINUING_FRAGMENT)
    {
        // if this is the continuing unreceived arq block, check whether it could be connect with the previous and following arq block to become a sdu
        cidq->arq_block[current_bsn] = frag;
        (cidq->arq_block[current_bsn])->next = NULL;
        before_bsn = (current_bsn - 1 + bsn_modulo) % bsn_modulo;
        next_bsn = (current_bsn+1) % bsn_modulo;
        payload_len = frag->length;
        
        if (cidq->arq_block[before_bsn])
        {
            // link the previous arq blocks
            (cidq->arq_block[before_bsn])->next = cidq->arq_block[current_bsn];
        }
        if (cidq->arq_block[next_bsn])
        {
            // link the following blocks
            (cidq->arq_block[current_bsn])->next = cidq->arq_block[next_bsn];
        }
        if ((is_order && current_bsn == rx_win_start) || (!is_order))
        {
            // this is the block that the window wait for, so it may be the first sdu that could be delivered in order
            // otherwise, if the deliver is not in order, check whether a complete sdu has been assembled

            while (cidq->arq_block[next_bsn])
            {
                payload_len += (cidq->arq_block[next_bsn])->length;
                if ((cidq->arq_block[next_bsn])->blk_type == LAST_FRAGMENT)
                {
                    is_next_complete = 1;
                }
                else 
                {
                    next_bsn = (next_bsn +1 ) % bsn_modulo;
                    if (next_bsn == end_bsn)
                    {
                        break;
                    }
                }
            }

            if (is_next_complete)
            {
                while (cidq->arq_block[before_bsn])
                {
                    payload_len += (cidq->arq_block[before_bsn])->length;
                    if ((cidq->arq_block[before_bsn])->blk_type == FIRST_FRAGMENT)
                    {
                        is_before_complete = 1;
                    }
                    else 
                    {
                        before_bsn = (before_bsn - 1 + bsn_modulo) % bsn_modulo;
                        if (before_bsn == ((rx_win_start + arq_win_size)%bsn_modulo))
                        {
                            break;
                        }
                    }
                }
            }

            if (is_before_complete && is_next_complete)
            {
                // find a complete sdu, now move it to the sduq
                sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
                memset(sdupkt, 0, sizeof(logical_packet));
                sdupkt->cid = cid;
                sdupkt->element_head = cidq->arq_block[before_bsn];
                sdupkt->length = payload_len;
                if (is_mgt_con)
                {
                    enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
                }
                else
                {
                    enqueue_sduq(sduq, cid, sdupkt);

                    cs_msg.len = cid;

                    if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                         sizeof(struct queue_msg)) == -1)
                    {
                        FLOG_ERROR("enqueue error\n");
                    }

                }
                // remove these arq block from the array
                while (before_bsn != next_bsn)
                {
                    cidq->arq_block[before_bsn] = NULL;
                    before_bsn = (before_bsn + 1) % bsn_modulo;
                }
                cidq->arq_block[next_bsn] = NULL;
            }
            else
            {
                is_before_complete = 0;
                is_next_complete = 0;
            }

        }

    }

    if (is_order && (is_before_complete || is_next_complete))
    {
        // a sdu has been successfully assembled, now check whether the later arq block can be successfully assembled
        next_bsn = (next_bsn +1) % bsn_modulo;
        found_start = 0;
        
        while (next_bsn != end_bsn)
        {
            if (cidq->arq_block[next_bsn] == NULL)
            {
                break;
            }
            if ((cidq->arq_block[next_bsn])->blk_type == FIRST_FRAGMENT)
            {
                found_start = 1;
                current_bsn = next_bsn;
                payload_len = (cidq->arq_block[next_bsn])->length;
            } 
            else if ((cidq->arq_block[next_bsn])->blk_type == LAST_FRAGMENT)
            {
                if (found_start)
                {
                    payload_len += (cidq->arq_block[next_bsn])->length;
                    // find a complete sdu, now move it to the sduq
                    sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
                    memset(sdupkt, 0, sizeof(logical_packet));
                    sdupkt->cid = cid;
                    sdupkt->element_head = cidq->arq_block[current_bsn];
                    sdupkt->length = payload_len;
                    if (is_mgt_con)
                    {
                        enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
                    }
                    else
                    {
                        enqueue_sduq(sduq, cid, sdupkt);

                        cs_msg.len = cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                    // remove these arq block from the array
                    while (current_bsn != next_bsn)
                    {
                        cidq->arq_block[current_bsn] = NULL;
                        current_bsn = (current_bsn +1) % bsn_modulo;
                    }
                    cidq->arq_block[current_bsn] = NULL;
                }
                else 
                {
                    break;
                }
                found_start = 0;
                payload_len = 0;
            }
            else if ((cidq->arq_block[next_bsn])->blk_type == NO_FRAGMENTATION)
            {
                if (! found_start)
                {
                    payload_len = (cidq->arq_block[next_bsn])->length;
                    sdupkt = (logical_packet *) malloc( sizeof(logical_packet));
                    memset(sdupkt, 0, sizeof(logical_packet));
                    sdupkt->cid = cid;
                    sdupkt->element_head = cidq->arq_block[current_bsn];
                    sdupkt->length = payload_len;
                    if (is_mgt_con)
                    {
                        enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
                    }
                    else
                    {
                        enqueue_sduq(sduq, cid, sdupkt);

                        cs_msg.len = cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                    // remove these arq block from the array
                    cidq->arq_block[next_bsn] = NULL;
                    found_start = 0;    
                }
                else 
                {
                    // printf error message
                }
            }
            else if ((cidq->arq_block[next_bsn])->blk_type == CONTINUING_FRAGMENT)
            {
                if (found_start)
                {
                    payload_len += (cidq->arq_block[next_bsn])->length;
                }
                else 
                {
                    // printf error message
                }
            }

            next_bsn = (next_bsn +1) % bsn_modulo;
         }
        
     }
    
    return 0;
}

int enqueue_fragq(frag_queue* fragq, unsigned int frame_num, int cid, logical_element* frag, int bsn_modulo, u_int8_t is_mgt_con, sdu_queue* sduq){
    frag_cid_queue* cidq;
    u_char* payload;
    logical_element* cur_le;
    logical_element* next_le;
    logical_packet* sdupkt;
    int frag_msg_type = FRAG_MANAGEMENT_MESSAGE;

    struct queue_msg cs_msg;

    cs_msg.my_type = csforw_en_id;


    // get the frag queue related with this cid
    get_cid_fragq(fragq, cid, &cidq);
        if (frag->blk_type == FIRST_FRAGMENT)
        {
            // release the previous fragments, since a new first fragments comes
            if (cidq->frag_num != 0)
            {
                next_le = cidq->head;
                while(next_le)
                {
                    cur_le = next_le;
                    next_le = cur_le->next;
                    free(cur_le->data);
                    free(cur_le);
                }
                cidq->head = NULL;
                cidq->tail = NULL;
                cidq->bytes_num = 0; 
                cidq->frag_num = 0;
            }
            cidq->head =  frag;
            cidq->tail = frag;
            // memory copy the payload of logical element
            payload = (u_char *) malloc(frag->length);
            memcpy(payload, frag->data, frag->length);
            frag->data = payload;
            frag->next = NULL;
            cidq->frag_num++;
            cidq->bytes_num += frag->length;
        } 
        else if (cidq->frag_num == 0)
        {
                // this frag should be discarded since part of the previous fragments are lost  
            free(frag);
        }
        else 
        {
            // there are some previous fragments in this queue
            if (((cidq->tail->start_bsn+1)% bsn_modulo) == frag->start_bsn)
            {
                cidq->tail->next = frag;
                cidq->tail = frag;
                cidq->bytes_num += frag->length;
                cidq->frag_num++;
                // memory copy the payload of logical element
                payload = (u_char *) malloc(frag->length);
                memcpy(payload, frag->data, frag->length);
                frag->data = payload;
                frag->next = NULL;
                
                if (frag->blk_type == LAST_FRAGMENT)
                {
                    sdupkt = (logical_packet *) malloc(sizeof(logical_packet));
                    memset(sdupkt, 0, sizeof(logical_packet));
                    sdupkt->cid = cid;
                    sdupkt->element_head = cidq->head;
                    sdupkt->length = cidq->bytes_num;
                    if (is_mgt_con)
                    {
                        enqueue_ul_mgt_msg(ul_msg_queue, frame_num, cid, frag_msg_type, sdupkt->length, sdupkt);
                    }
                    else
                    {
                        enqueue_sduq(sduq, cid, sdupkt);

                        cs_msg.len = cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                    cidq->head = NULL;
                    cidq->tail = NULL;
                    cidq->bytes_num = 0;
                    cidq->frag_num = 0;
                }
            }
            else
            {
                // remove the previous fragments and current fragments
                next_le = cidq->head;
                while(next_le)
                {
                    cur_le = next_le;
                    next_le = cur_le->next;
                    free(cur_le->data);
                    cur_le->data = NULL;
                    free(cur_le);
                    cur_le = NULL;
                }
                cidq->head = NULL;
                cidq->tail = NULL;
                cidq->bytes_num = 0; 
                cidq->frag_num = 0;
                // this current frag should also be discarded since part of the previous fragments are lost  
                free(frag);
                frag = NULL;
            }

        }
        return 0;
    
}


