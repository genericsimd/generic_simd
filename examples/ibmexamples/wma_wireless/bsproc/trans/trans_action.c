/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_action.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 05-July.2011      Created                                          E Wulan

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
#include <trans_list.h>
#include <trans_transaction.h>

#include <trans_action.h>
#include <trans_debug.h>



/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/

struct trans_action_list g_trans_action_list;

/*****************************************************************************+
 *Code 
+*****************************************************************************/

/*****************************************************************************+
* Function: trans_action_get_action_id()
* Description: calculate and return the action id
* Parameters:
*           NONE
* Return Values:
*           uw_action_id
*
*  
*  Data:    2011-07-18
* 
+*****************************************************************************/
u_int32_t  trans_action_get_action_id(struct trans_action_list * p_action_list) 
{
    u_int32_t uw_action_id = 0;
    
    pthread_mutex_lock(&(p_action_list->a_mutex));  
    
    uw_action_id = p_action_list->uw_action_id;

    (p_action_list->uw_action_id)++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == p_action_list->uw_action_id)
    {
        p_action_list->uw_action_id = 1;

    }
    
    pthread_mutex_unlock(&(p_action_list->a_mutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_action, "Exit uw_action_id = %d\r\n", uw_action_id); 
    
    return(uw_action_id);
}

/*****************************************************************************+
* Function: trans_action_add()
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
u_int32_t trans_action_add(struct trans_action_info *p_action_info,
                            size_t len,
                            void * p_action_msg)
{
    struct trans_action * p_action = NULL;     
    struct trans_unordered_list * p_unordered_list = NULL; 

    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;
   
    FLOG_DEBUG_TRANS(g_trans_debug_action, "Enter \r\n");    
     
    if ((NULL == p_action_info) || (NULL == p_action_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    /* Allocate a memory.  */
    p_action = (struct trans_action *)malloc(SIZEOF_TRANS_ACTION);
    if (NULL == p_action)
    {
        FLOG_ERROR("Malloc p_action error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_action, 0, SIZEOF_TRANS_ACTION);

    uw_len = len;
    
    p_action->uw_action_id = trans_action_get_action_id(p_action_info->p_action_list);
    p_action->uw_msg_len = len;
    p_action->p_msg = p_action_msg;
    p_action->uw_src_moudle = p_action_info->uw_src_moudle;
    p_action->f_callback = p_action_info->f_callback;
    
    p_action->p_info = p_action_info->p_info;
    
    
    //determine the ordered list for this thread
    p_unordered_list = p_action_info->p_action_list->p_unordered_list; // let us use one global list for now

    //trans_debug_msg_print(p_action, 30, g_trans_debug_action);

    // insert it into to timer/event queue
    uw_ret = trans_unordered_list_add(p_unordered_list, 
                                    SIZEOF_TRANS_ACTION, 
                                    p_action);
    if(TRANS_SUCCESS != uw_ret) 
    {
        //TRACE(4, "Error: inserting a timer in dll ordered lsit\n");
        FLOG_ERROR("Call trans_unordered_list_add error! \r\n");
        return uw_ret;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_action, "Completed add action , action_id = %d, num = %d.\n", 
                        p_action->uw_action_id, p_unordered_list->uw_node_num);

    FLOG_DEBUG_TRANS(g_trans_debug_action, "Exit \r\n"); 

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_action_handler()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
void trans_action_handler(struct trans_action_list * p_action_list)
{
    struct trans_unordered_list *p_unordered_list = NULL;
    struct trans_action * p_action = NULL;

    void * p_field = NULL;
    //void * p_curr = NULL;

    int32_t w_ret = 0;
    struct timeval tv;
    
    if (NULL == p_action_list)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return;
    } 

    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_action, "Enter \r\n");    
    
    p_unordered_list = p_action_list->p_unordered_list;
    
    while (p_action_list->uw_execute_action) 
    {
        if (0 == TRANS_UNORDERED_LIST_NUM(p_unordered_list))
        {
            tv.tv_sec = TRANS_ACTION_DEFULT_INTERVAL;
            tv.tv_usec = 0;
            
            while (select(0,0,0,0,&tv)<0 && errno==EINTR);  

            FLOG_DEBUG_TRANS(g_trans_debug_action, "Action wait. \r\n");
        }
        else
        {
            pthread_mutex_lock(&(p_unordered_list->qmutex));
            
            TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
            {
                p_action = TRANS_UNORDERED_LIST_DATA(p_field);
                
                FLOG_DEBUG_TRANS(g_trans_debug_action, "Action node. \r\n");

                if (NULL != p_action->f_callback)
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_action, "Action start. \r\n");
            
                    FLOG_DEBUG_TRANS(g_trans_debug_action, "src_moudle = %d \r\n", p_action->uw_src_moudle);
                    
                    (*(p_action->f_callback))(p_action->p_info, p_action->uw_msg_len, p_action->p_msg); 
            
                    FLOG_DEBUG_TRANS(g_trans_debug_action, "Action end. \r\n");
                }
                else
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_action, "No action function callback. \r\n");
                }
            
                /*Delete the node from the list*/
                FLOG_DEBUG_TRANS(g_trans_debug_action, "Delete node \r\n"); 
            
                TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field)

                /*Delete transaction ------It could call in  f_callback*/
                if (NULL != p_action->p_info)
                {
                    trans_transaction_set_comn(p_action->p_info, 
                                                            TRANS_TRANSACTION_FLAG_DELETE,
                                                            TRANS_MOUDLE_BUF);
                    trans_transaction_delete(p_action->p_info);
                }
                    
                free(p_action);

                p_action->p_msg = NULL;
                p_action->p_info = NULL;
                p_action = NULL;
                
            }
            
            pthread_mutex_unlock(&(p_unordered_list->qmutex));

        }
    }

    FLOG_DEBUG_TRANS(g_trans_debug_action, "Exit \r\n"); 
    
    return;
}

