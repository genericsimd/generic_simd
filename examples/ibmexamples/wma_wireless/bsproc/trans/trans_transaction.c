/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_transaction.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 30-Nov.2012      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#include <flog.h>
//#include "queue_util.h"
//#include "thd_util.h"

#include <trans.h>
#include <trans_common.h>
#include <trans_transaction.h>
#include <trans_list.h>
#include <trans_device.h>
#include <trans_timer.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_rrh.h>
#include <trans_debug.h>
#include <trans_monitor.h>


#include <malloc.h>
#include <pthread.h>
//#include <semaphore.h>
//#include <sched.h>
//#include <mutex.h>

#include <sys/socket.h>
#include <netinet/in.h>


struct trans_transaction_list  g_trans_transaction_list;

extern u_int32_t trans_transaction_add(struct trans_transaction *p_info);

/*****************************************************************************+
* Function: trans_transaction_creat()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_transaction_creat(void** pp_trans)
{
    struct trans_transaction * p_trans = NULL;
    u_int32_t uw_ret = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter \r\n");  

    *pp_trans = (struct trans_transaction *)malloc(SIZEOF_TRANS_TRANSACTION);

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "*pp_trans : %p. \r\n", *pp_trans);   
    
    if(NULL != *pp_trans) 
    {
        p_trans = (struct trans_transaction *)*pp_trans;

        /*struct trans_transaction_common*/
        p_trans->st_common.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_common.p_module = NULL;

        /*struct trans_transaction_socket*/
        p_trans->st_src_socket.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_src_socket.p_module = NULL;

        /*struct trans_transaction_socket*/
        p_trans->st_dst_socket.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_dst_socket.p_module = NULL;

        /*struct trans_transaction_func*/
        p_trans->st_func.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_func.p_module = NULL;
        
        /*struct trans_transaction_timer*/
        p_trans->st_timer.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_timer.p_module = NULL;

        #if 0
        /*struct trans_transaction_action*/
        (*pp_trans)->st_action.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        (*pp_trans)->st_action.p_module = NULL;
        #endif
        
        /*struct trans_transaction_agent*/
        p_trans->st_agent.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_agent.p_module = NULL;

         /*struct trans_transaction_rrh*/
        p_trans->st_rrh.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_rrh.p_module = NULL;

        /*struct trans_transaction_monitor*/
        p_trans->st_monitor.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_monitor.p_module = NULL;

        /*struct trans_transaction_bs*/
        p_trans->st_bs.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_bs.p_module = NULL;
        
        /*struct trans_transaction_ms*/
        p_trans->st_ms.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_ms.p_module = NULL;
        
        /*struct trans_transaction_user*/
        p_trans->st_user.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_user.p_module = NULL;
        
         /*struct trans_transaction_result*/
        p_trans->st_result.uc_used_flag = TRANS_TRANSACTION_FLAG_NO_USE;
        p_trans->st_result.p_module = NULL;
       
    }
    else 
    {
        FLOG_ERROR("malloc pp_trans error! \r\n");
        return TRANS_FAILD;   

    }

    uw_ret = trans_transaction_add(p_trans);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_transaction_add error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit \r\n"); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_delete()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_transaction_delete(void *p_info)
{
    u_int32_t uw_ret = 0;
    
    struct trans_transaction * p_trans = NULL;
    struct trans_transaction_common * p_common = NULL;
    
    struct trans_transaction_agent  *p_agent = NULL;
    struct trans_transaction_rrh  *p_rrh = NULL;
    struct trans_transaction_monitor  *p_monitor = NULL;

    struct trans_transaction_timer  *p_timer = NULL;
    
    struct trans_transaction_user  *p_user = NULL;
    struct trans_transaction_result  *p_result = NULL;
    
    struct trans_transaction_socket  *p_src_socket = NULL;       
    struct trans_transaction_socket  *p_dst_socket = NULL;       
    struct trans_transaction_func  *p_func = NULL;  
    
    struct trans_transaction_bs  *p_bs = NULL;     
    struct trans_transaction_ms  *p_ms = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter \r\n");  

    if (NULL == p_info )
    {
        FLOG_ERROR("NULL PTR!p_trans \r\n");
        return TRANS_FAILD;  
    }

    p_trans = (struct trans_transaction *)p_info;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "p_info : %p. \r\n", p_info);  

    if (NULL == p_trans->st_common.p_module)
    {
        FLOG_ERROR("NULL PTR!st_common.uc_used_flag = %d. \r\n", p_trans->st_common.uc_used_flag);
        return TRANS_FAILD;  
    }
    else
    {
        p_common = (struct trans_transaction_common *)p_trans->st_common.p_module;

        if (TRANS_TRANSACTION_FLAG_DELETE != p_common->uc_deleted)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Do not need free p_trans.\r\n");  
            
            return TRANS_SUCCESS;  
        }
        else
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "need free p_trans.\r\n");  
            
            free (p_trans->st_common.p_module);

            p_trans->st_common.p_module = NULL;
        }      
    }

    uw_ret = trans_transaction_get_out_by_ptr(p_trans, &p_info); 
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_get_out_by_ptr error!uw_ret = %d.\r\n", uw_ret);
        //FLOG_DEBUG_TRANS(g_trans_debug_transaction, "the transaction info is not in list.\r\n");  
        /*If can not find the node in the list---It is normal---go on delete p_trans*/
        //return TRANS_FAILD;   
    }

    if (NULL == p_trans->st_agent.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_agent.uc_used_flag = %d. \r\n", p_trans->st_agent.uc_used_flag);
    
    }
    else
    {
        p_agent = (struct trans_transaction_agent *)p_trans->st_agent.p_module;

        if (NULL != p_agent->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_agent->p_msg = %p! \r\n", p_agent->p_msg);
            free (p_agent->p_msg);
        }

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_agent = %p! \r\n", p_agent);
        free (p_agent);    
        
        p_trans->st_agent.p_module = NULL;
    }

    if (NULL == p_trans->st_rrh.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_rrh.uc_used_flag = %d. \r\n", p_trans->st_rrh.uc_used_flag);
    
    }
    else
    {
        p_rrh = (struct trans_transaction_rrh *)p_trans->st_rrh.p_module;

        if (NULL != p_rrh->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_rrh->p_msg = %p! \r\n", p_rrh->p_msg);
            
            free (p_rrh->p_msg);
        }

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_rrh = %p! \r\n", p_rrh);
        
        free (p_rrh);    

        p_trans->st_rrh.p_module = NULL;
    }

    if(NULL == p_trans->st_monitor.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_monitor.uc_used_flag = %d. \r\n", p_trans->st_monitor.uc_used_flag);
    
    }
    else
    {
        p_monitor = (struct trans_transaction_monitor *)p_trans->st_monitor.p_module;
        
        if (NULL != p_monitor->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_monitor->p_msg = %p! \r\n", p_monitor->p_msg);
            free (p_monitor->p_msg);
        }
        
        free (p_monitor);   

        p_trans->st_monitor.p_module = NULL;
    }
    
    if(NULL == p_trans->st_bs.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_bs.uc_used_flag = %d. \r\n", p_trans->st_bs.uc_used_flag);
    
    }
    else
    {
        p_bs = (struct trans_transaction_bs *)p_trans->st_bs.p_module;
        
        if (NULL != p_bs->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_bs->p_msg = %p! \r\n", p_bs->p_msg);
            free (p_bs->p_msg);
        }
        
        free (p_bs);   
    
        p_trans->st_bs.p_module = NULL;
    }

    if(NULL == p_trans->st_ms.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_ms.uc_used_flag = %d. \r\n", p_trans->st_ms.uc_used_flag);
    
    }
    else
    {
        p_ms = (struct trans_transaction_ms *)p_trans->st_ms.p_module;
        
        if (NULL != p_ms->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_ms->p_msg = %p! \r\n", p_ms->p_msg);
            free (p_ms->p_msg);
        }
        
        free (p_ms);   
    
        p_trans->st_ms.p_module = NULL;
    }

    if (NULL == p_trans->st_timer.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_result.uc_used_flag = %d. \r\n", p_trans->st_timer.uc_used_flag);
    }
    else
    {
        p_timer = (struct trans_transaction_timer *)p_trans->st_timer.p_module;
            
        free (p_timer);
        
        p_trans->st_timer.p_module = NULL;
    
    }

    if(NULL == p_trans->st_user.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_user.uc_used_flag = %d. \r\n", p_trans->st_user.uc_used_flag);
    }
    else
    {
        p_user = (struct trans_transaction_user *)p_trans->st_user.p_module;
        
        if (NULL != p_user->p_msg)
        {
            FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_user->p_msg = %p! \r\n", p_user->p_msg);
            free (p_user->p_msg);
        }
        
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_user = %p! \r\n", p_user);
        free (p_user);    
        
        p_trans->st_user.p_module = NULL;
    }

    if (NULL == p_trans->st_result.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_result.uc_used_flag = %d. \r\n", p_trans->st_result.uc_used_flag);
    }
    else
    {
        p_result = (struct trans_transaction_result *)p_trans->st_result.p_module;
            
        free (p_result);
        
        p_trans->st_result.p_module = NULL;

    }

    if (NULL == p_trans->st_src_socket.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_src_socket.uc_used_flag = %d. \r\n", p_trans->st_src_socket.uc_used_flag);
    }
    else
    {
        p_src_socket = (struct trans_transaction_socket *)p_trans->st_src_socket.p_module;
            
        free (p_src_socket);
        
        p_trans->st_src_socket.p_module = NULL;
    
    }

    if (NULL == p_trans->st_dst_socket.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_dst_socket.uc_used_flag = %d. \r\n", p_trans->st_dst_socket.uc_used_flag);
    }
    else
    {
        p_dst_socket = (struct trans_transaction_socket *)p_trans->st_dst_socket.p_module;
            
        free (p_dst_socket);
        
        p_trans->st_dst_socket.p_module = NULL;
    
    }

    if (NULL == p_trans->st_func.p_module)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "NULL PTR!st_func.uc_used_flag = %d. \r\n", p_trans->st_func.uc_used_flag);
    }
    else
    {
        p_func = (struct trans_transaction_func *)p_trans->st_func.p_module;
            
        free (p_func);
        
        p_trans->st_func.p_module = NULL;
    
    }

    free (p_trans);

    p_trans = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit \r\n"); 
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_transaction_set_comn()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_comn(void *p_info, 
                            u_int8_t uc_deleted,
                            u_int32_t  uw_src_module)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_common  *p_common = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "p_info : %p. \r\n", p_info);  

    p_trans = (struct trans_transaction*)p_info;

    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_common.uc_used_flag)
        && (NULL != p_trans->st_common.p_module))
    {
        p_common = (struct trans_transaction_common *)p_trans->st_common.p_module;

        /*Not change*/
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Old: %d, new: %d. \r\n",
                            p_common->uc_deleted, uc_deleted);   
        //p_common->uw_src_module = uw_src_module;
        p_common->uc_deleted = uc_deleted;        

    }
    else
    {
        p_common = (struct trans_transaction_common *)malloc(SIZEOF_TRANS_TRANSACTION_COMMON);
        
        if (NULL == p_common)
        {
            FLOG_ERROR("malloc p_common error! \r\n");
            return TRANS_FAILD;   
        }
        
        /*Device Type: WMA, WMB, Monitor, RRH, Agent*/
        p_common->uc_deleted = uc_deleted; 
        /*Socket fd*/
        p_common->uw_src_module = uw_src_module;
    }

    /*Socket fd*/
    //p_common->uc_deleted = uc_deleted; 
    //p_common->uw_src_module = uw_src_module; 

    p_trans->st_common.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_common.p_module = p_common;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. SRC = %d.\r\n", p_common->uw_src_module);    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_comn()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_get_comn_src(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_common  *p_common = NULL;
    int32_t w_src = 0;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return 0xff;
    }  

    p_trans = (struct trans_transaction*)p_info;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "p_trans : %p. \r\n", p_trans);  
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_common.uc_used_flag)
        || (NULL == p_trans->st_common.p_module))
    {
        FLOG_ERROR("No result error!Flag = %d. \r\n", p_trans->st_common.uc_used_flag);

        return 0xff;
    }
    else
    {
        p_common = (struct trans_transaction_common *)p_trans->st_common.p_module;
        
        w_src = (int32_t)p_common->uw_src_module;
    } 
   
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. SRC: %d. \r\n", w_src); 
    
    return (w_src);
}

