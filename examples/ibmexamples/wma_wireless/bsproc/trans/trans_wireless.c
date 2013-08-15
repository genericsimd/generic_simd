/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_wireless.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-Apr.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_common.h>
#include <trans_transaction.h>
#include <trans_list.h>
#include <trans_device.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_timer.h>
#include <trans_debug.h>
#include <trans_monitor.h>

/*TCP*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>


#ifdef TRANS_BS_COMPILE
#include "bs_cfg.h"
#endif

#ifdef TRANS_MS_COMPILE
//#include "macphy_init_process.h"
#include "ms_cfg.h"

#endif

/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/
/*****************************************************************************+
* Function: trans_wireless_enqueue_monitor_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_enqueue_monitor_msg(struct trans_monitor_build_msg_info *p_build_info)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_send_len = 0;
    
    struct trans_common_queue st_queue_info;
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Enter \r\n");
    
    /* Allocate a memory.  */
    p_send_buf = (u_int8_t *)malloc(SIZEOF_TRANS_MONITOR_HEADER + p_build_info->uw_payload_len);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        return TRANS_FAILD;   
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "1 p_send_buf = %p! \r\n", p_send_buf);  
    
    /*Build monitor message header*/
    uw_ret = trans_monitor_build_msg(p_build_info, p_send_buf, &uw_send_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_build_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD; 
    }
    else
    {
        /*Enqueue*/
        st_queue_info.p_msg = p_send_buf;
        st_queue_info.uc_module_type = g_trans_local_device_info.uc_module_type;
        st_queue_info.uw_len = uw_send_len;
        st_queue_info.w_sockfd = 0;
        
        memcpy(st_queue_info.a_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
        
        uw_ret = trans_common_msg_en_queue(&st_queue_info);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);
       
        } 

    }
    
    //free (p_send_buf);

    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Exit \r\n");

    return uw_ret; 
    
}

#ifdef TRANS_UI_COMPILE

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_data_port()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-25
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_data_port(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};

    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_IQ_DATA_PORT;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

#endif

#if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)


#endif

/*****************************************************************************+
 *Code 
+*****************************************************************************/
#ifdef TRANS_BS_COMPILE

extern u_int32_t trans_msg_en_quene(void *p_msg, struct trans_en_queue_msg *p_en_quene);
extern u_int32_t trans_msg_de_quene(u_int8_t *p_result);