/*****************************************************************************+
* Function: trans_action_init()
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
u_int32_t trans_action_init(void)
{
    u_int32_t uw_ret = 0;

    struct trans_action_list * p_action_list = NULL;

    p_action_list = &g_trans_action_list;
    
    uw_ret = trans_unordered_list_init(&(p_action_list->p_unordered_list));
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_unordered_list_init error! \r\n");
    
        return uw_ret;        
    } 

    p_action_list->uw_action_id = 1;
    p_action_list->uw_execute_action = 1;   
    
    if (pthread_mutex_init(&(p_action_list->a_mutex), NULL))
    {
        FLOG_ERROR("Init g_trans_action_list.a_mutex error! \r\n");        
        return uw_ret;  
    }
   
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_action_release()
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
u_int32_t trans_action_release(void)
{
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_action_list * p_action_list = NULL;
    struct trans_action *p_action = NULL;
    void * p_field = NULL;
    //void * p_curr = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_action, "Enter \r\n"); 
    
    p_action_list = &g_trans_action_list;

    p_action_list->uw_action_id = 0;
    p_action_list->uw_execute_action = 0;

    if (NULL == p_action_list->p_unordered_list)
    {
        FLOG_ERROR("The list is not init! \r\n");
        return TRANS_FAILD;
    }
    
    p_unordered_list = p_action_list->p_unordered_list;
    
    //cleaup the List
    pthread_mutex_lock(&(p_unordered_list->qmutex));
    
    TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
    {
        p_action = TRANS_UNORDERED_LIST_DATA(p_field);
        
        TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
            
        /*Free Timer*/
        if (NULL != p_action)
        {
            free(p_action);
        }
        
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    /*Free Timer List Head*/
    free(p_unordered_list);
    p_unordered_list = NULL;
    p_action_list->p_unordered_list = NULL;
    p_action_list = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_action, "Exit \r\n"); 
 
    return TRANS_SUCCESS;
}



