/*****************************************************************************+
*
*  File Name: trans_monitor.c
*
*  Function: Monitor Message Process
*
*  
*  Data:    2011-09-05
*  Modify:
*
+*****************************************************************************/


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_timer.h>
#include <trans_list.h>
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
#include "monitor_proc.h"
#endif

#ifdef TRANS_MS_COMPILE
#include "macphy_init_process.h"
#include "phylog_proc.h"
#endif

/*****************************************************************************+
 *Global Variables
 +*****************************************************************************/
struct trans_monitor_socket  g_trans_monitor_socket;

#ifdef TRANS_MONITOR_COMPILE
/*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
u_int32_t g_trans_monitor_serial_number = 1;
pthread_mutex_t  g_trans_monitor_serial_num_mutex;

struct trans_monitor_config_info g_trans_monitor_config_info;

//extern u_int32_t trans_send_monitor_msg(u_int8_t uc_send_type, void * p_send_info);

#endif

/*****************************************************************************+
 *Code 
+*****************************************************************************/

#ifdef TRANS_MONITOR_COMPILE


/******************FOR TEST*******************/
#ifdef TRANS_MONITOR_TEST_COMPILE

int trans_monitor_operation_127_2(void *p_user_info, 
                                    size_t len,
                                    void *p_msg)
{
    u_int32_t  uw_ret = 0;
    u_int8_t    uc_value = 0;

    struct trans_resp_msg_header *p_resp_header = NULL;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }    

    FLOG_INFO("Enter \r\n");

    trans_debug_msg_print(p_user_info, 10, g_trans_debug_monitor);
    trans_debug_msg_print(p_msg, len, g_trans_debug_monitor);

    if (NULL != p_user_info)
    {
        uc_value = (*(u_int8_t *)p_user_info);
        
        FLOG_INFO("%d \r\n", uc_value);

        /*Its memory malloc in trans_monitor_operation_127_1(),*/
        free(p_user_info);

    }
    else
    {
        
        FLOG_INFO("NULL PTR!p_user_info \r\n");
    }
    
    p_resp_header = (struct trans_resp_msg_header *)p_msg;

    if (TRANS_ACK_FLAG_OK != p_resp_header->uc_result)
    {
        FLOG_ERROR("RRH response message error! uc_result = %d\r\n", p_resp_header->uc_result);
    }

    struct trans_send_msg_to_monitor st_monitor;

    st_monitor.p_payload = p_resp_header->p_buf;
    st_monitor.uc_ack_flag = p_resp_header->uc_result;
    st_monitor.us_opration = 127;
    st_monitor.uw_payload_len = p_resp_header->uw_len;   

    uw_ret = trans_send_action_msg(TRANS_SEND_TO_MONITOR, &st_monitor);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_action_msg error! uw_ret = %d\r\n", uw_ret);
    
        //return TRANS_FAILD;
    }

    FLOG_INFO("Exit \r\n");

    return uw_ret;
}


int trans_monitor_operation_127_1(void *p_user_info, 
                                    size_t len,
                                    void *p_msg)
{
    u_int16_t  a_param_type[2] = {0};
    int32_t      a_param_value[2] = {0};
    u_int32_t  uw_ret = 0;
    u_int8_t    *p_send_msg = NULL;
    struct trans_resp_msg_header *p_resp_header = NULL;
    
    /*p_user_info is NULL*/
    p_user_info = p_user_info;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }    

    FLOG_INFO("Enter \r\n");

    trans_debug_msg_print(p_msg, len, g_trans_debug_monitor);

    p_resp_header = (struct trans_resp_msg_header *)p_msg;

    FLOG_INFO("%d \r\n", p_resp_header->uw_len);

    if (TRANS_ACK_FLAG_OK != p_resp_header->uc_result)
    {
        FLOG_ERROR("uc_result = %d error! \r\n", p_resp_header->uc_result);
        return TRANS_FAILD;   
    }
    
    if (0 >= p_resp_header->uw_len)
    {
        p_send_msg = NULL;
    }
    else
    {
        /*Can't free it now, It coule be free in function trans_monitor_operation_127_2() or after it*/
        p_send_msg = (u_int8_t *)malloc(p_resp_header->uw_len);
        if (NULL == p_send_msg)
        {
            FLOG_ERROR("malloc p_send_msg error! \r\n");
            return TRANS_FAILD;   
        }  
        
        /*memset*/
        memset((u_int8_t*)p_send_msg, 0, p_resp_header->uw_len);
        
        memcpy(p_send_msg, p_resp_header->p_buf, p_resp_header->uw_len);
    }
    
    a_param_type[0] = 0x0405;
    a_param_type[1] = 0x0406;
    
    a_param_value[0] = 229200;
    a_param_value[1] = 229200;

    struct trans_send_cfg_to_rrh st_cfg_rrh;
    
    st_cfg_rrh.f_callback = trans_monitor_operation_127_2;
    st_cfg_rrh.us_param_num = 2;
    /*10s*/
    st_cfg_rrh.uw_timeout = 10;
    st_cfg_rrh.p_param_type = a_param_type;
    st_cfg_rrh.p_param_value = a_param_value;

    st_cfg_rrh.p_info = p_send_msg;
   
    //uw_ret = trans_rrh_send_monitor_query(&st_query_rrh, len, p_send_msg);
    uw_ret = trans_send_action_msg(TRANS_SEND_TO_RRH_CFG, &st_cfg_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_action_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    FLOG_INFO("Exit \r\n");

    return TRANS_SUCCESS;

}