#ifdef TRANS_RRH_COMPILE

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_block_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-17
* 
+*****************************************************************************/
int trans_wireless_send2_rrh_block_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;   
    int32_t  w_result = 0;       
    struct trans_en_queue_msg   st_en_quene; 
    
    //len = len;
    
    if (NULL == p_info) 
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    if (TRANS_TIMER_STATE_TIMEOUT == len)
    {
        FLOG_ERROR("Time out : callback function\r\n"); 
        
        uw_ret = trans_transaction_time_out(p_info); 
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
        
            //return;
        }

        w_result = TRANS_ACK_FLAG_RRH_TIMEOUT;
    }
    else
    {
        w_result = trans_transaction_get_result(p_info);
        if (0 > w_result)
        {
            FLOG_ERROR("Call trans_transaction_get_result error. w_result = %d. \r\n", w_result);
            FLOG_ERROR("RRH response message error! w_result = %d\r\n", w_result);
        
            w_result = TRANS_ACK_FLAG_OTHER_ERR;
        }
    }

    
    st_en_quene.uc_result = w_result;
    st_en_quene.uw_len = len;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_RRH;
    
    uw_ret = trans_msg_en_quene(p_msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    return TRANS_SUCCESS;    

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_noblock_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-17
* 
+*****************************************************************************/
int trans_wireless_send2_rrh_noblock_func(void *p_info, size_t len, void *p_msg)
{
    int32_t  w_result = 0;
    u_int32_t uw_ret = 0;   

    //len = len;
    (void)p_msg;
        
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    if (TRANS_TIMER_STATE_TIMEOUT == len)
    {
        FLOG_ERROR("Time out : callback function\r\n"); 
        
        uw_ret = trans_transaction_time_out(p_info); 
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
        
            //return;
        }
    
        w_result = TRANS_ACK_FLAG_RRH_TIMEOUT;
    }
    else
    {
        w_result = trans_transaction_get_result(p_info);
        if (0 > w_result)
        {
            FLOG_ERROR("Call trans_transaction_get_result error. w_result = %d. \r\n", w_result);
            FLOG_ERROR("RRH response message error! w_result = %d\r\n", w_result);
        
            w_result = TRANS_ACK_FLAG_OTHER_ERR;
        }
    }

    /*Do not handle exceptions -----need do it in future*/  
 
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_wireless_send_rrh_ch_mode()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_ch_mode(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;

    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
   
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  

    (void) p_info;  //NULL
    uc_num = len;

    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
      
    a_rrh_tag[0] = RRH_MSG_CHAN1_WORKMODE;
    a_rrh_tag[1] = RRH_MSG_CHAN2_WORKMODE;

    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;

    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }

    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);

        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;

    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);

            uw_ret = TRANS_FAILD;
        }
    }

    free(p_rrh_payload);
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_ch_flag()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_ch_flag(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
   
    struct trans_monitor_build_msg_info st_build_info;
   
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_CHAN1_SWITCH;
    a_rrh_tag[1] = RRH_MSG_CHAN2_SWITCH;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_ch_freq()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_ch_freq(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_CHAN1_FREQ;
    a_rrh_tag[1] = RRH_MSG_CHAN2_FREQ;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_dl_pre_time()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_dl_pre_time(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;

    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_DL_PRESEND_TIME;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_tx_len()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_tx_len(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};
   
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_TX_LEN;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_rx_len()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_rx_len(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_RX_LEN;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_pa_switch()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_pa_switch(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
   
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_PA_SWITCH_A_CFG;
    a_rrh_tag[1] = RRH_MSG_PA_SWITCH_B_CFG;

    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }

    free(p_rrh_payload);
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_gps_enable()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_gps_enable(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   
   
    a_rrh_tag[0] = RRH_MSG_GPS_ENABLE_CFG;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_output_pow()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_output_pow(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};

    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_NORM_OUTPUT_POWER;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);    
    
    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_agc_enable()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_agc_enable(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};

    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_AGC_ENABLE;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_data_port()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-25
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_data_port(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};

    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_IQ_DATA_PORT;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}



/*****************************************************************************+
* Function: trans_wireless_send2_rrh_ch_rx_pgc()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_ch_rx_pgc(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_CHAN1_RX_PGC;
    a_rrh_tag[1] = RRH_MSG_CHAN2_RX_PGC;

    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_carrier_info()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-06-05
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_carrier_info(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    struct trans_rrh_carrier_info st_carrier_info;
    struct trans_rrh_carrier_info *p_carrier_info = NULL;
    
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
   
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set carrier_info*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_CARRIER_INFO;

    p_carrier_info = (struct trans_rrh_carrier_info *)p_value;
    
    st_carrier_info.uc_carr_no = p_carrier_info->uc_carr_no;
    st_carrier_info.uw_carr_freq = TRANS_HTONL(p_carrier_info->uw_carr_freq);
    st_carrier_info.uw_carr_bw = TRANS_HTONL(p_carrier_info->uw_carr_bw);
    st_carrier_info.uc_carr_pwr = p_carrier_info->uc_carr_pwr;

    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = &st_carrier_info;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_byte_order()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-25
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_byte_order(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[1] = {0};

    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }   

    a_rrh_tag[0] = RRH_MSG_BYTEORDER;
    
    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_ttg_rtg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_ttg_rtg(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    u_int16_t   a_rrh_tag[2] = {0};
    
    struct trans_monitor_build_msg_info st_build_info;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    (void) p_info;  //NULL
    uc_num = len;
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    a_rrh_tag[0] = RRH_MSG_TTG;
    a_rrh_tag[1] = RRH_MSG_RTG;

    st_build_rrh.p_tag = a_rrh_tag;
    st_build_rrh.p_value = p_value;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_CONFIG;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}


/*****************************************************************************+
* Function: trans_wireless_send2_rrh_q_gps()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_q_gps(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
   
    struct trans_monitor_build_msg_info st_build_info;
    
    u_int16_t  a_param_type[2] = {0};
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    (void) p_info;
    (void) len;
    (void) p_value;    
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Enter \r\n");
    
    uc_num = 2;
    
    a_param_type[0] = RRH_MSG_LONGITUDE;
    a_param_type[1] = RRH_MSG_LATITUDE;  

    st_build_rrh.p_tag = a_param_type;
    st_build_rrh.p_value = NULL;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_QUERY;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    /*2----a_param_type*/
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Exit \r\n");

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_q_power()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_q_power(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_monitor_build_msg_info st_build_info;

    u_int16_t  a_param_type[4] = {0};
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    (void) p_info;
    (void) len;
    (void) p_value;    
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Enter \r\n");
    
    uc_num = 4;
    
    a_param_type[0] = RRH_MSG_CHAN1_NORM_POW_VALUE;
    a_param_type[1] = RRH_MSG_CHAN2_NORM_POW_VALUE;
    a_param_type[2] = RRH_MSG_CHAN1_POWER_NORM_VALUE;
    a_param_type[3] = RRH_MSG_CHAN2_POWER_NORM_VALUE;
    
    st_build_rrh.p_tag = a_param_type;
    st_build_rrh.p_value = NULL;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_QUERY;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    
    /*2----a_param_type*/
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Exit \r\n");
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_q_rru_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_q_rru_id(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    //u_int8_t uc_result = 0;
    
    struct trans_monitor_build_msg_info st_build_info;

    u_int16_t  a_param_type[1] = {0};
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    (void) p_info;
    (void) len;
    (void) p_value;    

    uc_num = 1;

    a_param_type[0] = RRH_MSG_RRU_NO;

    st_build_rrh.p_tag = a_param_type;
    st_build_rrh.p_value = NULL;

    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }

    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                  uc_num,
                  p_rrh_payload, 
                  &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);

        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
  
    st_build_info.uc_type = TRANS_MONITOR_TYPE_QUERY;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 2;
    st_build_info.uw_transaction = 0;

    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);

    /*2----a_param_type*/
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
  
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);

    }
    #if 0
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);

        (*((int *)(p_value))) = uc_result;

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);

            //return TRANS_FAILD;
        }
    }
    #endif
    
    free(p_rrh_payload);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_rrh_q_gps_lock()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_rrh_q_gps_lock(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t uc_result = 0;
    
    struct trans_monitor_build_msg_info st_build_info;

    u_int16_t  a_param_type[1] = {0};
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    (void) p_info;
    (void) len;
    (void) p_value;    

    uc_num = 1;

    a_param_type[0] = RRH_MSG_GPS_CLK_LOCK_VALUE;

    st_build_rrh.p_tag = a_param_type;
    st_build_rrh.p_value = NULL;

    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }

    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                  uc_num,
                  p_rrh_payload, 
                  &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);

        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
  
    st_build_info.uc_type = TRANS_MONITOR_TYPE_QUERY;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 1;
    st_build_info.uw_transaction = 0;

    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);

    /*2----a_param_type*/
    st_build_info.uw_payload_len = uw_rrh_len;
    st_build_info.p_payload = p_rrh_payload;
  
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);

    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);

        (*((int *)(p_value))) = uc_result;

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);

            //return TRANS_FAILD;
        }
    }
    
    free(p_rrh_payload);

    return uw_ret;
}



