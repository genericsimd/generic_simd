/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_monitor.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Sep.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_common.h>
#include <trans_list.h>
#include <trans_transaction.h>
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

#include <sys/time.h>


#ifdef TRANS_BS_COMPILE
#include "bs_cfg.h"
#include "monitor_proc.h"
#endif

#ifdef TRANS_MS_COMPILE
#include "macphy_init_process.h"
#include "ms_cfg.h"
#endif

/*****************************************************************************+
 *Global Variables
 +*****************************************************************************/

#ifdef TRANS_MONITOR_COMPILE
/*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
//u_int32_t g_trans_monitor_serial_number = 1;
//pthread_mutex_t  g_trans_monitor_serial_num_mutex;

struct trans_monitor_config_info g_trans_monitor_config_info;

//extern u_int32_t trans_send_monitor_msg(u_int8_t uc_send_type, void * p_send_info);

int32_t   g_trans_monitor_socket_now = 0;

struct trans_monitor_register  *g_trans_monitor_register;

#endif

/*****************************************************************************+
 *Code 
+*****************************************************************************/

#ifdef TRANS_MONITOR_COMPILE

/*****************************************************************************+
* Function: trans_monitor_cal_serial_num()
* Description: calculate and return the serial number
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int8_t  trans_monitor_get_module_by_device(u_int8_t uc_device_type) 
{
    u_int8_t uc_module_type = TRANS_MOUDLE_BUF;

    switch (uc_device_type)
    {
        case TRANS_MONITOR_DEVICE_TYPE_MONITOR: 
        
            uc_module_type = TRANS_MOUDLE_MONITOR;
            break; 
    
        case TRANS_MONITOR_DEVICE_TYPE_WMA: 
        
            uc_module_type = TRANS_MOUDLE_BS;
            break; 
            
        case TRANS_MONITOR_DEVICE_TYPE_WMB: 
        
            uc_module_type = TRANS_MOUDLE_MS;
            break; 

        #ifdef TRANS_UI_COMPILE
        case TRANS_MONITOR_DEVICE_TYPE_UI: 
        
            uc_module_type = TRANS_MOUDLE_UI;
            break; 
        #endif
            
        default:
            FLOG_ERROR("Input device type error!uc_device_type = %d. \r\n", uc_device_type);
            
            break;
    }


    return(uc_module_type);
}

/*****************************************************************************+
* Function: trans_monitor_request_timeout_func()
* Description: 
* Parameters:
*           p_info : It could be NULL
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_monitor_request_timeout_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
     u_int32_t uw_ret = 0;
     int32_t      w_result = 0;
    
    struct trans_send_msg_to_monitor  st_send_info;
    
     FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
     
     FLOG_INFO("Monitor request Timeout\r\n");
    
     len = len;
     (void) p_msg;
     
     if (NULL == p_info)
     {
         FLOG_ERROR("NULL PTR! \r\n");
         return TRANS_FAILD;
     }
     
     w_result = trans_transaction_get_result(p_info);
     if (0 > w_result)
     {
         FLOG_ERROR("Call trans_transaction_get_result error. w_result = %d. \r\n", w_result);
     }
    
     switch (w_result)
     {
         /*sucessful*/ 
         case TRANS_ACK_FLAG_OK:
          /*Part error*/
         case TRANS_ACK_FLAG_P_ERR:
          /*Type error*/ 
         case TRANS_ACK_FLAG_TYPE_ERR:
         /*length error*/    
         case TRANS_ACK_FLAG_LEN_ERR:
         /*RRH CRC error*/ 
         case TRANS_ACK_FLAG_RRH_CRC_ERR:
    
             st_send_info.uc_ack_flag = w_result;
             st_send_info.p_info = p_info;
             st_send_info.p_payload = NULL;
             st_send_info.uw_payload_len = len;
    
             break;
    
         /*RRH timeout error*/ 
         case TRANS_ACK_FLAG_RRH_TIMEOUT:
         /*Other error*/    
         case TRANS_ACK_FLAG_OTHER_ERR:
    
             st_send_info.uc_ack_flag = w_result;
             st_send_info.p_payload = NULL;
             st_send_info.p_info = p_info;
             st_send_info.uw_payload_len = 0;
    
             break;
     
         /*Order no response back*/
         case TRANS_ACK_FLAG_ORDER_NO_RESP:
         /*Order need response back*/
         case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
     
         /*Other*/
         default:
     
             FLOG_ERROR("Rev result type error! result = %d\r\n", w_result);
    
             st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
             st_send_info.p_payload = NULL;
             st_send_info.p_info = p_info;
             st_send_info.uw_payload_len = 0;
    
             uw_ret = TRANS_FAILD;
     
     }
    
     /*Send response message to monitor*/
     uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_send_info);
    
     if (TRANS_SUCCESS != uw_ret)
     {
         FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
     
         return TRANS_FAILD;
     }
     
     FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
     
     return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_monitor_registration_timeout_func()
* Description: 
* Parameters:
*           p_info : 
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_monitor_registration_timeout_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    int32_t w_socket = 0;   
    
    FLOG_ERROR("Register timeout ! \r\n");
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    (void)len;    
    (void)p_msg;

    w_socket = *((int32_t *)p_info); 

    close(w_socket);
    
    g_trans_monitor_register[w_socket].w_sockfd = -1;
    g_trans_monitor_register[w_socket].p_timer = NULL;
    
    /*Delete device info-----trans_common.c*/

    return TRANS_SUCCESS;
    
}


/*****************************************************************************+
* Function: trans_monitor_build_msg()
* Description: 
* Parameters:
*           p_build_info : struct trans_monitor_build_msg_info
*           p_send_msg : 
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_build_msg(struct trans_monitor_build_msg_info *p_build_info, 
                                    u_int8_t *p_send_msg,
                                    u_int32_t *p_send_len)
{
    struct trans_monitor_header *p_monitor_header = NULL;
    u_int8_t  *p_payload = NULL;
    
    if ((NULL == p_build_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    p_monitor_header = (struct trans_monitor_header *)p_send_msg;
        
    /*Type : enum trans_monitor_type_enum */
    p_monitor_header->uc_type = p_build_info->uc_type;
    
    /*1.Message length : 4 Bytes, Header Length + Payload Length.*/
    p_monitor_header->uw_msg_len = TRANS_HTONL(SIZEOF_TRANS_MONITOR_HEADER
                                    + p_build_info->uw_payload_len);       
    
    /*Payload Length*/
    p_monitor_header->uw_payload_len = TRANS_HTONL(p_build_info->uw_payload_len);  
    /*ACK Flag : enum trans_ack_flag_enum*/
    p_monitor_header->uc_ack_flag = p_build_info->uc_ack_flag;          
    /*Operation*/                 
    p_monitor_header->us_operation = TRANS_HTONS(p_build_info->us_operation);      
    /*Transaction ID*/
    p_monitor_header->uw_transaction = TRANS_HTONL(p_build_info->uw_transaction);     

    /*Source MAC*/
    memcpy(p_monitor_header->a_src_mac, p_build_info->a_src_mac, TRANS_MAC_ADDR_LEN);
     
    /*Dest MAC*/
    memcpy(p_monitor_header->a_dst_mac, p_build_info->a_dst_mac, TRANS_MAC_ADDR_LEN);

    /*Serial NO.*/
    if ((TRANS_MONITOR_TYPE_REGISTER == p_monitor_header->uc_type)
        || ((TRANS_MONITOR_TYPE_REGISTER_RESP == p_monitor_header->uc_type)))
    {
        p_monitor_header->uw_serial_no = TRANS_HTONL(0);       
    }
    else
    {
        #ifdef TRANS_MONITOR_TEST_COMPILE
        
        p_monitor_header->uw_serial_no = TRANS_HTONL(0);       
        
        #else
        p_monitor_header->uw_serial_no = TRANS_HTONL(trans_device_get_serial_num(p_monitor_header->a_dst_mac));  

        #endif
    }

    /*Extend*/
    //p_monitor_header->a_reserve[4] ;  
    memset(p_monitor_header->a_reserve, 0, 4);
    
    p_payload = p_send_msg + SIZEOF_TRANS_MONITOR_HEADER;

    if (0 < p_build_info->uw_payload_len)
    {
        memcpy(p_payload, p_build_info->p_payload, p_build_info->uw_payload_len);
    }

    *p_send_len = SIZEOF_TRANS_MONITOR_HEADER + p_build_info->uw_payload_len;

    trans_debug_msg_print(p_send_msg, 40, g_trans_debug_monitor);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_monitor_send()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-05-04
