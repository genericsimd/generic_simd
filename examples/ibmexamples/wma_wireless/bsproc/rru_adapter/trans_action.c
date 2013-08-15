/*****************************************************************************+
*
*  File Name: trans_action.c
*
*  Function: TRANS ACTION MOUDLE
*
*  
*  Data:    2011-09-05
*  Modify:
*
+*****************************************************************************/

#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
//#include <trans_timer.h>
#include <trans_action.h>
#include <trans_list.h>
#include <trans_debug.h>


/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/

struct trans_ordered_list *g_trans_action_list = NULL;

u_int32_t g_trans_action_id = 1;
pthread_mutex_t  g_trans_action_id_mutex;

static u_int32_t g_trans_execute_action = 1;

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
u_int32_t  trans_action_get_action_id(void) 
{
    u_int32_t uw_action_id = 0;
    
    pthread_mutex_lock(&(g_trans_action_id_mutex));  
    
    uw_action_id = g_trans_action_id;

    g_trans_action_id++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == g_trans_action_id)
    {
        g_trans_action_id = 1;

    }
    
    pthread_mutex_unlock(&(g_trans_action_id_mutex));
    
    FLOG_DEBUG("Exit uw_action_id = %d\r\n", uw_action_id); 
    
    return(uw_action_id);
}

