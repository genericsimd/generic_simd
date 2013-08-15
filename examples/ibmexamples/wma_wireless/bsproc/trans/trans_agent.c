/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_agent.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-April.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_common.h>
#include <trans_transaction.h>
#include <trans_device.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_timer.h>
#include <trans_list.h>
#include <trans_debug.h>
#ifdef TRANS_BS_COMPILE
#include <bs_cfg.h>
#endif
#ifdef TRANS_MS_COMPILE
#include <ms_cfg.h>
#endif

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

/*****************************************************************************+
 *Global Variables
 +*****************************************************************************/
pthread_mutex_t  g_trans_agent_metric_mutex;

struct trans_agent_socket  g_trans_agent_socket;


#ifdef TRANS_AGENT_COMPILE
struct trans_agent_config_info g_trans_agent_config_info;

void *g_trans_agent_hb_timer = NULL;

#endif


#ifdef TRANS_AGENT_COMPILE

/*****************************************************************************+
 *Code 
+*****************************************************************************/

/*****************************************************************************+
* Function: trans_agent_get_time_stamp()
* Description: Get TimeStamp
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int64_t trans_agent_get_time_stamp()
{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday (&tp, &tzp);
//    return (((u_int64_t)tp.tv_sec) * 1000000000) + (((u_int64_t)tp.tv_usec) * 1000);
    return ((u_int64_t)tp.tv_sec);
}

/*****************************************************************************+
* Function: trans_agent_get_timestamp_string()
* Description: Get TimeStamp
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_get_timestamp_string(u_int8_t *p_timestamp)
{
    #if (defined TRANS_BS_COMPILE) || (defined TRANS_RRH_COMPILE)
    sprintf((char *)p_timestamp, "%ld", trans_agent_get_time_stamp());
    #endif

    #ifdef TRANS_MS_COMPILE
    sprintf((char *)p_timestamp, "%lld", trans_agent_get_time_stamp());
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_comnn_msg()
* Description: Send Connection message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_send_conn_msg()
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;

    //u_int64_t  ul_timestamp = 0;
    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t  *p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;

    json_t *msg = json_object();        // create an empty object;  

    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("connect");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
    
        json_decref(msg);
        free(p_msg);
    
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"CTRL");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send Connect MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
    
        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send Connect message to Agent OK! \r\n");
    
    #if 0
    //#ifdef TRANS_MS_COMPILE   
    u_int8_t uc_result = 0;
    
    FLOG_ERROR("Waite for message from agent\r\n");
    
    w_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);
        return TRANS_FAILD; 
    }
    //#endif
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_state_msg()
* Description: Send State Change message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_send_state_msg()
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;

    //u_int64_t  ul_timestamp = 0;
    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t  *p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;

    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("stateChange");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    //itoa(12, a_str, 10); 

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *state = json_string("1");// object for STATE;
    json_object_set_new(body, "STATE", state);// set STATE;
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
    
        json_decref(msg);
        free(p_msg);
    
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send StateChange MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
    
        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send StateChange message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_send_hb_msg()
* Description: Send HeartBeat message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-30
* 
+*****************************************************************************/
int trans_agent_send_hb_msg(void *p_info, size_t len, void *p_msg_info)
{
    int32_t w_ret = 0;
    //u_int32_t uw_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_timestamp[20+1] = {0};

    u_int32_t uw_len = 0;
    char *p_msg = NULL;
    
    u_int8_t * p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;
    
    (void) p_info;
    (void) len;
    (void) p_msg_info;

    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(msg, "SOURCE", source);// set DEVID;

    //json_t *dest = json_string("51");// object for DEST
    json_t *dest = json_string(TRANS_AGENT_HEARTBEAT_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
 
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "module_id", devid);// set DEVID;

    json_t *action = json_string("heartbeat");// object for ACTION
    json_object_set_new(body, "action", action);// set ACTION

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
    
        json_decref(msg);
        free(p_msg);
    
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send Heartbeat MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
    
        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send Heartbeat message to Agent OK! \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_hb_timer()
* Description: Send HeartBeat message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-30
* 
+*****************************************************************************/
u_int32_t trans_agent_hb_timer(void)
{
    u_int32_t uw_ret = 0;
    struct trans_timer_info st_timer_info;
    
    void* p_timer = NULL;

    trans_agent_send_hb_msg(NULL, 0, NULL);
    
    /*ADD TIMER*/
    /*Send Heartbeat Msg Again  per 60 second*/
   
    st_timer_info.f_callback = trans_agent_send_hb_msg;
    st_timer_info.p_data = NULL;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_CIRCLE;
    st_timer_info.uw_interval = g_trans_agent_config_info.uw_agent_hb_time;
    
    uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    g_trans_agent_hb_timer = p_timer;

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_query_rrh_func()
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
int trans_agent_query_rrh_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;
    int32_t      w_result = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    FLOG_ERROR("Agent Query RRH Timeout\r\n");
    
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

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            #if 0
            uw_ret = trans_transaction_time_out(p_info); 
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }
            #endif
            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! result = %d\r\n", w_result);

            return TRANS_FAILD;
    
    }

    #if 0
    /*Send response message to agent*/
    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_cfg_rrh_func()
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
int trans_agent_cfg_rrh_func(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;
    int32_t      w_result = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    FLOG_INFO("Agent Config RRH Timeout\r\n");
    
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

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            #if 0
            uw_ret = trans_transaction_time_out(p_info); 
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_time_out error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }
            #endif

            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! uc_result = %d\r\n", w_result);
            return TRANS_FAILD;
    
    }

    #if 0
    /*Send response message to agent*/
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_build_conf_update_msg()
* Description: Build confUpdate message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-05-16
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_conf_update_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t  *p_send_len,
                            u_int8_t **pp_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    //int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    struct trans_agent_confupdate_info *p_confupdate_info = NULL;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    *pp_send_buf = NULL;

    p_confupdate_info = (struct trans_agent_confupdate_info *)(p_msg_info->p_resp_msg);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("confUpdate");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *conf = json_object(); // create an empty object
    
    json_t *freq = json_real(p_confupdate_info->f_freq);// object for freq;
    json_object_set_new(conf, "assignedFrequency", freq);// set element_id;

    json_object_set_new(body, "CONF", conf);  // set body elements in msg

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)*pp_send_buf, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);

    *p_send_len = uw_msg_len - 1;
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send confUpdate MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("send TopologyUpdate MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    #endif
    
    FLOG_INFO("send confUpdate message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_build_t_update_msg()
* Description: Build TopologyUpdate message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-19
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_t_update_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t  *p_send_len,
                            u_int8_t **pp_send_buf)


{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    //int w_ret = 0;
    u_int32_t uw_msg_len = 0;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    //p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("topologyUpdate");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *array = json_array();
    
    json_t *value1 = json_string((char *)(p_msg_info->p_resp_msg));// object for element_id;
    json_array_append_new(array, value1);

    json_t *value2 = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for element_id;
    json_array_append_new(array, value2);

    json_object_set_new(body, "TOPOLOGY", array);  // set body elements in msg  

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    *p_send_len = uw_msg_len - 1;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send TopologyUpdate MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("send TopologyUpdate MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    #endif
    
    FLOG_INFO("send TopologyUpdate message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


#ifdef TRANS_RRH_NEW_CONNECT
/*****************************************************************************+
* Function: trans_agent_build_metric_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-06-06
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_metric_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t  *p_send_len,
                            u_int8_t **pp_send_buf)

{
    struct trans_agent_metric_info *p_agent_metric = NULL;

    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_srt[20+1] = {0};
    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    u_int8_t uc_index = 0;
    u_int32_t uw_num = 0;
    int32_t w_value = 0;
    u_int32_t  uw_value = 0;
    float     f_value = 0.0;
    u_int8_t * p_temp = NULL;

    json_t *value1 = NULL;
    json_t *value2 = NULL;

    //int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    int8_t c_metric = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter! \r\n");

    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_metric = (struct trans_agent_metric_info *)p_msg_info->p_reqs_msg;

    if (NULL == p_agent_metric)
    {
        FLOG_ERROR("NULL PTR p_agent_metric! \r\n");
        return TRANS_FAILD;
    }

    uw_num = p_agent_metric->uc_metric_num;
   
    json_t *msg = json_object();        // create an empty object;  
    
    sprintf((char *)a_srt, "%d", p_agent_metric->w_dest_id);
    json_t *source = json_string((char *)(a_srt));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    memset(a_srt, 0, 20+1);
    sprintf((char *)a_srt, "%d", p_agent_metric->w_source_id);
    json_t *dest = json_string((char *)a_srt);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    json_t *id = json_integer(p_agent_metric->w_msg_id);// object for id;
    json_object_set_new(body, "id", id);// set id;
    
    json_t *ref = json_integer(p_agent_metric->w_msg_id);// object for ref;
    json_object_set_new(body, "ref", ref);// set ref;
    
    trans_agent_get_timestamp_string(a_srt);
    
    json_t *timestamp = json_string((char *)(a_srt));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp    
    
    json_t *sendaction = json_string("getMetricValues_res");// object for action
    json_object_set_new(body, "action", sendaction);// set action
    
    json_t *moduleid = json_string((char *)(p_agent_metric->a_module_id));// object for module_id;
    json_object_set_new(body, "module_id", moduleid);// set module_id;
    
    json_t *content = json_object(); // create an empty object
    
    json_t *elementid = json_string((char *)(p_agent_metric->a_element_id));// object for element_id;
    json_object_set_new(content, "element_id", elementid);// set element_id;
    
    json_t *array1 = json_array();
    json_t *array2 = json_array();
    

    if (0 == strcmp ("rrh", (char *)p_agent_metric->a_element_id))
    {
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of int8_t , if want to change int32_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + ((uc_index + 1) * 3);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Power Amplifier Temperature*/ 
                case TRANS_AGENT_RRH_METRIC_ID_PAT:   
                    w_value = *((int8_t *)(p_temp));
                    
                    value2 = json_real(w_value);// object for element_id;

                    p_temp = p_temp + 1;
                    break;

                /*Downlink Output Power for channel 1#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:
                /*Downlink Output Power for channel 2#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR:
                    w_value = *((int8_t *)(p_temp));
                    
                    value2 = json_real(w_value);// object for element_id;
                    
                    p_temp = p_temp + 4;
                    break;
            
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 
            
                    w_value = *((u_int8_t *)(p_temp));      
                    
                    value2 = json_real(w_value);// object for element_id;
                    
                    p_temp = p_temp + 1;
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow RRH message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  

            value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
            json_array_append_new(array1, value1);
            
            //value2 = json_real(w_value);// object for value;
            json_array_append_new(array2, value2); 

        }

    }
    else if (0 == strcmp ("bs", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Per WMB DL throughput(avg)*/ 
                case TRANS_AGENT_BS_METRIC_ID_WMB_DL:                     
                /*Per WMB UL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_WMB_UL:
                /*Per conn. DL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_CONN_DL:
                /*Per conn. UL throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN_UL:
                /*UL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_UL_MANG:
                /*DL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_DL_MANG:

                    f_value = *((float *)(p_temp));
                    
                    value2 = json_real(f_value);// object for element_id;

                    break;
            
                /*Ranging Success conut*/    
                case TRANS_AGENT_BS_METRIC_ID_RANG: 
                /*connection conut*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN: 
                /*CRCErrorCount*/    
                case TRANS_AGENT_BS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_BS_METRIC_ID_PDU:

                    uw_value = *((u_int32_t *)(p_temp));   
                    
                    value2 = json_real(uw_value);// object for element_id;

                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow BS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }

            value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
            json_array_append_new(array1, value1);
            
            //value2 = json_real(w_value);// object for value;
            json_array_append_new(array2, value2); 

        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else if (0 == strcmp ("cpe", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*RSSI*/ 
                case TRANS_AGENT_MS_METRIC_ID_RSSI:                     
                /*SINR*/
                case TRANS_AGENT_MS_METRIC_ID_SINR:
                /*Tx Power*/
                case TRANS_AGENT_MS_METRIC_ID_TX_PWR:
                /*Temperature*/  
                case TRANS_AGENT_MS_METRIC_ID_T:
                    w_value = *((u_int32_t *)(p_temp));
                    
                    c_metric = (int8_t)w_value;
                    w_value = (int32_t)c_metric;
                    
                    value2 = json_real(w_value);// object for element_id;
                    
                    break;

                /*CRCErrorCount*/    
                case TRANS_AGENT_MS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_MS_METRIC_ID_PDU:

                    w_value = *((u_int32_t *)(p_temp));
                    
                    value2 = json_real(w_value);// object for element_id;

                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow MS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  

            value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
            json_array_append_new(array1, value1);
            
            //value2 = json_real(w_value);// object for value;
            json_array_append_new(array2, value2); 

        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else
    {
        FLOG_ERROR("Source module error! src_moudle = %s\r\n", p_agent_metric->a_element_id);
    }
   
    json_object_set_new(content, "metrics", array1);  // set body elements in msg  
    json_object_set_new(content, "values", array2);  // set body elements in msg  
    
    json_object_set_new(body, "content", content);  // set body elements in msg
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string  
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;

    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");

        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);

    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    *p_send_len = uw_msg_len - 1;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "sendMetric MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);
    #endif

    FLOG_INFO("send getMetricValues_res message to Agent OK! \r\n");

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit! \r\n");
    
    return TRANS_SUCCESS;

}


#else

/*****************************************************************************+
* Function: trans_agent_build_metric_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_metric_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t  *p_send_len,
                            u_int8_t **pp_send_buf)

{
    struct trans_agent_metric_info *p_agent_metric = NULL;

    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_srt[20+1] = {0};
    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    u_int8_t uc_index = 0;
    u_int32_t uw_num = 0;
    int32_t w_value = 0;
    u_int32_t  uw_value = 0;
    float     f_value = 0.0;
    u_int8_t * p_temp = NULL;

    json_t *value1 = NULL;
    json_t *value2 = NULL;

    //int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    int8_t c_metric = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter! \r\n");

    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_metric = (struct trans_agent_metric_info *)p_msg_info->p_reqs_msg;

    if (NULL == p_agent_metric)
    {
        FLOG_ERROR("NULL PTR p_agent_metric! \r\n");
        return TRANS_FAILD;
    }

    uw_num = p_agent_metric->uc_metric_num;
   
    json_t *msg = json_object();        // create an empty object;  
    
    sprintf((char *)a_srt, "%d", p_agent_metric->w_dest_id);
    json_t *source = json_string((char *)(a_srt));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    memset(a_srt, 0, 20+1);
    sprintf((char *)a_srt, "%d", p_agent_metric->w_source_id);
    json_t *dest = json_string((char *)a_srt);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    json_t *id = json_integer(p_agent_metric->w_msg_id);// object for id;
    json_object_set_new(body, "id", id);// set id;
    
    json_t *ref = json_integer(p_agent_metric->w_msg_id);// object for ref;
    json_object_set_new(body, "ref", ref);// set ref;
    
    trans_agent_get_timestamp_string(a_srt);
    
    json_t *timestamp = json_string((char *)(a_srt));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp    
    
    json_t *sendaction = json_string("getMetricValues_res");// object for action
    json_object_set_new(body, "action", sendaction);// set action
    
    json_t *moduleid = json_string((char *)(p_agent_metric->a_module_id));// object for module_id;
    json_object_set_new(body, "module_id", moduleid);// set module_id;
    
    json_t *content = json_object(); // create an empty object
    
    json_t *elementid = json_string((char *)(p_agent_metric->a_element_id));// object for element_id;
    json_object_set_new(content, "element_id", elementid);// set element_id;
    
    json_t *array1 = json_array();
    json_t *array2 = json_array();
    

    if (0 == strcmp ("rrh", (char *)p_agent_metric->a_element_id))
    {
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of int8_t , if want to change int32_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + ((uc_index + 1) * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Power Amplifier Temperature*/ 
                case TRANS_AGENT_RRH_METRIC_ID_PAT:                     
                /*Downlink Output Power for channel 1#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:
                /*Downlink Output Power for channel 2#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR:
                    w_value = *((int8_t *)(p_temp - 1));

                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

           
                    break;
            
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 
            
                    w_value = *((u_int8_t *)(p_temp - 1));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow RRH message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  
        
        }

//        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) )
//        {
            /*Its memory was malloced in function ---trans_agent_rev_query_rrh()*/
//            if (p_agent_metric != NULL)
//            {
        //free(p_agent_metric);
//            }
//        }
//        else
//        {

            /*********Test**********/
            //free(p_agent_metric);
            /*********Test**********/
//        }        
    }
    else if (0 == strcmp ("bs", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Per WMB DL throughput(avg)*/ 
                case TRANS_AGENT_BS_METRIC_ID_WMB_DL:                     
                /*Per WMB UL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_WMB_UL:
                /*Per conn. DL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_CONN_DL:
                /*Per conn. UL throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN_UL:
                /*UL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_UL_MANG:
                /*DL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_DL_MANG:

                    f_value = *((float *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(f_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
            
                /*Ranging Success conut*/    
                case TRANS_AGENT_BS_METRIC_ID_RANG: 
                /*connection conut*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN: 
                /*CRCErrorCount*/    
                case TRANS_AGENT_BS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_BS_METRIC_ID_PDU:

                    uw_value = *((u_int32_t *)(p_temp));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(uw_value);// object for element_id;
                    json_array_append_new(array2, value2);
        
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow BS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else if (0 == strcmp ("cpe", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*RSSI*/ 
                case TRANS_AGENT_MS_METRIC_ID_RSSI:                     
                /*SINR*/
                case TRANS_AGENT_MS_METRIC_ID_SINR:
                /*Tx Power*/
                case TRANS_AGENT_MS_METRIC_ID_TX_PWR:
                /*Temperature*/  
                case TRANS_AGENT_MS_METRIC_ID_T:
                    w_value = *((u_int32_t *)(p_temp));
                    
                    c_metric = (int8_t)w_value;
                    w_value = (int32_t)c_metric;
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;

                /*CRCErrorCount*/    
                case TRANS_AGENT_MS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_MS_METRIC_ID_PDU:

                    w_value = *((u_int32_t *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for value;
                    json_array_append_new(array2, value2);        
           
                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow MS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else
    {
        FLOG_ERROR("Source module error! src_moudle = %s\r\n", p_agent_metric->a_element_id);
    }
   
    json_object_set_new(content, "metrics", array1);  // set body elements in msg  
    json_object_set_new(content, "values", array2);  // set body elements in msg  
    
    json_object_set_new(body, "content", content);  // set body elements in msg
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string  
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;

    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");

        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);

    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    *p_send_len = uw_msg_len - 1;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "sendMetric MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);
    #endif

    FLOG_INFO("send getMetricValues_res message to Agent OK! \r\n");

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit! \r\n");
    
    return TRANS_SUCCESS;

}

#endif

/*****************************************************************************+
* Function: trans_agent_build_alert_msg()
* Description: Build Alert message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-19
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_alert_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t  *p_send_len,
                            u_int8_t **pp_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    struct trans_agent_alert_info *p_alert_info = NULL;
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    //int w_ret = 0;
    u_int32_t uw_msg_len = 0;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("alert");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *alertid = json_integer(p_alert_info->us_alarm_id);// object for ALERTID;
    json_object_set_new(body, "ALERTID", alertid);// set ALERTID;

    json_t *alertval = json_real(p_alert_info->w_alarm_value);// object for VALUE;
    json_object_set_new(body, "VALUE", alertval);// set VALUE;

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    *p_send_len = uw_msg_len - 1;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send Alert MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    #endif
    
    FLOG_INFO("send alert message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_build_spectrum_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_spectrum_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t  *p_send_len,
                        u_int8_t **pp_send_buf)

{
    #ifdef TRANS_BS_COMPILE
    int32_t w_ret = 0;
    #endif
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_policy_code = 0;

    u_int32_t uw_msg_len = 0;

    struct trans_timer_info st_timer_info;    
    u_int32_t uw_ret = 0;
    void* p_timer = NULL;
    
    //struct trans_agent_spectrum_info *p_spectrum_info, 
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    #ifdef TRANS_BS_COMPILE
    uw_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != uw_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }

    f_longitude = atof((char *)a_str); 

    uw_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != uw_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }

    f_latitude = atof((char *)a_str); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "GPS: %f, %f \r\n", f_longitude, f_latitude);    

    w_ret = get_global_param ("INITIAL_SENSING_POLICY", & (w_policy_code));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_POLICY error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("INITIAL_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }
    #endif
     
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    //sprintf((char *)a_str, "%d", 53);/*Msg Length£º4Bytes*/
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("getSpectrumResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(body, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *policycode = json_integer(w_policy_code);// object for POLICYCODE;
    json_object_set_new(body, "POLICYCODE", policycode);// set POLICYCODE;

    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    json_object_set_new(gps, "LATITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(body, "GPS", gps);  // set body elements in msg
    
    json_t *array = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_SENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_msg_info->p_resp_msg) + uw_index);
        json_t *value = json_real(d_sensing);
        json_array_append_new(array, value);
    }
    
    json_object_set_new(body, "SENSING", array);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");
        
        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    *p_send_len = uw_msg_len - 1;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send getSpectrumResult MSG: %s \r\n", (*pp_send_buf));    
    //FLOG_INFO("send getSpectrumResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    #endif
    
    FLOG_INFO("send getSpectrumResult message to Agent OK! \r\n");

    /*For search*/
    uw_ret = trans_transaction_set_dst(p_msg_info->p_info, 
                                0x01,
                                g_trans_agent_socket.w_agent_socket,
                                g_trans_agent_socket.a_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    /*ADD TIMER*/
    st_timer_info.f_callback = p_msg_info->f_callback;
    st_timer_info.p_data = p_msg_info->p_info;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
    st_timer_info.uw_interval = TRANS_SEND_AGENT_MSG_TIMEOUT;
    
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

    #if 0
    /*ADD TIMER*/
    gettimeofday(&st_time_val, NULL);  
    /*Timeout -- 30s*/
    st_time_val.tv_sec = st_time_val.tv_sec + TRANS_SEND_AGENT_MSG_TIMEOUT;   
    //st_time_val.tv_usec = st_time_val.tv_usec;
    
    w_ret = trans_timer_add(&st_time_val, 
                                        p_msg_info->f_callback, 
                                        g_trans_agent_socket.a_mac, 
                                        0x01, 
                                        p_msg_info->p_info,
                                        &p_timer_id);   
    if (TRANS_SUCCESS != w_ret) 
    {   
        
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
        return TRANS_FAILD;     
    } 

    
    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        gettimeofday(&st_time_val, NULL);  
        /*Timeout -- 30s*/
        st_time_val.tv_sec = st_time_val.tv_sec + TRANS_SEND_AGENT_MSG_TIMEOUT;   
        //st_time_val.tv_usec = st_time_val.tv_usec;

        w_ret = trans_timer_add(&st_time_val, 
                                            trans_agent_block_msg_timer_func, 
                                            TRANS_MOUDLE_BS, 
                                            TRANS_MOUDLE_AGENT, 
                                            0x01, 
                                            NULL,
                                            &p_timer_id);   
        if (TRANS_SUCCESS != w_ret) 
        {   
            
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;     
        } 

    
        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);

            free(p_send_msg);
            return TRANS_FAILD; 
        }    
    }
    #endif

    //free(p_send_msg);
  
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_build_periodic_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-29
* 
+*****************************************************************************/
static u_int32_t trans_agent_build_periodic_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t  *p_send_len,
                        u_int8_t **pp_send_buf)
{
    #ifdef TRANS_BS_COMPILE
    int32_t w_ret = 0;
    #endif
    struct trans_agent_msg *p_agent_head = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_periodic_len = 0;
    u_int32_t uw_agent_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_periodic_msg = NULL; 
    char *p_agent_msg = NULL; 
    double  d_sensing = 0.0;
    int        w_interref = 0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_working_freq = 0;

    struct trans_periodic_sensing_info * p_periodical = NULL;

    u_int32_t uw_msg_len = 0;
   
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_periodical = (struct trans_periodic_sensing_info *)p_msg_info->p_resp_msg;
    if ((NULL == p_periodical) 
        || (NULL == p_periodical->p_per_interref) 
        || (NULL == p_periodical->p_per_sensing))
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    #ifdef TRANS_BS_COMPILE
    w_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }
    
    f_longitude = atof((char *)a_str); 
    
    w_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }
    
    f_latitude = atof((char *)a_str); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "GPS: %f, %f \r\n", f_longitude, f_latitude);    
    
    w_ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(w_working_freq));
    if (w_ret != 0)
    {
        FLOG_ERROR ("get parameters DEFAULT_WORKING_FREQ error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("PERIODIC_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }
    #endif

    trans_agent_get_timestamp_string(a_str);
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    
    json_t *action = json_string("sendPeriodicResult");// object for ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;

    /*Creat JSON message for periodic sensing result stream */    
    json_t *periodic_msg = json_object();        // create an empty object;  

    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(periodic_msg, "TIMESTAMP", timestamp);// set timestamp
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(periodic_msg, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(periodic_msg, "DEVID", devid);// set DEVID;
    
    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(periodic_msg, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *freq = json_real(w_working_freq);// object for POLICYCODE;
    json_object_set_new(periodic_msg, "FREQ", freq);// set POLICYCODE;
    
    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    //json_object_set_new(gps, "LATITUDE", latitude);// set LATITUDE;
    json_object_set_new(gps, "ALTITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(periodic_msg, "GPS", gps);  // set body elements in msg
    
    json_t *array1 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERSENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_periodical->p_per_sensing) + uw_index);
        json_t *value1 = json_real(d_sensing);
        json_array_append_new(array1, value1);
    }
    
    json_t *array2 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERINTERREF_NUM; uw_index++)
    {
        w_interref = *(((int *)p_periodical->p_per_interref) + uw_index);
        json_t *value2 = json_real(w_interref);
        json_array_append_new(array2, value2);
    }

    json_object_set_new(periodic_msg, "PERSENSING", array1);  // set body elements in msg  
    
    json_object_set_new(periodic_msg, "PERINTERREF", array2);  // set body elements in msg  
    
    p_periodic_msg = json_dumps(periodic_msg, 0); //dump msg as a string    
    uw_periodic_len = strlen((char *)p_periodic_msg);


    /*Creat JSON message for agent */    
    json_t *agent_msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(agent_msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(agent_msg, "DEST", dest);// set DEST
    
    json_t *agent_body = json_object(); // create an empty object    
    
    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(agent_body, "TIMESTAMP", timestamp);// set timestamp

    sprintf((char *)a_str, "%d", uw_periodic_len);/*Msg Length£º4Bytes*/
    json_t *len= json_string((char *)(a_str));// object for DEST
    json_object_set_new(agent_body, "LENGTH", len);// set DEST
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(agent_body, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(agent_body, "DEVID", devid);// set DEVID;

    json_object_set_new(agent_msg, "BODY", agent_body);  // set body elements in msg
    
    p_agent_msg = json_dumps(agent_msg, 0); //dump msg as a string    
    uw_agent_len = strlen((char *)p_agent_msg);
    
    /*Creat the whole message*/
    uw_msg_len = 2 + uw_agent_len + uw_periodic_len + SIZEOF_TRANS_AGENT_MSG;
    
    *pp_send_buf = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == *pp_send_buf)
    {
        FLOG_ERROR("malloc *pp_send_buf error! \r\n");
        
        json_decref(agent_msg);
        json_decref(periodic_msg);
        free(p_agent_msg);
        free(p_periodic_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)(*pp_send_buf), 0, uw_msg_len);
    
    p_agent_head = (struct trans_agent_msg *)(*pp_send_buf);
    
    strcpy((char *)p_agent_head->a_magic, (char *)"FILE");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_head->a_len, "%04d", uw_agent_len);/*Msg Length£º4Bytes*/
    
    memcpy((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG, p_agent_msg, uw_agent_len);   

    memcpy((*pp_send_buf) + SIZEOF_TRANS_AGENT_MSG + uw_agent_len, p_periodic_msg, uw_periodic_len);   
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_agent_msg);
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG + uw_agent_len), (char *)p_periodic_msg);
    
    json_decref(agent_msg);
    json_decref(periodic_msg);
    free(p_agent_msg);
    free(p_periodic_msg);
    
    *p_send_len = uw_msg_len - 2;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send sendPeriodicResult MSG: %s \r\n", (*pp_send_buf));     
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "uw_agent_len = %d, uw_periodic_len = %d, uw_msg_len = %d. \r\n", uw_agent_len, uw_periodic_len, uw_msg_len); 
    //FLOG_INFO("send sendPeriodicResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    #if 0
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 2, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);
    #endif
    
    FLOG_INFO("send sendPeriodicResult message to Agent OK! \r\n");

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send()
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
u_int32_t trans_agent_send(void * p_send_buf,
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
        if (uw_send_len - w_total_send_len > 6*1024)
        {
            w_send_len_temp = 6*1024 ;
        }
        else
        {
            w_send_len_temp = uw_send_len - w_total_send_len;
        }

        uw_congestion_num = trans_device_get_congestion_num(
                g_trans_agent_socket.a_mac, 0); 
        
        /*Not Congestion*/
        if (0 == uw_congestion_num)
        {
            /*Send New Message to Monitor*/
            w_ret = send(g_trans_agent_socket.w_agent_socket, (p_send_buf + w_total_send_len), w_send_len_temp, 0);
            
            if(w_ret <= 0)
            {
                //close(w_sockfd);
                if (EAGAIN == errno)
                {
                    FLOG_ERROR("Congestion start! \r\n");
                
                    uw_congestion_num = trans_device_get_congestion_num(
                            g_trans_agent_socket.a_mac, 1); 
                }  

                TRANS_COMMON_SOCKFD_PRINT(g_trans_agent_socket.w_agent_socket, w_ret, errno);

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
* Function: trans_agent_send_msg()
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
u_int32_t trans_agent_send_msg(struct trans_send_msg_to_agent *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg)
{

    //int32_t w_ret = 0;
    u_int8_t  a_action[TRANS_AGENT_ACTION_SIZE+1] = {0};
    //int32_t w_agent_sockfd = 0;
    u_int32_t uw_ret = 0;

    u_int8_t      uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;   

    u_int32_t uw_send_len = 0;
    u_int8_t *p_send_buf = NULL;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    (void) len;
    (void) p_send_msg;
        
    strcpy((char *)(a_action), (char *)(p_msg_info->p_reqs_msg));

    if (0 == strcmp ("getMetricValues", (char *)a_action))
    {
        uw_ret = trans_agent_build_metric_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_metric_msg error! uw_ret = %d\r\n", uw_ret);
        }  
        
        uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;   
        
        uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                    uc_delete_flag,
                                    TRANS_MOUDLE_BUF);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        }
        
        //return uw_ret
            
    }
    /*transaction info-----call trans_transaction_set_comn in the other function*/
    else if (0 == strcmp ("alert", (char *)a_action))
    {
        uw_ret = trans_agent_build_alert_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_alert_msg error! uw_ret = %d\r\n", uw_ret);
        }
        //return uw_ret
    }
    else if (0 == strcmp ("spectrum", (char *)a_action))
    {
        uw_ret = trans_agent_build_spectrum_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_spectrum_msg error! uw_ret = %d\r\n", uw_ret);
        }

        uc_delete_flag = TRANS_TRANSACTION_FLAG_NO_DELETE;   

        uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                    uc_delete_flag,
                                    TRANS_MOUDLE_BUF);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        }
        //return uw_ret
        
    }
    /*No transaction info-----Do not call trans_transaction_set_comn*/
    else if (0 == strcmp ("periodic", (char *)a_action))
    {
        uw_ret = trans_agent_build_periodic_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_periodic_msg error! uw_ret = %d\r\n", uw_ret);
        }
        //return uw_ret
    }    
    /*No transaction info-----Do not call trans_transaction_set_comn*/
    else if (0 == strcmp ("topologyUpdate", (char *)a_action))
    {
        uw_ret = trans_agent_build_t_update_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_t_update_msg error! uw_ret = %d\r\n", uw_ret);
        }
        
        //return uw_ret

    }    
    /*transaction info-----call trans_transaction_set_comn in the other function*/
    else if (0 == strcmp ("confUpdate", (char *)a_action))
    {
        uw_ret = trans_agent_build_conf_update_msg(p_msg_info, 
                            &uw_send_len, 
                            &p_send_buf);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_build_conf_update_msg error! uw_ret = %d\r\n", uw_ret);
        }
        
        //return uw_ret
    }

    else
    {
        FLOG_ERROR("action/ACTION error! action = %s .\r\n", a_action);
        
        uw_ret = TRANS_FAILD;
    }

    if ((TRANS_SUCCESS == uw_ret) && (NULL != p_send_buf))
    {
        uw_ret = trans_agent_send(p_send_buf, uw_send_len);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send error! uw_ret = %d\r\n", uw_ret);
        }
    }
    
    /*it could be free , it not be set in transaction*/
    if (NULL != p_send_buf)
    {
        free(p_send_buf);
    }
       
        
    #if 0
    //trans_debug_msg_print(p_send_msg, 40, g_trans_debug_agent);

    w_agent_sockfd = g_trans_agent_socket.w_agent_socket;
    
    w_ret = send(w_agent_sockfd, p_send_msg, uw_len, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    FLOG_INFO("send Agent message OK! \r\n");

    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        struct timeval st_time_val;
        gettimeofday(&st_time_val, NULL);  
        
        /*Timeout -- 20s*/
        st_time_val.tv_sec = st_time_val.tv_sec + 20;   
        //st_time_val.tv_usec = st_time_val.tv_usec;
        
        void* p_timer_id = NULL;
        struct trans_timer_msg_info st_msg_info;
        
        st_msg_info.us_serial_number = 0xffff;
        st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
        st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
        st_msg_info.f_callback = trans_agent_block_msg_timer_func;
        st_msg_info.p_user_info = NULL;
        
        w_ret = trans_timer_add(&st_time_val, 
                                trans_agent_block_msg_timer_func, 
                                p_send_msg, 
                                uw_len + SIZEOF_TRANS_AGENT_MSG, 
                                &st_msg_info,
                                &p_timer_id);
        
        if (TRANS_SUCCESS != w_ret) 
        {   
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;     
        } 

        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);
            return TRANS_FAILD; 
        }

    }
    #endif
    
    return uw_ret;
    
}


