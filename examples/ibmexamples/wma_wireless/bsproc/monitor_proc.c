/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: monitor_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


/** OS */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//#define _GNU_SOURCE
#include <pthread.h>

/** Application */
#include "monitor_proc.h"
#include "queue_util.h"
#include "thd_util.h"
#include "flog.h"
#include "dump_util.h"

#include "bs_cfg.h"
#include "bs_debug.h"
#include "trans.h"
#include "trans_monitor.h"
#include "trans_timer.h"

pthread_t monitor_thd = 0;

static pthread_mutex_t monitor_q_mutex;
unsigned int monitor_queue_len = 0;

struct hook_tlv_node
{
    int type;
    void * buf;
    int len;
    struct hook_dst_device dev;
    struct hook_tlv_node * next;
};


static int send_peri_senssing_to_agent (void * buf);
static int send_trace_to_monitor (void * buf);
static int send_network_entry_to_agent (void * buf);

static int monitor_phy_dump_trace_func(void *p_user_info, size_t len, void *p_msg);
static int monitor_phy_reset_metric_func(void *p_user_info, size_t len, void *p_msg);

void * process_monitor (void *arg __attribute__ ((unused)));


int monitor_process (void)
{
    int ret = 0;

    pthread_attr_t tattr;

    pthread_attr_init (&tattr);

    ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);

    if (ret != 0)
    {
        FLOG_WARNING("set thread scheduling error\n");
    }

    pthread_create (&monitor_thd, &tattr, process_monitor, NULL);

    pthread_attr_destroy (&tattr);

    FLOG_DEBUG("initial Monitor Process finish\n");

    return 0;
}

int monitor_release (void)
{
    if (monitor_thd != 0)
    {
        pthread_cancel (monitor_thd);
        pthread_join (monitor_thd, NULL);
    }

    pthread_mutex_destroy(&(monitor_q_mutex));

    return 0;
}

void * process_monitor (void *arg __attribute__ ((unused)))
{
    FLOG_INFO ("Monitor thread Started");

    struct queue_msg p_msg;

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

    p_msg.my_type = monitor_de_id;

    pthread_mutex_init( &(monitor_q_mutex), NULL );

    while (1)
    {
        if (wmrt_dequeue (monitor_de_id, &p_msg, sizeof(struct queue_msg))
                == -1)
        {
            FLOG_ERROR ("DEQUEUE ERROR\n");
            continue;
        }

        switch(p_msg.len)
        {
            case RRH_PERIOD_SENSING:
                send_peri_senssing_to_agent (p_msg.p_buf);
                break;

            case HOOK_DEBUG_TRACE:

                pthread_mutex_lock(&monitor_q_mutex);
                if (monitor_queue_len > 0)
                {
                    monitor_queue_len --;
                }
                pthread_mutex_unlock(&monitor_q_mutex);

                send_trace_to_monitor (p_msg.p_buf);
                break;

            case WIRELESS_CPE_ENTRY:
                send_network_entry_to_agent (p_msg.p_buf);
                break;

            default:
                FLOG_ERROR("unsupported Monitor Message\n");
                break;
        }
    }

    return NULL;
}

int send_network_entry_msg (char * mac_addr)
{
    struct queue_msg p_msg;
    char * wmb_mac = (char *)malloc(128);

    strcpy(wmb_mac, mac_addr);

    p_msg.my_type = monitor_en_id;
    p_msg.p_buf = (void *)wmb_mac;
    p_msg.len = WIRELESS_CPE_ENTRY;

    if (wmrt_enqueue (monitor_en_id, &p_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("DEQUEUE ERROR\n");
        return 1;
    }

    return 0;
}

static int send_network_entry_to_agent (void * buf)
{
    char * mac_addr = (char * )buf;
    int uw_ret = 0;

    #ifdef _NEW_TRANS_ENABLE_
    
    uw_ret = trans_send_bs_msg_to_agent(TRANS_SEND_AGENT_TOPOLGY, 0, mac_addr);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_agent for AGENT_TOPOLGY error! uw_ret = %d\r\n", uw_ret);
    }
    
    #else
    
    struct trans_send_msg_to_agent *p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);

    p_msg_info->f_callback = NULL;
    p_msg_info->uc_block_flag = TRANS_QUENE_NO_BLOCK;
    p_msg_info->uc_ack_flag = TRANS_ACK_FLAG_OK;
    p_msg_info->uw_resp_len = strlen(mac_addr);
    p_msg_info->p_resp_msg = mac_addr;
    p_msg_info->p_reqs_msg = "topologyUpdate";

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
    }

    free (p_msg_info);
    
    #endif

    free (mac_addr);

    return 0;
}