#endif


/*****************************************************************************+
* Function: trans_wireless_action_bs_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2011-07-14
* 
+*****************************************************************************/
int trans_wireless_action_bs_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;
    //char tmp[128];
    u_int8_t  *p_send_msg = NULL;

    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;

    struct trans_agent_metric_info  *p_metric_info = NULL;
    struct trans_send_msg_to_agent st_agent_msg_info;

    (void) p_info;
        
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    if (0 == len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;
    }

    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_send_msg, 0, TRANS_WIRELESS_METRIC_MSG_MAX_LEN);

    /*If the Metric value is string,-----Must modify the code, it just can process float and int32*/
    p_metric_info = (struct trans_agent_metric_info *)(p_rev_buf);
   
    uc_num = p_metric_info->uc_metric_num;
    
    /*Get Metric*/
    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        /*4:  float or int32*/
        uw_ret = get_global_param ((char *)p_metric_name, (p_send_msg + uw_len));
       
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", p_metric_name);

            free(p_send_msg);
            return TRANS_FAILD;     
        }

        uw_len = (uc_index*4);
    }
        
    st_agent_msg_info.f_callback = NULL;
    st_agent_msg_info.p_reqs_msg = p_rev_buf;
    st_agent_msg_info.p_resp_msg = p_send_msg;
    st_agent_msg_info.p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_agent_msg_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;     
    }  

    free(p_send_msg);
    
    return uw_ret;
}