#if 0
/*****************************************************************************+
* Function: trans_agent_send_conf_update_msg()
* Description: Send confUpdate message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-05-08
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_conf_update_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    struct trans_agent_confupdate_info *p_confupdate_info = NULL;

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_confupdate_info = (struct trans_agent_confupdate_info *)(p_msg_info->p_resp_msg);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("confUpdate");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *conf = json_object(); // create an empty object
    
    json_t *freq = json_real(p_confupdate_info->f_freq);// object for freq;
    json_object_set_new(conf, "assignedFrequency", freq);// set element_id;

    json_object_set_new(body, "CONF", conf);  // set body elements in msg

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send confUpdate MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send TopologyUpdate MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send confUpdate message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_send_t_update_msg()
* Description: Send TopologyUpdate message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-19
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_t_update_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    //p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_HEARTBEAT_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("topologyUpdate");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *array = json_array();
    
    json_t *value1 = json_string((char *)(p_msg_info->p_resp_msg));// object for element_id;
    json_array_append_new(array, value1);

    json_t *value2 = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for element_id;
    json_array_append_new(array, value2);

    json_object_set_new(body, "TOPOLOGY", array);  // set body elements in msg  

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send TopologyUpdate MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send TopologyUpdate MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send TopologyUpdate message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_metric_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_metric_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)
{
    struct trans_agent_metric_info *p_agent_metric = NULL;

    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_srt[20+1] = {0};
    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    u_int8_t uc_index = 0;
    u_int32_t uw_num = 0;
    int32_t w_value = 0;
    u_int32_t  uw_value = 0;
    float     f_value = 0.0;
    u_int8_t * p_temp = NULL;

    json_t *value1 = NULL;
    json_t *value2 = NULL;

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;
    int8_t c_metric = 0;


    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter! \r\n");

    (void) p_send_buf;
    (void) uw_buf_len;

    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_metric = (struct trans_agent_metric_info *)p_msg_info->p_reqs_msg;

    if (NULL == p_agent_metric)
    {
        FLOG_ERROR("NULL PTR p_agent_metric! \r\n");
        return TRANS_FAILD;
    }

    uw_num = p_agent_metric->uc_metric_num;
   
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    sprintf((char *)a_srt, "%d", p_agent_metric->w_source_id);
    json_t *dest = json_string((char *)a_srt);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    json_t *id = json_integer(p_agent_metric->w_msg_id);// object for id;
    json_object_set_new(body, "id", id);// set id;
    
    json_t *ref = json_integer(p_agent_metric->w_msg_id);// object for ref;
    json_object_set_new(body, "ref", ref);// set ref;
    
    trans_agent_get_timestamp_string(a_srt);
    
    json_t *timestamp = json_string((char *)(a_srt));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp    
    
    json_t *sendaction = json_string("getMetricValues_res");// object for action
    json_object_set_new(body, "action", sendaction);// set action
    
    json_t *moduleid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for module_id;
    json_object_set_new(body, "module_id", moduleid);// set module_id;
    
    json_t *content = json_object(); // create an empty object
    
    json_t *elementid = json_string((char *)(p_agent_metric->a_element_id));// object for element_id;
    json_object_set_new(content, "element_id", elementid);// set element_id;
    
    json_t *array1 = json_array();
    json_t *array2 = json_array();
    

    if (0 == strcmp ("rrh", (char *)p_agent_metric->a_element_id))
    {
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of int8_t , if want to change int32_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + ((uc_index + 1) * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Power Amplifier Temperature*/ 
                case TRANS_AGENT_RRH_METRIC_ID_PAT:                     
                /*Downlink Output Power for channel 1#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:
                /*Downlink Output Power for channel 2#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR:
                    w_value = *((int8_t *)(p_temp - 1));

                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

           
                    break;
            
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 
            
                    w_value = *((u_int8_t *)(p_temp - 1));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow RRH message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  
        
        }

//        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) )
//        {
            /*Its memory was malloced in function ---trans_agent_rev_query_rrh()*/