int send_peri_senssing_msg (char * p_active_band, float * p_result)
{
    struct trans_periodic_sensing_info * p_periodic_info =
        (struct trans_periodic_sensing_info *)malloc (sizeof (struct trans_periodic_sensing_info));

    int * p_per_interref = (int *)malloc(21 * sizeof(int));
    float * p_per_result = (float *)malloc(1024 * sizeof (float));
    int i;

    struct queue_msg p_msg;

    for (i = 0; i < 21; i++)
    {
        p_per_interref[i] = (int)p_active_band[i];
    }

    memset (p_per_result, 0, 1024 * sizeof (float));
    memcpy (p_per_result, p_result, 24 * sizeof (float));

    p_periodic_info->p_per_interref = p_per_interref;
    p_periodic_info->p_per_sensing = p_per_result;

    p_msg.my_type = monitor_en_id;
    p_msg.p_buf = (void *)p_periodic_info;
    p_msg.len = RRH_PERIOD_SENSING;

    if (wmrt_enqueue (monitor_en_id, &p_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("DEQUEUE ERROR\n");
        return 1;
    }

    return 0;
}

static int send_peri_senssing_to_agent (void * buf)
{
    int uw_ret = 0;

    struct trans_periodic_sensing_info * p_periodic_info;

    #ifndef _NEW_TRANS_ENABLE_
    struct trans_send_msg_to_agent * p_msg_info = NULL;
    #endif

    if (buf == NULL)
    {
        FLOG_ERROR("Wrong Buffer\n");
        return 1;
    }

    p_periodic_info = (struct trans_periodic_sensing_info *) buf;

    if ( (p_periodic_info->p_per_interref == NULL) || (p_periodic_info->p_per_sensing == NULL) )
    {
        FLOG_ERROR("Wrong Buffer\n");
        return 1;
    }

    #ifdef _NEW_TRANS_ENABLE_
    
    uw_ret = trans_send_bs_msg_to_agent(TRANS_SEND_AGENT_PERIODIC, 0, p_periodic_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_agent for AGENT_PERIODIC error! uw_ret = %d\r\n", uw_ret);
    }
    
    #else

    p_msg_info =
        (struct trans_send_msg_to_agent *) malloc (sizeof (struct trans_send_msg_to_agent));

    if (p_msg_info == NULL)
    {
        FLOG_ERROR("Failed to malloc memory\n");
        return 1;
    }

    p_msg_info->f_callback = NULL;
    p_msg_info->uc_block_flag = TRANS_QUENE_NO_BLOCK;
    p_msg_info->uc_ack_flag = TRANS_ACK_FLAG_OK;
    p_msg_info->uw_resp_len = 8;
    p_msg_info->p_resp_msg = p_periodic_info;
    p_msg_info->p_reqs_msg = "periodic";

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg for Agent periodic  error! uw_ret = %d\r\n", uw_ret);
    }

    free (p_msg_info);

    #endif

    free (p_periodic_info->p_per_interref);
    free (p_periodic_info->p_per_sensing);
    free (p_periodic_info);

    return uw_ret;
}