int trans_monitor_operation_216_1(void *p_user_info, 
                                    size_t len,
                                    void *p_msg)
{
    u_int32_t  uw_ret = 0;
    //u_int8_t    *p_send_msg = NULL;

    u_int32_t uw_payload_len = 0;
    struct trans_resp_msg_header *p_resp_header = NULL;
    u_int8_t * p_payload = NULL;

    struct trans_send_msg_to_monitor st_monitor;
    
    st_monitor.uc_ack_flag = TRANS_ACK_FLAG_OK;

    /*p_user_info is NULL*/
    p_user_info = p_user_info;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }    

    FLOG_INFO("Enter \r\n");

    trans_debug_msg_print(p_msg, len, g_trans_debug_monitor);

    p_resp_header = (struct trans_resp_msg_header *)p_msg;
    
    FLOG_INFO("%d \r\n", p_resp_header->uw_len);

    uw_payload_len = p_resp_header->uw_len;
    
    if (0 >= uw_payload_len)
    {
        //p_send_msg = NULL;
        FLOG_WARNING ("No payload \r\n");
        p_payload = NULL;
    }
    else
    {
      
        //u_int8_t * p_payload_temp = NULL;
        u_int8_t * a_tag[21] = {0};
        u_int8_t    uc_tag_len = 0;
        u_int32_t  uw_len_temp = 0;
        //u_int32_t  uw_send_len = 0;    
        u_int16_t  uw_value_len = 0;
        
        //p_payload = p_resp_header->p_buf;
        p_payload = (u_int8_t *)p_msg + SIZEOF_TRANS_RESP_MSG_HEADER;
        
        //p_send_temp = p_send_msg;
        
        while (uw_payload_len - uw_len_temp)
        {
            /*1---length of tag length*/
            uc_tag_len = *((u_int8_t * )p_payload + uw_len_temp);
            uw_len_temp = uw_len_temp + 1;        
        
            memcpy(a_tag, (p_payload + uw_len_temp), uc_tag_len);    
            uw_len_temp = uw_len_temp + uc_tag_len;
        
            /*2---Length for Value Length*/
            uw_value_len = ntohs(*((u_int16_t *)(p_payload + uw_len_temp)));
            uw_len_temp = uw_len_temp + 2;
        
            FLOG_INFO("len:%d, tag:%s, len2:%d, value:%d.\n", 
                    uc_tag_len, a_tag, uw_value_len, *((int32_t *)(p_payload + uw_len_temp)));
        
            /*Get value from wireless*/
            #ifdef TRANS_BS_COMPILE
        
            uw_ret = set_global_param((char *)a_tag, (p_payload + uw_len_temp));
        
            if (0 != uw_ret)
            {
                FLOG_ERROR ("get parameters %s error\n", a_tag);

                st_monitor.uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
        
                break;
            
            }    
            #endif
            
            #ifdef TRANS_MS_COMPILE
        
            /*Nothing*/
        
            #endif
        
            uw_len_temp = uw_len_temp + uw_value_len;  
           
        }

    }
    
    st_monitor.p_payload = p_payload;

    st_monitor.us_opration = 216;
    st_monitor.uw_payload_len = uw_payload_len;   
    
    uw_ret = trans_send_action_msg(TRANS_SEND_TO_MONITOR, &st_monitor);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_action_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }


    FLOG_INFO("Exit \r\n");

    return TRANS_SUCCESS;

}