/*****************************************************************************+
* Function: trans_transaction_set_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_func(void *p_info, 
                            u_int8_t uc_exe_flag,
                            u_int32_t uw_func_id,
                            int32_t w_msg_len,
                            void *p_msg)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_func  *p_func = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "p_trans : %p. \r\n", p_trans);   

    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_func.uc_used_flag)
        && (NULL != p_trans->st_func.p_module))
    {
        p_func = (struct trans_transaction_func *)p_trans->st_func.p_module;

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Change function. \r\n");   
    }
    else
    {
        p_func = (struct trans_transaction_func *)malloc(SIZEOF_TRANS_TRANSACTION_FUNC);
        
        if (NULL == p_func)
        {
            FLOG_ERROR("malloc p_func error! \r\n");
            return TRANS_FAILD;   
        }
        
        /*Device Type: WMA, WMB, Monitor, RRH, Agent*/
        p_func->uc_exe_flag = 0xff;
        /*Funtion Callback*/
        p_func->uw_func_id = 0;
        p_func->w_msg_len = 0;
        p_func->p_msg = NULL;

    }

    p_func->uc_exe_flag = uc_exe_flag;
    /*Funtion Callback*/
    p_func->uw_func_id = uw_func_id;

    /*Change*/
    if ((0 != w_msg_len) && (NULL != p_msg))
    {
        /*Can not free the old p_msg*/
        
        p_func->w_msg_len = w_msg_len;
        p_func->p_msg = p_msg;
    
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_func->p_msg = %p! \r\n", p_func->p_msg);
    }

    p_trans->st_func.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_func.p_module = p_func;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Flag: %d, func: %d, len: %d. \r\n", 
        p_func->uc_exe_flag, p_func->uw_func_id, p_func->w_msg_len); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. \r\n");    

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_transaction_get_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void * trans_transaction_get_func(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    //struct trans_transaction_func  *p_func = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_func.uc_used_flag)
        || (NULL == p_trans->st_func.p_module))
    {
        FLOG_ERROR("No st_func error!Flag = %d. \r\n", p_trans->st_func.uc_used_flag);

        return NULL;
    }
    else
    {
        return (p_trans->st_func.p_module);

    } 
}