int hook_debug_trace(int debug_idx, void * p_buf, int buf_len, int inc)
{
//    int i = 0;
    int active_dst_count = 0;
    struct debug_hook_unit * p_hook_node = NULL;
    struct hook_unit_dst * p_dst = NULL;
    struct hook_tlv_node * p_tlv_node = NULL;
    struct hook_tlv_node * p_tlv_hdr = NULL;
    void * p_tmp_buf = NULL;
    struct monitor_trace_hdr * p_trace_hdr = NULL;
    int min_pkt_size = 0;
    int trace_len = 0;

    struct queue_msg p_msg;

    if( idx_get_global_hook (debug_idx, &p_hook_node) != 0 )
    {
        return 1;
    }

    if (buf_len > 0)
    {
        if ( (p_hook_node->buf_len < (unsigned int)buf_len) || (p_buf == NULL) )
        {
            FLOG_WARNING("Wrong buffer");
            return 1;
        }
/*
        if (p_hook_node->buf_len < (unsigned int)buf_len)
        {
            return 1;
        }
*/
        p_dst = p_hook_node->next;

//        for (i = 0; i < p_hook_node->dst_count; i++)
        while(p_dst != NULL)
        {

            if ( (p_hook_node->count)%(p_dst->sampling) != 0)
            {
                p_dst = p_dst->next;
                continue;
            }

            if (active_dst_count == 0)
            {
                p_tlv_hdr =
                    (struct hook_tlv_node *)malloc (sizeof (struct hook_tlv_node));
                p_tlv_node = p_tlv_hdr;

                if (p_tlv_node == NULL)
                {
                    FLOG_ERROR("error malloc buffer");
                    return 1;
                }

                memset(p_tlv_node, 0, sizeof (struct hook_tlv_node));

                if ( (buf_len + sizeof(struct monitor_trace_hdr)) < MIN_MONITOR_PKT_SIZE )
                {
                    min_pkt_size = MIN_MONITOR_PKT_SIZE;
                    trace_len = MIN_MONITOR_PKT_SIZE - sizeof(struct monitor_trace_hdr);
                }else
                {
                    min_pkt_size = buf_len + sizeof(struct monitor_trace_hdr);
                    trace_len = buf_len;
                }

                p_tmp_buf = malloc (min_pkt_size);

                if (p_tmp_buf == NULL)
                {
                    free (p_tlv_node);
                    FLOG_ERROR("error malloc buffer");
                    return 1;
                }

                p_trace_hdr = (struct monitor_trace_hdr *)p_tmp_buf;
//                p_trace_hdr->len = buf_len;
                p_trace_hdr->len = trace_len;
                strcpy(p_trace_hdr->hook_name, p_hook_node->hook_name);

                p_tlv_node->type = debug_idx;
//                p_tlv_node->len = buf_len + sizeof (struct monitor_trace_hdr);
                p_tlv_node->len = min_pkt_size;
                p_tlv_node->buf = p_tmp_buf;

                memcpy (p_tlv_node->buf + sizeof (struct monitor_trace_hdr), p_buf, buf_len);
                memcpy ( &(p_tlv_node->dev), &(p_dst->dev), sizeof(struct hook_dst_device) );

                active_dst_count ++;
            }else
            {
                p_tlv_node->next =
                    (struct hook_tlv_node *)malloc (sizeof (struct hook_tlv_node));
                p_tlv_node = p_tlv_node->next;

                if (p_tlv_node == NULL)
                {
                    free (p_tmp_buf);

                    while(p_tlv_hdr->next != NULL)
                    {
                        p_tlv_node = p_tlv_hdr->next;
                        p_tlv_hdr->next = p_tlv_hdr->next->next;
                        free(p_tlv_node);
                    }

                    free (p_tlv_hdr);

                    FLOG_ERROR("error malloc buffer");
                    return 1;
                }

                memset(p_tlv_node, 0, sizeof (struct hook_tlv_node));

                p_tlv_node->type = debug_idx;
//                p_tlv_node->len = buf_len + sizeof (struct monitor_trace_hdr);
                p_tlv_node->len = min_pkt_size;
                p_tlv_node->buf = p_tmp_buf;

                memcpy ( &(p_tlv_node->dev), &(p_dst->dev), sizeof(struct hook_dst_device) );

                active_dst_count ++;
            }

            p_dst = p_dst->next;
        }

        if (p_tlv_hdr != NULL)
        {
            p_msg.my_type = monitor_en_id;
            p_msg.p_buf = (void *)p_tlv_hdr;
            p_msg.len = HOOK_DEBUG_TRACE;

            pthread_mutex_lock(&monitor_q_mutex);
            if (monitor_queue_len > MAX_QUEUE_LEN)
            {
                pthread_mutex_unlock(&monitor_q_mutex);
                FLOG_ERROR("Queue bloacked");

                trans_monitor_release();

                free (p_tlv_hdr->buf);

                while(p_tlv_hdr->next != NULL)
                {
                    p_tlv_node = p_tlv_hdr->next;
                    p_tlv_hdr->next = p_tlv_hdr->next->next;
                    free(p_tlv_node);
                }

                free (p_tlv_hdr);

                return 1;
            }
            monitor_queue_len ++;
            pthread_mutex_unlock(&monitor_q_mutex);

            if (wmrt_enqueue (monitor_en_id, &p_msg, sizeof(struct queue_msg))
                    == -1)
            {
                FLOG_ERROR ("ENQUEUE ERROR\n");
                free (p_tlv_node);
                free (p_buf);

                return 1;
            }
        }

        p_hook_node->count += inc;

    }else if ( (buf_len == 0) && (p_buf == NULL) )
    {
        p_dst = p_hook_node->next;

//        for (i = 0; i < p_hook_node->dst_count; i++)
        while(p_dst != NULL)
        {

            if ( (p_hook_node->count)%(p_dst->sampling) == 0)
            {
                return ( (int)(p_hook_node->count) );
            }

            p_dst = p_dst->next;
        }
    }else if (p_buf == NULL)
    {
        p_hook_node->count += inc;
    }else
    {

    }

    return 0;
}