* 
+*****************************************************************************/
u_int32_t trans_monitor_send(int32_t w_sockfd,
                                u_int8_t *p_mac,
                                void * p_send_buf,
                                u_int32_t uw_send_len)
{
    int32_t w_ret = 0;
    int32_t     w_send_len_temp = 0;
    int32_t w_total_send_len = 0;

    u_int32_t uw_congestion_num = 0;
    
    time_t   now;
    struct tm   *timenow;
    
    /*Send New Message to Monitor*/
    while (w_total_send_len < ((int)uw_send_len))
    {
        if (uw_send_len - w_total_send_len > 2*1024)
        {
            w_send_len_temp = 2*1024 ;
        }
        else
        {
            w_send_len_temp = uw_send_len - w_total_send_len;
        }

        uw_congestion_num = trans_device_get_congestion_num(p_mac, 0); 
        
        /*Not Congestion*/
        if (0 == uw_congestion_num)
        {
            /*Send New Message to Monitor*/
            w_ret = send(w_sockfd, (p_send_buf + w_total_send_len), w_send_len_temp, 0);
            
            if(w_ret <= 0)
            {
                //close(w_sockfd);
                if (EAGAIN == errno)
                {
                    FLOG_ERROR("Congestion start! \r\n");
                
                    uw_congestion_num = trans_device_get_congestion_num(p_mac, 1); 
                }  

                #if 0
                time(&now);
                timenow = localtime(&now);
                
                FLOG_ERROR("Send socket:%d recv() error! return:%d, errno:%d, errortext:'%s', time:%s", 
                    w_sockfd, w_ret, errno, strerror(errno), asctime(timenow));
                #endif

                TRANS_COMMON_SOCKFD_PRINT(w_sockfd, w_ret, errno);

                return TRANS_FAILD;
            }  
    
            if (w_ret != w_send_len_temp)
            {
                time(&now);
                timenow = localtime(&now);
                
                FLOG_ERROR("send length error! %d, %d. Time : %s.\r\n", w_ret, w_send_len_temp, asctime(timenow));
                
                w_send_len_temp = w_ret;
            }
    
            w_total_send_len = w_total_send_len + w_send_len_temp;
            
        }
        /*Congestion-----discard or disconnect?*/
        else
        {
            FLOG_ERROR("Congestion : discard the message! \r\n");
            
            return TRANS_FAILD;
        }

    }
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_send_msg()
* Description: 
* Parameters:
*           p_build_info : struct trans_monitor_build_msg_info
*           p_send_msg : 
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_msg(struct trans_monitor_build_msg_info *p_build_info,
                                                            int32_t  w_sockfd)
{
    int32_t w_ret = 0;
    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_send_len = 0;
    int32_t     w_monitor_sockfd = 0;
    
    if (NULL == p_build_info) 
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    w_monitor_sockfd = w_sockfd;
    
    /* Allocate a memory.  */
    p_send_buf = (u_int8_t *)malloc(SIZEOF_TRANS_MONITOR_HEADER + p_build_info->uw_payload_len);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        return TRANS_FAILD;   
    }

    w_ret = trans_monitor_build_msg(p_build_info, p_send_buf, &uw_send_len);
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR("Call trans_monitor_build_msg error! w_ret = %d\r\n", w_ret);
    
        free(p_send_buf);
        return TRANS_FAILD;
    }
      
    trans_debug_msg_print(p_send_buf, uw_send_len, g_trans_debug_monitor);
    
    #ifdef TRANS_MONITOR_TEST_COMPILE
    /*Send New Message to Monitor*/
    w_ret = send(w_monitor_sockfd, p_send_buf, uw_send_len, 0);
    
    if(w_ret <= 0)
    {
        free(p_send_buf);
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }     

    #else
    w_ret = trans_monitor_send(w_monitor_sockfd,
                                                        p_build_info->a_dst_mac,
                                                        p_send_buf,
                                                        uw_send_len);
    if(TRANS_SUCCESS != w_ret)
    {
        //close(sock);
        free(p_send_buf);
        FLOG_ERROR("Call trans_common_socket_send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    #endif

    free(p_send_buf);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_send_action_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_action_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    void * p_user_info = NULL;
    int32_t  w_len = 0;

    int32_t w_socket = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    /*Get src module type*/
    p_user_info = trans_transaction_get_user(p_msg_info->p_info, &w_len);
    if (NULL == p_user_info)
    {
        FLOG_ERROR("NULL PTR!p_user_info \r\n");
        return TRANS_FAILD;
    }
    
    p_monitor_info = (struct trans_monitor_info *)p_user_info;
    
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_OPRATION_RESP;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_monitor_info->us_operation;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
    /*Transaction ID*/
    st_build_info.uw_transaction = p_monitor_info->uw_transaction;
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, p_monitor_info->a_dst_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    memcpy(st_build_info.a_dst_mac, p_monitor_info->a_src_mac, TRANS_MAC_ADDR_LEN);

    w_socket = trans_device_get_socket(st_build_info.a_dst_mac);

    uw_ret = trans_monitor_send_msg(&st_build_info, w_socket);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_send_rrh_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_rrh_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    void * p_user_info = NULL;
    int32_t  w_len = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    int32_t w_socket = 0;  
    
    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    /*Get src module type*/
    p_user_info = trans_transaction_get_user(p_msg_info->p_info, &w_len);
    if (NULL == p_user_info)
    {
        FLOG_ERROR("NULL PTR!p_user_info \r\n");
        return TRANS_FAILD;
    }
    
    p_monitor_info = (struct trans_monitor_info *)p_user_info;
    
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = p_monitor_info->uc_type + 1;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_monitor_info->us_operation;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
    /*Transaction ID*/
    st_build_info.uw_transaction = p_monitor_info->uw_transaction;
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, p_monitor_info->a_dst_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    memcpy(st_build_info.a_dst_mac, p_monitor_info->a_src_mac, TRANS_MAC_ADDR_LEN);
    
    w_socket = trans_device_get_socket(st_build_info.a_dst_mac);

    uw_ret = trans_monitor_send_msg(&st_build_info, w_socket);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    /*Delete*/
    uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_send_agent_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_agent_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;
    
    void * p_user_info = NULL;
    int32_t  w_len = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    int32_t w_socket = 0;  

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    /*Get src module type*/
    p_user_info = trans_transaction_get_user(p_msg_info->p_info, &w_len);
    if (NULL == p_user_info)
    {
        FLOG_ERROR("NULL PTR!p_user_info \r\n");
        return TRANS_FAILD;
    }
    
    p_monitor_info = (struct trans_monitor_info *)p_user_info;
    
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = p_monitor_info->uc_type + 1;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_monitor_info->us_operation;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
    /*Transaction ID*/
    st_build_info.uw_transaction = p_monitor_info->uw_transaction;
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, p_monitor_info->a_dst_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    memcpy(st_build_info.a_dst_mac, p_monitor_info->a_src_mac, TRANS_MAC_ADDR_LEN);
    
    w_socket = trans_device_get_socket(st_build_info.a_dst_mac);
    
    uw_ret = trans_monitor_send_msg(&st_build_info, w_socket);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    /*Delete*/
    uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_send_wireless_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_wireless_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;
    
    void * p_user_info = NULL;
    int32_t  w_len = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    int32_t w_socket = 0;  
    
    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    /*Get src module type*/
    p_user_info = trans_transaction_get_user(p_msg_info->p_info, &w_len);
    if (NULL == p_user_info)
    {
        FLOG_ERROR("NULL PTR!p_user_info \r\n");
        return TRANS_FAILD;
    }

    p_monitor_info = (struct trans_monitor_info *)p_user_info;
    
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = p_monitor_info->uc_type + 1;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_monitor_info->us_operation;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
    /*Transaction ID*/
    st_build_info.uw_transaction = p_monitor_info->uw_transaction;
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, p_monitor_info->a_dst_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    memcpy(st_build_info.a_dst_mac, p_monitor_info->a_src_mac, TRANS_MAC_ADDR_LEN);
    
    w_socket = trans_device_get_socket(st_build_info.a_dst_mac);

    uw_ret = trans_monitor_send_msg(&st_build_info, w_socket);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    /*Delete*/
    uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_send_monitor_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_monitor_send_monitor_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;
    
    void * p_user_info = NULL;
    int32_t  w_len = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    int32_t w_socket = 0;     
    
    struct trans_timer_info st_timer_info;    
    void* p_timer = NULL;

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    /*Get src module type*/
    p_user_info = trans_transaction_get_user(p_msg_info->p_info, &w_len);
    if (NULL == p_user_info)
    {
        FLOG_ERROR("NULL PTR!p_user_info \r\n");
        return TRANS_FAILD;
    }
    
    p_monitor_info = (struct trans_monitor_info *)p_user_info;

    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = p_monitor_info->uc_type;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_monitor_info->us_operation;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
    /*Transaction ID*/
    //st_build_info.uw_transaction = trans_device_get_transaction_id(p_monitor_info->a_dst_mac) ;
    st_build_info.uw_transaction = trans_transaction_get_dst_num(p_msg_info->p_info);
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    if (TRANS_MONITOR_TYPE_REGISTER != st_build_info.uc_type)
    {
        memcpy(st_build_info.a_dst_mac, p_monitor_info->a_dst_mac, TRANS_MAC_ADDR_LEN);
    }
    else
    {
        memset(st_build_info.a_dst_mac, 0, TRANS_MAC_ADDR_LEN);
    }
    
    w_socket = trans_device_get_socket(st_build_info.a_dst_mac); 
 
    uw_ret = trans_monitor_send_msg(&st_build_info, w_socket);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    if ((TRANS_MONITOR_TYPE_HOOK != st_build_info.uc_type) 
        || (TRANS_MONITOR_TYPE_OPRATION != st_build_info.uc_type))
    {
        /*ADD TIMER LIST*/    
        st_timer_info.f_callback = trans_monitor_request_timeout_func;
        st_timer_info.p_data = p_msg_info->p_info;
        st_timer_info.p_timer_list = &g_trans_timer_list;
        st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
        st_timer_info.uw_interval = TRANS_SEND_MONITOR_MSG_TIMEOUT;
        
        uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        uw_ret = trans_transaction_set_timer(p_msg_info->p_info, p_timer);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_timer error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

    }


    uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                        TRANS_TRANSACTION_FLAG_NO_DELETE,
                        TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rrh_query_resp_func()
* Description: 
* Parameters:
*           p_info : It could be NULL
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_monitor_rrh_query_resp_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;
    int32_t      w_result = 0;

   struct trans_send_msg_to_monitor  st_send_info;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    FLOG_INFO("Monitor Query RRH Timeout\r\n");

    len = len;
    (void) p_msg;
    
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
        }
    }

    switch (w_result)
    {
        /*sucessful*/ 
        case TRANS_ACK_FLAG_OK:
         /*Part error*/
        case TRANS_ACK_FLAG_P_ERR:
         /*Type error*/ 
        case TRANS_ACK_FLAG_TYPE_ERR:
        /*length error*/    
        case TRANS_ACK_FLAG_LEN_ERR:
        /*RRH CRC error*/ 
        case TRANS_ACK_FLAG_RRH_CRC_ERR:

            st_send_info.uc_ack_flag = w_result;
            st_send_info.p_payload = p_msg;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = len;

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            st_send_info.uc_ack_flag = w_result;
            st_send_info.p_payload = NULL;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = 0;

            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! result = %d\r\n", w_result);

            st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
            st_send_info.p_payload = NULL;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = 0;

            uw_ret = TRANS_FAILD;
    
    }

    /*Send response message to monitor*/
    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rrh_cfg_resp_func()