#endif

#ifdef TRANS_MS_COMPILE

/*****************************************************************************+
* Function: trans_wireless_action_ms_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
int trans_wireless_action_ms_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;
    //char tmp[128];
    u_int8_t  *p_send_msg = NULL;
    
    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;
    
    struct trans_agent_metric_info  *p_metric_info = NULL;
    struct trans_send_msg_to_agent st_agent_msg_info;
    
    //int w_value = 0;    
    
    (void) p_info;

    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    if (0 == len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;
    }
    
    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }

    memset((u_int8_t*)p_send_msg, 0, TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    
    /*If the Metric value is string,-----Must modify the code, it just can process float and int32*/
    p_metric_info = (struct trans_agent_metric_info *)(p_rev_buf);
    
    uc_num = p_metric_info->uc_metric_num;

    /*Get Metric*/
    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        /*4:  float or int32*/
        uw_ret = get_global_param ((char *)p_metric_name, (p_send_msg + uw_len));
       
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", p_metric_name);
    
            free(p_send_msg);
            return TRANS_FAILD;     
        }
    
        uw_len = (uc_index*4);
    }
   
    #if 0
    /*Get Metric*/
    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        /*4:  float or int32*/
        if (0 == strcmp ("RSSI", (char *)p_metric_name))
        {
//            w_value = st_result.rssi;
            get_global_param("RSSI", &w_value);

        }
        else if (0 == strcmp ("SINR", (char *)p_metric_name))
        {
//            w_value = st_result.sinr;
            get_global_param("SINR", &w_value);
        }
        else if (0 == strcmp ("TxPower", (char *)p_metric_name))
        {
//            w_value = st_result.tx_power; 
            get_global_param("TxPower", &w_value);
        }
        else if (0 == strcmp ("Temperature", (char *)p_metric_name))
        {
//            w_value = st_result.temperature;          
            get_global_param("Temperature", &w_value);
        }
        else
        {
            FLOG_ERROR("Metric name error! metric_name = %s .\r\n", p_metric_name);
            
            free(p_send_msg);
            return TRANS_FAILD;   
        }

        FLOG_DEBUG("%s: value %d\n", (char *)p_metric_name, w_value);

        /*4----sizeof(w_value)*/
        memcpy(p_send_msg + uw_len, &w_value, 4);
        
        uw_len = (uc_index*4);
    }
    #endif
   
    st_agent_msg_info.f_callback = NULL;
    st_agent_msg_info.p_reqs_msg = p_rev_buf;
    st_agent_msg_info.p_resp_msg = p_send_msg;
    st_agent_msg_info.p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_agent_msg_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;     
    }  
    
    free(p_send_msg);   

    
    return uw_ret;
}

#endif

#if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)

#ifdef TRANS_AGENT_COMPILE

/*****************************************************************************+
* Function: trans_wireless_send2_agent_block_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-17
* 
+*****************************************************************************/
int trans_wireless_send2_agent_block_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;   
    int32_t  w_result = 0;     
    struct trans_en_queue_msg   st_en_quene;
    
    //len = len;
        
    if (NULL == p_info) 
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    if (TRANS_TIMER_STATE_TIMEOUT == len)
    {
        FLOG_ERROR("Time out : callback function\r\n"); 
        
        uw_ret = trans_transaction_time_out(p_info); 
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
        
            //return;
        }
    
        w_result = TRANS_ACK_FLAG_RRH_TIMEOUT;
    }
    else
    {
        w_result = trans_transaction_get_result(p_info);
        if (0 > w_result)
        {
            FLOG_ERROR("Call trans_transaction_get_result error. w_result = %d. \r\n", w_result);
            FLOG_ERROR("Agent response message error! w_result = %d\r\n", w_result);
        
            w_result = TRANS_ACK_FLAG_OTHER_ERR;
        }
    }
    
    st_en_quene.uc_result = w_result;
    st_en_quene.uw_len = len;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_AGENT;
    
    uw_ret = trans_msg_en_quene(p_msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    return TRANS_SUCCESS;    

}