#endif
/******************FOR TEST*******************/

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
u_int32_t  trans_monitor_cal_serial_num() 
{
    u_int32_t uw_serial_num = 0;
    
    pthread_mutex_lock(&(g_trans_monitor_serial_num_mutex));  
    
    uw_serial_num = g_trans_monitor_serial_number;

    g_trans_monitor_serial_number++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == g_trans_monitor_serial_number)
    {
        g_trans_monitor_serial_number = 1;

    }
    
    pthread_mutex_unlock(&(g_trans_monitor_serial_num_mutex));
    
    //FLOG_DEBUG("Exit us_serial_num = %d\r\n", us_serial_num); 
    
    return(uw_serial_num);
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
    
    FLOG_DEBUG("Enter \r\n");

    p_monitor_header = (struct trans_monitor_header *)p_send_msg;
    
    /*Type : enum trans_monitor_type_enum */
    p_monitor_header->uc_type = p_build_info->uc_type;
    
    /*1.Message length : 4 Bytes, Header Length + Payload Length.*/
    p_monitor_header->uw_msg_len = htonl(SIZEOF_TRANS_MONITOR_HEADER
                                    + p_build_info->uw_payload_len);       
    
    /*Serial NO.*/
    p_monitor_header->uw_serial_no = htonl(trans_monitor_cal_serial_num());       
    /*Payload Length*/
    p_monitor_header->uw_payload_len = htonl(p_build_info->uw_payload_len);  
    /*ACK Flag : enum trans_ack_flag_enum*/
    p_monitor_header->uc_ack_flag = p_build_info->uc_ack_flag;          
    /*Operation*/                 
    p_monitor_header->us_operation = htons(p_build_info->us_operation);      
    /*Transaction ID*/
    p_monitor_header->uw_transaction = htonl(0);
    /*Server ID*/
    p_monitor_header->uw_server_id = htonl((int)(g_trans_rrh_eqp_config.uc_server_id));
    /*Extend*/
    //p_monitor_header->a_reserve ;  
    memset(p_monitor_header->a_reserve, 0, 8);
    
    p_payload = p_send_msg + SIZEOF_TRANS_MONITOR_HEADER;

    if (0 < p_build_info->uw_payload_len)
    {
        memcpy(p_payload, p_build_info->p_payload, p_build_info->uw_payload_len);
    }

    *p_send_len = SIZEOF_TRANS_MONITOR_HEADER + p_build_info->uw_payload_len;

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
u_int32_t trans_monitor_send_msg(struct trans_monitor_build_msg_info *p_build_info)
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

    FLOG_DEBUG("Enter \r\n");
    
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
      
    trans_debug_msg_print(p_send_buf, 40, g_trans_debug_monitor);
    
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));

    w_monitor_sockfd = g_trans_monitor_socket.w_monitor_socket;
    
    if (0 < g_trans_monitor_socket.w_monitor_socket)
    {
        /*Send New Message to Monitor*/
        w_ret = send(w_monitor_sockfd, p_send_buf, uw_send_len, 0);
        
        if(w_ret <= 0)
        {
            free(p_send_buf);
            //close(sock);
            FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }   
    }
    else
    {
        FLOG_ERROR("Sockfd error! monitor_socket = %d\r\n", g_trans_monitor_socket.w_monitor_socket);
    }

    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));            
    
    free(p_send_buf);
    
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
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    len = len;
    p_send_msg = p_send_msg;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG("Enter \r\n");

    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_OPERATION;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_msg_info->us_opration;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
      

    uw_ret = trans_monitor_send_msg(&st_build_info);
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
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    len = len;
    p_send_msg = p_send_msg;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");
    
    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);

    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_RRH_Q_RESP;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_msg_info->us_opration;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
      

    uw_ret = trans_monitor_send_msg(&st_build_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
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
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    len = len;
    p_send_msg = p_send_msg;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");

    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = 0xff;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_msg_info->us_opration;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
      

    uw_ret = trans_monitor_send_msg(&st_build_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
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
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    len = len;
    p_send_msg = p_send_msg;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");

    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_TRACE;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_msg_info->us_opration;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
      

    uw_ret = trans_monitor_send_msg(&st_build_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
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
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_monitor_build_msg_info st_build_info;

    len = len;
    p_send_msg = p_send_msg;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");

    memset(&st_build_info, 0, SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO);
    
    /*Type : enum trans_monitor_type_enum */
    st_build_info.uc_type = TRANS_MONITOR_TYPE_WIRELESS_Q_RESP;
    /*ACK Flag : enum trans_ack_flag_enum*/
    st_build_info.uc_ack_flag = p_msg_info->uc_ack_flag;
    /*Operation*/
    st_build_info.us_operation = p_msg_info->us_opration;
    /*Payload Length*/
    st_build_info.uw_payload_len = p_msg_info->uw_payload_len;
    /*Payload*/
    st_build_info.p_payload = p_msg_info->p_payload;
      

    uw_ret = trans_monitor_send_msg(&st_build_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_send_msg error! uw_ret = %d\r\n", uw_ret);
    
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
                           void * p_send_msg)
{
    u_int32_t uw_ret = 0;
   struct trans_resp_msg_header   *p_resp_result = NULL;
   struct trans_send_msg_to_monitor  st_send_info;

    FLOG_DEBUG("Enter \r\n");
    
    p_info = p_info;
    len = len;
    
    /*p_info could be NULL*/
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");
    
    p_resp_result = (struct trans_resp_msg_header *)p_send_msg;

    switch (p_resp_result->uc_result)
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

            st_send_info.uc_ack_flag = p_resp_result->uc_result;
            st_send_info.p_payload = p_resp_result->p_buf;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = p_resp_result->uw_len;

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            st_send_info.uc_ack_flag = p_resp_result->uc_result;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = 0;

            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! uc_result = %d\r\n", p_resp_result->uc_result);

            st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
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
    
    FLOG_DEBUG("Exit \r\n");
    
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
int trans_monitor_rrh_cfg_resp_func(void *p_info,
                           size_t len,
                           void * p_send_msg)
{
    u_int32_t uw_ret = 0;
   struct trans_resp_msg_header   *p_resp_result = NULL;
   struct trans_send_msg_to_monitor  st_send_info;

    FLOG_DEBUG("Enter \r\n");

    p_info = p_info;
    len = len;

    /*p_info could be NULL*/
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    FLOG_DEBUG("Enter \r\n");

    p_resp_result = (struct trans_resp_msg_header *)p_send_msg;

    switch (p_resp_result->uc_result)
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

            st_send_info.uc_ack_flag = p_resp_result->uc_result;
            st_send_info.p_payload = p_resp_result->p_buf;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = p_resp_result->uw_len;

            break;

        /*RRH timeout error*/
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/
        case TRANS_ACK_FLAG_OTHER_ERR:

            st_send_info.uc_ack_flag = p_resp_result->uc_result;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = 0;

            break;

        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:

        /*Other*/
        default:

            FLOG_ERROR("Rev result type error! uc_result = %d\r\n", p_resp_result->uc_result);

            st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
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

    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rev_rrh_query()
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
u_int32_t trans_monitor_rev_rrh_query(struct trans_msg_info *p_msg_info, 
                           size_t len,
                           void * p_send_msg)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    //struct trans_msg_info *p_msg_info;
    struct trans_thread_info *p_thread_info = NULL;
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    //u_int8_t uc_rrh_param_len = 0;
        
    FLOG_DEBUG("Enter \r\n");

    len = len;
    
    if ((NULL == p_msg_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_thread_info = (struct trans_thread_info *)p_msg_info->p_thread_info;
    p_rev_msg = p_msg_info->p_rev_msg;
    
    if ((NULL == p_thread_info) ||(NULL == p_rev_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;

    /*Length check ??  256*/
    uw_payload_len = ntohl(p_monitor_head->uw_payload_len);

    struct trans_send_query_to_rrh st_query_rrh;
    st_query_rrh.f_callback = trans_monitor_rrh_query_resp_func;
    st_query_rrh.p_info = NULL;

    /*struct trans_monitor_payload_rrh_q*/
    st_query_rrh.p_param_type = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
    /*u_int16_t   us_tag;        Tag:    2Bytes    */
    st_query_rrh.us_param_num = ((uw_payload_len)/2);
    /*10s*/
    st_query_rrh.uw_timeout = 10;

    //uw_ret = trans_rrh_send_monitor_query(&st_query_rrh, len, p_send_msg);
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH_QUERY, &st_query_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_send_monitor_query error! uw_ret = %d\r\n", uw_ret);
        
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rev_rrh_query()
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
u_int32_t trans_monitor_rev_rrh_cfg(struct trans_msg_info *p_msg_info,
                           size_t len,
                           void * p_send_msg)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    //struct trans_msg_info *p_msg_info;
    struct trans_thread_info *p_thread_info = NULL;
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    //u_int8_t uc_rrh_param_len = 0;

    u_int16_t  a_param_type = {0};
    int32_t      a_param_value = {0};

    FLOG_DEBUG("Enter \r\n");

    len = len;

    if ((NULL == p_msg_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    p_thread_info = (struct trans_thread_info *)p_msg_info->p_thread_info;
    p_rev_msg = p_msg_info->p_rev_msg;

    if ((NULL == p_thread_info) ||(NULL == p_rev_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;

    /*Length check ??  256*/
    uw_payload_len = ntohl(p_monitor_head->uw_payload_len);

    struct trans_send_cfg_to_rrh st_cfg_rrh;

    st_cfg_rrh.f_callback = trans_monitor_rrh_cfg_resp_func;

    st_cfg_rrh.us_param_num = 1;
    /*10s*/
    st_cfg_rrh.uw_timeout = 10;

    p_rev_msg += sizeof(struct trans_monitor_header);

    a_param_type = ntohs(*(short *)p_rev_msg);
    a_param_value = ntohl(*((int *)(p_rev_msg + sizeof (short))));

printf("%x  %d\n", a_param_type, a_param_value);

    st_cfg_rrh.p_param_type = &a_param_type;
    st_cfg_rrh.p_param_value = &a_param_value;

    st_cfg_rrh.p_info = p_send_msg;

    //uw_ret = trans_rrh_send_monitor_query(&st_query_rrh, len, p_send_msg);
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH_CFG, &st_cfg_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_send_monitor_query error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rev_wireless_query()
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
u_int32_t trans_monitor_rev_wireless_query(struct trans_msg_info *p_msg_info, 
                           size_t len,
                           void * p_send_msg)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    //struct trans_msg_info *p_msg_info;
    struct trans_thread_info *p_thread_info = NULL;
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
   
    FLOG_DEBUG("Enter \r\n");
    
    len = len;
    
    if ((NULL == p_msg_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_thread_info = (struct trans_thread_info *)p_msg_info->p_thread_info;
    p_rev_msg = p_msg_info->p_rev_msg;
    
    if ((NULL == p_thread_info) ||(NULL == p_rev_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;

    uw_payload_len = ntohl(p_monitor_head->uw_payload_len);

    u_int8_t * p_payload = NULL;
    //u_int8_t * p_payload_temp = NULL;
    u_int8_t * a_tag[21] = {0};
    u_int8_t    uc_tag_len = 0;
    u_int32_t  uw_len_temp = 0;
    u_int32_t  uw_send_len = 0;


    p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;

    //p_send_temp = p_send_msg;

    while (uw_payload_len)
    {
        uc_tag_len = *((u_int8_t * )p_payload + uw_len_temp);
        
        /*1---length of tag length*/
        memcpy(a_tag, (p_payload + uw_len_temp + 1), uc_tag_len);

        memcpy((p_send_msg + uw_send_len), p_payload + uw_len_temp, uc_tag_len + 1);        
        uw_send_len = uw_send_len + uc_tag_len + 1;

        /*4---Length for Value */
        *((u_int16_t *)(p_send_msg + uw_send_len)) = htons(4);
        /*2---Length for Value Length*/
        uw_send_len = uw_send_len + 2 ;
        
        /*Get value from wireless*/
        #ifdef TRANS_BS_COMPILE
        uw_ret = get_global_param ((char *)a_tag, (p_send_msg + uw_send_len));
        
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", a_tag);
            
            return TRANS_FAILD;
        }

        /*4---Length for Value */
        uw_send_len = uw_send_len + 4;

        #endif
        
        #ifdef TRANS_MS_COMPILE
        struct ss_perf_metrics st_result;
        int w_value = 0;

        uw_ret = init_report_reg(&st_result);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call init_report_reg error! uw_ret = %d\r\n", uw_ret);
            
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

        uw_len_temp = uw_len_temp + 1 + uc_tag_len;

        uw_payload_len = uw_payload_len - uc_tag_len - 1;
        
    }
    
    struct trans_send_msg_to_monitor st_msg_info;    
    
    st_msg_info.us_opration = 0;
    st_msg_info.p_payload = p_send_msg;
    st_msg_info.uw_payload_len = uw_send_len;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;

    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_rev_msg_process()
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
u_int32_t trans_monitor_rev_operation(struct trans_msg_info *p_msg_info, 
                           size_t len,
                           void * p_send_msg)
{
    u_int32_t uw_payload_len = 0;
    u_int32_t uw_ret = 0;
    //struct trans_msg_info *p_msg_info;
    struct trans_thread_info *p_thread_info = NULL;
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
    u_int16_t  us_operation = 0;

    fun_callback f_callback = NULL;

    struct trans_action_info st_action_info;
    struct trans_resp_msg_header *p_resp_msg = NULL;

    FLOG_DEBUG("Enter \r\n");

    len = len;
    
    if ((NULL == p_msg_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_thread_info = (struct trans_thread_info *)p_msg_info->p_thread_info;
    p_rev_msg = p_msg_info->p_rev_msg;
    
    if ((NULL == p_thread_info) ||(NULL == p_rev_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;
    
    uw_payload_len = ntohl(p_monitor_head->uw_payload_len);
    
    us_operation = TRANS_REGISTER_FUN_MONITOR_OP 
        + ntohs(p_monitor_head->us_operation);

    if (TRANS_REGISTER_FUN_BUF <= us_operation)
    {
        FLOG_ERROR("Out of rang : us_operation = %d!\r\n", us_operation);  
        return TRANS_FAILD;
    }

    if ((1 == g_trans_register_exe_func[us_operation].uc_use_flag)
            &&(NULL != g_trans_register_exe_func[us_operation].f_callback))
    {
        #if 0
        (* (g_trans_register_func[us_operation].f_callback))
            (p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER, uw_payload_len, NULL);
        #endif
        f_callback = g_trans_register_exe_func[us_operation].f_callback;
    }
    else
    {
        FLOG_ERROR("Call f_callback error!Flag = %d. \r\n", 
            g_trans_register_exe_func[us_operation].uc_use_flag);

        return TRANS_FAILD;
    }
   
    gettimeofday(&(st_action_info.st_tv), NULL);
    
    st_action_info.f_callback = f_callback;
    st_action_info.p_user_info = NULL;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_MONITOR;

    p_resp_msg = (struct trans_resp_msg_header *)p_send_msg;

    p_resp_msg->uc_result = TRANS_ACK_FLAG_OK;
    p_resp_msg->uw_len = uw_payload_len;

    p_resp_msg->p_buf = p_send_msg + SIZEOF_TRANS_RESP_MSG_HEADER;
        
    if (0 == uw_payload_len)
    {
         /*Do nothing*/

    }
    else
    {
        memcpy(p_send_msg + SIZEOF_TRANS_RESP_MSG_HEADER, 
            p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER, 
            uw_payload_len);
    }
    
    uw_ret = trans_action_add(&st_action_info, 
                uw_payload_len + SIZEOF_TRANS_RESP_MSG_HEADER, 
                p_send_msg);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_monitor_rev_msg_process()
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
u_int32_t trans_monitor_rev_msg_process(struct trans_msg_info *p_msg_info, 
                           size_t len,
                           void * p_send_msg)
{
    u_int8_t uc_type = 0;
    u_int32_t uw_ret = 0;
    
    //struct trans_msg_info *p_msg_info;
    struct trans_thread_info *p_thread_info = NULL;
    u_int8_t *p_rev_msg = NULL;
    struct trans_monitor_header *p_monitor_head = NULL;
        
    FLOG_DEBUG("Enter \r\n");

    if ((NULL == p_msg_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_thread_info = (struct trans_thread_info *)p_msg_info->p_thread_info;
    p_rev_msg = p_msg_info->p_rev_msg;

    if ((NULL == p_thread_info) ||(NULL == p_rev_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_monitor_head = (struct trans_monitor_header *)p_rev_msg;

    /*Get Message Type*/
    uc_type = p_monitor_head->uc_type;

    /*Length check ??*/

    switch (uc_type)
    {
        /*0x01 : Query RRH*/
        case TRANS_MONITOR_TYPE_RRH_Q:
    
            /*Query RRH：Send message to rrh and replay to monitor */
            uw_ret = trans_monitor_rev_rrh_query(p_msg_info, len, p_send_msg);
    
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_rrh_query error! uw_ret = %d\r\n", uw_ret);

                /*Send error mesage to Monitor for query RRH failed*/
                struct trans_send_msg_to_monitor  st_send_info;

                st_send_info.p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
                st_send_info.uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
                st_send_info.us_opration = ntohs(p_monitor_head->us_operation);
                st_send_info.uw_payload_len = ntohl(p_monitor_head->uw_payload_len);

                uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, &st_send_info);
                
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
                
                    return TRANS_FAILD;
                }
            }
                            
            break;
 
        /*0x01 : Query RRH*/
        case TRANS_MONITOR_TYPE_RRH_C:

            /*Query RRH：Send message to rrh and replay to monitor */
            uw_ret = trans_monitor_rev_rrh_cfg(p_msg_info, len, p_send_msg);

            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_rrh_query error! uw_ret = %d\r\n", uw_ret);

                /*Send error mesage to Monitor for query RRH failed*/
                struct trans_send_msg_to_monitor  st_send_info;

                st_send_info.p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
                st_send_info.uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
                st_send_info.us_opration = ntohs(p_monitor_head->us_operation);
                st_send_info.uw_payload_len = ntohl(p_monitor_head->uw_payload_len);

                uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, &st_send_info);

                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);

                    return TRANS_FAILD;
                }
            }

            break;

         /*0x03 : Query Wireless*/
        case TRANS_MONITOR_TYPE_WIRELESS_Q:
         
            /*Query Wireless：Get info from Wireless and replay to monitor */
            uw_ret = trans_monitor_rev_wireless_query(p_msg_info, len, p_send_msg);
    
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_wireless_query error! uw_ret = %d\r\n", uw_ret);

                struct trans_send_msg_to_monitor st_msg_info;    
                
                st_msg_info.us_opration = ntohs(p_monitor_head->us_operation);
                st_msg_info.p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
                st_msg_info.uw_payload_len = ntohl(p_monitor_head->uw_payload_len);
                st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
                
                uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
                
                    return TRANS_FAILD;
                }
            }
            break;

        /*0x10 : Operation*/
        case TRANS_MONITOR_TYPE_OPERATION:
         
            /*Operation：Insert to Action Quene*/    
            uw_ret = trans_monitor_rev_operation(p_msg_info, len, p_send_msg);
        
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_operation error! uw_ret = %d\r\n", uw_ret);

                struct trans_send_msg_to_monitor st_monitor;
                
                st_monitor.p_payload = p_rev_msg + SIZEOF_TRANS_MONITOR_HEADER;
                st_monitor.uc_ack_flag = TRANS_ACK_FLAG_P_ERR;
                st_monitor.us_opration = ntohs(p_monitor_head->us_operation);
                st_monitor.uw_payload_len = ntohl(p_monitor_head->uw_payload_len);   
                
                uw_ret = trans_send_action_msg(TRANS_SEND_TO_MONITOR, &st_monitor);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_action_msg error! uw_ret = %d\r\n", uw_ret);
                
                    return TRANS_FAILD;
                }
            }
            break;

        default:
    
            FLOG_ERROR("Rev message type error! uc_type = %d\r\n", uc_type);
            uw_ret = TRANS_FAILD;
    
    } 

    FLOG_DEBUG("Exit uw_ret = %d,\r\n", uw_ret);
    
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
u_int32_t trans_monitor_rev_msg(u_int8_t *p_rev_msg, int32_t w_monitor_socket)
{
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;
    int32_t w_rev_len = 0;    
    int w_len_tmp = 0;    
    //int32_t w_param_len = 0;
    int32_t w_msg_len = 0;
        
    /*First rev the len : 4 Bytes*/
    w_len_tmp = 4;
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_monitor_socket, 
                                p_rev_msg + (4 - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
            return TRANS_FAILD;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len1;    
        w_rev_len = w_rev_len + w_rev_len1;
    }
    
    if (4 != w_rev_len)
    {
        FLOG_ERROR("Receive monitor Message Header Length error! header_len  = %d, rev_len  = %d\r\n", 4, w_rev_len);
        return TRANS_FAILD;
    }
    
    #if 0
    w_rev_len1 = recv(w_monitor_socket, 
                            p_rev_msg, 
                            4, 
                            0);
    
    if (w_rev_len1 <= 0)
    {
        FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
        return TRANS_FAILD;
    }
    #endif
    
    /*Get the len*/
    w_msg_len = ntohl(*((u_int32_t*)(p_rev_msg)));
    //FLOG_DEBUG("Rev msg len =%d . \r\n", w_param_len);
    
    /*First rev the len : 4 Bytes*/
    if ((w_msg_len) > TRANS_REV_MSG_MAX_LEN - 4)
    {
        FLOG_ERROR("recv monitor msg length error! msg_len = %d, w_rev_len1 = %d\r\n", w_msg_len, w_rev_len1);
        return TRANS_FAILD;
    }
    
    w_rev_len = 0;
    w_len_tmp = 0;
    
    /*Then rev a totle message except the len*/
    w_len_tmp = w_msg_len -4;
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_monitor_socket, 
                                p_rev_msg + (w_msg_len - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            FLOG_ERROR("Receivev Monitor Message error! w_rev_len2 = %d\r\n", w_rev_len2);
            return TRANS_FAILD;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len2;    
        w_rev_len = w_rev_len + w_rev_len2;
    }
    
    if (w_msg_len != w_rev_len + 4)
    {
        FLOG_ERROR("Receivev Monitor Message Length error! msg_len  = %d, rev_len  = %d\r\n", w_msg_len, w_rev_len);
        return TRANS_FAILD;
    }

    #if 0
    w_rev_len2 = recv(w_monitor_socket, 
                            p_rev_msg + 4,   /*len : 4 Bytes*/
                            w_msg_len - 4, 
                            0);
    /*Error*/
    if (w_rev_len2 <= 0)
    {
        #if 0
        //close(sock);   
        /*重复关闭有没有问题*/
        close (g_rrh_client_socket.w_sockFd);
        close (g_rrh_server_socket.w_sockFd);
        #endif
        FLOG_ERROR("Rev monitor msg error! w_rev_len2 = %d\r\n", w_rev_len2);
        return TRANS_FAILD;
    }
    #endif
    
    FLOG_DEBUG("Rev monitor Msg OK len = %d. \r\n", w_rev_len+w_rev_len1);
  
    trans_debug_msg_print(p_rev_msg, 40, g_trans_debug_monitor);

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
    //fun_callback f_callback = NULL;
    struct trans_resp_msg_header *p_resp_header = NULL;

    p_resp_header = (struct trans_resp_msg_header *)malloc(SIZEOF_TRANS_RESP_MSG_HEADER);
    if (NULL == p_resp_header)
    {
        FLOG_ERROR("malloc p_resp_header error! \r\n");
        return TRANS_FAILD;   
    }

    for (us_operation = TRANS_REGISTER_FUN_MONITOR_OP; 
        us_operation < TRANS_REGISTER_FUN_BUF; 
        us_operation++)
    {
        if ((1 == g_trans_register_delete_func[us_operation].uc_use_flag)
                &&(NULL != g_trans_register_delete_func[us_operation].f_callback))
        {
            p_resp_header->uc_result = TRANS_ACK_FLAG_CLEAN_OPERATION;
            p_resp_header->uw_len = 0;
            p_resp_header->p_buf = NULL;
            
            (* (g_trans_register_delete_func[us_operation].f_callback))
                (NULL, SIZEOF_TRANS_RESP_MSG_HEADER, p_resp_header);
             
        }
    } 

    free(p_resp_header);

    return TRANS_SUCCESS;   
}

/*****************************************************************************+
* Function: trans_monitor_delete()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-09
* 
+*****************************************************************************/
u_int32_t trans_monitor_delete()
{
    u_int32_t uw_ret = 0;
   
    /*Delete action list according the src_module == TRANS_MOUDLE_ACTION*/
    uw_ret = trans_action_delete_by_src(TRANS_MOUDLE_ACTION);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_delete_by_src for action error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   

    /*Delete action list according the src_module == TRANS_MOUDLE_MONITOR*/
    uw_ret = trans_action_delete_by_src(TRANS_MOUDLE_MONITOR);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_delete_by_src error for monitor! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   

    /*Delete wireless operation*/
    uw_ret = trans_monitor_delete_operation();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_delete_operation error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    /*Delete timer list according the src_module == TRANS_MOUDLE_ACTION*/
    uw_ret = trans_timer_delete_by_src(TRANS_MOUDLE_ACTION);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_delete_by_src error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   

    return TRANS_SUCCESS;   
}

#if 0
/*****************************************************************************+
* Function: trans_monitor_conn_failed_func()
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
*  Data:    2011-10-09
* 
+*****************************************************************************/
int trans_monitor_conn_failed_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    len = len;
    p_info = p_info;

    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));  
    
    uw_ret = trans_monitor_delete();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_delete error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   
    
    g_trans_monitor_socket.w_monitor_socket = -1;
    g_trans_monitor_socket.uc_lock_flag = 1;
    //g_trans_monitor_socket.uw_connect_num ++;
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));   

    return TRANS_SUCCESS;   
}

#endif

/*****************************************************************************+
* Function: trans_monitor_conn_failed()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-09
* 
+*****************************************************************************/
void trans_monitor_conn_failed()
{
    u_int32_t uw_ret = 0;
    
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));  
    
    uw_ret = trans_monitor_delete();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_delete error! uw_ret = %d\r\n", uw_ret);
    
        return;    
    }   
    
    g_trans_monitor_socket.w_monitor_socket = -1;
    g_trans_monitor_socket.uc_lock_flag = 1;
    //g_trans_monitor_socket.uw_connect_num ++;
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));   

    return;   
}

/*****************************************************************************+
* Function: trans_monitor_conn_success()
* Description: 
* Parameters:
*           w_monitor_socket :
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-09
* 
+*****************************************************************************/
void trans_monitor_conn_success(int32_t w_monitor_socket)
{
    
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));  
    
    #if 0
    u_int32_t uw_ret = 0;
    
    uw_ret = trans_monitor_delete();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_delete error! uw_ret = %d\r\n", uw_ret);
    
        return;    
    }  
    #endif
    
    g_trans_monitor_socket.w_monitor_socket = w_monitor_socket;
    g_trans_monitor_socket.uc_lock_flag = 0;
    g_trans_monitor_socket.uw_connect_num ++;
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));   

    return;   
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
    
    FLOG_DEBUG("Enter \r\n");
    
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
    st_server_addr.sin_port = htons(g_trans_monitor_config_info.us_monitor_tcp_port);     
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
    w_total = 1; 
    w_ret = listen(w_server_socket, w_total);
    if (w_ret < 0)
    {
        FLOG_ERROR("Listen error! w_ret = %d\r\n", w_ret);
    
        //close (w_server_socket);        
        return TRANS_FAILD;
    }

    while (1)
    {
       
        if (0 >= g_trans_monitor_socket.w_monitor_socket)
        {
            /*Accept the connection->Connect establish*/
            w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
                    
            if (w_client_socket <= 0)
            {
                
                FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
                close (w_server_socket);
                //return TRANS_FAILD;
            }  
            
            FLOG_WARNING("New client connected\n");

            g_trans_moudle_socket_fd[TRANS_MOUDLE_MONITOR] = w_client_socket;

            trans_monitor_conn_success(w_client_socket);          
            
        }
        sleep(1);
    }
        
    #if 0
    /*Accept the connection->Connect establish*/
    w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
            
    if (w_client_socket <= 0)
    {
        
        FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
        close (w_server_socket);
        return TRANS_FAILD;
    }  
    
    //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    g_trans_moudle_socket_fd[TRANS_MOUDLE_MONITOR] = w_client_socket;
    
    g_trans_monitor_socket.w_monitor_socket = w_client_socket;
    g_trans_monitor_socket.uc_lock_flag = 0;
    g_trans_monitor_socket.uw_connect_num ++;
    #endif
    
    FLOG_DEBUG("Exit monitor_socket = %d\r\n", g_trans_monitor_socket.w_monitor_socket);
    
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

    FLOG_DEBUG("Enter \r\n");
    
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
    
    FLOG_DEBUG("Exit \r\n");

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
* Function: trans_monitor_register_op_func()
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
u_int32_t trans_monitor_register_op_func()
{    
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    /****************FOR TEST****************/
    #ifdef TRANS_MONITOR_TEST_COMPILE
    us_op = 127;
    trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    trans_monitor_operation_127_1,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback for OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 216;
    trans_register_func_callback(TRANS_REGISTER_FUN_MONITOR_OP,
                                    &us_op,  
                                    trans_monitor_operation_216_1,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  
    #endif
    /****************FOR TEST****************/

    return TRANS_SUCCESS;
}

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
    /*Init the Global variables */
    
    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    memset((u_int8_t*)&g_trans_monitor_config_info, 0, sizeof(struct trans_monitor_config_info));

    /*Monitor  TCP Port*/
    g_trans_monitor_config_info.us_monitor_tcp_port = p_init_info->us_monitor_port; 

    /*Monitor IP */
    //g_trans_monitor_config_info.uw_monitor_ip_addr = inet_addr("10.13.1.43");
    g_trans_monitor_config_info.uw_monitor_ip_addr = htonl(INADDR_ANY);

    //memset((u_int8_t*)&g_trans_monitor_socket, 0, sizeof(struct trans_monitor_socket));

    g_trans_monitor_socket.uc_lock_flag = 1;
    g_trans_monitor_socket.uw_connect_num = 0;
    g_trans_monitor_socket.w_monitor_socket = -1;

    if(pthread_mutex_init(&(g_trans_monitor_socket.m_mutex), NULL)) 
    {
        FLOG_ERROR("Initializing m_mutex error! \r\n");
    }

    uw_ret = trans_monitor_register_op_func();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_register_op_func error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    } 
    
    return TRANS_SUCCESS;
}

#endif

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
    memset((u_int8_t*)&g_trans_monitor_config_info, 0, sizeof(struct trans_monitor_config_info));

    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
   
    /*Close socket*/
    close(g_trans_monitor_socket.w_monitor_socket);

    g_trans_monitor_socket.w_monitor_socket = -1;
    g_trans_monitor_socket.uc_lock_flag = 1;
    g_trans_monitor_socket.uw_connect_num = 0;

    pthread_mutex_unlock (&(g_trans_monitor_socket.m_mutex));
#endif
    return TRANS_SUCCESS;
}