* Description: 
* Parameters:
*           p_info : It could be NULL
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_monitor_rrh_cfg_resp_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;

   struct trans_send_msg_to_monitor  st_send_info;
   
   int32_t      w_result = 0;
   
    FLOG_DEBUG("Enter \r\n");

    len = len;
    (void) p_msg;
    
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
        }
    }

    switch (w_result)
    {
        /*sucessful*/ 
        case TRANS_ACK_FLAG_OK:
         /*Part error*/
        case TRANS_ACK_FLAG_P_ERR:
         /*Type error*/ 
        case TRANS_ACK_FLAG_TYPE_ERR:
        /*length error*/    
        case TRANS_ACK_FLAG_LEN_ERR:
        /*RRH CRC error*/ 
        case TRANS_ACK_FLAG_RRH_CRC_ERR:
    
            st_send_info.uc_ack_flag = w_result;
            st_send_info.p_payload = p_msg;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = len;
    
            break;
    
        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:
    
            st_send_info.uc_ack_flag = w_result;
            st_send_info.p_payload = NULL;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = 0;
    
            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! result = %d\r\n", w_result);
    
            st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
            st_send_info.p_payload = NULL;
            st_send_info.p_info = p_info;
            st_send_info.uw_payload_len = 0;
    
            uw_ret = TRANS_FAILD;
    
    }

    /*Send response message to monitor*/
    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_func_register()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_register(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");    
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }   
     
    (void) len;
    (void) p_rev_buf;
    
    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_register_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_register_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }   
     
    (void) len;
    (void) p_rev_buf;

    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_query_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_query_rrh(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    //u_int32_t uw_payload_len = 0;
    //void * p_payload = NULL;
    u_int16_t * p_payload = NULL;
    
    //u_int16_t us_num = 0;
    //u_int16_t us_type = 0;

    u_int16_t us_operation = 0;
    struct trans_send_msg_to_rrh   *p_rrh = NULL;
        
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    (void)len;
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = (u_int16_t *)(p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER);
    
    #if 0
    while (us_num < ((uw_payload_len)/2))
    {
        us_type = TRANS_NTOHS(*(p_payload + us_num));
        
        memcpy((p_payload + us_num), &us_type, 2);
    
        us_num++;
    }
    #endif
    
    p_rrh = (struct trans_send_msg_to_rrh *)malloc(SIZEOF_TRANS_SEND_MSG_TO_RRH);
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR!p_rrh \r\n");
        return TRANS_FAILD;
    }
    
    /*message from wireless*/
    us_operation = TRANS_NTOHS(p_monitor_head->us_operation);

    if ((0 != us_operation)
        && (TRANS_REGISTER_FUN_BUF > us_operation))
    {
        p_rrh->f_callback = g_trans_register_exe_func[us_operation].f_callback;

        FLOG_DEBUG_TRANS(g_trans_debug_monitor, "id :%d \r\n", us_operation);
        FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Flag :%d \r\n", g_trans_register_exe_func[us_operation].uc_use_flag);
    }
    else
    {
        p_rrh->f_callback = trans_monitor_rrh_query_resp_func;
    }

    p_rrh->p_info = p_info;
    p_rrh->uw_payload_len = uw_payload_len;
    p_rrh->p_payload = p_payload;
    p_rrh->uc_type = RRH_MONITOR_TYPE_QUERY;
    
    //uw_ret = trans_rrh_send_monitor_query(&st_query_rrh, len, p_send_msg);
    /*Send message to rrh*/
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH, p_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
        
    }
    
    free(p_rrh);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_monitor_func_query_local()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-05-24
* 
+*****************************************************************************/
int trans_monitor_func_query_trans(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t * p_payload = NULL;
    //u_int8_t    uc_tag_len = 0;
    u_int32_t  uw_len_temp = 0;
    u_int32_t  uw_send_len = 0;

    u_int32_t uw_index = 0;
    u_int32_t uw_num_elems = 0;
    u_int32_t uw_num_leaf = 0;

    u_int8_t uc_ack_flag = TRANS_ACK_FLAG_OK;
    
    struct trans_send_msg_to_monitor st_msg_info;   

    struct trans_monitor_payload_trans *p_q_trans = NULL;
    struct trans_monitor_payload_trans_device *p_device_info = NULL;
    u_int8_t *p_ptrans = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uw_payload_len = len;
    p_payload = (u_int8_t *)p_rev_buf;

    p_q_trans = (struct trans_monitor_payload_trans *)(p_payload);

    if (TRANS_MONITOR_QUERY_DEVICE_ROOT == p_q_trans->uw_type)
    {
        /*4: device number*/
        uw_send_len = SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS
                                    + 4
                                    + SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS_DEVICE;
        
        p_ptrans = (u_int8_t *)malloc(uw_send_len);
        if (NULL == p_ptrans)
        {
            FLOG_ERROR("malloc *p_ptrans error! \r\n");
            uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
        } 
        else
        {
            memcpy(p_ptrans, p_q_trans, SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS);
            uw_len_temp = SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS;
            
            memset(p_ptrans + uw_len_temp, 1, 4);
            uw_len_temp = uw_len_temp + 4;
            
            p_device_info = (struct trans_monitor_payload_trans_device *)(p_ptrans + uw_len_temp);
            
            memcpy(p_device_info->a_mac, g_trans_local_device_info.a_mac, 6);
            
            p_device_info->uc_type = g_trans_local_device_info.uc_module_type;
        }        
        
    }
    else if (TRANS_MONITOR_QUERY_DEVICE_LEAF== p_q_trans->uw_type)
    {
        uw_num_elems = g_trans_common_sockfd.uw_num;
        
        FLOG_DEBUG_TRANS(g_trans_debug_monitor, "uw_num_elems:%d.\r\n", uw_num_elems); 

        /*4: device number*/
        uw_send_len = SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS
                                    + 4
                                    + (uw_num_elems * SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS_DEVICE);
        
        p_ptrans = (u_int8_t *)malloc(uw_send_len);
        if (NULL == p_ptrans)
        {
            FLOG_ERROR("malloc *p_ptrans error! \r\n");
            uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
        } 
        else
        {
            memcpy(p_ptrans, p_q_trans, SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS);
            uw_len_temp = SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS;
            
            uw_num_leaf = uw_num_elems - 1;
            memcpy(p_ptrans + uw_len_temp, &uw_num_leaf, 4);
            uw_len_temp = uw_len_temp + 4;
            
            for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
            {
                p_device_info = (struct trans_monitor_payload_trans_device *)(p_ptrans + uw_len_temp);
                
                if (trans_mac_addr_cmp(g_trans_common_sockfd.st_sockfd[uw_index].a_mac, 
                            trans_transaction_get_src_mac(p_info)))
                {
                    continue;
                }
                else
                {
                    memcpy(p_device_info->a_mac, g_trans_common_sockfd.st_sockfd[uw_index].a_mac, 6);
                    
                    p_device_info->uc_type = g_trans_common_sockfd.st_sockfd[uw_index].uc_module_type;
                    
                    uw_len_temp = uw_len_temp + SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS_DEVICE;
                    
                    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "uc_type:%d.\r\n", p_device_info->uc_type); 
                }
            }
        }        

    }
    else
    {
        FLOG_ERROR("Query device type error! \r\n");
        
        uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
    }

    st_msg_info.p_payload = p_ptrans;
    st_msg_info.uw_payload_len = uw_send_len;
    st_msg_info.uc_ack_flag = uc_ack_flag;
    st_msg_info.p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");  
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_query_wireless()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-05-24
* 
+*****************************************************************************/
int trans_monitor_func_query_wireless(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t * p_payload = NULL;
    u_int8_t * a_tag[64] = {0};
    //u_int8_t    uc_tag_len = 0;
    u_int32_t  uw_len_temp = 0;
    u_int32_t  uw_send_len = 0;
    
    struct trans_send_msg_to_monitor st_msg_info;   

    struct trans_monitor_payload_wireless *p_wireless = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uw_payload_len = len;
    p_payload = (u_int8_t *)p_rev_buf;

    while (uw_payload_len)
    {
        p_wireless = (struct trans_monitor_payload_wireless *)(p_payload + uw_len_temp);
    
        strcpy((char *)a_tag, (char *)p_wireless->a_name);
        
        /*Get value from wireless*/
        #ifdef TRANS_BS_COMPILE
        uw_ret = get_global_param ((char *)a_tag, (p_wireless->a_value));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
    
        #endif
        
        #ifdef TRANS_MS_COMPILE
        
        uw_ret = get_global_param ((char *)a_tag, (p_wireless->a_value));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
    
        #endif
    
        uw_len_temp = uw_len_temp + SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
    
        uw_payload_len = uw_payload_len - SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
        
    }
    //#endif
    
    uw_send_len = uw_len_temp;
    
    //st_msg_info.us_opration = 0;
    st_msg_info.p_payload = p_wireless;
    st_msg_info.uw_payload_len = uw_send_len;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    st_msg_info.p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");  
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_query_local()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_query_local(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t * p_payload = NULL;
    u_int8_t * a_tag[64] = {0};
    //u_int8_t    uc_tag_len = 0;
    //u_int32_t  uw_len_temp = 0;
    //u_int32_t  uw_send_len = 0;
    
    //struct trans_send_msg_to_monitor st_msg_info;   

    //struct trans_monitor_payload_wireless *p_wireless = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uw_payload_len = len;
    p_payload = (u_int8_t *)p_rev_buf;

    strcpy((char *)a_tag, (char *)p_payload);
    a_tag[63] = '\0';
    
    if (0 == strcmp ("trans", (char *)a_tag))
    {
        uw_ret = trans_monitor_func_query_trans(p_info, uw_payload_len, p_payload);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_monitor_func_query_trans error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }
    else
    {
        uw_ret = trans_monitor_func_query_wireless(p_info, uw_payload_len, p_payload);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_monitor_func_query_wireless error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }
    
    #if 0
    while (uw_payload_len)
    {
        p_wireless = (struct trans_monitor_payload_wireless *)(p_payload + uw_len_temp);

        strcpy((char *)a_tag, (char *)p_wireless->a_name);
        
        /*Get value from wireless*/
        #ifdef TRANS_BS_COMPILE
        uw_ret = get_global_param ((char *)a_tag, (p_wireless->a_value));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
    
        /*4---Length for Value */
        //uw_send_len = uw_send_len + 4;
    
        #endif
        
        #ifdef TRANS_MS_COMPILE
        
        uw_ret = get_global_param ((char *)a_tag, (p_wireless->a_value));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
        
        /*4---Length for Value */
        //uw_send_len = uw_send_len + 4;        

        #if 0
        struct ss_perf_metrics st_result;
        int w_value = 0;
    
        uw_ret = init_report_reg(&st_result);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call init_report_reg error! uw_ret = %d\r\n", uw_ret);
            
            free (p_send_msg);
            return TRANS_FAILD;
        }  
        
        if (0 == strcmp ("RSSI", (char *)a_tag))
        {
            w_value = st_result.rssi;            
        }
        else if (0 == strcmp ("SINR", (char *)a_tag))
        {
            w_value = st_result.sinr;          
        }
        else if (0 == strcmp ("TxPower", (char *)a_tag))
        {
            w_value = st_result.tx_power;          
        }
        else if (0 == strcmp ("Temperature", (char *)a_tag))
        {
            w_value = st_result.temperature;          
        }
        else
        {
            FLOG_ERROR("Metric name error! metric_name = %s .\r\n", a_tag);
            return TRANS_FAILD;  
        }
        
        /*4----sizeof(w_value)*/
        memcpy((p_send_msg + uw_send_len), &w_value, 4);
        /*4---Length for Value */
        uw_send_len = uw_send_len + 4;
        #endif
    
        #endif
    
        uw_len_temp = uw_len_temp + SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
    
        uw_payload_len = uw_payload_len - SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
        
    }
    //#endif

    uw_send_len = uw_len_temp;
    
    //st_msg_info.us_opration = 0;
    st_msg_info.p_payload = p_wireless;
    st_msg_info.uw_payload_len = uw_send_len;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    st_msg_info.p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    #endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");  
    
    return TRANS_FAILD;

}