/*****************************************************************************+
* Function: trans_transaction_set_src()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_src(void *p_info, 
                            u_int32_t uw_num,
                            int32_t  w_sockfd,
                            u_int8_t *p_mac)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_socket  *p_socket = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;

    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_src_socket.uc_used_flag)
        && (NULL != p_trans->st_src_socket.p_module))
    {
        p_socket = (struct trans_transaction_socket *)p_trans->st_src_socket.p_module;

        /*Not change*/
        //p_common->uw_src_module = uw_src_module;
        //p_common->uc_deleted = uc_deleted;

    }
    else
    {
        p_socket = (struct trans_transaction_socket *)malloc(SIZEOF_TRANS_TRANSACTION_SOCKET);
        
        if (NULL == p_socket)
        {
            FLOG_ERROR("malloc p_socket error! \r\n");
            return TRANS_FAILD;   
        }
        
        /*Socket fd*/
        p_socket->w_sockfd = -1;
        p_socket->uw_num = 0;
        memset(p_socket->a_mac, 0, TRANS_MAC_ADDR_LEN);
    }

    /*Socket fd*/
    p_socket->w_sockfd = w_sockfd; 
    //p_socket->uc_module_type = uc_src_type; 
    p_socket->uw_num = uw_num;
    
    memcpy(p_socket->a_mac, p_mac, TRANS_MAC_ADDR_LEN);

    p_trans->st_src_socket.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_src_socket.p_module = p_socket;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. \r\n");    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_set_dst()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_dst(void *p_info, 
                            u_int32_t uw_num,
                            int32_t  w_sockfd,
                            u_int8_t *p_mac)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_socket  *p_socket = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;

    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_dst_socket.uc_used_flag)
        && (NULL != p_trans->st_dst_socket.p_module))
    {
        p_socket = (struct trans_transaction_socket *)p_trans->st_dst_socket.p_module;

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "st_dst_socket has been used. \r\n"); 

    }
    else
    {
        p_socket = (struct trans_transaction_socket *)malloc(SIZEOF_TRANS_TRANSACTION_SOCKET);
        
        if (NULL == p_socket)
        {
            FLOG_ERROR("malloc p_socket error! \r\n");
            return TRANS_FAILD;   
        }
        
        p_socket->uw_num = 0;
        /*Socket fd*/
        p_socket->w_sockfd = -1;
        memset(p_socket->a_mac, 0, TRANS_MAC_ADDR_LEN);
    }

    /*Socket fd*/
    p_socket->w_sockfd = w_sockfd; 
    p_socket->uw_num = uw_num;

    memcpy(p_socket->a_mac, p_mac, TRANS_MAC_ADDR_LEN);

    p_trans->st_dst_socket.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_dst_socket.p_module = p_socket;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. \r\n");    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_dst_sock()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
int32_t trans_transaction_get_dst_sock(void *p_info)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_socket  *p_socket = NULL;

    int32_t w_sockfd = 0;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return -1;
    }  
    
    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_dst_socket.uc_used_flag)
        || (NULL == p_trans->st_dst_socket.p_module))
    {
        FLOG_ERROR("No result error!Flag = %d. \r\n", p_trans->st_dst_socket.uc_used_flag);
    
        return -2;
    }
    else
    {
        p_socket = (struct trans_transaction_socket *)p_trans->st_dst_socket.p_module;
        
        w_sockfd = (int32_t)p_socket->w_sockfd;
    } 
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. w_sockfd: %d. \r\n", w_sockfd); 
    
    return (w_sockfd);
}