/*****************************************************************************+
* Function: trans_action_fun_exe()
* Description: 
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
u_int32_t trans_action_fun_exe(struct trans_action * p_action)
{
    if (NULL == p_action)
    {
        FLOG_ERROR("NULL PTR!p_action \r\n");
        return TRANS_FAILD;
    }

    // a function can be called
    if (NULL != p_action->f_callback)
    {
        // Call the function. This shouldn't stall the main 
        // timer_handler thread, else spawn a separate thread?

        /*p_user_info could be NULL*/
        if ((NULL == p_action->p_msg) || (0 == p_action->uw_msg_len))
        {
            FLOG_ERROR("NULL PTR!p_msg.  msg_len =  %d.\r\n", p_action->uw_msg_len);

            return TRANS_FAILD;
        }
        else
        {
            /*Print Info*/
            trans_debug_msg_print(p_action->p_msg, 30, g_trans_debug_action);
            
            FLOG_DEBUG("src_moudle = %d \r\n", p_action->uw_src_moudle);

            //(*(p_action->f_callback))(p_action->p_user_info, p_action->uw_msg_len, p_action->p_msg);            

            switch (p_action->uw_src_moudle)
            {
                /*AGENT*/
                case TRANS_MOUDLE_AGENT:            
                /*ACTION*/  
                case TRANS_MOUDLE_ACTION:
            
                    (*(p_action->f_callback))(p_action->p_user_info, p_action->uw_msg_len, p_action->p_msg);            

                    break;
                    
                case TRANS_MOUDLE_MONITOR:

                    (*(p_action->f_callback))(p_action->p_user_info, p_action->uw_msg_len, p_action->p_msg);            
            
                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow source module! src_moudle = %d\r\n", p_action->uw_src_moudle);
                    return TRANS_FAILD;
            
            }    

        }                     
    
        FLOG_DEBUG("Timer out : call  f_callback OK . \r\n");
    }
    else
    {
        FLOG_ERROR("NULL PTR!f_callback \r\n");

        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;

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

    u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;

    struct trans_resp_msg_header *p_resp_msg = NULL;
    
     FLOG_DEBUG("Enter \r\n");    
     
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

    /* Allocate a memory.  */
    //uw_len = sizeof(p_action_info->u_action_info);
    uw_len = len;
    
    p_action->p_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_action->p_msg)
    {
        FLOG_ERROR("Malloc p_action->p_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_action->p_msg, 0, uw_len);
    
    p_action->uw_action_id = trans_action_get_action_id();
    p_action->uw_msg_len = uw_len;
    //p_action->uc_action = p_action_info->uc_action;
    p_action->uw_src_moudle = p_action_info->uw_src_moudle;
    p_action->f_callback = p_action_info->f_callback;
    
    memcpy(p_action->p_msg, p_action_msg, uw_len);   

    if ((TRANS_MOUDLE_MONITOR == p_action->uw_src_moudle)
        ||(TRANS_MOUDLE_ACTION == p_action->uw_src_moudle))
    {
        p_resp_msg = (struct trans_resp_msg_header *)p_action->p_msg;

        p_resp_msg->p_buf = p_action->p_msg + SIZEOF_TRANS_RESP_MSG_HEADER;
    }

    /*p_action->u_action_info : It could be NULL*/
    if (NULL == p_action_info->p_user_info)
    {
        FLOG_DEBUG("No user info! \r\n");

        p_action->p_user_info = NULL;
    }
    else
    {
        p_action->p_user_info = p_action_info->p_user_info;
        
        FLOG_DEBUG("Copy user info! \r\n");
    }
    
    #if 0
    switch (p_action_info->uc_action)
    {
        /* BS METRIC */
        case TRANS_BS_ACTION_GET_METRIC_FROM_BS: 
    
        /* MS METRIC */
        case TRANS_BS_ACTION_GET_METRIC_FROM_MS: 
            
            /* Allocate a memory.  */
            uw_len = sizeof(p_action_info->u_action_info);
            
            p_action->p_msg = (u_int8_t *)malloc(uw_len);
            if (NULL == p_action->p_msg)
            {
                FLOG_ERROR("Malloc p_action->p_msg error! \r\n");
                return TRANS_FAILD;   
            }

            memset((u_int8_t*)p_action->p_msg, 0, uw_len);

            p_action->uw_action_id = trans_action_get_action_id();
            p_action->uw_len = uw_len;
            p_action->uc_action = p_action_info->uc_action;
            p_action->uw_src_moudle = p_action_info->uw_src_moudle;

            memcpy(p_action->p_msg, (u_int8_t *)(&(p_action_info->u_action_info)), uw_len);    
            
            break; 
    
        default:
    
            FLOG_ERROR("Action error uc_action =%d! \r\n", p_action_info->uc_action);
            return TRANS_FAILD;
    
        break;
    
    } 
    #endif
    
    //determine the ordered list for this thread
    struct trans_ordered_list *p_ordered_list = g_trans_action_list; // let us use one global list for now

    trans_debug_msg_print(p_action, 30, g_trans_debug_action);

    //trans_timer_print_timeval("trans_action_add", &(p_action_info->st_tv));

    // insert it into to timer/event queue
    uw_ret = trans_ordered_list_insert(p_ordered_list, p_action, 
                                    &(p_action_info->st_tv), 
                                    SIZEOF_TRANS_ACTION, 
                                    &uw_num_elems);
    if(TRANS_SUCCESS != uw_ret) 
    {
        //TRACE(4, "Error: inserting a timer in dll ordered lsit\n");
        FLOG_ERROR("Inserting a action in dll ordered lsit error! \r\n");
        return uw_ret;
    }

    FLOG_DEBUG("Completed add action , action_id = %d, num = %d.\n", 
                        p_action->uw_action_id, uw_num_elems);

    FLOG_DEBUG("Exit \r\n"); 

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
void trans_action_handler(void* p_action_list)
{
    struct trans_ordered_list *p_ordered_list = NULL;
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    struct timespec ts;
    struct timeval tv;
    
    if (NULL == p_action_list)
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return;
    } 

    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("2 Call pthread_setcanceltype error! \r\n");    
        return ;
    }

    p_ordered_list = (struct trans_ordered_list *)p_action_list;
    
    while (g_trans_execute_action) 
    {
        u_int32_t uw_num_elems = 0;
        struct trans_action * p_action = NULL;
        void * pp_action = NULL;
        w_ret = 0;
        int32_t w_min_timer = 0;
        
        if (trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_action, &uw_num_elems)) 
        {
            FLOG_ERROR("3 Call trans_ordered_list_peek_head error! \r\n");
        }

        p_action = (struct trans_action *)pp_action;

        //if(dol is empty)
        // block and wait for 1s
        if (0 == uw_num_elems) 
        {
            pthread_mutex_lock (&(p_ordered_list->qmutex));
            
            gettimeofday(&tv, NULL);
            
            tv.tv_sec +=1;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = (tv.tv_usec) * 1000;
            //wait for 1 sec if timer q is empty
            FLOG_DEBUG("4 Action Q empty, wait for 1 sec \r\n"); 
       
            while ((0 == p_ordered_list->uc_head_chgd)&&(0 == w_ret)) 
            {
                w_ret = pthread_cond_timedwait( &(p_ordered_list->checkq), &(p_ordered_list->qmutex), &ts);
            }
            
            if (p_ordered_list->uc_head_chgd) 
            {
                w_min_timer=1;
                p_ordered_list->uc_head_chgd=0;
            }
            
            pthread_mutex_unlock(&(p_ordered_list->qmutex));
            
            //We are out of timed wait
            //dol head changed
            if(w_min_timer) 
            {
                FLOG_DEBUG("5 Head changed \r\n"); 
                //continue;
            }

        }
        else 
        {
            /*Process the action*/
            if (NULL == p_action)
            {
                FLOG_ERROR("6 NULL PTR! \r\n");
                continue;
            }

            while((NULL != p_action) && (0 != uw_num_elems))
            {
                if(trans_ordered_list_pop_head(p_ordered_list, (void**)&pp_action, &uw_num_elems)) 
                {
                    //TRACE(4, "Unable to pop timer Q\n");
                    FLOG_ERROR("7 Call trans_ordered_list_pop_head error! \r\n");
                }
                p_action = (struct trans_action *)pp_action;
                /*Action*/
                //trans_debug_msg_print(p_action, 30, g_trans_debug_action);
                
                FLOG_DEBUG("8 Action start . \r\n");
                
                uw_ret = trans_action_fun_exe(p_action);
                if(TRANS_SUCCESS != uw_ret) 
                {
                    FLOG_ERROR("Call trans_action_exe error!uw_ret = %d. \r\n", uw_ret);
                    continue;
                }
                 
                free(p_action->p_msg);
                free(p_action);

                p_action->p_msg = NULL;
                p_action = NULL;
               
                FLOG_DEBUG("9 Action end . \r\n");

                if (trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_action, &uw_num_elems)) 
                {
                    FLOG_ERROR("10 Call trans_ordered_list_peek_head error! \r\n");
                }
                p_action = (struct trans_action *)pp_action;
            }               
        }
       
    }

    return;
}