/*****************************************************************************+
* Function: trans_monitor_func_config_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_config_rrh(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    //u_int32_t uw_payload_len = 0;
    u_int8_t * p_payload = NULL;
    
    //u_int16_t us_num = 0;
    //u_int16_t us_type = 0;

    u_int16_t us_operation = 0;
    //u_int16_t   us_len = 0;
    //u_int16_t  a_param_type[10] = {0};
    //int32_t      a_param_value[10] = {0};

    struct trans_send_msg_to_rrh   *p_rrh = NULL;
        
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    (void)len;

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    
    p_rrh = (struct trans_send_msg_to_rrh *)malloc(SIZEOF_TRANS_SEND_MSG_TO_RRH);
        
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR!p_rrh \r\n");
        return TRANS_FAILD;
    }

    /*message from wireless*/
    us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    
    if ((0 != us_operation)
        && (TRANS_REGISTER_FUN_BUF > us_operation))
    {
        p_rrh->f_callback = g_trans_register_exe_func[us_operation].f_callback;
    }
    else
    {
        p_rrh->f_callback = trans_monitor_rrh_cfg_resp_func;
    }
    
    p_rrh->p_info = p_info;
    p_rrh->uw_payload_len = uw_payload_len;
    p_rrh->p_payload = p_payload;
    p_rrh->uc_type = RRH_MONITOR_TYPE_CONFIG;

    /*Send message to rrh*/
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH, p_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
        
    }
    
    free(p_rrh);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    /*Delete ----TRANS_FAILD*/
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_monitor_func_config_local()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_config_local(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t * p_payload = NULL;
    u_int8_t * a_tag[21] = {0};
    //u_int8_t    uc_tag_len = 0;
    u_int32_t  uw_len_temp = 0;
    u_int32_t  uw_send_len = 0;
    //u_int16_t  uw_value_len = 0;

    struct trans_send_msg_to_monitor st_msg_info;   

    struct trans_monitor_payload_wireless *p_wireless = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uw_payload_len = len;
    p_payload = (u_int8_t *)p_rev_buf;
   
    #if 0
    while (uw_payload_len - uw_len_temp)
    {
        /*1---length of tag length*/
        uc_tag_len = *((u_int8_t * )p_payload + uw_len_temp);
        uw_len_temp = uw_len_temp + 1;        
    
        memcpy(a_tag, (p_payload + uw_len_temp), uc_tag_len);    
        uw_len_temp = uw_len_temp + uc_tag_len;
    
        /*2---Length for Value Length*/
        uw_value_len = TRANS_NTOHS(*((u_int16_t *)(p_payload + uw_len_temp)));
        uw_len_temp = uw_len_temp + 2;
    
        FLOG_INFO("len:%d, tag:%s, len2:%d, value:%d.\n", 
                uc_tag_len, a_tag, uw_value_len, *((int32_t *)(p_payload + uw_len_temp)));
    
        /*Get value from wireless*/
        #ifdef TRANS_BS_COMPILE
    
        uw_ret = set_global_param((char *)a_tag, (p_payload + uw_len_temp));
    
        if (0 != uw_ret)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
 
            return TRANS_FAILD;
        
        }    
        #endif
        
        #ifdef TRANS_MS_COMPILE
    
        /*Nothing*/
    
        #endif
    
        uw_len_temp = uw_len_temp + uw_value_len;  
       
    }

    #endif

    while (uw_payload_len)
    {
        p_wireless = (struct trans_monitor_payload_wireless *)(p_payload + uw_len_temp);
    
        strcpy((char *)a_tag, (char *)p_wireless->a_name);

        if (p_wireless->uw_type == 0)
        {
            FLOG_INFO("Get a change int param %s: %d\n", a_tag, *((int *)p_wireless->a_value));
        }else if (p_wireless->uw_type == 1)
        {
            FLOG_INFO("Get a change char param %s: %s\n", a_tag, (char *)p_wireless->a_value);
        }else if (p_wireless->uw_type == 2)
        {
            FLOG_INFO("Get a change float param %s: %f\n", a_tag, *((float *)p_wireless->a_value));
        }else
        {
            FLOG_WARNING("error type\n");
            return 1;
        }
        
        /*Get value from wireless*/
        #ifdef TRANS_BS_COMPILE
        uw_ret = update_global_param ((char *)a_tag, (p_wireless->a_value));

        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
    
        /*4---Length for Value */
        //uw_send_len = uw_send_len + 4;
    
        #endif
        
        #ifdef TRANS_MS_COMPILE
        
        uw_ret = update_global_param ((char *)a_tag, (p_wireless->a_value));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            return TRANS_FAILD;
        }
        
        /*4---Length for Value */
        //uw_send_len = uw_send_len + 4;        
   
        #endif
    
        uw_len_temp = uw_len_temp + SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
    
        uw_payload_len = uw_payload_len - SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS;
        
    }

    uw_send_len = uw_len_temp;

    st_msg_info.p_payload = p_wireless;
    st_msg_info.uw_payload_len = uw_send_len;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    st_msg_info.p_info = p_info;

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return TRANS_FAILD;
}