/*****************************************************************************+
* Function: trans_transaction_get_dst_num()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
int32_t trans_transaction_get_dst_num(void *p_info)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_socket  *p_socket = NULL;

    int32_t w_num = 0;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return -1;
    }  
    
    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_dst_socket.uc_used_flag)
        || (NULL == p_trans->st_dst_socket.p_module))
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "No result error!Flag = %d. \r\n", p_trans->st_dst_socket.uc_used_flag);
    
        return -2;
    }
    else
    {
        p_socket = (struct trans_transaction_socket *)p_trans->st_dst_socket.p_module;
        
        w_num = (int32_t)p_socket->uw_num;
    } 
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. w_num: %d. \r\n", w_num); 
    
    return (w_num);
}


/*****************************************************************************+
* Function: trans_transaction_get_dst_mac()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void * trans_transaction_get_dst_mac(void *p_info)
{
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_socket  *p_socket = NULL;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  
    
    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_dst_socket.uc_used_flag)
        || (NULL == p_trans->st_dst_socket.p_module))
    {
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "No result error!Flag = %d. \r\n", p_trans->st_dst_socket.uc_used_flag);
    
        return NULL;
    }
    else
    {
        p_socket = (struct trans_transaction_socket *)p_trans->st_dst_socket.p_module;
        
        return (p_socket->a_mac);
    } 
}


/*****************************************************************************+
* Function: trans_transaction_set_user()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_user(void *p_info, 
                           size_t len,
                           void * p_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_user* p_user = NULL;
    
    if ((NULL == p_info) ||(NULL == p_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
   
    if (0 == len)
    {
        FLOG_ERROR("Length error! \r\n", len);
        return TRANS_FAILD;
    }

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_user.uc_used_flag)
        && (NULL != p_trans->st_user.p_module))
    {
        p_user = (struct trans_transaction_user *)p_trans->st_user.p_module;
        
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "st_user has been used. Flag = %d.\r\n", p_trans->st_user.uc_used_flag); 
        
    }
    else
    {
        p_user = (struct trans_transaction_user *)malloc(SIZEOF_TRANS_TRANSACTION_USER);
        if (NULL == p_user)
        {
            FLOG_ERROR("malloc p_user error! \r\n");
            return TRANS_FAILD;   
        }
        p_user->p_msg = NULL;
        p_user->w_msg_len = 0;
        
    }

    /*Change*/
    if ((0 != len) && (NULL != p_buf))
    {

        if (NULL != p_user->p_msg)
        {
            free (p_user->p_msg);   
            
            p_user->p_msg = NULL;
            p_user->w_msg_len = 0;
        }
    
        p_user->w_msg_len = len;
        p_user->p_msg = p_buf;
    
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_user->p_msg = %p! \r\n", p_user->p_msg);
    }

    p_trans->st_user.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_user.p_module = p_user;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. \r\n");    
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_user()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void * trans_transaction_get_user(void *p_info, int32_t *p_msg_len)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_user* p_user = NULL;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_user.uc_used_flag)
        || (NULL == p_trans->st_user.p_module))
    {
        FLOG_ERROR("No user info error!Flag = %d. \r\n", p_trans->st_user.uc_used_flag);
        return NULL;
    }
    else
    {
        p_user = (struct trans_transaction_user *)p_trans->st_user.p_module;

        if (0 == p_user->w_msg_len)
        {
            FLOG_WARNING("user info length is 0. \r\n"); 
        }
        
        *p_msg_len = p_user->w_msg_len;

        return (p_user->p_msg);
    }
}

/*****************************************************************************+
* Function: trans_transaction_set_timer()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_timer(void *p_info, 
                           void * p_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction * p_trans = NULL;
    struct trans_transaction_timer * p_timer = NULL;
    
    if ((NULL == p_info) ||(NULL == p_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_timer.uc_used_flag)
        && (NULL != p_trans->st_timer.p_module))
    {
        p_timer = (struct trans_transaction_timer *)p_trans->st_timer.p_module;
        
        FLOG_WARNING("st_timer has been used. \r\n"); 
        
    }
    else
    {
        p_timer = (struct trans_transaction_timer *)malloc(SIZEOF_TRANS_TRANSACTION_TIMER);
        if (NULL == p_timer)
        {
            FLOG_ERROR("malloc p_timer error! \r\n");
            return TRANS_FAILD;   
        }
        p_timer->p_timer = NULL;
        
    }

    /*Change*/
    if (NULL != p_buf)
    {
        /*Do not free the p_timer----It is free in timer handler*/
        if (NULL != p_timer->p_timer)
        {

        }
    
        p_timer->p_timer = p_buf;
    
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_timer->p_timer = %p! \r\n", p_timer->p_timer);
    }

    p_trans->st_timer.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_timer.p_module = p_timer;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. \r\n");    
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_timer()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
void * trans_transaction_get_timer(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_timer * p_timer = NULL;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_timer.uc_used_flag)
        || (NULL == p_trans->st_timer.p_module))
    {
        //FLOG_ERROR("No timer info error!Flag = %d. \r\n", p_trans->st_timer.uc_used_flag);
        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "No timer info error!Flag = %d. \r\n", p_trans->st_timer.uc_used_flag);
        return NULL;
    }
    else
    {
        p_timer = (struct trans_transaction_timer *)p_trans->st_timer.p_module;

        return (p_timer->p_timer);
    }
}