//            if (p_agent_metric != NULL)
//            {
        //free(p_agent_metric);
//            }
//        }
//        else
//        {

            /*********Test**********/
            //free(p_agent_metric);
            /*********Test**********/
//        }        
    }
    else if (0 == strcmp ("bs", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Per WMB DL throughput(avg)*/ 
                case TRANS_AGENT_BS_METRIC_ID_WMB_DL:                     
                /*Per WMB UL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_WMB_UL:
                /*Per conn. DL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_CONN_DL:
                /*Per conn. UL throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN_UL:
                /*UL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_UL_MANG:
                /*DL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_DL_MANG:

                    f_value = *((float *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(f_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
            
                /*Ranging Success conut*/    
                case TRANS_AGENT_BS_METRIC_ID_RANG: 
                /*connection conut*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN: 
                /*CRCErrorCount*/    
                case TRANS_AGENT_BS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_BS_METRIC_ID_PDU:


                    uw_value = *((u_int32_t *)(p_temp));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(uw_value);// object for element_id;
                    json_array_append_new(array2, value2);
        
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow BS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else if (0 == strcmp ("cpe", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*RSSI*/ 
                case TRANS_AGENT_MS_METRIC_ID_RSSI:                     
                /*SINR*/
                case TRANS_AGENT_MS_METRIC_ID_SINR:
                /*Tx Power*/
                case TRANS_AGENT_MS_METRIC_ID_TX_PWR:
                /*Temperature*/  
                case TRANS_AGENT_MS_METRIC_ID_T:
                    w_value = *((u_int32_t *)(p_temp));

                    c_metric = (int8_t)w_value;
                    w_value = (int32_t)c_metric;

                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);

                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;

                /*CRCErrorCount*/    
                case TRANS_AGENT_MS_METRIC_ID_CRC:
                /*PDUTotalCount*/    
                case TRANS_AGENT_MS_METRIC_ID_PDU:

                    w_value = *((u_int32_t *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow MS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else
    {
        FLOG_ERROR("Source module error! src_moudle = %s\r\n", p_agent_metric->a_element_id);
    }
   
    json_object_set_new(content, "metrics", array1);  // set body elements in msg  
    json_object_set_new(content, "values", array2);  // set body elements in msg  
    
    json_object_set_new(body, "content", content);  // set body elements in msg
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string  
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;

    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "sendMetric MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);

    FLOG_INFO("send getMetricValues_res message to Agent OK! \r\n");

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit! \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_alert_msg()
* Description: Build Alert message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-19
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_alert_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    struct trans_agent_alert_info *p_alert_info = NULL;
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("alert");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *alertid = json_integer(p_alert_info->us_alarm_id);// object for ALERTID;
    json_object_set_new(body, "ALERTID", alertid);// set ALERTID;

    json_t *alertval = json_real(p_alert_info->w_alarm_value);// object for VALUE;
    json_object_set_new(body, "VALUE", alertval);// set VALUE;

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send Alert MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send alert message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_send_spectrum_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_spectrum_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t   uw_buf_len,
                        u_int8_t * p_send_buf)

{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_policy_code = 0;

    u_int8_t * p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;

    struct trans_timer_info st_timer_info;    
    u_int32_t uw_ret = 0;
    void* p_timer = NULL;
    
    //struct trans_agent_spectrum_info *p_spectrum_info, 
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    (void) p_send_buf;
    (void) uw_buf_len;
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    #ifdef TRANS_BS_COMPILE
    uw_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != uw_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }

    f_longitude = atof((char *)a_str); 

    uw_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != uw_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }

    f_latitude = atof((char *)a_str); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "GPS: %f, %f \r\n", f_longitude, f_latitude);    

    w_ret = get_global_param ("INITIAL_SENSING_POLICY", & (w_policy_code));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_POLICY error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("INITIAL_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }
    #endif
     
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    //sprintf((char *)a_str, "%d", 53);/*Msg Length£º4Bytes*/
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("getSpectrumResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(body, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *policycode = json_integer(w_policy_code);// object for POLICYCODE;
    json_object_set_new(body, "POLICYCODE", policycode);// set POLICYCODE;

    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    json_object_set_new(gps, "LATITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(body, "GPS", gps);  // set body elements in msg
    
    json_t *array = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_SENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_msg_info->p_resp_msg) + uw_index);
        json_t *value = json_real(d_sensing);
        json_array_append_new(array, value);
    }
    
    json_object_set_new(body, "SENSING", array);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        
        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length£º4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send getSpectrumResult MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send getSpectrumResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    FLOG_INFO("send getSpectrumResult message to Agent OK! \r\n");

    uw_ret = trans_transaction_set_dst(p_msg_info->p_info, 
                                0x01,
                                g_trans_agent_socket.w_agent_socket,
                                g_trans_agent_socket.a_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    /*ADD TIMER*/
    st_timer_info.f_callback = p_msg_info->f_callback;
    st_timer_info.p_data = p_msg_info->p_info;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
    st_timer_info.uw_interval = TRANS_SEND_AGENT_MSG_TIMEOUT;
    
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

    #if 0
    /*ADD TIMER*/
    gettimeofday(&st_time_val, NULL);  
    /*Timeout -- 30s*/
    st_time_val.tv_sec = st_time_val.tv_sec + TRANS_SEND_AGENT_MSG_TIMEOUT;   
    //st_time_val.tv_usec = st_time_val.tv_usec;
    
    w_ret = trans_timer_add(&st_time_val, 
                                        p_msg_info->f_callback, 
                                        g_trans_agent_socket.a_mac, 
                                        0x01, 
                                        p_msg_info->p_info,
                                        &p_timer_id);   
    if (TRANS_SUCCESS != w_ret) 
    {   
        
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
        return TRANS_FAILD;     
    } 

    
    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        gettimeofday(&st_time_val, NULL);  
        /*Timeout -- 30s*/
        st_time_val.tv_sec = st_time_val.tv_sec + TRANS_SEND_AGENT_MSG_TIMEOUT;   
        //st_time_val.tv_usec = st_time_val.tv_usec;

        w_ret = trans_timer_add(&st_time_val, 
                                            trans_agent_block_msg_timer_func, 
                                            TRANS_MOUDLE_BS, 
                                            TRANS_MOUDLE_AGENT, 
                                            0x01, 
                                            NULL,
                                            &p_timer_id);   
        if (TRANS_SUCCESS != w_ret) 
        {   
            
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;     
        } 

    
        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);

            free(p_send_msg);
            return TRANS_FAILD; 
        }    
    }
    #endif

    free(p_send_msg);
  
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_periodic_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-29
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_periodic_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t   uw_buf_len,
                        u_int8_t * p_send_buf)