/*****************************************************************************+
* Function: trans_monitor_func_forward2_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_forward2_agent(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
        
    struct trans_send_msg_to_agent *p_msg_info = NULL;
    struct trans_send_msg_to_agent *p_payload = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_rev_msg = (u_int8_t *)p_rev_buf;
    uw_payload_len = len;

    p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("malloc p_msg_info error! \r\n");
        return TRANS_FAILD;   
    }

    p_payload = (struct trans_send_msg_to_agent *)p_rev_msg;
    
    p_msg_info->f_callback = p_payload->f_callback;
    p_msg_info->p_resp_msg = p_payload->p_resp_msg;
    p_msg_info->p_reqs_msg = p_payload->p_reqs_msg;
    p_msg_info->p_info = p_info;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg for AGENT %s error! . uw_ret = %d\r\n", 
                            p_payload->p_reqs_msg, uw_ret);
        //return TRANS_FAILD;   

    }

    free (p_msg_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");    

    return uw_ret;
}


/*****************************************************************************+
* Function: trans_monitor_func_request()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_request(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    struct trans_send_msg_to_monitor st_msg_info;   

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    
    /*Send message to dest*/
    st_msg_info.p_payload = p_payload;
    st_msg_info.uw_payload_len = uw_payload_len;
    st_msg_info.uc_ack_flag = p_monitor_head->uc_ack_flag;
    st_msg_info.p_info = p_info;
    
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_response()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_response(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    struct trans_send_msg_to_monitor st_msg_info; 
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len;  
     
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    
   
    /*Send message to dest*/
    st_msg_info.p_payload = p_payload;
    st_msg_info.uw_payload_len = uw_payload_len;
    st_msg_info.uc_ack_flag = p_monitor_head->uc_ack_flag;
    st_msg_info.p_info = p_info;

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }
   
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_hook_local()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-01-05
* 
+*****************************************************************************/
int trans_monitor_func_hook_local(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
   
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    fun_callback f_callback = NULL;
   
    //int32_t w_socket = 0;      
    
    u_int32_t  uw_transaction = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len; 
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
 
    //w_socket = g_trans_local_device_info.w_sockfd;
    //uw_transaction = trans_device_get_transaction_id(g_trans_local_device_info.a_mac) ;
    uw_transaction = trans_transaction_get_dst_num(p_info);
    /*Call hook set function*/
    f_callback = g_trans_register_exe_func[TRANS_REGISTER_FUN_MONITOR_OP + 1].f_callback;
    
    if (NULL == f_callback)
    {
        FLOG_ERROR("NULL PTR! f_callback. \r\n");
        return TRANS_SUCCESS;       
    }
    
    (*(f_callback))(p_info, uw_transaction, p_payload);

    /*check the uw_ret ----if failed send error message to monitor*/

    uw_ret = trans_transaction_set_comn(p_info, 
                        TRANS_TRANSACTION_FLAG_NO_DELETE,
                        TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_func_hook_response()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_monitor_func_hook_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    struct trans_send_msg_to_monitor st_msg_info; 

    u_int8_t uc_delete_flag = 0xff;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len;  
     
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    
   
    /*Send message to dest*/
    st_msg_info.p_payload = p_payload;
    st_msg_info.uw_payload_len = uw_payload_len;
    st_msg_info.uc_ack_flag = p_monitor_head->uc_ack_flag;
    st_msg_info.p_info = p_info;

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    /*HOOK is end*/
    if (0xc1 == p_monitor_head->uc_ack_flag)
    {
        uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;
    }
    else
    {
        uc_delete_flag = TRANS_TRANSACTION_FLAG_NO_DELETE;

    }

    /*Delete*/
    uw_ret = trans_transaction_set_comn(p_info, 
                                uc_delete_flag,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_register()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_register(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int8_t uc_device_type = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    

    u_int8_t uc_module_type = TRANS_MOUDLE_BUF;

    struct trans_device_info st_device_info;    
    void * p_device = NULL;

    struct trans_send_msg_to_monitor *p_send_info = NULL; 

    struct trans_monitor_info *p_monitor_info = NULL;
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    (void)len;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);
    
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    
    uc_device_type = *((u_int8_t *)p_payload);
   
    uc_module_type = trans_monitor_get_module_by_device(uc_device_type); 
    
    /*Check device type*/
    if (TRANS_MOUDLE_BUF == uc_module_type)
    {
        FLOG_ERROR("Call trans_monitor_get_module_by_device error! \r\n");
        return TRANS_FAILD;
    }
    
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    p_monitor_info = (struct trans_monitor_info *)malloc(SIZEOF_TRANS_MONITOR_HEADER);
    if (NULL == p_monitor_info)
    {
        FLOG_ERROR("Malloc p_monitor_info error.\r\n");
        return TRANS_FAILD;  
    }
    
    p_monitor_info->uc_type = p_monitor_head->uc_type;
    p_monitor_info->us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    p_monitor_info->uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);

    memcpy(p_monitor_info->a_src_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    memcpy(p_monitor_info->a_dst_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
        
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_MONITOR_INFO, p_monitor_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    st_device_info.uc_module_type = uc_module_type;
    st_device_info.w_sockfd = g_trans_monitor_socket_now;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    memcpy(st_device_info.a_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    /*Sned register response message */
    p_send_info = (struct trans_send_msg_to_monitor *)malloc(SIZEOF_TRANS_SEND_MSG_TO_MONITOR);
    if (NULL == p_send_info)
    {
        FLOG_ERROR("Malloc p_send_info error.\r\n");

        trans_device_delete(st_device_info.a_mac);
        
        return TRANS_FAILD;    
    }
   
    p_send_info->p_payload = &(g_trans_local_device_info.uc_device_type);
    p_send_info->uw_payload_len = sizeof(g_trans_local_device_info.uc_device_type);
    p_send_info->uc_ack_flag = TRANS_ACK_FLAG_OK;
    p_send_info->p_info = *p_info_tmp;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_MONITOR, p_send_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        
        trans_device_delete(st_device_info.a_mac);
        
        return TRANS_FAILD;
    }
    
    free (p_send_info);

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 1;     /*Funtion Callback ID-----0---invalide*/  
    
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                0,
                                NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
        
        trans_device_delete(st_device_info.a_mac);

        return TRANS_FAILD;
    }
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_monitor_parse_register_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_register_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int8_t uc_device_type = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    #ifndef TRANS_MONITOR_TEST_COMPILE
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/  
    #endif
    
    u_int8_t uc_module_type = TRANS_MOUDLE_BUF;

    struct trans_device_info st_device_info;    
    void * p_device = NULL;
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    (void)len;    
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    #if 0 
    /*Just for test ver*/
    uw_ret =  trans_timer_delete(&g_trans_timer_list,
                            g_trans_monitor_register[g_trans_monitor_socket_now].p_timer);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;
    }
    #endif
    
    /*If dest mac is not local*/
    if (!trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
    {

        FLOG_ERROR("Revice dest mac error.\r\n");        
        return TRANS_FAILD;    
    }

    uc_device_type = *((u_int8_t *)p_payload);
    /*Check device type*/
    uc_module_type = trans_monitor_get_module_by_device(uc_device_type); 
    
    /*Check device type*/
    if (TRANS_MOUDLE_BUF == uc_module_type)
    {
        FLOG_ERROR("Call trans_monitor_get_module_by_device error! \r\n");
        return TRANS_FAILD;
    }

    /*Add device list*/
    st_device_info.uc_module_type = uc_module_type;
    st_device_info.w_sockfd = g_trans_monitor_socket_now;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    memcpy(st_device_info.a_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", uw_ret);
        return TRANS_FAILD;
    }

    #ifndef TRANS_MONITOR_TEST_COMPILE
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 2;     /*Funtion Callback ID-----0---invalide*/  
        
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                0,
                                NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_query()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_query(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;
     
    int32_t  w_type = 0;

    struct trans_monitor_info *p_monitor_info = NULL;

    void ** p_info_tmp = NULL;
        
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*get the device type*/
    
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        /*RRH*/
        case TRANS_MOUDLE_RRH:
            
            uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
            uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 3;     /*Funtion Callback ID-----0---invalide*/  
            w_func_len = len;
            p_func_msg = p_rev_buf;

            break;

        /*Agent*/
        case TRANS_MOUDLE_AGENT:
            
            uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
            uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 9;     /*Funtion Callback ID-----0---invalide*/  
            w_func_len = uw_payload_len;
            p_func_msg = p_payload;

            break;


        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:

            /*Check the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 4;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = uw_payload_len;
                p_func_msg = p_payload;

            }
            else
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 7;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;
                
            }
            
            break;
 
        default:
    
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    } 
    
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    /*Fill in transaction*/
    p_info_tmp = p_info;
    
    uw_ret = trans_transaction_set_dst(*p_info_tmp, 
                                trans_device_get_transaction_id(p_monitor_head->a_dst_mac),
                                trans_device_get_socket(p_monitor_head->a_dst_mac),
                                p_monitor_head->a_dst_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                w_func_len,
                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    p_monitor_info = (struct trans_monitor_info *)malloc(SIZEOF_TRANS_MONITOR_INFO);
    if (NULL == p_monitor_info)
    {
        FLOG_ERROR("Malloc p_monitor_info error.\r\n");
        return TRANS_FAILD;  
    }
    
    p_monitor_info->uc_type = p_monitor_head->uc_type;
    p_monitor_info->us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    p_monitor_info->uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    memcpy(p_monitor_info->a_src_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    memcpy(p_monitor_info->a_dst_mac, p_monitor_head->a_dst_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_MONITOR_INFO, p_monitor_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_query_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_query_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;

    u_int32_t   uw_transaction = 0;    /*Transaction ID*/

    int32_t  w_type = 0;
    
    void ** p_info_tmp = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*Get transaction ID*/
    uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_out_by_mac(uw_transaction,
                                                     p_monitor_head->a_src_mac,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }
    
    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        /**/
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:
        //case Monitor:
            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 8;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;

            }
            else
            {
                /*Drop*/ 
            }
                            
            break;
 
        default:
    
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    } 
    
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                                uc_exe_flag,
                                                uw_func_id,
                                                w_func_len,
                                                p_func_msg);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_config()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_config(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int8_t uc_type = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/  
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;

    int32_t  w_type = 0;

    void ** p_info_tmp = NULL;
    
    struct trans_monitor_info *p_monitor_info = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }   
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Get Message Type*/
    uc_type = p_monitor_head->uc_type;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        /*RRH*/
        case TRANS_MOUDLE_RRH:
            uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
            uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 5;     /*Funtion Callback ID-----0---invalide*/  
            w_func_len = len;
            p_func_msg = p_rev_buf;

            break;

        case TRANS_MOUDLE_MS:
        case TRANS_MOUDLE_BS:

            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 6;     /*Funtion Callback ID-----0---invalide*/  
                //uw_func_id = 3+ TRANS_REGISTER_FUN_MONITOR_OP;
                w_func_len = uw_payload_len;
                p_func_msg = p_payload;

            }
            else
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 7;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;
            }
            
            break;
 
        default:
    
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    } 

    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }

    p_info_tmp = p_info;

    uw_ret = trans_transaction_set_dst(*p_info_tmp, 
                                trans_device_get_transaction_id(p_monitor_head->a_dst_mac),
                                trans_device_get_socket(p_monitor_head->a_dst_mac),
                                p_monitor_head->a_dst_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                w_func_len,
                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
   
    p_monitor_info = (struct trans_monitor_info *)malloc(SIZEOF_TRANS_MONITOR_HEADER);
    if (NULL == p_monitor_info)
    {
        FLOG_ERROR("Malloc p_monitor_info error.\r\n");
        return TRANS_FAILD;  
    }
    
    p_monitor_info->uc_type = p_monitor_head->uc_type;
    p_monitor_info->us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    p_monitor_info->uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    memcpy(p_monitor_info->a_src_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    memcpy(p_monitor_info->a_dst_mac, p_monitor_head->a_dst_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_MONITOR_INFO, p_monitor_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_config_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_config_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;
    
    u_int32_t   uw_transaction = 0;    /*Transaction ID*/

    int32_t  w_type = 0;
    
    void ** p_info_tmp = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*Get transaction ID*/
    uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_out_by_mac(uw_transaction,
                                                     p_monitor_head->a_src_mac,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }
    
    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        /**/
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:

            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 8;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;

            }
            else
            {
                /*Drop*/ 
            }
                          
            break;
 
        default:
    
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
   
    } 
        
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                                uc_exe_flag,
                                                uw_func_id,
                                                w_func_len,
                                                p_func_msg);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
       
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_hook()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_hook(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;

    int32_t  w_type = 0;

    struct trans_monitor_info *p_monitor_info = NULL;    
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:

            /*Check the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                //uw_func_id = TRANS_REGISTER_FUN_MONITOR_OP + 1;     /*Funtion Callback ID-----0---invalide*/  
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 10;
                w_func_len = len;
                p_func_msg = p_rev_buf;

            }
            else
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 7;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;
            }
            
            break;
 
        default:
            
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    } 

    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }

    uw_ret = trans_transaction_set_dst(*p_info_tmp, 
                                trans_device_get_transaction_id(p_monitor_head->a_dst_mac),
                                trans_device_get_socket(p_monitor_head->a_dst_mac),
                                p_monitor_head->a_dst_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                                uc_exe_flag,
                                                uw_func_id,
                                                w_func_len,
                                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
        
    p_monitor_info = (struct trans_monitor_info *)malloc(SIZEOF_TRANS_MONITOR_HEADER);
    if (NULL == p_monitor_info)
    {
        FLOG_ERROR("Malloc p_monitor_info error.\r\n");
        return TRANS_FAILD;  
    }
    
    p_monitor_info->uc_type = p_monitor_head->uc_type;
    p_monitor_info->us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    p_monitor_info->uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    memcpy(p_monitor_info->a_src_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    memcpy(p_monitor_info->a_dst_mac, p_monitor_head->a_dst_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_MONITOR_INFO, p_monitor_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
   
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_hook_resp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-20
* 
+*****************************************************************************/
u_int32_t trans_monitor_parse_hook_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;

    u_int32_t   uw_transaction = 0;    /*Transaction ID*/

    //int32_t w_socket = 0;   
    int32_t  w_type = 0;
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }   
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
   
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
   
    //w_socket = trans_device_get_socket(p_monitor_head->a_dst_mac);

    /*Get transaction ID*/
    uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_in(uw_transaction,
                                                     p_monitor_head->a_src_mac,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }

    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);
        
    switch (w_type)
    {
        /**/
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:
            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                //uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 8;     /*Funtion Callback ID-----0---invalide*/  
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 11;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = len;
                p_func_msg = p_rev_buf;               

                /*HOOK is end*/
                if (0xc1 == p_monitor_head->uc_ack_flag)
                {
                    /*Delete node from the list*/
                    FLOG_DEBUG_TRANS(g_trans_debug_monitor,"Close HOOK! \r\n");

                    uw_ret = trans_transaction_get_out(uw_transaction,
                                                                     p_monitor_head->a_src_mac,
                                                                     p_info);    
                    p_info_tmp = p_info;
                    
                    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
                    {
                        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
                        return TRANS_FAILD;   
                    }
                }
                
            }
            else
            {
                /*Drop*/ 
            }
                          
            break;
        
        default:
        
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    } 

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                w_func_len,
                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");    
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_operation()
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
u_int32_t trans_monitor_parse_operation(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;
    
    int32_t  w_type = 0;
    
    struct trans_monitor_info *p_monitor_info = NULL;
    
    u_int16_t   us_operation = 0;        /*Operation ID*/ 
    
    void ** p_info_tmp = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    /*Get operation*/
    us_operation = TRANS_NTOHS(p_monitor_head->us_operation);

    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);

    switch (w_type)
    {
        /**/
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:
            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_ACTION;    /**/
                uw_func_id = us_operation+ TRANS_REGISTER_FUN_MONITOR_OP;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = uw_payload_len;
                p_func_msg = p_payload;

            }
            else
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 7;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = uw_payload_len;
                p_func_msg = p_payload;
            }
            
            break;
 
        default:
    
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
   
    } 

    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    p_info_tmp = p_info;

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_dst(*p_info_tmp, 
                                trans_device_get_transaction_id(p_monitor_head->a_dst_mac),
                                trans_device_get_socket(p_monitor_head->a_dst_mac),
                                p_monitor_head->a_dst_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                w_func_len,
                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    p_monitor_info = (struct trans_monitor_info *)malloc(SIZEOF_TRANS_MONITOR_HEADER);
    if (NULL == p_monitor_info)
    {
        FLOG_ERROR("Malloc p_monitor_info error.\r\n");
        return TRANS_FAILD;  
    }
    
    p_monitor_info->uc_type = p_monitor_head->uc_type;
    p_monitor_info->us_operation = TRANS_NTOHS(p_monitor_head->us_operation);
    p_monitor_info->uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);
    
    memcpy(p_monitor_info->a_src_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
    memcpy(p_monitor_info->a_dst_mac, p_monitor_head->a_dst_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_MONITOR_INFO, p_monitor_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");    
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_parse_operation_resp()
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
u_int32_t trans_monitor_parse_operation_resp(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    
    u_int32_t uw_payload_len = 0;
    void * p_payload = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    int32_t       w_func_len = 0;
    void *         p_func_msg = NULL;

    u_int32_t   uw_transaction = 0;    /*Transaction ID*/

    int32_t  w_type = 0;

    void ** p_info_tmp = NULL;
    //p_info_tmp = p_info;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    (void)len; 
     
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    /*Length check ??*/
    uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    uw_transaction = TRANS_NTOHL(p_monitor_head->uw_transaction);

    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_in(uw_transaction,
                                                     p_monitor_head->a_src_mac,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }

    /*get the device type*/
    w_type = trans_device_get_type(p_monitor_head->a_dst_mac);
        
    switch (w_type)
    {
        /**/
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_MS:
            /*Check if the dest mac is local*/
            if (trans_mac_addr_cmp(g_trans_local_device_info.a_mac, p_monitor_head->a_dst_mac))
            {
                uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
                uw_func_id = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + 8;     /*Funtion Callback ID-----0---invalide*/  
                w_func_len = uw_payload_len;
                p_func_msg = p_payload;
                
                /*Operation is end*/
                if (0xc1 == p_monitor_head->uc_ack_flag)
                {
                    /*Delete node from the list*/
                    FLOG_DEBUG_TRANS(g_trans_debug_monitor,"Close operation! \r\n");
                    
                    /*Get transaction info and Check time out*/
                    uw_ret = trans_transaction_get_out(uw_transaction,
                                                                     p_monitor_head->a_src_mac,
                                                                     p_info);    
                    p_info_tmp = p_info;
                    
                    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
                    {
                        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
                        return TRANS_FAILD;   
                    }
                
                    /*Delete the p_info when the process is end*/
                    uw_ret = trans_transaction_set_comn(*p_info_tmp, 
                                                TRANS_TRANSACTION_FLAG_DELETE,
                                                TRANS_MOUDLE_BUF);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
                    
                        return TRANS_FAILD;
                    }
                }
            }
            else
            {
                /*Drop*/ 
            }
                          
            break;
        
        default:
        
            FLOG_ERROR("Rev message module error! module = %d\r\n", w_type);
            return TRANS_FAILD;
    
    }

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                w_func_len,
                                p_func_msg);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");  
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_msg_parse()
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
u_int32_t trans_monitor_parse_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int8_t uc_type = 0;
    u_int32_t uw_ret = 0;
    
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;

    //u_int32_t uw_payload_len = 0;
    //void * p_payload = NULL;
        
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    trans_debug_msg_print(p_rev_buf, 40, g_trans_debug_monitor);
    
    //(void) len;
    p_rev_msg = (u_int8_t *)p_rev_buf;    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;

    /*Get Message Type*/
    uc_type = p_monitor_head->uc_type;
    
    /*Length check ??*/
    //uw_payload_len = TRANS_NTOHL(p_monitor_head->uw_payload_len);  /*Payload Length*/
    //p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    
    switch (uc_type)
    {
        /*0x01 : Register*/
        case TRANS_MONITOR_TYPE_REGISTER:
    
            /*Register£ºSend this message for registration when TCP connection establish successfully */
            uw_ret = trans_monitor_parse_register(p_info, len, p_rev_buf);

            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_register_parse error! uw_ret = %d\r\n", uw_ret);
            }
                            
            break;
    
        /*0x02 : Register Response*/
        case TRANS_MONITOR_TYPE_REGISTER_RESP:
         
            /*Register Response£ºRegistration successful*/
            uw_ret = trans_monitor_parse_register_resp(p_info, len, p_rev_buf);
    
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_register_resp error! uw_ret = %d\r\n", uw_ret);
            }
            break;

        /*0x03 : Query*/
        case TRANS_MONITOR_TYPE_QUERY:
         
            /*Query£ºquery RRH , WMA, WMB*/    
            uw_ret = trans_monitor_parse_query(p_info, len, p_rev_buf);
        
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_query error! uw_ret = %d\r\n", uw_ret);
            }
            break;

        /*0x04 : Query Response */
        case TRANS_MONITOR_TYPE_QUERY_RESP:
            
            /*Query Response£ºquery RRH , WMA, WMB*/    
            uw_ret = trans_monitor_parse_query_resp(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_query_resp error! uw_ret = %d\r\n", uw_ret);
            }

            break;
            
        /*0x05 : Config*/
        case TRANS_MONITOR_TYPE_CONFIG:
         
            /*Query£ºquery RRH , WMA, WMB*/    
            uw_ret = trans_monitor_parse_config(p_info, len, p_rev_buf);
        
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_query error! uw_ret = %d\r\n", uw_ret);
            }
            break;
        
        /*0x06 : Config Response */
        case TRANS_MONITOR_TYPE_CONFIG_RESP:
            
            /*Query Response£ºquery RRH , WMA, WMB*/    
            uw_ret = trans_monitor_parse_config_resp(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_query_resp error! uw_ret = %d\r\n", uw_ret);
            }
        
            break;

        /*0x07 : HOOK*/
        case TRANS_MONITOR_TYPE_HOOK:
            
            /*HOOK£ºWMA, WMB*/    
            uw_ret = trans_monitor_parse_hook(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_hook error! uw_ret = %d\r\n", uw_ret);
            }

            break;

        /*0x08 : HOOK Response */
        case TRANS_MONITOR_TYPE_HOOK_RESP:
            
            /*HOOK Response£ºWMA, WMB*/    
            uw_ret = trans_monitor_parse_hook_resp(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_hook_resp error! uw_ret = %d\r\n", uw_ret);
            }

            break;

        /*0x09 : Operation*/
        case TRANS_MONITOR_TYPE_OPRATION:
            
            /*Operation£ºWMA, WMB*/    
            uw_ret = trans_monitor_parse_operation(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_operation error! uw_ret = %d\r\n", uw_ret);
            }

            break;

        /*0x10 : Operation Response*/
        case TRANS_MONITOR_TYPE_OPRATION_RESP:
            
            /*Operation Response£ºWMA, WMB*/    
            uw_ret = trans_monitor_parse_operation_resp(p_info, len, p_rev_buf);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_parse_operation_resp error! uw_ret = %d\r\n", uw_ret);
            }

            break;

        default:
    
            FLOG_ERROR("Rev message type error! uc_type = %d\r\n", uc_type);
            uw_ret = TRANS_FAILD;
            break;
    
    } 

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit uw_ret = %d,\r\n", uw_ret);
    
    return uw_ret;
}