/*****************************************************************************+
* Function: trans_wireless_send2_agent_noblock_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-17
* 
+*****************************************************************************/
int trans_wireless_send2_agent_noblock_func(void *p_info, size_t len, void *p_msg)
{
    int32_t  w_result = 0;
    u_int32_t uw_ret = 0;  

    //len = len;
    (void)p_msg;
        
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    if (TRANS_TIMER_STATE_TIMEOUT == len)
    {
        FLOG_ERROR("Time out : callback function\r\n"); 
        
        uw_ret = trans_transaction_time_out(p_info); 
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
        
            //return;
        }   
    
        w_result = TRANS_ACK_FLAG_RRH_TIMEOUT;
    }
    else
    {
        w_result = trans_transaction_get_result(p_info);
        if (0 > w_result)
        {
            FLOG_ERROR("Call trans_transaction_get_result error. w_result = %d. \r\n", w_result);
            FLOG_ERROR("RRH response message error! w_result = %d\r\n", w_result);
        
            w_result = TRANS_ACK_FLAG_OTHER_ERR;
        }

    }

    
    /*Do not handle exceptions -----need do it in future*/ 
  
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_wireless_send2_agent_spectrum()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_agent_spectrum(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_result = 0;

    struct trans_send_msg_to_agent *p_msg_info = NULL;
    
    struct trans_monitor_build_msg_info st_build_info;
    
    (void) len;
    (void) p_info;   //NULL
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Enter \r\n");

    p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("malloc p_msg_info error! \r\n");
        return TRANS_FAILD;   
    }

    p_msg_info->f_callback = trans_wireless_send2_agent_block_func;
    p_msg_info->p_resp_msg = p_value;
    p_msg_info->p_reqs_msg = "spectrum";
    p_msg_info->p_info = NULL;
   
    st_build_info.uc_type = TRANS_MONITOR_TYPE_QUERY;
    st_build_info.uc_ack_flag = 0;
    st_build_info.us_operation = 0;
    st_build_info.uw_transaction = 0;

    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memset(st_build_info.a_dst_mac, 0, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = SIZEOF_TRANS_SEND_MSG_TO_AGENT;
    st_build_info.p_payload = p_msg_info;

    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    else
    {
        /*Wait for response message back*/
        uw_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
    
            uw_ret = TRANS_FAILD;
        }
    }
    
    free (p_msg_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_wireless, "Exit \r\n");

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_agent_periodic()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_agent_periodic(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    //u_int8_t uc_result = 0;

    struct trans_send_msg_to_agent *p_msg_info = NULL;
   
    (void) len;
    //(void) p_info;  
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
  
    p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("malloc p_msg_info error! \r\n");
        return TRANS_FAILD;   
    }

    p_msg_info->f_callback = NULL;
    p_msg_info->p_resp_msg = p_value;
    p_msg_info->p_reqs_msg = "periodic";
    p_msg_info->p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;
    }
    
    free (p_msg_info);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_wireless_send2_agent_topology()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_agent_topology(void *p_info, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = 0;
    //u_int8_t uc_result = 0;

    struct trans_send_msg_to_agent *p_msg_info = NULL;
    
    (void) len;
    //(void) p_info;  
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
  
    p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("malloc p_msg_info error! \r\n");
        return TRANS_FAILD;   
    }

    p_msg_info->f_callback = NULL;
    p_msg_info->p_resp_msg = p_value;
    p_msg_info->p_reqs_msg = "topologyUpdate";
    p_msg_info->p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;
    }
    
    free (p_msg_info);

    return uw_ret;
}

#endif