{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_head = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_periodic_len = 0;
    u_int32_t uw_agent_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_periodic_msg = NULL; 
    char *p_agent_msg = NULL; 
    double  d_sensing = 0.0;
    int        w_interref = 0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_working_freq = 0;

    struct trans_periodic_sensing_info * p_periodical = NULL;

    u_int8_t * p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;
   
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_periodical = (struct trans_periodic_sensing_info *)p_msg_info->p_resp_msg;
    if ((NULL == p_periodical) 
        || (NULL == p_periodical->p_per_interref) 
        || (NULL == p_periodical->p_per_sensing))
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    #ifdef TRANS_BS_COMPILE
    w_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }
    
    f_longitude = atof((char *)a_str); 
    
    w_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }
    
    f_latitude = atof((char *)a_str); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "GPS: %f, %f \r\n", f_longitude, f_latitude);    
    
    w_ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(w_working_freq));
    if (w_ret != 0)
    {
        FLOG_ERROR ("get parameters DEFAULT_WORKING_FREQ error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("PERIODIC_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }
    #endif

    trans_agent_get_timestamp_string(a_str);
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    
    json_t *action = json_string("sendPeriodicResult");// object for ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;

    /*Creat JSON message for periodic sensing result stream */    
    json_t *periodic_msg = json_object();        // create an empty object;  

    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(periodic_msg, "TIMESTAMP", timestamp);// set timestamp
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(periodic_msg, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(periodic_msg, "DEVID", devid);// set DEVID;
    
    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(periodic_msg, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *freq = json_real(w_working_freq);// object for POLICYCODE;
    json_object_set_new(periodic_msg, "FREQ", freq);// set POLICYCODE;
    
    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    //json_object_set_new(gps, "LATITUDE", latitude);// set LATITUDE;
    json_object_set_new(gps, "ALTITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(periodic_msg, "GPS", gps);  // set body elements in msg
    
    json_t *array1 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERSENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_periodical->p_per_sensing) + uw_index);
        json_t *value1 = json_real(d_sensing);
        json_array_append_new(array1, value1);
    }
    
    json_t *array2 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERINTERREF_NUM; uw_index++)
    {
        w_interref = *(((int *)p_periodical->p_per_interref) + uw_index);
        json_t *value2 = json_real(w_interref);
        json_array_append_new(array2, value2);
    }

    json_object_set_new(periodic_msg, "PERSENSING", array1);  // set body elements in msg  
    
    json_object_set_new(periodic_msg, "PERINTERREF", array2);  // set body elements in msg  
    
    p_periodic_msg = json_dumps(periodic_msg, 0); //dump msg as a string    
    uw_periodic_len = strlen((char *)p_periodic_msg);


    /*Creat JSON message for agent */    
    json_t *agent_msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(agent_msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(agent_msg, "DEST", dest);// set DEST
    
    json_t *agent_body = json_object(); // create an empty object    
    
    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(agent_body, "TIMESTAMP", timestamp);// set timestamp

    sprintf((char *)a_str, "%d", uw_periodic_len);/*Msg Length£º4Bytes*/
    json_t *len= json_string((char *)(a_str));// object for DEST
    json_object_set_new(agent_body, "LENGTH", len);// set DEST
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(agent_body, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(agent_body, "DEVID", devid);// set DEVID;

    json_object_set_new(agent_msg, "BODY", agent_body);  // set body elements in msg
    
    p_agent_msg = json_dumps(agent_msg, 0); //dump msg as a string    
    uw_agent_len = strlen((char *)p_agent_msg);
    
    /*Creat the whole message*/
    uw_msg_len = 2 + uw_agent_len + uw_periodic_len + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        
        json_decref(agent_msg);
        json_decref(periodic_msg);
        free(p_agent_msg);
        free(p_periodic_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_head = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_head->a_magic, (char *)"FILE");   /*Msg type£º4Bytes*/
    
    sprintf((char *)p_agent_head->a_len, "%04d", uw_agent_len);/*Msg Length£º4Bytes*/
    
    memcpy(p_send_msg + SIZEOF_TRANS_AGENT_MSG, p_agent_msg, uw_agent_len);   

    memcpy(p_send_msg + SIZEOF_TRANS_AGENT_MSG + uw_agent_len, p_periodic_msg, uw_periodic_len);   
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_agent_msg);
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG + uw_agent_len), (char *)p_periodic_msg);
    
    json_decref(agent_msg);
    json_decref(periodic_msg);
    free(p_agent_msg);
    free(p_periodic_msg);
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "send sendPeriodicResult MSG: %s \r\n", p_send_msg);     
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "uw_agent_len = %d, uw_periodic_len = %d, uw_msg_len = %d. \r\n", uw_agent_len, uw_periodic_len, uw_msg_len); 
    //FLOG_INFO("send sendPeriodicResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 2, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);
    
    FLOG_INFO("send sendPeriodicResult message to Agent OK! \r\n");

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_msg()
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
u_int32_t trans_agent_send_msg(struct trans_send_msg_to_agent *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg)
{

    //int32_t w_ret = 0;
    u_int8_t  a_action[TRANS_AGENT_ACTION_SIZE+1] = {0};
    //int32_t w_agent_sockfd = 0;
    u_int32_t uw_ret = 0;

    u_int8_t      uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;   
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    //uw_len = len;
        
    strcpy((char *)(a_action), (char *)(p_msg_info->p_reqs_msg));

    if (0 == strcmp ("getMetricValues", (char *)a_action))
    {
        uw_ret = trans_agent_send_metric_msg(p_msg_info, 
                            len, 
                            p_send_msg);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_metric_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }   

        uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;   

        uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                    uc_delete_flag,
                                    TRANS_MOUDLE_BUF);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;
        }

    }
    else if (0 == strcmp ("alert", (char *)a_action))
    {
        uw_ret = trans_agent_send_alert_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_alert_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        uc_delete_flag = TRANS_TRANSACTION_FLAG_DELETE;   
    }
    else if (0 == strcmp ("spectrum", (char *)a_action))
    {
        uw_ret = trans_agent_send_spectrum_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_spectrum_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        uc_delete_flag = TRANS_TRANSACTION_FLAG_NO_DELETE;   

        uw_ret = trans_transaction_set_comn(p_msg_info->p_info, 
                                    uc_delete_flag,
                                    TRANS_MOUDLE_BUF);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;
        }

    }
    /*No transaction info-----Do not call trans_transaction_set_comn*/
    else if (0 == strcmp ("periodic", (char *)a_action))
    {
        uw_ret = trans_agent_send_periodic_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_periodic_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

    }    
    /*No transaction info-----Do not call trans_transaction_set_comn*/
    else if (0 == strcmp ("topologyUpdate", (char *)a_action))
    {
        uw_ret = trans_agent_send_t_update_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_t_update_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }    
    else if (0 == strcmp ("confUpdate", (char *)a_action))
    {
        uw_ret = trans_agent_send_conf_update_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_send_conf_update_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }

    else
    {
        FLOG_ERROR("action/ACTION error! action = %s .\r\n", a_action);
        
        return TRANS_FAILD;
    }
        
    #if 0
    //trans_debug_msg_print(p_send_msg, 40, g_trans_debug_agent);

    w_agent_sockfd = g_trans_agent_socket.w_agent_socket;
    
    w_ret = send(w_agent_sockfd, p_send_msg, uw_len, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    FLOG_INFO("send Agent message OK! \r\n");

    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        struct timeval st_time_val;
        gettimeofday(&st_time_val, NULL);  
        
        /*Timeout -- 20s*/
        st_time_val.tv_sec = st_time_val.tv_sec + 20;   
        //st_time_val.tv_usec = st_time_val.tv_usec;
        
        void* p_timer_id = NULL;
        struct trans_timer_msg_info st_msg_info;
        
        st_msg_info.us_serial_number = 0xffff;
        st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
        st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
        st_msg_info.f_callback = trans_agent_block_msg_timer_func;
        st_msg_info.p_user_info = NULL;
        
        w_ret = trans_timer_add(&st_time_val, 
                                trans_agent_block_msg_timer_func, 
                                p_send_msg, 
                                uw_len + SIZEOF_TRANS_AGENT_MSG, 
                                &st_msg_info,
                                &p_timer_id);
        
        if (TRANS_SUCCESS != w_ret) 
        {   
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;     
        } 

        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);
            return TRANS_FAILD; 
        }

    }
    #endif
    
    return TRANS_SUCCESS;
    
}