/*********************************************************************+
* Function: trans_monitor_rev_msg()
* Description: Revice Message From Socket
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*********************************************************************/
u_int32_t trans_monitor_rev_msg(u_int8_t **pp_rev_msg, int32_t w_monitor_socket, int32_t *p_len)
{
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;
      
    //int32_t w_param_len = 0;
    int32_t w_msg_len = 0;
    u_int32_t uw_len = 0;
    int32_t w_msg_head_len = SIZEOF_TRANS_MONITOR_HEADER;
    
    int32_t w_rev_len = 0;    
    int w_len_tmp = 0;

    //u_int32_t uw_intretry = 0;
    u_int32_t uw_ret = 0;

    //time_t   now;
    //struct tm   *timenow;
    
    *pp_rev_msg = NULL;

    /*First rev the len : 4 Bytes*/
    w_len_tmp = 4;
    
    /*Rev JSON msg*/
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_monitor_socket, 
                                &uw_len + (4 - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            TRANS_COMMON_SOCKFD_ERROR(w_monitor_socket, errno, uw_ret);
            
            #if 0
            time(&now);
            timenow = localtime(&now);

            FLOG_ERROR("1 Recv socket:%d recv() error! return:%d, errno:%d, errortext:'%s', time:%s", 
                w_monitor_socket, w_rev_len1, errno, strerror(errno), asctime(timenow));
            #endif
            
            TRANS_COMMON_SOCKFD_PRINT(w_monitor_socket, w_rev_len1, errno);
            FLOG_ERROR("Call recv for length error!\r\n");
            return uw_ret;

            #if 0
            /*Disconnect*/
            if (0 == errno)
            {
                close(w_monitor_socket);
                return TRANS_FAILD;
            }
            else if (EINTR == errno)
            {
                #if 0
                if (10 >= uw_intretry) return TRANS_FAILD;
                uw_intretry ++;
                continue;
                #endif
                return 2;
            }
            else if (EAGAIN == errno)
            {

                return 2;
            }
            else
            {
                close(w_monitor_socket);
                return TRANS_FAILD;
            }
            #endif
        }
    
        w_len_tmp = w_len_tmp - w_rev_len1;    
        w_rev_len = w_rev_len + w_rev_len1;
    }
    
    if (4 != w_rev_len)
    {
        FLOG_ERROR("Receive Monitor Message Header Length error! len  = %d, rev_len  = %d\r\n", 4, w_rev_len);
        return TRANS_FAILD;
    }
    
    w_msg_len = TRANS_NTOHL(uw_len);
    
    if (w_msg_head_len > w_msg_len)
    {
        FLOG_ERROR("Receive Monitor Message Header Length error! header_len  = %d, rev_len  = %d\r\n", SIZEOF_TRANS_MONITOR_HEADER, w_msg_len);
        return TRANS_FAILD;
    }

    w_rev_len = 0;
    w_len_tmp = 0;
    
    *pp_rev_msg = (u_int8_t *)malloc (w_msg_len);
        
    if (NULL == *pp_rev_msg)
    {
        FLOG_ERROR("Malloc pp_rev_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "0 *pp_rev_msg  = %p! \r\n", *pp_rev_msg);    
    
    memset((u_int8_t*)*pp_rev_msg, 0, w_msg_len);
    memcpy((u_int8_t*)*pp_rev_msg, &uw_len, 4);
   
    /*Then rev a totle message except the len*/
    w_len_tmp = w_msg_len -4;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "w_len_tmp = %d! w_msg_len  = %d\r\n", w_len_tmp, w_msg_len);
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_monitor_socket, 
                                *pp_rev_msg + (w_msg_len - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            TRANS_COMMON_SOCKFD_ERROR(w_monitor_socket, errno, uw_ret);
            
            #if 0
            time(&now);
            timenow = localtime(&now);
            
            FLOG_ERROR("2 Recv socket :%d recv() error! return:%d, errno:%d, time:%s", 
                w_monitor_socket, w_rev_len2, errno, asctime(timenow));
            #endif
            
            TRANS_COMMON_SOCKFD_PRINT(w_monitor_socket, w_rev_len1, errno);

            FLOG_ERROR("Call recv for message error!\r\n");
            return uw_ret;
            
            #if 0
            /*Disconnect*/
            if (0 == errno)
            {
                close(w_monitor_socket);
                return TRANS_FAILD;
            }
            else if (EINTR == errno)
            {
                #if 0
                if (10 >= uw_intretry) return TRANS_FAILD;
                uw_intretry ++;
                continue;
                #endif
                return 2;

            }
            else if (EAGAIN == errno)
            {
            
                return 2;
            }
            else
            {
                close(w_monitor_socket);
                return TRANS_FAILD;
            }
            #endif

        }

        w_len_tmp = w_len_tmp - w_rev_len2;    
        w_rev_len = w_rev_len + w_rev_len2;
    }
    
    if (w_msg_len != w_rev_len + 4)
    {
        FLOG_ERROR("Receivev Monitor Message Length error! msg_len  = %d, rev_len  = %d\r\n", w_msg_len, w_rev_len);
        return TRANS_FAILD;
    }
    
    *p_len = w_msg_len;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Rev monitor Msg OK. rev_len = %d. \r\n", *p_len);
  
    trans_debug_msg_print(*pp_rev_msg, 40, g_trans_debug_monitor);

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_monitor_delete_operation()
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
u_int32_t trans_monitor_delete_operation()
{
    //u_int32_t uw_ret = 0;
    u_int16_t  us_operation = 0;

    for (us_operation = TRANS_REGISTER_FUN_MONITOR_OP; 
        us_operation < TRANS_REGISTER_FUN_BUF; 
        us_operation++)
    {
        if ((1 == g_trans_register_delete_func[us_operation].uc_use_flag)
                &&(NULL != g_trans_register_delete_func[us_operation].f_callback))
        {
            
            (* (g_trans_register_delete_func[us_operation].f_callback))(NULL, 0, NULL);
             
        }
    } 

    return TRANS_SUCCESS;   
}