static int send_trace_to_monitor (void * buf)
{
    int ret = 0;
    //struct debug_hook_unit * p_hook_node = NULL;
    struct hook_tlv_node * p_tlv_hdr = (struct hook_tlv_node *)buf;
    struct hook_tlv_node * p_tlv_node = p_tlv_hdr;

    #ifndef _NEW_TRANS_ENABLE_
    struct trans_send_msg_to_monitor * p_msg_info;
    #endif

    if (buf == NULL)
    {
        FLOG_WARNING("error TLV hdr");
        return 1;
    }
/*
    if (idx_get_global_hook (p_tlv_hdr->type, &p_hook_node) != 0)
    {
        FLOG_WARNING("Can not get hook by type %d\n", p_tlv_hdr->type);
        p_tlv_node = NULL;
        ret = 1;
    }
*/
    while(p_tlv_node != NULL)
    {
    //#if 0
        #ifdef _NEW_TRANS_ENABLE_
        
        ret = trans_send_bs_msg_to_monitor(TRANS_SEND_MONITOR_HOOK_RESP, 
                                    0,
                                    p_tlv_node->dev.trans_id, 
                                    p_tlv_node->len,
                                    p_tlv_node->buf);
        /*Error*/
        if (TRANS_SUCCESS != ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_monitor error! ret = %d\r\n", ret);
        }
        
        #else
        p_msg_info = (struct trans_send_msg_to_monitor *) malloc (sizeof (struct trans_send_msg_to_monitor));
        if (p_msg_info == NULL)
        {
            ret = 1;
            break;
        }

        p_msg_info->p_payload = p_tlv_hdr->buf;
        p_msg_info->uc_ack_flag = 0;
        p_msg_info->us_opration = 0;
        p_msg_info->uw_payload_len = p_tlv_hdr->len;

        ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, p_msg_info);
        /*Error*/
        if (TRANS_SUCCESS != ret)
        {
            FLOG_ERROR("Call trans_send_wireless_msg error! ret = %d\r\n", ret);
        }
    
        free(p_msg_info);
        #endif
        
        p_tlv_node = p_tlv_node->next;
    }
    //#endif

    free (p_tlv_hdr->buf);

    while(p_tlv_hdr->next != NULL)
    {
        p_tlv_node = p_tlv_hdr->next;
        p_tlv_hdr->next = p_tlv_hdr->next->next;
        free(p_tlv_node);
    }

    free (p_tlv_hdr);

    return ret;
}


#ifdef _NEW_TRANS_ENABLE_

