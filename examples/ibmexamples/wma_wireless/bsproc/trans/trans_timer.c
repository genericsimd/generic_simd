/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_timer.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Mar.2012      Created                                          E Wulan

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
#include <trans_timer.h>
#include <trans_debug.h>
//#include <trans_device.h>

struct trans_timer_list g_trans_timer_list;

u_int32_t g_trans_timer_counter = 0;


/*****************************************************************************+
* Function: trans_timer_get_index()
* Description: calculate and return the timer index
* Parameters:
*           NONE
* Return Values:
*           uw_action_id
*
*  
*  Data:    2012-03-06
+*****************************************************************************/
u_int32_t  trans_timer_get_index(struct trans_timer_list * p_timer_list) 
{
    u_int32_t uw_timer_index = 0;
    
    pthread_mutex_lock(&(p_timer_list->t_mutex));  
    
    uw_timer_index = p_timer_list->uw_timer_id;

    (p_timer_list->uw_timer_id)++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == p_timer_list->uw_timer_id)
    {
        p_timer_list->uw_timer_id = 1;
    }
    
    pthread_mutex_unlock(&(p_timer_list->t_mutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit uw_device_index = %d\r\n", uw_timer_index); 
    
    return(uw_timer_index);
}

/*****************************************************************************+
* Function: trans_device_cmp()
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
int trans_timer_cmp(void *p_data, void *p_key, void *p_result)
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
* Function: trans_timer_get_in()
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
u_int32_t trans_timer_get_in(struct trans_timer_list * p_timer_list,
                                                 void *p_timer,
                                                 void ** pp_data) 
{
    struct trans_unordered_list * p_unordered_list = NULL;
    u_int32_t uw_ret = 0;
    
    *pp_data = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter\r\n");

    p_unordered_list = p_timer_list->p_unordered_list;

    uw_ret = trans_unordered_list_get_in(p_unordered_list, 
                            p_timer,
                            trans_timer_cmp,
                            pp_data);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_in error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }
    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_timer_add()
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
u_int32_t trans_timer_add(struct trans_timer_info * p_timer_info,
            void** pp_timer)
{
    struct trans_timer * p_timer = NULL;     
    struct trans_unordered_list * p_unordered_list = NULL; 
    u_int32_t uw_ret = 0;

    (*pp_timer) = NULL;

    if (NULL == p_timer_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter \r\n"); 

    /* Allocate a memory.  */
    p_timer = (struct trans_timer *)malloc(SIZEOF_TRANS_TIMER);
    if (NULL == p_timer)
    {
        FLOG_ERROR("1 malloc p_timer error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_timer, 0, SIZEOF_TRANS_TIMER);
 
    // determine relative time of timeout
    
    p_timer->uw_timer_id = trans_timer_get_index(p_timer_info->p_timer_list);
    
    p_timer->uw_interval = ((p_timer_info->uw_interval)*1000);
    p_timer->uw_counter = p_timer->uw_interval;
    p_timer->uc_type = p_timer_info->uc_type;
    p_timer->uc_state = TRANS_TIMER_STATE_ALIVE;
    p_timer->p_data = p_timer_info->p_data;
    p_timer->f_callback = p_timer_info->f_callback;

    /*insert it into to timer/event queue*/
    p_unordered_list = p_timer_info->p_timer_list->p_unordered_list;
    
    uw_ret = trans_unordered_list_add(p_unordered_list, SIZEOF_TRANS_TIMER, p_timer);
    if(TRANS_SUCCESS != uw_ret) 
    {
        //TRACE(4, "Error: inserting a timer in dll ordered lsit\n");
        FLOG_ERROR("Call trans_unordered_list_add error! \r\n");
        return uw_ret;
    }

    *pp_timer = (void*)p_timer;

    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Completed trans_timer_add timer:%p\n", *pp_timer);
    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Exit \r\n"); 

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_timer_set_delete()
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
u_int32_t trans_timer_set_delete(void* p_timer)
{

    if (NULL == p_timer)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    ((struct trans_timer *)p_timer)->uc_state = TRANS_TIMER_STATE_STOP;    

    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Exit p_timer = %p\r\n", p_timer); 
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_timer_delete()
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
u_int32_t trans_timer_delete(struct trans_timer_list * p_timer_list,
                                        void* p_timer)
{
    u_int32_t uw_ret = 0;
    struct trans_unordered_list * p_unordered_list = NULL; 
    
    void * p_data = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter. \r\n");  
    
    p_unordered_list = p_timer_list->p_unordered_list;
    
    uw_ret = trans_timer_get_in(p_timer_list, p_timer, &p_data); 
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_timer_get_in error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }
    
    /*If not timeout----Can not find the timer*/
    if (NULL == p_data)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_timer, "Do not find timer. \r\n");  
        return TRANS_FAILD;  
    }
    else
    {
        /*Delete Timer*/
        uw_ret = trans_timer_set_delete(p_timer);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call trans_timer_set_delete error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;     
        } 
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_timer, "Exit. \r\n"); 

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_timer_handler()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
void trans_timer_handler(struct trans_timer_list * p_timer_list)
{
    struct trans_unordered_list *p_unordered_list = NULL;
    struct trans_timer *p_timer = NULL;
    
    void * p_field = NULL;
    //void * p_curr = NULL;
    
    int32_t w_ret = 0;

    u_int8_t uc_delete_flag = 0;
    
    u_int32_t uw_delay = TRANS_TIMER_DEFULT_INTERVAL;
    u_int32_t uw_min_time = TRANS_TIMER_DEFULT_INTERVAL;

    struct timeval tv;
    
    if (NULL == p_timer_list)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return;
    } 

    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }

    p_unordered_list = (struct trans_unordered_list *)(p_timer_list->p_unordered_list);
    
    while (p_timer_list->uw_execute_timer) 
    {
        //uw_delay = TRANS_MIN(TRANS_TIMER_DEFULT_INTERVAL, uw_min_time);
        uw_delay = uw_min_time;

        tv.tv_sec = 0;
        tv.tv_usec = uw_delay*1000;

        while ((select(0,0,0,0,&tv) < 0) && (errno==EINTR));

        pthread_mutex_lock(&(p_unordered_list->qmutex));
        
        TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
        {
            //p_timer->uw_counter<uw_delay?p_timer->uw_counter=0:p_timer->uw_counter-=uw_delay;
            p_timer = TRANS_UNORDERED_LIST_DATA(p_field);
            
            uc_delete_flag = 0;
            uw_min_time = TRANS_TIMER_DEFULT_INTERVAL;
            
            FLOG_DEBUG_TRANS(g_trans_debug_timer, "**** %p, %p\r\n", p_timer, p_field); 

            if (NULL == p_timer)
            {
                FLOG_ERROR("NULL PTR!p_timer \r\n");
                continue;
            }
        
            FLOG_DEBUG_TRANS(g_trans_debug_timer, "1 uw_counter = %d \r\n", p_timer->uw_counter); 
            
            if ((p_timer->uw_counter) <= uw_delay)
            {
                p_timer->uw_counter = 0;
            }
            else
            {
                p_timer->uw_counter -= uw_delay;

                uw_min_time = TRANS_MIN(p_timer->uw_counter, uw_min_time);
            }
            
            FLOG_DEBUG_TRANS(g_trans_debug_timer, "2 uw_counter = %d \r\n", p_timer->uw_counter); 
       
            FLOG_DEBUG_TRANS(g_trans_debug_timer, "uw_min_time = %d \r\n", uw_min_time); 
            FLOG_DEBUG_TRANS(g_trans_debug_timer, "uc_state = %d \r\n", p_timer->uc_state); 

            if ((TRANS_TIMER_STATE_ALIVE == p_timer->uc_state)
                &&(0 != p_timer->uw_counter))
            {
                /*Go on next node*/
                uc_delete_flag = 0;
                
                FLOG_DEBUG_TRANS(g_trans_debug_timer, "Go on next node\r\n"); 
        
            }
            else if ((TRANS_TIMER_STATE_STOP == p_timer->uc_state)
                &&(0 != p_timer->uw_counter))
            {
                /*Free Timer*/
                uc_delete_flag = 1;
        
                FLOG_DEBUG_TRANS(g_trans_debug_timer, "1 stop\r\n"); 
            }
            /*timeout*/
            else if ((TRANS_TIMER_STATE_ALIVE == p_timer->uc_state)
                &&(0 == p_timer->uw_counter))
            {
                FLOG_DEBUG_TRANS(g_trans_debug_timer, "Timer out, call function\r\n"); 
        
                if (NULL != p_timer->f_callback)
                {
                    (*(p_timer->f_callback))(p_timer->p_data, TRANS_TIMER_STATE_TIMEOUT, NULL);
                }
        
                if (TRANS_TIMER_TYPE_ONCE == p_timer->uc_type)
                {
                    /*Free Timer*/
                    uc_delete_flag = 1;
                }
                else if (TRANS_TIMER_TYPE_CIRCLE == p_timer->uc_type)
                {
                    /*reset the counter*/
                    p_timer->uw_counter = p_timer->uw_interval;
                    uc_delete_flag = 0;
                }
                /*ERROR*/
                else
                {
                    /*Free Timer*/
                    uc_delete_flag = 1;
                    
                    FLOG_ERROR("Timer type %d error! \r\n", p_timer->uc_type);    
                }
            }
            else
            {
                /*Free Timer*/
                uc_delete_flag = 1;
        
                FLOG_DEBUG_TRANS(g_trans_debug_timer, "2 stop\r\n"); 
            }
        
            /*Free Timer*/
            if (1 == uc_delete_flag)
            {
                /*Delete the node from the list*/
                FLOG_DEBUG_TRANS(g_trans_debug_timer, "Delete node \r\n"); 
                //FLOG_DEBUG_TRANS(g_trans_debug_timer, "7777**** %p, %p, %p \r\n", p_timer, p_curr, p_field); 
              
                TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
                //trans_unordered_list_delete(p_unordered_list, p_timer);
                    
                free(p_timer);
                p_timer = NULL;
            }
        }
        
        pthread_mutex_unlock(&(p_unordered_list->qmutex));

    }

    return;
}