/*****************************************************************************+
* Function: trans_monitor_rev_registration()
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
u_int32_t trans_monitor_rev_registration()
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    //u_int32_t uw_index = 0;
    
    int32_t w_max_sockfd = -1;
    
    fd_set readfds;    
    struct timeval st_time_val;
    
    u_int8_t  *p_rev_buf = NULL;
    int32_t  w_len = 0;    
    
    struct trans_monitor_header *p_monitor_head = NULL;
    u_int8_t uc_type = 0;
    
    int w_max_num = 1024;
    int w_socket = 0;

    struct trans_common_queue st_queue_info; 

    w_max_sockfd = -1;
    FD_ZERO(&readfds);   
    
    for (w_socket = 0; w_socket < w_max_num; w_socket ++)
    {
        if (1 < g_trans_monitor_register[w_socket].w_sockfd)
        {
            FD_SET(w_socket, &readfds);
            
            w_max_sockfd = TRANS_MAX(w_max_sockfd, w_socket);
            
            FLOG_DEBUG_TRANS(g_trans_debug_monitor, "sockfd:%d.\r\n", w_socket); 
        }
    }

    /*No socket*/
    if (1 >= w_max_sockfd)
    {
        return 2;    
    }
/*
    gettimeofday(&st_time_val, NULL);
    
    st_time_val.tv_sec += 1;
*/
    st_time_val.tv_sec = 1;
  
    w_ret = select(w_max_sockfd + 1, &readfds, NULL, NULL, &st_time_val);
    if(w_ret < 0)
    {
        /*???????If  error , what?????*/
        FLOG_ERROR("select error! w_ret = %d\r\n", w_ret);
        //close(w_socket);
        return TRANS_FAILD;
    }
    else if (0 == w_ret)
    {
        //FLOG_INFO("select timeout \r\n");
        return TRANS_SUCCESS;    
    }
    
    for (w_socket = 0; w_socket < w_max_num; w_socket ++)
    {
        if (1 >= g_trans_monitor_register[w_socket].w_sockfd)
        {
            continue;
        }
        
        if (FD_ISSET(w_socket, &readfds))
        {
            uw_ret = trans_monitor_rev_msg(&p_rev_buf, w_socket, &w_len);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                /*???????If  error , Do what?????*/                    
                FLOG_ERROR("Call trans_monitor_rev_msg for registration error ! uw_ret = %d\r\n", uw_ret);
                FLOG_WARNING("Lost the registration connection with peer. \r\n");
            
                /*Connection failed Process*/
                /*Delete timer*/
                if (NULL != g_trans_monitor_register[w_socket].p_timer)
                {
                    uw_ret =  trans_timer_delete(&g_trans_timer_list,
                                            g_trans_monitor_register[w_socket].p_timer);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
                        //return TRANS_FAILD;
                    }
                }

                g_trans_monitor_register[w_socket].w_sockfd = -1;
                g_trans_monitor_register[w_socket].p_timer = NULL;

                continue;

            } 

            if (NULL == p_rev_buf)
            {
                FLOG_ERROR("NULL PTR : p_rev_buf \r\n");
                
                g_trans_monitor_register[w_socket].w_sockfd = -1;
                g_trans_monitor_register[w_socket].p_timer = NULL;
                
                continue;
            }

            if (TRANS_MONITOR_REGISTER_MAX_LEN != w_len)
            {
                FLOG_ERROR("Receive message length %d error \r\n", w_len);
                
                g_trans_monitor_register[w_socket].w_sockfd = -1;
                g_trans_monitor_register[w_socket].p_timer = NULL;
                
                continue;
            }

            /*Enqueue---registration & response message process*/ 
            p_monitor_head = (struct trans_monitor_header *)p_rev_buf;
            uc_type = p_monitor_head->uc_type;
            
            if (TRANS_MONITOR_TYPE_REGISTER_RESP == uc_type)
            {
                /*Delete timer*/    
                FLOG_DEBUG_TRANS(g_trans_debug_monitor, "rev resp. \r\n");
                
                if (NULL != g_trans_monitor_register[w_socket].p_timer)
                {
                    uw_ret =  trans_timer_delete(&g_trans_timer_list,
                                            g_trans_monitor_register[w_socket].p_timer);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
                        //return TRANS_FAILD;
                    }
                }
                else
                {
                    FLOG_ERROR("NULL PTR : g_trans_monitor_register[w_socket].p_timer \r\n");
                }
                
                g_trans_monitor_register[w_socket].w_sockfd = -1;
                g_trans_monitor_register[w_socket].p_timer = NULL;
               
                /*Enqueue*/
                st_queue_info.p_msg = p_rev_buf;
                st_queue_info.uc_module_type = TRANS_MOUDLE_MONITOR;/*Temp*/
                st_queue_info.uw_len = w_len;
                st_queue_info.w_sockfd = w_socket;
                
                memcpy(st_queue_info.a_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
                
                #ifndef TRANS_MONITOR_TEST_COMPILE
                uw_ret = trans_common_msg_en_queue(&st_queue_info);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);
                    continue;
                } 
                #else 
                uw_ret = trans_monitor_parse_register_resp(NULL, 
                                           w_len,
                                           p_rev_buf);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_monitor_parse_register_resp error! uw_ret = %d\r\n", uw_ret);
                    return TRANS_FAILD;
                } 

                #endif
            }
            else if (TRANS_MONITOR_TYPE_REGISTER == uc_type)
            {
                g_trans_monitor_register[w_socket].w_sockfd = -1;
                g_trans_monitor_register[w_socket].p_timer = NULL;
                
                FLOG_DEBUG_TRANS(g_trans_debug_monitor, "rev req. \r\n");      
                
                /*Enqueue*/
                st_queue_info.p_msg = p_rev_buf;
                st_queue_info.uc_module_type = TRANS_MOUDLE_MONITOR; /*Temp*/
                st_queue_info.uw_len = w_len;
                st_queue_info.w_sockfd = w_socket;
                
                memcpy(st_queue_info.a_mac, p_monitor_head->a_src_mac, TRANS_MAC_ADDR_LEN);
                
                uw_ret = trans_common_msg_en_queue(&st_queue_info);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);
                    continue;
                } 
            }           
            /*The message is not the registration & response message---discard*/
            else
            {
                FLOG_WARNING(" Revice type error in registration ! uc_type = %d\r\n", uc_type);
                continue;
            }
        }
    }
    
    return TRANS_SUCCESS;    
}