/*****************************************************************************+
* Function: trans_transaction_get_tra_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
int32_t trans_transaction_get_tra_id(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_user  *p_user = NULL;

    struct trans_monitor_info *p_monitor_info = NULL;    
    
    int32_t w_transaction = 0;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return -1;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_user.uc_used_flag)
        || (NULL == p_trans->st_user.p_module))
    {
        FLOG_ERROR("No result error!Flag = %d. \r\n", p_trans->st_user.uc_used_flag);

        return -2;
    }
    else
    {
        p_user = (struct trans_transaction_user *)p_trans->st_user.p_module;

        p_monitor_info = (struct trans_monitor_info *)p_user->p_msg;
        
        w_transaction = (int32_t)p_monitor_info->uw_transaction;
    } 
   
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. w_transaction: %d. \r\n", w_transaction); 
    
    return (w_transaction);
}

/*****************************************************************************+
* Function: trans_transaction_get_src_mac()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void * trans_transaction_get_src_mac(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_user  *p_user = NULL;

    struct trans_monitor_info *p_monitor_info = NULL;    
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_user.uc_used_flag)
        || (NULL == p_trans->st_user.p_module))
    {
        FLOG_ERROR("No result error!Flag = %d. \r\n", p_trans->st_user.uc_used_flag);

        return NULL;
    }
    else
    {
        p_user = (struct trans_transaction_user *)p_trans->st_user.p_module;

        p_monitor_info = (struct trans_monitor_info *)p_user->p_msg;
        
        //uw_transaction = (int32_t)p_monitor_info->a_src_mac;
        return (p_monitor_info->a_src_mac);
    } 
   
    //FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. uw_transaction: %d. \r\n", uw_transaction); 


}


/*****************************************************************************+
* Function: trans_transaction_set_result()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_result(void *p_info, 
                           u_int32_t  uw_result)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_result  *p_result = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_result.uc_used_flag)
        && (NULL != p_trans->st_result.p_module))
    {
        p_result = (struct trans_transaction_result *)p_trans->st_result.p_module;

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Result.New: %d, Old: %d; \r\n", uw_result, p_result->uw_result); 

    }
    else
    {
        p_result = (struct trans_transaction_result *)malloc(SIZEOF_TRANS_TRANSACTION_RESULT);
        if (NULL == p_result)
        {
            FLOG_ERROR("malloc p_result error! \r\n");
            return TRANS_FAILD;   
        }

        p_result->uw_result = 0;
        
        p_trans->st_result.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
        p_trans->st_result.p_module = p_result;
    } 
    
    p_result->uw_result = uw_result;
   
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. Result: %d. \r\n", p_result->uw_result); 
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_transaction_get_result()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
int32_t trans_transaction_get_result(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_result  *p_result = NULL;
    int32_t w_result = 0;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return -1;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_result.uc_used_flag)
        || (NULL == p_trans->st_result.p_module))
    {
        FLOG_ERROR("No result error!Flag = %d. \r\n", p_trans->st_result.uc_used_flag);

        return -2;
    }
    else
    {
        p_result = (struct trans_transaction_result *)p_trans->st_result.p_module;
        
        w_result = (int32_t)p_result->uw_result;
    } 
   
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit. Result: %d. \r\n", w_result); 
    
    return (w_result);
}

/*****************************************************************************+
* Function: trans_transaction_set_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_rrh(void *p_info,
                                        fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_rrh  *p_rrh = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_rrh.uc_used_flag)
        && (NULL != p_trans->st_rrh.p_module))
    {
        p_rrh = (struct trans_transaction_rrh *)p_trans->st_rrh.p_module;

        if (g_trans_rrh_socket != p_rrh->w_rrh_socket)
        {
            FLOG_WARNING("Socket is different.New: %d, Old: %d; \r\n", 
                g_trans_rrh_socket, p_rrh->w_rrh_socket); 
        }

    }
    else
    {
        p_rrh = (struct trans_transaction_rrh *)malloc(SIZEOF_TRANS_TRANSACTION_RRH);
        if (NULL == p_rrh)
        {
            FLOG_ERROR("malloc p_rrh error! \r\n");
            return TRANS_FAILD;   
        }

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "++++ p_rrh = %p! \r\n", p_rrh);

        p_rrh->f_callback = NULL;
        p_rrh->p_msg = NULL;
        p_rrh->w_msg_len = 0;
        p_rrh->w_rrh_socket = -1;
    } 
    
    p_rrh->w_rrh_socket = g_trans_rrh_socket; /*Socketfd*/

    if (NULL != f_callback)
    {
        p_rrh->f_callback = f_callback;          /*Funtion Callback*/
    }
    
    /*Change*/
    if ((0 != len) && (NULL != p_rev_buf))
    {
        if (NULL != p_rrh->p_msg)
        {
            free (p_rrh->p_msg);   
            
            p_rrh->p_msg = NULL;
            p_rrh->w_msg_len = 0;
        }

        p_rrh->w_msg_len = len;
        p_rrh->p_msg = p_rev_buf;

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_rrh->p_msg = %p! \r\n", p_rrh->p_msg);
    }

    p_trans->st_rrh.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_rrh.p_module = p_rrh;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_trans->st_rrh.p_module = %p! \r\n", p_trans->st_rrh.p_module);
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit. RRH: socket: %d, msg_len: %d. \r\n", 
        p_rrh->w_rrh_socket, p_rrh->w_msg_len); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
fun_callback trans_transaction_get_rrh(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_rrh  *p_rrh = NULL;

    //fun_callback f_callback = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_rrh.uc_used_flag)
        || (NULL == p_trans->st_rrh.p_module))
    {
        FLOG_ERROR("No st_rrh error!Flag = %d. \r\n", p_trans->st_rrh.uc_used_flag);

        return NULL;
    }
    else
    {
        p_rrh = (struct trans_transaction_rrh *)p_trans->st_rrh.p_module;
        
        return (p_rrh->f_callback);
    } 

}