static int monitor_phy_dump_trace_func(void *p_info, size_t len, void *p_msg)
{
    struct monitor_dump_metric *p_cfg_trace = NULL;

    u_int32_t uw_payload_len = 0;
//    void * p_buf;

    (void) p_info;
    (void) len;

    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }

    uw_payload_len = len;
    
    if (0 >= uw_payload_len)
    {
        FLOG_ERROR ("No payload \r\n");

        return 1;
    }
    else
    {
        p_cfg_trace = (struct monitor_dump_metric *)(p_msg);

        if (NULL == p_cfg_trace)
        {
            FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
            return 1;
        }

        if (p_cfg_trace->on_off == 1)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, -1 );
        }else if (p_cfg_trace->on_off == 0)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, 0 );
        }else if (p_cfg_trace->on_off > 0)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, p_cfg_trace->on_off);
        }else
        {
            FLOG_ERROR ("Reset dump on-off error\n");
            return 1;
        }

        FLOG_INFO("Reset a dump %s: %d\n", p_cfg_trace->a_cfg_name, p_cfg_trace->on_off);
    }

    FLOG_DEBUG("Reset %s success\n", p_cfg_trace->a_cfg_name);

    if (strcmp(p_cfg_trace->a_cfg_name, "RX_ALL_POOL") == 0)
    {
        DO_DUMP(DUMP_RX_ALL_POOL_ID, 0, NULL, 0);
    }

    return 0;
}

static int monitor_phy_reset_metric_func(void *p_user_info, size_t len, void *p_msg)
{
    int ret = 0;
    struct monitor_reset_metric *p_cfg_trace = NULL;
    u_int32_t uw_payload_len = len;
    void * p_buf;

    (void) p_user_info;
    //(void) len;

    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }

    if (0 >= uw_payload_len)
    {
        FLOG_ERROR ("No payload \r\n");

        return 1;
    }
    else
    {
        p_cfg_trace = (struct monitor_reset_metric *)(p_msg);

        if (NULL == p_cfg_trace)
        {
            FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
            return 1;
        }

        p_buf = p_msg + sizeof (struct monitor_reset_metric);

        if (p_cfg_trace->type == 0)
        {
            FLOG_INFO("Get a change param %s: %d\n", p_cfg_trace->a_cfg_name, *((int *)p_buf));
        }else if (p_cfg_trace->type == 1)
        {
            FLOG_INFO("Get a change param %s: %s\n", p_cfg_trace->a_cfg_name, (char *)p_buf);
        }else if (p_cfg_trace->type == 2)
        {
            FLOG_INFO("Get a change param %s: %f\n", p_cfg_trace->a_cfg_name, *((float *)p_buf));
        }else
        {
            FLOG_WARNING("error type\n");
            return 1;
        }

        ret = update_global_param (p_cfg_trace->a_cfg_name, p_buf);
//        ret = update_global_param (p_cfg_trace->a_cfg_name, &(p_cfg_trace->type));

        if (ret != 0)
        {
            FLOG_WARNING("Set %s failed\n", p_cfg_trace->a_cfg_name);
            return 1;
        }
    }

    FLOG_DEBUG("Set %s success\n", p_cfg_trace->a_cfg_name);

    return 0;
}