/*****************************************************************************+
* Function: trans_monitor_send_registration()
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
u_int32_t trans_monitor_send_registration(int32_t w_sockfd)
{
    u_int32_t uw_ret = 0;
   
    struct trans_monitor_build_msg_info st_build_info;
    
    //struct trans_monitor_info *p_monitor_info = NULL;
    
    u_int8_t uc_device_type = g_trans_local_device_info.uc_device_type;

    //void * p_info = NULL;

    //struct timeval st_time_val;
    void *p_timer = NULL;

    u_int8_t  a_dst_mac[TRANS_MAC_ADDR_LEN];
    memset(a_dst_mac, 0x15, TRANS_MAC_ADDR_LEN);
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");

    /*Send message*/
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_REGISTER;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = 0;
    /*Operation*/
    st_build_info.us_operation = 0;
    /*Payload Length*/
    st_build_info.uw_payload_len = sizeof(uc_device_type);
    /*Payload*/
    st_build_info.p_payload = &uc_device_type;
    /*Transaction ID*/
    st_build_info.uw_transaction = 1;
    /*Source MAC*/
    memcpy(st_build_info.a_src_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    /*Dest MAC*/
    memcpy(st_build_info.a_dst_mac, a_dst_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_monitor_send_msg(&st_build_info, w_sockfd);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
        
        return TRANS_FAILD;
    }
    
    /*ADD TIMER LIST*/    
    struct trans_timer_info st_timer_info;

    st_timer_info.f_callback = trans_monitor_registration_timeout_func;
    st_timer_info.p_data = &(g_trans_monitor_register[w_sockfd].w_sockfd);
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
    st_timer_info.uw_interval = TRANS_SEND_MONITOR_MSG_TIMEOUT;

    uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    g_trans_monitor_register[w_sockfd].w_sockfd = w_sockfd;
    g_trans_monitor_register[w_sockfd].p_timer = p_timer;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return TRANS_SUCCESS;    
}

/*****************************************************************************+
* Function: trans_monitor_tcp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0

*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_monitor_tcp_socket()
{
    int32_t w_server_socket = 0, w_client_socket = 0, w_ret = 0;
    struct sockaddr_in st_server_addr; 
    struct sockaddr_in st_client_addr; 
    socklen_t sin_size = sizeof(st_client_addr);
    
    int32_t w_total = 0; 
    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;
    
    struct timeval st_time_val;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    /* Create an IPv4 Internet Socket */
    w_ret = w_server_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);
    
        close (w_server_socket);
        return TRANS_FAILD;
    
    }
    
    /*Set the length of the revice buffer*/
    w_recvbuflen = TRANS_MONITOR_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);
    
        close (w_server_socket);
        return TRANS_FAILD;
    
    }
    
    /*Set the length of the Send buffer*/
    w_sendbuflen = TRANS_MONITOR_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
    
        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);
    
        close (w_server_socket);
        return TRANS_FAILD;
    
    }
    
    /* Create an AF_INET address */
    st_server_addr.sin_family = AF_INET;      
    st_server_addr.sin_port = TRANS_HTONS(g_trans_monitor_config_info.us_monitor_tcp_port);     
    st_server_addr.sin_addr.s_addr = g_trans_monitor_config_info.uw_monitor_ip_addr; 
    
    bzero (& ( st_server_addr.sin_zero ), 8);
    
    /* Now bind the address to the socket */
    w_ret = bind (w_server_socket, (struct sockaddr *) &st_server_addr, sizeof(struct sockaddr));
    
    if (w_ret < 0)
    {
    
        FLOG_ERROR("Bind error! w_ret = %d\r\n", w_ret);
        close (w_server_socket);
        
        return TRANS_FAILD;
    }
    
    /*listen*/
    w_total = TRANS_MONITOR_CONNECT_MAX_NUM; 
    w_ret = listen(w_server_socket, w_total);
    if (w_ret < 0)
    {
        FLOG_ERROR("Listen error! w_ret = %d\r\n", w_ret);
    
        //close (w_server_socket);        
        return TRANS_FAILD;
    }

    while (1)
    {

        /*Accept the connection->Connect establish*/
        w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
                
        if (w_client_socket <= 0)
        {
            
            FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
            close (w_server_socket);
            continue;
            //return TRANS_FAILD;
        }  

        //#if 0
        st_time_val.tv_sec = 0;   
        st_time_val.tv_usec = 60000;  /*60ms*/
        
        w_ret = setsockopt (w_client_socket, SOL_SOCKET, SO_RCVTIMEO, &st_time_val, sizeof(st_time_val));
        
        if (w_ret < 0)
        {
            FLOG_ERROR("Call setsockopt SO_RCVTIMEO error! w_ret = %d\r\n", w_ret);
        
            close (w_client_socket);
            continue;    
        }

        st_time_val.tv_sec = 0;   
        st_time_val.tv_usec = 6000; /*6ms*/
        
        w_ret = setsockopt (w_client_socket, SOL_SOCKET, SO_SNDTIMEO, &st_time_val, sizeof(st_time_val));
        
        if (w_ret < 0)
        {
            FLOG_ERROR("Call setsockopt SO_SNDTIMEO error! w_ret = %d\r\n", w_ret);
        
            close (w_client_socket);
            continue;    
        }
            
        //#endif
        
        //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), TRANS_NTOHS(client_addr.sin_port));
        
        //g_trans_moudle_socket_fd[TRANS_MOUDLE_MONITOR] = w_client_socket;

        g_trans_monitor_register[w_client_socket].w_sockfd = w_client_socket;

    }
            
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit monitor_socket = %d\r\n", w_server_socket);
    
    return TRANS_SUCCESS;

}
/*****************************************************************************+
* Function: trans_monitor_tcp_connect()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0

*  
*  Data:    2011-09-15
* 
+*****************************************************************************/
void trans_monitor_tcp_connect()
{
    int32_t  w_ret = 0;
    //struct trans_monitor_socket  *p_monitor_socket;

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Enter \r\n");
    
    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! w_ret =%d\r\n", w_ret);    
        return ;
    }
    
    w_ret = trans_monitor_tcp_socket();
    if (0 != w_ret)
    {
        FLOG_ERROR("Call trans_monitor_tcp_socket error! w_ret =%d\r\n", w_ret);    
        return;
    }
    
    #if 0
    p_monitor_socket = &g_trans_monitor_socket;
    
    while (1)
    {
        pthread_mutex_lock (&(p_monitor_socket->m_mutex));

        /*TCP connect crash*/
        if (0 > p_monitor_socket->w_monitor_socket)
        {
            w_ret = trans_monitor_tcp_socket();
            if (0 != w_ret)
            {
                FLOG_ERROR("Call trans_monitor_tcp_socket error! w_ret =%d\r\n", w_ret);    
                continue;
            }

        }
        /*TCP connect has been connected*/
        else
        {
            /*Wait ??*/
        }

        pthread_mutex_unlock(&(p_monitor_socket->m_mutex));
        
    }
    #endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit \r\n");

    return;
}

#if 0
/*****************************************************************************+
* Function: trans_monitor_tcp_connect_failed()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0

*  
*  Data:    2011-10-09
* 
+*****************************************************************************/
void trans_monitor_tcp_connect_failed()
{
    u_int32_t uw_ret = 0;
    struct trans_action_info st_action_info;    
    struct trans_resp_msg_header st_resp_msg;
    
    /*Add an action*/    
    gettimeofday(&(st_action_info.st_tv), NULL);
    
    st_action_info.f_callback = trans_monitor_conn_failed_func;
    st_action_info.p_user_info = NULL;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_MONITOR;
   
    st_resp_msg.uc_result = TRANS_ACK_FLAG_CLEAN_OPERATION;
    st_resp_msg.uw_len = 0;
    st_resp_msg.p_buf = NULL;

    uw_ret = trans_action_add(&st_action_info, 
                SIZEOF_TRANS_RESP_MSG_HEADER, 
                &st_resp_msg);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
    
        return;    
    }   
    
    return;
}
#endif


/*****************************************************************************+
* Function: trans_monitor_register_func()
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
u_int32_t trans_monitor_register_func()
{    
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    us_op = 1;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_register,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback for OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 2;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_register_resp,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 3;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_query_rrh,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 4;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_query_local,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    us_op = 5;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_config_rrh,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 6;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_config_local,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 7;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_request,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 8;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                    &us_op,  
                                    trans_monitor_func_response,
                                    NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);

        return TRANS_FAILD;    
    }  

    us_op = 9;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                        &us_op,  
                                        trans_monitor_func_forward2_agent,
                                        NULL);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);

        return TRANS_FAILD;    
    }  
    
    us_op = 10;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                        &us_op,  
                                        trans_monitor_func_hook_local,
                                        NULL);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 11;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_MSG_PRO,
                                            &us_op,  
                                            trans_monitor_func_hook_resp,
                                            NULL);
  
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);

        return TRANS_FAILD;    
    }  

    return TRANS_SUCCESS;
}

#endif


/*****************************************************************************+
* Function: trans_monitor_init()
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
u_int32_t trans_monitor_init(struct trans_init_info *p_init_info)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_index = 0;
    /*Init the Global variables */
    
    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    #ifdef TRANS_MONITOR_COMPILE
    g_trans_monitor_socket_now = -1;
    
    memset((u_int8_t*)&g_trans_monitor_config_info, 0, sizeof(struct trans_monitor_config_info));

    /*Monitor  TCP Port*/
    g_trans_monitor_config_info.us_monitor_tcp_port = p_init_info->us_monitor_port; 

    /*Monitor IP */
    //g_trans_monitor_config_info.uw_monitor_ip_addr = inet_addr("10.13.1.43");
    g_trans_monitor_config_info.uw_monitor_ip_addr = TRANS_HTONL(INADDR_ANY);

    g_trans_monitor_register = (struct trans_monitor_register *)malloc
        (sizeof(struct trans_monitor_register)*TRANS_MONITOR_MAX_SOCKFD);
         
    if (NULL == g_trans_monitor_register)
    {
        FLOG_ERROR("malloc g_trans_monitor_register error! \r\n");
    
        return TRANS_FAILD;   
    }

    for (uw_index = 0; uw_index < TRANS_MONITOR_MAX_SOCKFD; uw_index ++)
    {
        g_trans_monitor_register[uw_index].p_timer = NULL;
        g_trans_monitor_register[uw_index].w_sockfd = -1;
    }
    
    uw_ret = trans_monitor_register_func();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_register_func error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    } 

    #endif
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_monitor_release()
* Description: init 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_monitor_release()
{
    #ifdef TRANS_MONITOR_COMPILE
    g_trans_monitor_socket_now = -1;
    
    memset((u_int8_t*)&g_trans_monitor_config_info, 0, sizeof(struct trans_monitor_config_info));
    
    /*Close socket*/

    free(g_trans_monitor_register);

    #endif
    
    return TRANS_SUCCESS;
}