/*****************************************************************************+
* Function: trans_action_delete_by_src()
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
u_int32_t trans_action_delete_by_src(u_int32_t uw_src_moudle)
{
    struct trans_ordered_list *p_ordered_list = g_trans_action_list;
    
    u_int32_t uw_num_elems = 0;
    struct trans_action * p_action = NULL;
    struct trans_list *p_crru = NULL;
    
    uw_num_elems = p_ordered_list->uw_node_num;
    
    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    p_crru = p_ordered_list->p_head;
    
    FLOG_DEBUG("p_crru = %p \n", p_crru);
    
    while ((0 != uw_num_elems) || (NULL != p_crru))
    {
        p_action = (struct trans_action *)p_crru->p_data;
        
        FLOG_INFO("uw_num_elems = %d.  p_timer = %p, src_moudle = %d\n", 
                    uw_num_elems, p_action, p_action->uw_src_moudle);
    
         /*Check serial number*/
        if (uw_src_moudle == p_action->uw_src_moudle)
        {
            /*Free Action Node*/
            free(p_action->p_msg);
            free(p_action);
            
            p_action->p_msg = NULL;
            p_action = NULL;

        }
        else
        {
            p_crru = p_crru->p_next;
            uw_num_elems--;
        }  
    }
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    return TRANS_SUCCESS;
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
    
    uw_ret = trans_ordered_list_init(&g_trans_action_list);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_ordered_list_init error! \r\n");
    
        return uw_ret;        
    } 
    
    g_trans_action_id = 1;   
    g_trans_execute_action = 1;    
   
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
    struct trans_ordered_list * p_ordered_list = NULL;
    struct trans_list  *p_list = NULL;
    struct trans_list  *p_tmp_cntr = NULL;
    struct trans_action *p_action = NULL;

    FLOG_DEBUG("Enter \r\n"); 
    
    p_ordered_list = g_trans_action_list;

    //cleaup the List
    //pthread_mutex_lock(&(p_ordered_list->qmutex));

    p_list = p_ordered_list->p_head;
    
    while(NULL != p_list)
    {
        p_tmp_cntr = p_list;
        p_action = p_tmp_cntr->p_data;

        //FLOG_DEBUG("p_action = %p \r\n", p_action); 
        
        /*Free Msg*/
        free(p_action->p_msg);     
        /*Free Timer*/
        free(p_tmp_cntr->p_data);
        /*Free List*/
        free(p_tmp_cntr);

        /*Find the next list node*/
        p_list = p_list->p_next;
    }
    
    p_ordered_list->uc_head_chgd = 1;
    p_ordered_list->uw_node_num = 0;
    p_ordered_list->p_head = NULL;
   
    //pthread_mutex_unlock(&(p_ordered_list->qmutex));

    /*Free Timer List Head*/
    free(g_trans_action_list);
    g_trans_action_list = NULL;
    
    g_trans_action_id = 1;

    FLOG_DEBUG("Exit \r\n"); 
 
    return TRANS_SUCCESS;
}