static int monitor_phy_cfg_trace_func(void *p_info, size_t len, void *p_msg)
{
    int ret = 0;
    struct monitor_cfg_trace *p_cfg_trace = NULL;
    //u_int32_t uw_payload_len = 0;

    /** faked DEV */
    struct hook_dst_device dev;
    struct hook_dst_device dev_tmp;
    //memset(dev.mac, 0xff, 6);
    //dev.trans_id = 1;

    //char mac_tmp[6] = {0};
    //unsigned int trans_id_tmp = 0;
    
    if ((NULL == p_msg) || (NULL == p_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }    

    /*Get transaction ID from p_info*/
    //dev.trans_id = trans_transaction_get_tra_id(p_info);
    dev.trans_id = len;
    memcpy(dev.mac, trans_transaction_get_src_mac(p_info), 6);
   
    p_cfg_trace = (struct monitor_cfg_trace *)(p_msg);
    
    if (NULL == p_cfg_trace)
    {
        FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
        return 1;
    }
    
    //FLOG_INFO("Change a hook set %s, %d, %d\n", p_cfg_trace->a_cfg_name, p_cfg_trace->w_on_off, p_cfg_trace->w_sampling);
    
    /*Open*/
    if (p_cfg_trace->w_on_off == 1)
    {          
        ret = key_set_global_hook (p_cfg_trace->a_cfg_name,
                                   &dev,
                                   p_cfg_trace->w_on_off, 
                                   p_cfg_trace->w_sampling);
    }
    /*Close*/
    else if (p_cfg_trace->w_on_off == 0)
    {
        memcpy(dev_tmp.mac, dev.mac, 6);

        ret = key_find_global_hook(p_cfg_trace->a_cfg_name, &dev_tmp);
        if (0 != ret)
        {
            FLOG_ERROR("key_find_global_hook 1 error! \r\n");
            return 1;
        }
    
        ret = key_set_global_hook (p_cfg_trace->a_cfg_name,
                                   &dev_tmp,
                                   0,
                                   p_cfg_trace->w_sampling);
    
        ret = trans_send_bs_msg_to_monitor(TRANS_SEND_MONITOR_HOOK_RESP, 
                                    0xc1,
                                    dev.trans_id, 
                                    sizeof(struct monitor_cfg_trace),
                                    p_cfg_trace);
        /*Error*/
        if (TRANS_SUCCESS != ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_monitor error! ret = %d\r\n", ret);
            return 1;
        }
    
        ret = trans_send_bs_msg_to_monitor(TRANS_SEND_MONITOR_HOOK_RESP, 
                                    0xc1,
                                    dev_tmp.trans_id, 
                                    sizeof(struct monitor_cfg_trace),
                                    p_cfg_trace);
        /*Error*/
        if (TRANS_SUCCESS != ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_monitor error! ret = %d\r\n", ret);
            return 1;
        }
    
    
    }
    /*Change Sampling*/
    else if (p_cfg_trace->w_on_off == 2)
    {
        memcpy(dev_tmp.mac, dev.mac, 6);
        ret = key_find_global_hook(p_cfg_trace->a_cfg_name, &dev_tmp);
        if (0 != ret)
        {
            FLOG_ERROR("key_find_global_hook 2 error! \r\n");
            return 1;
        }
    
        ret = key_set_global_hook (p_cfg_trace->a_cfg_name,
                                   &dev,
                                   -1,
                                   p_cfg_trace->w_sampling);
    
    
        ret = trans_send_bs_msg_to_monitor(TRANS_SEND_MONITOR_HOOK_RESP, 
                                    0xc1,
                                    dev_tmp.trans_id, 
                                    sizeof(struct monitor_cfg_trace),
                                    p_cfg_trace);
        /*Error*/
        if (TRANS_SUCCESS != ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_monitor error! ret = %d\r\n", ret);
            return 1;
        }
    
    }        
    else
    {
        ret = 1;
    }
    
    if (ret != 0)
    {
        FLOG_WARNING("Set %s failed\n", p_cfg_trace->a_cfg_name);
        return 1;
    }


    FLOG_DEBUG("Set %s success\n", p_cfg_trace->a_cfg_name);

    return 0;
}


static int monitor_phy_delete_trace_func(void *p_info, size_t len, void *p_msg)
{
    //struct hook_dst_device dev;
    struct hook_dst_device dev_tmp;
    int ret = 0;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    } 

    (void)p_info;
    (void)len;
  
    memcpy(dev_tmp.mac, p_msg, 6);
  
       
    ret = key_find_global_hook("FCH", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook FCH error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("FCH",
                                   &dev_tmp,
                                   0,
                                   -1);
    }

    ret = key_find_global_hook("Constellation", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook Constellation error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("Constellation",
                               &dev_tmp,
                               0,
                               -1);
    }


    ret = key_find_global_hook("Channel_quality", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook Channel_quality error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("Channel_quality",
                               &dev_tmp,
                               0,
                               -1);
    }

    ret = key_find_global_hook("BER", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook BER error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("BER",
                              &dev_tmp,
                              0,
                              -1);
    }    

     ret = key_find_global_hook("PS_info", &dev_tmp);
     if (0 != ret)
     {
         FLOG_DEBUG("key_find_global_hook PS_info error! \r\n");
         //return 1;
     }
     else
     {
         ret = key_set_global_hook ("PS_info",
                                &dev_tmp,
                                0,
                                -1);
     }   


     ret = key_find_global_hook("Ranging_power", &dev_tmp);
     if (0 != ret)
     {
         FLOG_DEBUG("key_find_global_hook Ranging_power error! \r\n");
         //return 1;
     }
     else
     {
         ret = key_set_global_hook ("Ranging_power",
                                &dev_tmp,
                                0,
                                -1);
     }        

    ret = key_find_global_hook("Ranging_result", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook Ranging_result error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("Ranging_result",
                               &dev_tmp,
                               0,
                               -1);
    }  

    ret = key_find_global_hook("CRC_count", &dev_tmp);
    if (0 != ret)
    {
        FLOG_DEBUG("key_find_global_hook CRC_count error! \r\n");
        //return 1;
    }
    else
    {
        ret = key_set_global_hook ("CRC_count",
                               &dev_tmp,
                               0,
                               -1);

    }  


    return 0;

}