#endif

/*****************************************************************************+
* Function: trans_agent_get_metric_id()
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
u_int32_t trans_agent_get_metric_id(const char * p_metric, int32_t *p_metric_id)
{
    if ((NULL == p_metric) ||(NULL == p_metric_id))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    /*BS metric*/
    if (0 == strcmp ("RangingSuccessCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_RANG;
    }
    #ifdef TRANS_BS_COMPILE
    else if (0 == strcmp ("PerWmbDLThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_WMB_DL;
    }
    else if (0 == strcmp ("PerWmbULThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_WMB_UL;
    }
    else if (0 == strcmp ("PerConnDLThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN_DL;
    }
    else if (0 == strcmp ("PerConnULThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN_UL;
    }
    else if (0 == strcmp ("ULMgmtThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_UL_MANG;
    }    
    else if (0 == strcmp ("DLMgmtThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_DL_MANG;
    }       
    else if (0 == strcmp ("ConnectionCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN;
    }   
    else if (0 == strcmp ("CRCErrorCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CRC;
    }  
    else if (0 == strcmp ("PDUTotalCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_PDU;
    }  
    #endif

    #ifdef TRANS_RRH_COMPILE
    /*RRH metric*/
    else if (0 == strcmp ("PwrAmplTemp", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_PAT;
    }
    else if (0 == strcmp ("DL_OutputPwr1", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_PWR;    
    }
    else if (0 == strcmp ("DL_OutputPwr2", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_PWR;    
    }
    else if (0 == strcmp ("DL_VSWR1", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR;    
    }    
    else if (0 == strcmp ("DL_VSWR2", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR;    
    }  
    #endif

    #ifdef TRANS_MS_COMPILE
    /*MS metric*/
    else if (0 == strcmp ("RSSI", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_RSSI;
    }   
    else if (0 == strcmp ("SINR", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_SINR;    
    }   
    else if (0 == strcmp ("TxPower", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_TX_PWR;    
    }   
    else if (0 == strcmp ("Temperature", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_T;    
    }   
    else if (0 == strcmp ("CRCErrorCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_CRC;    
    }  
    else if (0 == strcmp ("PDUTotalCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_PDU;
    }  
    #endif
    else
    {
        FLOG_ERROR("p_metric error! p_metric = %s .\r\n", p_metric);
        
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "p_metric: %s, *p_metric_id; %d \r\n", p_metric, *p_metric_id);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_get_rrh_id()
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
u_int32_t trans_agent_get_rrh_id(const char * p_metric, u_int16_t *p_rrh_id)
{
    if ((NULL == p_metric) ||(NULL == p_rrh_id))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    /*RRH metric*/
    if (0 == strcmp ("PwrAmplTemp", (char *)p_metric))
    {

        *p_rrh_id = (RRH_MSG_PA_TEMP_VALUE);

    }
    else if (0 == strcmp ("DL_OutputPwr1", (char *)p_metric))
    {

        *p_rrh_id = (RRH_MSG_DL_INPUT1_LEVEL);
    
    }
    else if (0 == strcmp ("DL_OutputPwr2", (char *)p_metric))
    {

        *p_rrh_id = (RRH_MSG_DL_INPUT2_LEVEL);
    
    }
    else if (0 == strcmp ("DL_VSWR1", (char *)p_metric))
    {

        *p_rrh_id = (RRH_MSG_DL_VSWR1_VALUE);
    
    }
    else if (0 == strcmp ("DL_VSWR2", (char *)p_metric))
    {

        *p_rrh_id = (RRH_MSG_DL_VSWR2_VALUE);
    
    }
    else
    {
        FLOG_ERROR("p_metric error! p_metric = %s .\r\n", p_metric);
        
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "p_metric: %s, *p_rrh_id; 0x%x \r\n", p_metric, *p_rrh_id);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_get_metric_info()
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
u_int32_t trans_agent_get_metric_info(json_t *msg,  
                                    struct trans_agent_metric_info *p_metric_info)
{
    const char * p_tr = NULL;
    u_int32_t     uw_num = 0;
    u_int8_t  uc_index = 0;
    u_int32_t uw_ret = 0;
    
    if ((NULL == msg) ||(NULL == p_metric_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    /*Get SOURCE*/
    json_t *source = json_object_get(msg, "SOURCE"); // get SOURCE
    if (NULL == source)
    {
        FLOG_ERROR("Call json_object_get for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(source);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_metric_info->w_source_id = atoi((char *)(p_tr)); 
    
    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "w_source_id =0x%x, source: %s. \r\n", p_metric_info->w_source_id, p_tr);    

    /*Get DEST*/
    json_t *dest = json_object_get(msg, "DEST"); // get SOURCE
    if (NULL == source)
    {
        FLOG_ERROR("Call json_object_get for DEST  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(dest);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for DEST  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_metric_info->w_dest_id = atoi((char *)(p_tr)); 

    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "w_dest_id =0x%x, dest: %s. \r\n", p_metric_info->w_dest_id, p_tr);    
        
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    json_t *id= json_object_get(body, "id");// get metric_id
    if (NULL == id)
    {
        FLOG_ERROR("Call json_object_get for id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    p_metric_info->w_msg_id = json_integer_value(id);
    if (0 == p_metric_info->w_msg_id)
    {
        FLOG_ERROR("Call json_integer_value for w_msg_id error! .\r\n");
        
        return TRANS_FAILD;
    }
    
    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "w_msg_id = %d. \r\n", p_metric_info->w_msg_id);

    json_t *module_id= json_object_get(body, "module_id");// get metric_id
    if (NULL == id)
    {
        FLOG_ERROR("Call json_object_get for module_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(module_id);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for module_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    strcpy((char *)(p_metric_info->a_module_id), p_tr);
    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "a_module_id =%s. \r\n", p_metric_info->a_module_id);

    //p_metric_info->w_module_id = atoi((char *)(p_tr)); 

    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "w_module_id =0x%x, module_id: %s. \r\n", p_metric_info->w_module_id, p_tr);    
 
    json_t *content = json_object_get(body, "content"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for content error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    json_t *elementid = json_object_get(content, "element_id");// get metric_id
    if (NULL == elementid)
    {
        FLOG_ERROR("Call json_object_get for element_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    p_tr = json_string_value(elementid);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for element_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    strcpy((char *)(p_metric_info->a_element_id), p_tr);

    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "a_element_id =%s. \r\n", p_metric_info->a_element_id);

    json_t *array= json_object_get(content, "metrics");// get metric_id
    if (NULL == array)
    {
        FLOG_ERROR("Call json_object_get for array error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    uw_num = json_array_size(array);
    #if 0
    if (1 != uw_num)
    {
        FLOG_ERROR("Call json_array_size for array error! uw_num = %d.\r\n", uw_num);
        
        return TRANS_FAILD;
    }
    #endif

    p_metric_info->uc_metric_num = uw_num;

    for (uc_index = 0; uc_index < uw_num; uc_index++)
    {
        json_t *metrics = json_array_get(array, uc_index);
        if (NULL == metrics)
        {
            FLOG_ERROR("Call json_array_get for metrics error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
        
        p_tr = json_string_value(metrics);
        if (NULL == p_tr)
        {
            FLOG_ERROR("Call json_string_value for metrics  error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
        
        strcpy((char *)(p_metric_info->a_metrics[uc_index]), p_tr);

        FLOG_DEBUG_TRANS(g_trans_debug_agent, "a_metrics[%d] =%s. \r\n", uc_index, p_metric_info->a_metrics[uc_index]);   
        
        uw_ret = trans_agent_get_metric_id((char *)p_metric_info->a_metrics[uc_index], 
                        &(p_metric_info->a_metric_id[uc_index]));
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_get_metric_id error! uw_ret = %d.\r\n", uw_ret);
            
            return TRANS_FAILD;
        }

    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_get_spectrum_info()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-21
* 
+*****************************************************************************/
u_int32_t trans_agent_get_spectrum_info(json_t *msg,  
                    struct trans_agent_spectrum_resp_info *p_sectrum_resp_info)
{
    //int32_t w_ret = 0;
    //const char * p_tr = NULL;
    u_int32_t uw_index = 0;
    //u_int32_t uw_ret = 0;
    
    //int32_t w_source_id =0;
    float f_freq = 0;
    int   a_interref[TRANS_AGENT_INTERREF_NUM] = {0};     /*1* 21 int*/
    //char a_interref_str[TRANS_AGENT_INTERREF_NUM + 1] = {0};  /*1* 22 int  -> 22 char*/
    int w_error_code = 0;
  
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    if ((NULL == msg) ||(NULL == p_sectrum_resp_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    #if 0
    /*Get SOURCE*/
    json_t *source = json_object_get(msg, "SOURCE"); // get SOURCE
    if (NULL == source)
    {
        FLOG_ERROR("Call json_object_get for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(source);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for SOURCE  error! NULL PTR  .\r\n");

        return TRANS_FAILD;
    }
    
    w_source_id = atoi((char *)(p_tr)); 
    #endif
    
    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "w_source_id =0x%x, source: %s. \r\n", w_source_id, p_tr);    
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");

        return TRANS_FAILD;
    }
    
    /*ERRORCODE*/
    json_t *errorcode= json_object_get(body, "ERRORCODE");// get metric_id
    if (NULL == errorcode)
    {
        FLOG_ERROR("Call json_object_get for errorcode  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    w_error_code = json_integer_value(errorcode);

    p_sectrum_resp_info->w_error_code = w_error_code;
    
    /*The result is faild*/
    if (0 != w_error_code)
    {
        FLOG_ERROR("Spectrum errorcode = %d error!.\r\n", w_error_code);
        
        return TRANS_SUCCESS;
    }

    json_t *freq= json_object_get(body, "FREQ");// get metric_id
    if (NULL == freq)
    {
        FLOG_ERROR("Call json_object_get for freq  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    f_freq = (float)json_number_value(freq);
    if (0.0 == f_freq)
    {
        FLOG_ERROR("Call json_number_value for freq error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_sectrum_resp_info->w_f_freq = (int)(f_freq*1000);
    
    FLOG_INFO("Rev DEFAULT_WORKING_FREQ %d\n", p_sectrum_resp_info->w_f_freq);
    
    json_t *array= json_object_get(body, "SENSING");// get metric_id
    if (NULL == array)
    {
        FLOG_ERROR("Call json_object_get for array  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    for (uw_index = 0; uw_index < TRANS_AGENT_INTERREF_NUM; uw_index++)
    {
        json_t *value = json_array_get(array, uw_index);
        if (NULL == value)
        {
            FLOG_ERROR("Call json_array_get for value error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }        
    
        /*It could be 0*/
        a_interref[uw_index] = json_integer_value(value);
    
        //FLOG_DEBUG_TRANS(g_trans_debug_agent, "a_interref[%d] = %d. \r\n", uw_index, a_interref[uw_index]);
    
        sprintf((char *)p_sectrum_resp_info->a_interref_str, "%s%d", 
                    p_sectrum_resp_info->a_interref_str, a_interref[uw_index]);
    }

    FLOG_INFO("Rev. INTERFERENCE_ACTIVE %s\n", p_sectrum_resp_info->a_interref_str);
  
    return TRANS_SUCCESS;
}

#ifdef TRANS_RRH_COMPILE

/*****************************************************************************+
* Function: trans_agent_forward2_query_rrh()
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
u_int32_t trans_agent_forward2_query_rrh(void *p_info,
                           u_int8_t  uc_num,
                           void    *p_type)
{
    u_int32_t uw_ret = 0;

    struct trans_send_msg_to_rrh   *p_rrh = NULL;

    if ((NULL == p_info) ||(NULL == p_type))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    st_build_rrh.p_tag = p_type;
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
    
    p_rrh = (struct trans_send_msg_to_rrh *)malloc(SIZEOF_TRANS_SEND_MSG_TO_RRH);

    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR!p_query_rrh \r\n");
        free(p_rrh_payload);
        return TRANS_FAILD;
    }
    
    p_rrh->f_callback = trans_agent_query_rrh_func;
    
    p_rrh->p_info = p_info;
    p_rrh->uw_payload_len = uw_rrh_len;
    p_rrh->p_payload = p_rrh_payload;
    p_rrh->uc_type = RRH_MONITOR_TYPE_QUERY;

    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH, p_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        //return TRANS_FAILD;    
    } 

    free(p_rrh_payload);
    free (p_rrh);

    return uw_ret;    
}


/*****************************************************************************+
* Function: trans_agent_forward2_cfg_rrh()
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
u_int32_t trans_agent_forward2_cfg_rrh(void *p_info,
                           u_int8_t  uc_num,
                           void    *p_type,
                           void    *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_send_cfg_to_rrh st_cfg_rrh;
    
    struct trans_send_msg_to_rrh   *p_rrh = NULL;

    if ((NULL == p_info) ||(NULL == p_type) ||(NULL == p_value))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    struct trans_rrh_build_payload_info st_build_rrh;
    u_int32_t uw_rrh_len = 0;
    u_int8_t *  p_rrh_payload = NULL;
    
    st_build_rrh.p_tag = p_type;
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
    
    p_rrh = (struct trans_send_msg_to_rrh *)malloc(SIZEOF_TRANS_SEND_MSG_TO_RRH);
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR!p_query_rrh \r\n");
        free(p_rrh_payload);
        return TRANS_FAILD;
    }
    
    p_rrh->f_callback = trans_agent_cfg_rrh_func;
    
    p_rrh->p_info = p_info;
    p_rrh->uw_payload_len = uw_rrh_len;
    p_rrh->p_payload = p_rrh_payload;
    p_rrh->uc_type = RRH_MONITOR_TYPE_CONFIG;

    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH, &st_cfg_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        //return TRANS_FAILD;    
    } 
    free(p_rrh_payload);
    free (p_rrh);
    
    return uw_ret;    
}

/*****************************************************************************+
* Function: trans_agent_rev_query_rrh()
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
u_int32_t trans_agent_rev_query_rrh(void *p_info, 
                           size_t len,
                           void * p_metric)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;
    u_int16_t  a_rrh_type[TRANS_AGENT_METRIC_MAX_NUM] = {0};

    struct trans_agent_metric_info *p_metric_info = NULL;
    
    len =  len;
        
    if ((NULL == p_info) ||(NULL == p_metric))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_metric_info = ( struct trans_agent_metric_info *)p_metric;
    
    uc_num = p_metric_info->uc_metric_num;

    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        uw_ret = trans_agent_get_rrh_id((char *)p_metric_name, &(a_rrh_type[uc_index]));
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_get_rrh_id error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;    
        } 
    }
        
    uw_ret = trans_agent_forward2_query_rrh(p_info, uc_num, a_rrh_type);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_forward2_query_rrh error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 
    
    return TRANS_SUCCESS;  
}
#endif

/*****************************************************************************+
* Function: trans_agent_check_msg_content()
* Description: Check the header parameters in the msg
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-05
* 
+*****************************************************************************/
u_int32_t trans_agent_check_msg_content(void *p_msg, char *p_action)
{
    //u_int32_t uw_ret = 0;
    //const char * p_dest = NULL;
    const char * p_action_t = NULL;
    json_t *msg = NULL;

    if ((NULL == p_msg) ||(NULL == p_action))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    msg = (json_t *)p_msg;
    /*DEST  check*/
    json_t *dest = json_object_get(msg, "DEST"); // get DEST
    if (NULL == dest)
    {
        FLOG_ERROR("Call json_object_get for DEST  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    #if 0
    p_dest = json_string_value(dest);
    if (0 != strcmp ((char *)g_trans_agent_config_info.a_device_id, (char *)p_dest))
    {
        FLOG_ERROR("Device_id error! device_id = %s .\r\n", p_dest);
        
        return TRANS_FAILD;
    }
    #endif
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    json_t *action = json_object_get(body, "ACTION"); // get action    
    if (NULL == action)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_agent, "ACTION action\r\n");  
        
        //json_t *action = json_object_get(body, "action"); // get ACTION  
        action = json_object_get(body, "action"); // get ACTION  
        if (NULL == action)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_agent, "Call json_object_get for action/ACTION  error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
    }
    
    p_action_t = (char *)json_string_value(action);
    if (NULL == p_action_t)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_agent, "Call json_string_value for action/ACTION  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    strcpy(p_action, p_action_t);
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_func_metric()
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
int trans_agent_func_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_metric_info  *p_metric_info = NULL;
       
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");

    (void)len;

    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_metric_info = (struct trans_agent_metric_info *)p_rev_buf;

    #ifdef TRANS_RRH_COMPILE
    if (0 == strcmp ("rrh", (char *)p_metric_info->a_element_id))
    {
        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) && (g_enable_metric == 1) )
        {
            uw_ret = trans_agent_rev_query_rrh(p_info, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_rev_query_rrh error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }            
        }
        else
        {
            /*********Test**********/
            struct trans_send_msg_to_agent st_msg_info;

            u_int8_t  *p_param = (u_int8_t *)malloc((p_metric_info->uc_metric_num * 4));

            memset((u_int8_t*)p_param, 0, p_metric_info->uc_metric_num * 4); 
            
            st_msg_info.f_callback = NULL;
            st_msg_info.p_reqs_msg = (struct trans_agent_metric_info *)malloc (sizeof (struct trans_agent_metric_info));
            memcpy(st_msg_info.p_reqs_msg, p_metric_info, (sizeof (struct trans_agent_metric_info)));

            st_msg_info.p_resp_msg = p_param;
            st_msg_info.p_info = p_info;
            
            uw_ret = trans_agent_send_msg(&st_msg_info, 0, NULL);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_send_msg error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }

            free(p_param);
            
            /*********Test**********/

        }    
        
        return TRANS_SUCCESS;
    }
    #endif
    #ifdef TRANS_BS_COMPILE
    if (0 == strcmp ("bs", (char *)p_metric_info->a_element_id))
    {
        //uw_ret = trans_agent_rev_query_bs(&st_metric_info, 4096, p_send_msg);
        uw_ret = trans_wireless_action_bs_metric(p_info, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_query_bs error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        return TRANS_SUCCESS;
    }
    #endif
    #ifdef TRANS_MS_COMPILE
    if (0 == strcmp ("cpe", (char *)p_metric_info->a_element_id))
    {
        //uw_ret = trans_agent_rev_query_ms(&st_metric_info, 4096, p_send_msg);
        uw_ret = trans_wireless_action_ms_metric(p_info, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_wireless_action_ms_metric error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        return TRANS_SUCCESS;
    }
    #endif

    FLOG_ERROR("Source module %s error \r\n", p_metric_info->a_element_id);

   
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_FAILD;
}

/*****************************************************************************+
* Function: trans_agent_func_spectrum()
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
int trans_agent_func_spectrum(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_spectrum_resp_info *p_spectrum_resp = NULL;
        
    u_int8_t  uc_flag = TRANS_SET_SENSING_RESULT_FAKE;
    //json_t *msg = NULL;

    //u_int8_t uc_find_flag = 0;
    //u_int16_t  us_no = 0x01;
    //void* p_info_old = NULL;

    int32_t w_result = 0;
    fun_callback f_callback = NULL; 

    struct trans_send_msg_to_agent st_msg_info;
    struct trans_agent_alert_info        st_alert_info;
    #ifdef TRANS_BS_COMPILE
    struct trans_agent_confupdate_info  st_confupdate_info;
    #endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");

    (void) len;
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_spectrum_resp = (struct trans_agent_spectrum_resp_info *)p_rev_buf;

    /*Get result*/
    w_result = trans_transaction_get_result(p_info);

    if (TRANS_SUCCESS == w_result)
    {
        if (0 != p_spectrum_resp->w_error_code)
        {
            FLOG_ERROR("Send spectrum alert to Agent.\r\n");
            
            w_result = TRANS_FAILD;
        
            /*Send Alert to Agent*/
            st_alert_info.us_alarm_id = (u_int16_t)(0x0300 + p_spectrum_resp->w_error_code);
            st_alert_info.w_alarm_value = 0;
            
            st_msg_info.f_callback = NULL;
            st_msg_info.p_resp_msg = &st_alert_info;
            st_msg_info.p_reqs_msg = "alert";
            st_msg_info.p_info = p_info;
            
            uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
            
                //st_en_quene.uc_result = TRANS_SUCCESS;
            }
        }
        /*The result is sucessful*/
        else
        {
            //#if 0
            if (0 == uc_flag)
            {
                #ifdef TRANS_BS_COMPILE
                uw_ret = set_global_param("DEFAULT_WORKING_FREQ", (void *)&(p_spectrum_resp->w_f_freq));
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call set_global_param for FREQ error");
                    
                    w_result = TRANS_FAILD;
                
                }
                
                //FLOG_DEBUG_TRANS(g_trans_debug_agent, "a_interref_str = %s. \r\n", a_interref_str);
                uw_ret = set_global_param("INTERFERENCE_ACTIVE", (void *)p_spectrum_resp->a_interref_str);
                
                FLOG_INFO("Rev. INTERFERENCE_ACTIVE %s\n", p_spectrum_resp->a_interref_str);
                
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call set_global_param for INTERFERENCE error");
                    
                    w_result = TRANS_FAILD;
                }
        
                /*Send Configuration update to Agent*/
                st_confupdate_info.f_freq = (((float)(p_spectrum_resp->w_f_freq))/1000);
               
                //FLOG_INFO("send  f_freq : %f %d\n", st_confupdate_info.f_freq, p_spectrum_resp->w_f_freq);
                
                st_msg_info.f_callback = NULL;
                st_msg_info.p_resp_msg = &st_confupdate_info;
                st_msg_info.p_reqs_msg = "confUpdate";
                st_msg_info.p_info = p_info;
                
                uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
                /*Error*/
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
                
                    w_result = TRANS_FAILD;
                }            
        
                #endif
            }
        
            //#endif
        }   
        
        uw_ret = trans_transaction_set_result(p_info, w_result);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_result_set_transaction error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;    
        }
    }   
           
    //f_callback = trans_transaction_get_agent(p_info);
    f_callback = g_trans_register_exe_func[TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 3].f_callback;
    if (NULL == f_callback)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_agent, "NULL PTR! f_callback\r\n");
    }
    else
    {
        (*(f_callback))(p_info, 0, NULL);
    }

    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BS);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_func_idupdate()
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
int trans_agent_func_idupdate(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    //int32_t w_result = 0;

    struct trans_agent_idupdate_info *p_idupdate = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    (void)len;
    
    if ((NULL == p_info) ||(NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    /*Get result*/
    #if 0
    w_result = trans_transaction_get_result(p_info);

    if (TRANS_SUCCESS != w_result)
    {
        FLOG_ERROR("Call trans_transaction_get_result error! w_result = %d\r\n", w_result);

        return TRANS_FAILD;
    }
    #endif

    p_idupdate = (struct trans_agent_idupdate_info *)p_rev_buf;
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    strcpy((char *)(g_trans_agent_config_info.a_device_id), 
        (char *)p_idupdate->a_device_id);

    FLOG_WARNING("Update Devid %s\r\n", g_trans_agent_config_info.a_device_id);
     
    /*Send State Change message to Agent*/
    uw_ret = trans_agent_send_state_msg();   
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
    
        FLOG_ERROR("Call trans_agent_send_state_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    } 

    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_AGENT);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_parse_metric()
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
u_int32_t trans_agent_parse_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_metric_info  *p_metric_info = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    

    json_t *msg = NULL;
    
    void ** p_info_tmp = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    (void)len;
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    msg = (json_t *)p_rev_buf;
    
    p_metric_info = (struct trans_agent_metric_info *)malloc(SIZEOF_TRANS_AGENT_METRIC_INFO);
        
    if (NULL == p_metric_info)
    {
        FLOG_ERROR("malloc p_metric_info error! \r\n");
        return TRANS_FAILD;   
    }

    memset(p_metric_info, 0, SIZEOF_TRANS_AGENT_METRIC_INFO);
    
    uw_ret = trans_agent_get_metric_info(msg, p_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_get_metric_info error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    strcpy((char *)(p_metric_info->a_action), "getMetricValues");

    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        /*???????If  error , Do what?????*/  
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_AGENT_MSG_PRO + 1;     /*Funtion Callback ID*/  

    p_info_tmp = p_info;

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                SIZEOF_TRANS_AGENT_METRIC_INFO,
                                p_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
   
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_parse_spectrum()
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
u_int32_t trans_agent_parse_spectrum(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_spectrum_resp_info *p_spectrum_resp = NULL;
        
    json_t *msg = NULL;
    
    u_int32_t  uw_no = 0x01;
    
    void ** p_info_tmp = NULL;

    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    
    
    int32_t w_result = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    (void) len;
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
       
    msg = (json_t *)p_rev_buf;
    
    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_out_by_mac(uw_no,
                                                     g_trans_agent_socket.a_mac,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }

    p_spectrum_resp = (struct trans_agent_spectrum_resp_info *)malloc(SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO);
        
    if (NULL == p_spectrum_resp)
    {
        FLOG_ERROR("malloc p_spectrum_resp error! \r\n");

        w_result = TRANS_FAILD;
    }
    
    memset((u_int8_t*)p_spectrum_resp, 0, SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO);
    
    uw_ret = trans_agent_get_spectrum_info(msg, p_spectrum_resp);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_get_spectrum_info error! uw_ret = %d\r\n", uw_ret);
    
        w_result = TRANS_FAILD;
    }

    uw_ret = trans_transaction_set_result(*p_info_tmp, w_result);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_result_set_transaction error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_AGENT_MSG_PRO + 2;     /*Funtion Callback ID*/  

    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO,
                                p_spectrum_resp);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
   
    //#if 0
    uw_ret = trans_transaction_set_user(*p_info_tmp, 
                            SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO, 
                            p_spectrum_resp);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    //#endif
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_parse_idupdate()
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
u_int32_t trans_agent_parse_idupdate(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    const char * p_tr = NULL;
    u_int32_t uw_ret = 0;
    json_t *msg = NULL;
    
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/   
    
    void ** p_info_tmp = NULL;

    struct trans_agent_idupdate_info *p_idupdate = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    (void)len;
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    msg = (json_t *)p_rev_buf;
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
    
        return TRANS_FAILD;
    }
    
    json_t *dev_id = json_object_get(body, "DEVID");// get DEVID
    if (NULL == dev_id)
    {
        FLOG_ERROR("Call json_object_get for dev_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(dev_id);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for dev_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    if (TRANS_AGENT_MODULE_ID_SIZE < strlen(p_tr))
    {
        FLOG_ERROR("dev_id  length %d error!\r\n", strlen(p_tr));
        
        return TRANS_FAILD;
    }
    else
    {
        p_idupdate = (struct trans_agent_idupdate_info *)malloc(SIZEOF_TRANS_AGENT_IDUPDATE_INFO);
            
        if (NULL == p_idupdate)
        {
            FLOG_ERROR("malloc p_idupdate error! \r\n");
        
            return TRANS_FAILD;
        }
        
        memset((u_int8_t*)p_idupdate, 0, SIZEOF_TRANS_AGENT_IDUPDATE_INFO);
        
        strcpy((char *)(p_idupdate->a_device_id), p_tr);
    } 
  
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for monitor error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    p_info_tmp = p_info;

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_AGENT_MSG_PRO + 3;     /*Funtion Callback ID*/  
    
    /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                SIZEOF_TRANS_AGENT_IDUPDATE_INFO,
                                p_idupdate);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    uw_ret = trans_transaction_set_user(*p_info_tmp, SIZEOF_TRANS_AGENT_IDUPDATE_INFO, p_idupdate);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_user error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
        
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_parse_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_agent_parse_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct trans_agent_msg *p_agent_rev_msg = NULL;
    char p_action[TRANS_AGENT_ACTION_SIZE + 1] = {0};
    json_error_t error;    
    u_int32_t  uw_json_len = 0;
    u_int32_t uw_ret = 0;

    u_int8_t * p_rev_msg = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");

    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    (void)len;
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "1 p_rev_buf = %p! \r\n", p_rev_buf);

    p_rev_msg = (u_int8_t *)p_rev_buf;
    p_agent_rev_msg = (struct trans_agent_msg *)p_rev_buf;

    if (0 != strncmp ("MESS", (char *)p_agent_rev_msg->a_magic, (int)4))
    {
        FLOG_ERROR("Magic error! a_magic = %s .\r\n", p_agent_rev_msg->a_magic);
        
        return TRANS_FAILD;
    }

    /*Length check ????*/    
    //FLOG_INFO("json: %s \r\n", p_rev_msg);    
    uw_json_len = atoi((char *)(p_agent_rev_msg->a_len)); 
    //FLOG_INFO("json len : %d \r\n", uw_json_len);
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "json len : %d, json: %s \r\n", uw_json_len, p_rev_msg);

    json_t *msg = json_loads((char *)(p_rev_msg+SIZEOF_TRANS_AGENT_MSG), &error); //decodes the json string and returns the object it contains
    if (NULL == msg)
    {
        FLOG_ERROR("Call json_loads error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    uw_ret = trans_agent_check_msg_content(msg, p_action);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_check_msg_content error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "action: %s \r\n", p_action);    
    
    if (0 == strcmp ("getMetricValues", (char *)p_action))
    {
        uw_ret = trans_agent_parse_metric(p_info, uw_json_len, msg);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_metric_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    else if (0 == strcmp ("sendSpectrumResult", (char *)p_action))
    {
        uw_ret = trans_agent_parse_spectrum(p_info, uw_json_len, msg);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_spectrum_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    //#ifdef TRANS_MS_COMPILE
    else if (0 == strcmp ("idUpdate", (char *)p_action))
    {
        uw_ret = trans_agent_parse_idupdate(p_info, uw_json_len, msg);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_idupdate_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    //#endif
    else
    {
        FLOG_ERROR("action/ACTION error! action = %s .\r\n", p_action);
        
        uw_ret = TRANS_FAILD;
    }

    json_decref(msg); 

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit uw_ret = %d,\r\n", uw_ret);
    
    return uw_ret;

}

/*****************************************************************************+
* Function: trans_agent_rev_msg()
* Description: Revice Message From Socket
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_msg(u_int8_t **pp_rev_msg, int32_t w_agent_socket, int32_t *p_len)
{
    int32_t w_json_len = 0;
    int32_t w_msg_len = 0;    
    int w_len_tmp = 0;
    int32_t w_rev_len = 0;  
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;

    char a_agent_header[SIZEOF_TRANS_AGENT_MSG + 1] = {0};
    
    u_int32_t uw_ret = 0;
    
    *pp_rev_msg = NULL;

    w_len_tmp = SIZEOF_TRANS_AGENT_MSG;
    
    /*Rev JSON msg*/
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_agent_socket, 
                                a_agent_header + (SIZEOF_TRANS_AGENT_MSG - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            #if 0
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
            return TRANS_FAILD;
            #endif
            
            TRANS_COMMON_SOCKFD_ERROR(w_agent_socket, errno, uw_ret);
            
            TRANS_COMMON_SOCKFD_PRINT(w_agent_socket, w_rev_len1, errno);
            
            FLOG_ERROR("Call recv for length error!\r\n");
            
            return uw_ret;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len1;    
        w_rev_len = w_rev_len + w_rev_len1;
    }

    a_agent_header[SIZEOF_TRANS_AGENT_MSG] = '\0';

    /*trans_debug_msg_print(a_agent_header, SIZEOF_TRANS_AGENT_MSG + 1, 1);
    FLOG_ERROR("*******%s. \r\n", a_agent_header);
    */

    if (SIZEOF_TRANS_AGENT_MSG != w_rev_len)
    {
        FLOG_ERROR("Receive Agent Message Header Length error! header_len  = %d, rev_len  = %d\r\n", SIZEOF_TRANS_AGENT_MSG, w_rev_len);
        return TRANS_FAILD;
    }
            
    w_json_len = atoi((char *)(a_agent_header + 4));
    /*len check*/
    if (10 >= w_json_len)
    {
        FLOG_ERROR("recv agent msg length error! w_json_len = %d\r\n", w_json_len);

        return TRANS_FAILD;
    }
    
    w_rev_len = 0;
    w_len_tmp = 0;
    
    w_msg_len = w_json_len + SIZEOF_TRANS_AGENT_MSG + 1;
    
    *pp_rev_msg = (u_int8_t *)malloc (w_msg_len);
        
    if (NULL == *pp_rev_msg)
    {
        FLOG_ERROR("Malloc pp_rev_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "0 *pp_rev_msg  = %p! \r\n", *pp_rev_msg);    
    
    memset((u_int8_t*)*pp_rev_msg, 0, w_msg_len);
    memcpy((u_int8_t*)*pp_rev_msg, a_agent_header, SIZEOF_TRANS_AGENT_MSG);

    w_len_tmp = w_json_len;
    
    /*Rev JSON msg*/
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_agent_socket, 
                                *pp_rev_msg + (w_json_len - w_len_tmp) + SIZEOF_TRANS_AGENT_MSG,  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            #if 0
            //free (*pp_rev_msg);            
            FLOG_ERROR("Receivev Agent message error! w_rev_len2 = %d\r\n", w_rev_len2);
            return TRANS_FAILD;
            #endif

            TRANS_COMMON_SOCKFD_ERROR(w_agent_socket, errno, uw_ret);

            TRANS_COMMON_SOCKFD_PRINT(w_agent_socket, w_rev_len2, errno);
            
            FLOG_ERROR("Call recv for message error!\r\n");

            return uw_ret;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len2;  
        w_rev_len = w_rev_len + w_rev_len2;
    }
    
     if (w_json_len != w_rev_len)
    {
        FLOG_ERROR("Receivev Agent Message Length error! json_len  = %d, rev_len  = %d\r\n", w_json_len, w_rev_len);
        return TRANS_FAILD;
    }   
     
    *p_len = w_msg_len;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Rev Agent Msg OK w_rev_len = %d. \r\n", *p_len);

    trans_debug_msg_print(*pp_rev_msg, 40, g_trans_debug_agent);
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_tcp_socket()
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
u_int32_t trans_agent_tcp_socket()
{
    int32_t w_agent_socket = 1;

    int32_t w_ret = 0;
    struct sockaddr_in st_peer_addr; 
    //struct sockaddr_in st_client_addr; 
    socklen_t sin_size = sizeof(struct sockaddr_in);    

    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;

    struct timeval st_time_val;

    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Enter \r\n");
   
    /* Create an IPv4 Internet Socket */
    w_ret = w_agent_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    if (w_ret < 0)
    {    
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;    
    }
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot,
            sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;
     }
    
    
    /*Set the length of the REV buffer*/
    w_recvbuflen = TRANS_AGENT_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;
    }
    
    /*Set the length of the revice buffer*/
    w_sendbuflen = TRANS_AGENT_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;    
    }
    
    /* Zero out structure */
    memset(&st_peer_addr, 0, sizeof(st_peer_addr));    
    /* Create an AF_INET address */
    st_peer_addr.sin_family = AF_INET;     
    st_peer_addr.sin_port = TRANS_HTONS(g_trans_agent_config_info.us_agent_tcp_port); 
    //st_client_addr.sin_addr.s_addr = g_ad_agent_server_socket.uw_ipAddr; 
    //st_peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    st_peer_addr.sin_addr.s_addr = g_trans_agent_config_info.uw_agent_ip_addr; 

    /*Connect  persistent */
    w_ret = connect(w_agent_socket, (struct sockaddr * )&st_peer_addr, sin_size); 
    
    //if ((w_ret < 0) && (0 == g_trans_agent_config_info.uc_connect_num))
    if (w_ret < 0)
    {
        FLOG_ERROR("Call connect error! w_ret = %d, w_agent_socket = %d.\r\n", w_ret, w_agent_socket);

        close(w_agent_socket);
        return TRANS_FAILD;
    }
    
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 60000;  /*60ms*/
    
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_RCVTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_agent_socket);
        return TRANS_FAILD;    
    }
    
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 6000; /*6ms*/
    
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_SNDTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_agent_socket);
        return TRANS_FAILD;
    }
    
    pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));                   
    g_trans_agent_socket.w_agent_socket = w_agent_socket;                     
    pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex)); 
    
    /*Add device List-----but not active*/
    struct trans_device_info st_device_info;
    void * p_device = NULL;
    
    st_device_info.uc_module_type = TRANS_MOUDLE_AGENT;
    st_device_info.w_sockfd = w_agent_socket;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    
    memset(st_device_info.a_mac, 0, TRANS_MAC_ADDR_LEN);
    
    w_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != w_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", w_ret);
        return w_ret;
    }

    //FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exist agent_socket = %d\r\n", w_agent_socket); 
    
    return w_ret;
}

/*****************************************************************************+
* Function: trans_agent_register_func()
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
u_int32_t trans_agent_register_func()
{    
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    us_op = 1;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_AGENT_MSG_PRO,
                                    &us_op,  
                                    trans_agent_func_metric,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback for OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 2;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_AGENT_MSG_PRO,
                                    &us_op,  
                                    trans_agent_func_spectrum,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 3;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_AGENT_MSG_PRO,
                                    &us_op,  
                                    trans_agent_func_idupdate,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_init()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/

u_int32_t trans_agent_init(struct trans_init_info *p_init_info)
{
    u_int32_t uw_ret = 0;
    /*Init the Global variables */
    
    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    memset((u_int8_t*)&g_trans_agent_config_info, 0, sizeof(struct trans_agent_config_info));

    //g_trans_agent_config_info.uc_connect_num = TRANS_AGENT_TCP_CONNECT_NUM;
    
    g_trans_agent_config_info.us_agent_tcp_port = p_init_info->us_agent_tcp_port;
    /*HEART BEAT TIME INTERVAL  (second)*/
    g_trans_agent_config_info.uw_agent_hb_time = p_init_info->uw_agent_hb_time;
    
    #ifdef TRANS_BS_COMPILE
    g_trans_agent_config_info.uw_agent_ip_addr = p_init_info->uw_agent_ip_addr;
    
    strcpy((char *)g_trans_agent_config_info.a_device_id, (char *)p_init_info->a_agent_device_id);

    strcpy((char *)g_trans_agent_config_info.a_agent_id, (char *)p_init_info->a_agent_id);
    #endif
    
    #ifdef TRANS_MS_COMPILE   
    g_trans_agent_config_info.uw_agent_ip_addr = p_init_info->uw_agent_ip_addr;
    
    strcpy((char *)g_trans_agent_config_info.a_device_id, (char *)TRANS_AGENT_SELF_DEVICE_ID);
    
    strcpy((char *)g_trans_agent_config_info.a_agent_id, (char *)TRANS_AGENT_AGENT_DEVICE_ID);

    g_trans_agent_config_info.us_agent_tcp_port = p_init_info->us_agent_tcp_port;

    g_trans_agent_config_info.uw_agent_hb_time = TRANS_AGENT_HEARTBEAT_TIMEOUT_NUM;

    #endif

    g_trans_agent_socket.w_agent_socket = -1;    

    pthread_mutex_init (&g_trans_agent_socket.m_mutex, NULL);

    memset(g_trans_agent_socket.a_mac, 0, TRANS_MAC_ADDR_LEN);
    
    pthread_mutex_init (&g_trans_agent_metric_mutex, NULL);

    uw_ret = trans_agent_register_func();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_register_func error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    } 

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_release()
* Description: init 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_agent_release()
{
    memset((u_int8_t*)&g_trans_agent_config_info, 0, sizeof(struct trans_agent_config_info));
    
    /*Close socket*/
    close(g_trans_agent_socket.w_agent_socket);

    g_trans_agent_socket.w_agent_socket = -1;    

    return TRANS_SUCCESS;
}

#endif