/*****************************************************************************+
* Function: trans_timer_init()
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
u_int32_t trans_timer_init(void)
{
    struct trans_timer_list * p_timer_list = NULL;
    u_int32_t uw_ret = 0;
    //FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter \r\n"); 

    p_timer_list = &g_trans_timer_list;
    
    uw_ret = trans_unordered_list_init(&(p_timer_list->p_unordered_list));
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_unordered_list_init for g_trans_timer_list error! \r\n");
    
        return uw_ret;        
    }  
    
    p_timer_list->uw_timer_id = 1;
    p_timer_list->uw_execute_timer = 1;   
    
    if (pthread_mutex_init(&(p_timer_list->t_mutex), NULL))
    {
        FLOG_ERROR("Init g_trans_timer_list.t_mutex error! \r\n");        
        return uw_ret;  
    }

    g_trans_timer_counter++;
   
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_timer_release()
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
u_int32_t trans_timer_release(void)
{
    struct trans_timer_list * p_timer_list = NULL;
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_timer *p_timer = NULL;
    void * p_field = NULL;
    //void * p_curr = NULL;
    
    p_timer_list = &g_trans_timer_list;
    p_timer_list->uw_execute_timer = 0;
    p_timer_list->uw_timer_id = 1;

    if (NULL == p_timer_list->p_unordered_list)
    {
        FLOG_ERROR("The list is not init! \r\n");
        return TRANS_FAILD;
    }

    p_unordered_list = p_timer_list->p_unordered_list;
    
    //cleaup the List
    pthread_mutex_lock(&(p_unordered_list->qmutex));

    TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
    {
        p_timer = TRANS_UNORDERED_LIST_DATA(p_field);
        
        TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
            
        if (NULL != p_timer)
        {
            /*Free Timer*/
            free(p_timer);
        }

    }
   
    pthread_mutex_unlock(&(p_unordered_list->qmutex));

    /*Free Timer List Head*/
    free(p_unordered_list);
    p_unordered_list = NULL;
    p_timer_list->p_unordered_list = NULL;
    p_timer_list = NULL;
   
    g_trans_timer_counter = 0;
 
    return TRANS_SUCCESS;
}