#else

static int monitor_phy_dump_trace_func(void *p_user_info, size_t len, void *p_msg)
{
    struct monitor_dump_metric *p_cfg_trace = NULL;
    struct trans_resp_msg_header *p_resp_header = NULL;
    u_int32_t uw_payload_len = 0;
//    void * p_buf;

    (void) p_user_info;
    (void) len;

    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }

    p_resp_header = (struct trans_resp_msg_header *)p_msg;

    uw_payload_len = p_resp_header->uw_len;

    if (0 >= uw_payload_len)
    {
        FLOG_ERROR ("No payload \r\n");

        return 1;
    }
    else
    {
        p_cfg_trace = (struct monitor_dump_metric *)(p_resp_header->p_buf);

        if (NULL == p_cfg_trace)
        {
            FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
            return 1;
        }

        if (p_cfg_trace->on_off == 1)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, -1 );
        }else if (p_cfg_trace->on_off == 0)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, 0 );
        }else if (p_cfg_trace->on_off > 0)
        {
            RESET_DUMP (0, p_cfg_trace->a_cfg_name, 0, p_cfg_trace->on_off);
        }else
        {
            FLOG_ERROR ("Reset dump on-off error\n");
            return 1;
        }

        FLOG_INFO("Reset a dump %s: %d\n", p_cfg_trace->a_cfg_name, p_cfg_trace->on_off);
    }

    FLOG_DEBUG("Reset %s success\n", p_cfg_trace->a_cfg_name);

    if (strcmp(p_cfg_trace->a_cfg_name, "RX_ALL_POOL") == 0)
    {
        DO_DUMP(DUMP_RX_ALL_POOL_ID, 0, NULL, 0);
    }

    return 0;
}

static int monitor_phy_reset_metric_func(void *p_user_info, size_t len, void *p_msg)
{
    int ret = 0;
    struct monitor_reset_metric *p_cfg_trace = NULL;
    struct trans_resp_msg_header *p_resp_header = NULL;
    u_int32_t uw_payload_len = 0;
    void * p_buf;

    (void) p_user_info;
    (void) len;

    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }

    p_resp_header = (struct trans_resp_msg_header *)p_msg;

    uw_payload_len = p_resp_header->uw_len;

    if (0 >= uw_payload_len)
    {
        FLOG_ERROR ("No payload \r\n");

        return 1;
    }
    else
    {
        p_cfg_trace = (struct monitor_reset_metric *)(p_resp_header->p_buf);

        if (NULL == p_cfg_trace)
        {
            FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
            return 1;
        }

        p_buf = p_resp_header->p_buf + sizeof (struct monitor_reset_metric);

        if (p_cfg_trace->type == 0)
        {
            FLOG_INFO("Get a change param %s: %d\n", p_cfg_trace->a_cfg_name, *((int *)p_buf));
        }else if (p_cfg_trace->type == 1)
        {
            FLOG_INFO("Get a change param %s: %s\n", p_cfg_trace->a_cfg_name, (char *)p_buf);
        }else if (p_cfg_trace->type == 2)
        {
            FLOG_INFO("Get a change param %s: %f\n", p_cfg_trace->a_cfg_name, *((float *)p_buf));
        }else
        {
            FLOG_WARNING("error type\n");
            return 1;
        }

        ret = update_global_param (p_cfg_trace->a_cfg_name, p_buf);
//        ret = update_global_param (p_cfg_trace->a_cfg_name, &(p_cfg_trace->type));

        if (ret != 0)
        {
            FLOG_WARNING("Set %s failed\n", p_cfg_trace->a_cfg_name);
            return 1;
        }
    }

    FLOG_DEBUG("Set %s success\n", p_cfg_trace->a_cfg_name);

    return 0;
}