/*****************************************************************************+
* Function: trans_wireless_send2_monitor_hook_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_wireless_send2_monitor_hook_resp(u_int8_t uc_ack_flag,
                                            u_int32_t uw_transaction, 
                                            size_t len,
                                            void * p_value)
{
    u_int32_t uw_ret = TRANS_SUCCESS;
   
    struct trans_monitor_build_msg_info st_build_info;
   
    
    if (NULL == p_value)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    st_build_info.uc_type = TRANS_MONITOR_TYPE_HOOK_RESP;
    st_build_info.uc_ack_flag = uc_ack_flag;
    st_build_info.us_operation = 0;
    st_build_info.uw_transaction = uw_transaction;
    
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    memcpy(st_build_info.a_dst_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    
    st_build_info.uw_payload_len = len;
    st_build_info.p_payload = p_value;
    
    uw_ret = trans_wireless_enqueue_monitor_msg(&st_build_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_enqueue_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
    }

    return uw_ret;

}

/*****************************************************************************+
* Function: trans_wireless_tcp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0

*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_wireless_tcp_socket(u_int32_t uw_ip_addr, u_int16_t us_port)
{
    int32_t w_wireless_socket = -1;

    int32_t w_ret = 0;
    struct sockaddr_in st_peer_addr; 
    //struct sockaddr_in st_client_addr; 
    socklen_t sin_size = sizeof(struct sockaddr_in);    

    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;
    
    struct timeval st_time_val;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
   
    /* Create an IPv4 Internet Socket */
    w_ret = w_wireless_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    if (w_ret < 0)
    {    
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;    
    }
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_wireless_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot,
            sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);

        close (w_wireless_socket);
        return TRANS_FAILD;
     }
    
    
    /*Set the length of the REV buffer*/
    w_recvbuflen = TRANS_AGENT_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_wireless_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);

        close (w_wireless_socket);
        return TRANS_FAILD;
    }
    
    /*Set the length of the revice buffer*/
    w_sendbuflen = TRANS_AGENT_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_wireless_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);

        close (w_wireless_socket);
        return TRANS_FAILD;    
    }
    
    /* Zero out structure */
    memset(&st_peer_addr, 0, sizeof(st_peer_addr));    
    /* Create an AF_INET address */
    st_peer_addr.sin_family = AF_INET;         
    //st_peer_addr.sin_port = TRANS_HTONS(TRANS_MONITOR_LOCAL_IP_PORT);     
    st_peer_addr.sin_port = TRANS_HTONS(us_port); 

    st_peer_addr.sin_addr.s_addr = uw_ip_addr; 
    //st_peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    /*Connect  persistent */
    w_ret = connect(w_wireless_socket, (struct sockaddr * )&st_peer_addr, sin_size); 
    
    //if ((w_ret < 0) && (0 == g_trans_agent_config_info.uc_connect_num))
    if (w_ret < 0)
    {
        FLOG_ERROR("Call connect error! w_ret = %d, w_agent_socket = %d.\r\n", w_ret, w_wireless_socket);

        close(w_wireless_socket);
        return TRANS_FAILD;
    }
    
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 60000;  /*60ms*/
    
    w_ret = setsockopt (w_wireless_socket, SOL_SOCKET, SO_RCVTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_wireless_socket);
        return TRANS_FAILD;
    }
    
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 6000; /*6ms*/
    
    w_ret = setsockopt (w_wireless_socket, SOL_SOCKET, SO_SNDTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_wireless_socket);
        return TRANS_FAILD;
    }

    /*Send regisrtration message*/
    w_ret = trans_monitor_send_registration(w_wireless_socket);
    if(TRANS_SUCCESS != w_ret) 
    {
        FLOG_ERROR("Call trans_monitor_send_registration error!w_ret = %d. \r\n", w_ret);
        return w_ret;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exist agent_socket = %d\r\n", w_wireless_socket); 
    
    return w_ret;
}

/*****************************************************************************+
* Function: trans_wireless_register_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_wireless_register_func()
{    
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    #ifdef TRANS_RRH_COMPILE
    us_op = 1;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_WIRELESS_MSG_PRO,
                                    &us_op,  
                                    trans_wireless_send2_rrh_block_func,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback for OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 2;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_WIRELESS_MSG_PRO,
                                    &us_op,  
                                    trans_wireless_send2_rrh_noblock_func,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    #endif

    #ifdef TRANS_AGENT_COMPILE
    us_op = 3;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_WIRELESS_MSG_PRO,
                                    &us_op,  
                                    trans_wireless_send2_agent_block_func,
                                    NULL);
        
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 4;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_WIRELESS_MSG_PRO,
                                    &us_op,  
                                    trans_wireless_send2_agent_noblock_func,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    #endif

    (void)us_op;
    (void)uw_ret;

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_wireless_init()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
u_int32_t trans_wireless_init(void)
{
    #if (defined TRANS_BS_COMPILE)
    u_int32_t uw_ret = 0;
    
    uw_ret = trans_wireless_register_func();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_register_func error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    }

    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_wireless_release()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
u_int32_t trans_wireless_release(void)
{

 
    return TRANS_SUCCESS;
}


#endif