/*****************************************************************************+
* Function: trans_transaction_set_monitor()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_monitor(void *p_info,
                                        //fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_monitor  *p_monitor = NULL;
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_monitor.uc_used_flag)
        && (NULL != p_trans->st_monitor.p_module))
    {
        p_monitor = (struct trans_transaction_monitor *)p_trans->st_monitor.p_module;
    
        /*
        if (g_trans_monitor_socket.w_monitor_socket != p_monitor->w_monitor_socket)
        {
            FLOG_WARNING("Socket is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.w_monitor_socket, p_monitor->w_monitor_socket); 
        }
        
        if (g_trans_monitor_socket.uw_connect_num != p_monitor->uw_connect_num)
        {
            FLOG_WARNING("Socket connect_num is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.uw_connect_num, p_monitor->w_monitor_socket); 
        }
        */
    }
    else
    {
        p_monitor = (struct trans_transaction_monitor *)malloc(SIZEOF_TRANS_TRANSACTION_MONITOR);
        if (NULL == p_monitor)
        {
            FLOG_ERROR("malloc p_monitor error! \r\n");
            return TRANS_FAILD;   
        }
        //p_monitor->w_monitor_socket = -1; /*Socketfd*/
        //p_monitor->uw_connect_num = 0;/*Connect Number total */
        p_monitor->p_msg = NULL;
        p_monitor->w_msg_len = 0;
    
    } 
    
    #if 0
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
    
    p_monitor->w_monitor_socket = g_trans_monitor_socket.w_monitor_socket; /*Socketfd*/
    p_monitor->uw_connect_num = g_trans_monitor_socket.uw_connect_num;          /*Funtion Callback*/
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));    
    #endif
    
    /*Change*/
    if ((0 != len) && (NULL != p_rev_buf))
    {
        if (NULL != p_monitor->p_msg)
        {
            free (p_monitor->p_msg);   
            
            p_monitor->p_msg = NULL;
            p_monitor->w_msg_len = 0;
        }
        
        p_monitor->w_msg_len = len;
        p_monitor->p_msg = p_rev_buf;
    }
    
    p_trans->st_monitor.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_monitor.p_module = p_monitor;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit. Monitor: msg_len: %d. \r\n", 
        p_monitor->w_msg_len); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_set_monitor()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_bs(void *p_info,
                                        size_t len,
                                        void * p_rev_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_bs  *p_bs = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_bs.uc_used_flag)
        && (NULL != p_trans->st_bs.p_module))
    {
        p_bs = (struct trans_transaction_bs *)p_trans->st_bs.p_module;

        /*
        if (g_trans_monitor_socket.w_monitor_socket != p_monitor->w_monitor_socket)
        {
            FLOG_WARNING("Socket is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.w_monitor_socket, p_monitor->w_monitor_socket); 
        }
        
        if (g_trans_monitor_socket.uw_connect_num != p_monitor->uw_connect_num)
        {
            FLOG_WARNING("Socket connect_num is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.uw_connect_num, p_monitor->w_monitor_socket); 
        }
        */
    }
    else
    {
        p_bs = (struct trans_transaction_bs *)malloc(SIZEOF_TRANS_TRANSACTION_BS);
        if (NULL == p_bs)
        {
            FLOG_ERROR("malloc p_bs error! \r\n");
            return TRANS_FAILD;   
        }
        //p_monitor->w_monitor_socket = -1; /*Socketfd*/
        //p_monitor->uw_connect_num = 0;/*Connect Number total */
        p_bs->p_msg = NULL;
        p_bs->w_msg_len = 0;

    } 
    
    #if 0
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
    
    p_monitor->w_monitor_socket = g_trans_monitor_socket.w_monitor_socket; /*Socketfd*/
    p_monitor->uw_connect_num = g_trans_monitor_socket.uw_connect_num;          /*Funtion Callback*/
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));    
    #endif
    
    /*Change*/
    if ((0 != len) && (NULL != p_rev_buf))
    {
        if (NULL != p_bs->p_msg)
        {
            free (p_bs->p_msg);   
            
            p_bs->p_msg = NULL;
            p_bs->w_msg_len = 0;
        }
        
        p_bs->w_msg_len = len;
        p_bs->p_msg = p_rev_buf;

        FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_bs->p_msg = %p! \r\n", p_bs->p_msg);
    }

    p_trans->st_bs.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_bs.p_module = p_bs;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "1 p_trans->st_bs.p_module = %p! \r\n", p_trans->st_bs.p_module);

    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit. msg_len: %d. \r\n", 
                            p_bs->w_msg_len); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_set_ms()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_ms(void *p_info,
                                        //fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_ms  *p_ms = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_ms.uc_used_flag)
        && (NULL != p_trans->st_ms.p_module))
    {
        p_ms = (struct trans_transaction_ms *)p_trans->st_ms.p_module;

        /*
        if (g_trans_monitor_socket.w_monitor_socket != p_monitor->w_monitor_socket)
        {
            FLOG_WARNING("Socket is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.w_monitor_socket, p_monitor->w_monitor_socket); 
        }
        
        if (g_trans_monitor_socket.uw_connect_num != p_monitor->uw_connect_num)
        {
            FLOG_WARNING("Socket connect_num is different.New: %d, Old: %d; \r\n", 
                g_trans_monitor_socket.uw_connect_num, p_monitor->w_monitor_socket); 
        }
        */
    }
    else
    {
        p_ms = (struct trans_transaction_ms *)malloc(SIZEOF_TRANS_TRANSACTION_BS);
        if (NULL == p_ms)
        {
            FLOG_ERROR("malloc p_bs error! \r\n");
            return TRANS_FAILD;   
        }
        //p_monitor->w_monitor_socket = -1; /*Socketfd*/
        //p_monitor->uw_connect_num = 0;/*Connect Number total */
        p_ms->p_msg = NULL;
        p_ms->w_msg_len = 0;

    } 
    
    #if 0
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
    
    p_monitor->w_monitor_socket = g_trans_monitor_socket.w_monitor_socket; /*Socketfd*/
    p_monitor->uw_connect_num = g_trans_monitor_socket.uw_connect_num;          /*Funtion Callback*/
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));    
    #endif
    
    /*Change*/
    if ((0 != len) && (NULL != p_rev_buf))
    {
        if (NULL != p_ms->p_msg)
        {
            free (p_ms->p_msg);   
            
            p_ms->p_msg = NULL;
            p_ms->w_msg_len = 0;
        }
        
        p_ms->w_msg_len = len;
        p_ms->p_msg = p_rev_buf;
    }

    p_trans->st_ms.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_ms.p_module = p_ms;
    
    FLOG_DEBUG_TRANS(g_trans_debug_monitor, "Exit. msg_len: %d. \r\n", 
                            p_ms->w_msg_len); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_set_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_transaction_set_agent(void *p_info, 
                           fun_callback f_callback,
                           size_t len,
                           void * p_rev_buf)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_agent  *p_agent = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE == p_trans->st_agent.uc_used_flag)
        && (NULL != p_trans->st_agent.p_module))
    {
        p_agent = (struct trans_transaction_agent *)p_trans->st_agent.p_module;

        if (g_trans_agent_socket.w_agent_socket != p_agent->w_agent_socket)
        {
            FLOG_WARNING("Socket is different.New: %d, Old: %d; \r\n", 
                g_trans_agent_socket.w_agent_socket, p_agent->w_agent_socket); 
        }

    }
    else
    {
        p_agent = (struct trans_transaction_agent *)malloc(SIZEOF_TRANS_TRANSACTION_AGENT);
        if (NULL == p_agent)
        {
            FLOG_ERROR("malloc p_agent error! \r\n");
            return TRANS_FAILD;   
        }
        
        p_agent->f_callback = NULL;
        p_agent->p_msg = NULL;
        p_agent->w_msg_len = 0;
        p_agent->w_agent_socket = -1;
    } 
    
    /*Socketfd*/
    p_agent->w_agent_socket = g_trans_agent_socket.w_agent_socket; 
    p_agent->f_callback = f_callback;          /*Funtion Callback*/
    
    if ((0 != len) && (NULL != p_rev_buf))
    {
        if (NULL != p_agent->p_msg)
        {
            free (p_agent->p_msg);   
            
            p_agent->p_msg = NULL;
            p_agent->w_msg_len = 0;
        }
        
        p_agent->w_msg_len = len;
        p_agent->p_msg = p_rev_buf;
    }

    p_trans->st_agent.uc_used_flag = TRANS_TRANSACTION_FLAG_USE;
    p_trans->st_agent.p_module = p_agent;
    
    FLOG_DEBUG_TRANS(g_trans_debug_agent, "Exit. Agent: socket: %d, msg_len: %d. \r\n", 
        p_agent->w_agent_socket, p_agent->w_msg_len); 

    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