static void monitor_phy_delete_trace (void )
{
    cleanall_dst();

    return;
}

static int monitor_phy_cfg_trace_func(void *p_user_info, size_t len, void *p_msg)
{
    int ret = 0;
    struct monitor_cfg_trace *p_cfg_trace = NULL;
    struct trans_resp_msg_header *p_resp_header = NULL;
    u_int32_t uw_payload_len = 0;

    /** faked DEV */
    struct hook_dst_device dev;
    memset(dev.mac, 0xff, 6);
    dev.trans_id = 1;

    (void) p_user_info;
    (void) len;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 1;
    }    

    p_resp_header = (struct trans_resp_msg_header *)p_msg;

    if (TRANS_ACK_FLAG_CLEAN_OPERATION == p_resp_header->uc_result)
    {
        monitor_phy_delete_trace();
        
        return 0;
    }

    if ((0 != p_resp_header->uc_result) && (TRANS_ACK_FLAG_CLEAN_OPERATION != p_resp_header->uc_result))
    {
        FLOG_ERROR("Result eror %d! \r\n", p_resp_header->uc_result);
        
        return 1;
    }
    
    uw_payload_len = p_resp_header->uw_len;
    
    if (0 >= uw_payload_len)
    {
        FLOG_ERROR ("No payload \r\n");

        return 1;
    }
    else
    {
        p_cfg_trace = (struct monitor_cfg_trace *)(p_resp_header->p_buf);

        if (NULL == p_cfg_trace)
        {
            FLOG_ERROR("NULL PTR p_cfg_trace! \r\n");
            return 1;
        }

        FLOG_INFO("Change a hook set %s, %d, %d\n", p_cfg_trace->a_cfg_name, p_cfg_trace->w_on_off, p_cfg_trace->w_sampling);

        if (p_cfg_trace->w_on_off == 1)
        {
            ret = key_set_global_hook (p_cfg_trace->a_cfg_name,
                                       &dev,
                                       p_cfg_trace->w_on_off, 
                                       p_cfg_trace->w_sampling);
        }
        else if (p_cfg_trace->w_on_off == 0)
        {
            ret = key_set_global_hook (p_cfg_trace->a_cfg_name,
                                       &dev,
                                       -1,
                                       p_cfg_trace->w_sampling);
        }else
        {
            ret = 1;
        }

        if (ret != 0)
        {
            FLOG_WARNING("Set %s failed\n", p_cfg_trace->a_cfg_name);
            return 1;
        }

    }

    FLOG_DEBUG("Set %s success\n", p_cfg_trace->a_cfg_name);

    return 0;
}
#endif

int init_monitor(void)
{
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    us_op = 1;
    #ifdef _NEW_TRANS_ENABLE_
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    monitor_phy_cfg_trace_func,
                                    monitor_phy_delete_trace_func);
    #else
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    monitor_phy_cfg_trace_func,
                                    monitor_phy_cfg_trace_func);

    #endif
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP %d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return 1;    
    } 

    us_op = 2;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    monitor_phy_dump_trace_func,
                                    NULL);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP %d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return 1;    
    } 
    
    us_op = 3;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    monitor_phy_reset_metric_func,
                                    NULL);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP %d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return 1;    
    } 
   
    return 0;
}