fun_callback trans_transaction_get_agent(void *p_info)
{
    //u_int32_t uw_ret = 0;
    struct trans_transaction* p_trans = NULL;
    struct trans_transaction_agent  *p_agent = NULL;

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return NULL;
    }  

    p_trans = (struct trans_transaction*)p_info;
    
    if ((TRANS_TRANSACTION_FLAG_USE != p_trans->st_agent.uc_used_flag)
        || (NULL == p_trans->st_agent.p_module))
    {
        FLOG_ERROR("No st_agent error!Flag = %d. \r\n", p_trans->st_agent.uc_used_flag);

        return NULL;
    }
    else
    {
        p_agent = (struct trans_transaction_agent *)p_trans->st_agent.p_module;
        
        return (p_agent->f_callback);
    } 

}

/*****************************************************************************+
* Function: trans_transaction_cmp_mac()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
int trans_transaction_cmp_mac(void *p_data, void *p_key, void *p_result)
{
    struct trans_transaction_cmp_info *p_cmp = NULL;

    u_int8_t  *p_cmp_result = NULL;
    
    u_int8_t  *p_mac = NULL;
    int32_t  w_num = 0;

    #if 0
    if ((NULL == p_data) || (NULL == p_key) || (NULL == p_result))
    {
        FLOG_ERROR("NULL PTR. \r\n");        
        return TRANS_FAILD;     
    }
    #endif

    p_cmp = (struct trans_transaction_cmp_info *)p_key;
    p_cmp_result = (u_int8_t  *)p_result;

    p_mac = trans_transaction_get_dst_mac(p_data);
    if (NULL == p_mac)
    {
        return TRANS_SUCCESS;
    }
    w_num = trans_transaction_get_dst_num(p_data);
    if (0 >= w_num)
    {
        return TRANS_SUCCESS;
    }

    if (trans_mac_addr_cmp(p_cmp->a_mac, p_mac) 
        && (p_cmp->uw_num == (u_int32_t)w_num))
    {   
        *p_cmp_result = 1;

    } 
    else
    {
        *p_cmp_result = 0;
    }
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_transaction_cmp_ptr()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
int trans_transaction_cmp_ptr(void *p_data, void *p_key, void *p_result)
{
    u_int8_t  *p_cmp = NULL;

    #if 0
    if ((NULL == p_data) || (NULL == p_key) || (NULL == p_result))
    {
        FLOG_ERROR("NULL PTR. \r\n");        
        return TRANS_FAILD;     
    }
    #endif

    p_cmp = (u_int8_t  *)p_result;

    if (p_data == p_key)
    {   
        *p_cmp = 1;
    
    } 
    else
    {
        *p_cmp = 0;
    }
    
    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_transaction_get_in()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_get_in(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info) 
{
    u_int32_t uw_ret = 0;
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_transaction_cmp_info st_cmp;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter\r\n");
        
    *pp_info = NULL;
    
    p_unordered_list = g_trans_transaction_list.p_unordered_list;

    st_cmp.uw_num = uw_num;
    memcpy(st_cmp.a_mac, p_mac, TRANS_MAC_ADDR_LEN);

    uw_ret = trans_unordered_list_get_in(p_unordered_list, 
                            &st_cmp,
                            trans_transaction_cmp_mac,
                            pp_info);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_in error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_out()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_get_out(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info) 
{
    u_int32_t uw_ret = 0;
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_transaction_cmp_info st_cmp;
    //void * p_data = NULL;

    *pp_info = NULL;
    
    p_unordered_list = g_trans_transaction_list.p_unordered_list;
    
    st_cmp.uw_num = uw_num;
    memcpy(st_cmp.a_mac, p_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_unordered_list_get_out(p_unordered_list, 
                            &st_cmp,
                            trans_transaction_cmp_mac,
                            pp_info);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_out error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_get_out_by_ptr()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_get_out_by_ptr(void *p_info,
                                                 void ** pp_info) 
{
    u_int32_t uw_ret = 0;
    struct trans_unordered_list * p_unordered_list = NULL;

    //void * p_timer = NULL;
    
    *pp_info = NULL;
    
    p_unordered_list = g_trans_transaction_list.p_unordered_list;
   
    uw_ret = trans_unordered_list_get_out(p_unordered_list, 
                            p_info,
                            trans_transaction_cmp_ptr,
                            pp_info);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_out error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }
    
    /*Can not op timer list here*/
    #if 0
    if (NULL == *pp_info)
    {
        FLOG_INFO("Can not find the transaction info in the list!\r\n");
        return TRANS_SUCCESS;   
    }
    
    /*Check if timeout*/
    p_timer = trans_transaction_get_timer(*pp_info);
    
    if (NULL != p_timer)
    {
        uw_ret = trans_timer_delete(&g_trans_timer_list, p_timer);
        if(TRANS_SUCCESS != uw_ret) 
        {
            FLOG_ERROR("Call trans_timer_delete error!uw_ret = %d \r\n", uw_ret);
            
            /*Can not return TRANS_FAILD*/
            //return TRANS_FAILD;    
        }
    }
    #endif

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_transaction_get_out_timer()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_get_out_by_mac(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info) 
{
    u_int32_t uw_ret = 0;
    void * p_timer = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter \r\n");
    
    uw_ret = trans_transaction_get_out(uw_num, p_mac, pp_info);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_out error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    if (NULL == *pp_info)
    {
        FLOG_ERROR("Timeout, can not find the transaction info!\r\n");
        return TRANS_FAILD;   
    }

    /*Check if timeout*/
    p_timer = trans_transaction_get_timer(*pp_info);

    if (NULL != p_timer)
    {
        uw_ret = trans_timer_delete(&g_trans_timer_list, p_timer);
        if(TRANS_SUCCESS != uw_ret) 
        {
            FLOG_ERROR("Call trans_timer_delete error!uw_ret = %d \r\n", uw_ret);
            
            /*Can not return TRANS_FAILD*/
            //return TRANS_FAILD;    
        }
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_transaction_add()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t trans_transaction_add(struct trans_transaction *p_info)
{
    struct trans_unordered_list * p_unordered_list = NULL;
    //u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;

    //struct timeval st_tv;
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter \r\n");    
    
    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    p_unordered_list = g_trans_transaction_list.p_unordered_list;

    //Insert it into to timer/event queue
    uw_ret = trans_unordered_list_add(p_unordered_list, 
                                    SIZEOF_TRANS_TRANSACTION, 
                                    p_info);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_add error! \r\n");
        return uw_ret;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Completed add : %p.\n", p_info);

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit \r\n"); 

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_transaction_time_out()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_time_out(void *p_trans) 
{
    //void *p_info = NULL;
    u_int32_t uw_ret = 0;

    if (NULL == p_trans)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    /*Delete transaction from list if could find this node*/
    uw_ret = trans_transaction_set_comn(p_trans, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BUF);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        return uw_ret;      
    }
    
    /*Delete p_trans*/
    uw_ret = trans_transaction_delete(p_trans);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_delete error! uw_ret = %d\r\n", uw_ret);
        return uw_ret;      
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_clear()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-20
* 
+*****************************************************************************/
u_int32_t trans_transaction_clear(u_int8_t  *p_mac) 
{
    u_int32_t uw_ret = 0;
    void * p_timer = NULL;
    void * p_field = NULL;

    u_int8_t  *p_curr_mac = NULL;
    
    struct trans_transaction_list * p_transaction_list = NULL;
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_transaction *p_transaction = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Enter \r\n");
    
    p_transaction_list = &g_trans_transaction_list;
    
    if (NULL == p_transaction_list->p_unordered_list)
    {
        FLOG_ERROR("The list is not init! \r\n");
        return TRANS_FAILD;
    }
    
    p_unordered_list = p_transaction_list->p_unordered_list;
    
    //cleaup the List
    pthread_mutex_lock(&(p_unordered_list->qmutex));
    
    TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
    {
        p_transaction = TRANS_UNORDERED_LIST_DATA(p_field);

        p_curr_mac = trans_transaction_get_dst_mac(p_transaction);
        if (NULL == p_curr_mac)
        {
            continue;
        }
        else
        {
            if (trans_mac_addr_cmp(p_mac, p_curr_mac))
            {   
                TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
                    
                /*Free p_transaction*/
                uw_ret = trans_transaction_delete(p_transaction);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_transaction_delete error! uw_ret = %d\r\n", uw_ret);
                    //return;
                }   

                /*Check if add timer*/
                p_timer = trans_transaction_get_timer(p_transaction);
                
                if (NULL != p_timer)
                {
                    uw_ret = trans_timer_delete(&g_trans_timer_list, p_timer);
                    if(TRANS_SUCCESS != uw_ret) 
                    {
                        FLOG_ERROR("Call trans_timer_delete error!uw_ret = %d \r\n", uw_ret);
                        
                        /*Can not return TRANS_FAILD*/
                        //return TRANS_FAILD;    
                    }
                }
            } 
        }         
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_transaction, "Exit \r\n");
    
    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_transaction_init()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_init(void)
{
    struct trans_transaction_list * p_transaction_list = NULL;
    u_int32_t uw_ret = 0;
    //FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter \r\n"); 

    p_transaction_list = &g_trans_transaction_list;
    
    uw_ret = trans_unordered_list_init(&(p_transaction_list->p_unordered_list));
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_unordered_list_init for g_trans_timer_list error! \r\n");
    
        return uw_ret;        
    }  
    
    p_transaction_list->uw_transaction_id = 1;
    p_transaction_list->uw_execute_transaction = 1;   
    
    if (pthread_mutex_init(&(p_transaction_list->ta_mutex), NULL))
    {
        FLOG_ERROR("Init g_trans_transaction_list.ta_mutex error! \r\n");        
        return uw_ret;  
    }
   
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_transaction_release()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_transaction_release(void)
{
    struct trans_transaction_list * p_transaction_list = NULL;
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_transaction *p_transaction = NULL;
    
    void * p_field = NULL;
    //void * p_timer = NULL;

    u_int32_t uw_ret = 0;
    
    p_transaction_list = &g_trans_transaction_list;
    
    p_transaction_list->uw_transaction_id = 0;
    p_transaction_list->uw_execute_transaction = 0;   

    if (NULL == p_transaction_list->p_unordered_list)
    {
        FLOG_ERROR("The list is not init! \r\n");
        return TRANS_FAILD;
    }

    p_unordered_list = p_transaction_list->p_unordered_list;
    
    //cleaup the List
    pthread_mutex_lock(&(p_unordered_list->qmutex));

    TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
    {
        p_transaction = TRANS_UNORDERED_LIST_DATA(p_field);
        
        TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
            
        /*Free p_transaction*/
        uw_ret = trans_transaction_delete(p_transaction);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_delete error! uw_ret = %d\r\n", uw_ret);
            //return;
        }    

        #if 0
        /*Check if add timer*/
        p_timer = trans_transaction_get_timer(p_transaction);
        
        if (NULL != p_timer)
        {
            uw_ret = trans_timer_delete(&g_trans_timer_list, p_timer);
            if(TRANS_SUCCESS != uw_ret) 
            {
                FLOG_ERROR("Call trans_timer_delete error!uw_ret = %d \r\n", uw_ret);
                
                /*Can not return TRANS_FAILD*/
                //return TRANS_FAILD;    
            }
        }
        #endif

    }
   
    pthread_mutex_unlock(&(p_unordered_list->qmutex));

    /*Free Timer List Head*/
    free(p_unordered_list);
    p_unordered_list = NULL;
    
    p_transaction_list->p_unordered_list = NULL;
    p_transaction_list = NULL;

    return TRANS_SUCCESS;
}



