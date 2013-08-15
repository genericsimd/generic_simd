
/*****************************************************************************+
*
*  File Name: trans_timer.c
*
*  Function: TRANS Timer
*
*  
*  Data:    2011-04-14
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
#include <trans_timer.h>
#include <trans_list.h>
#include <trans_debug.h>

struct trans_ordered_list *g_p_timer_list = NULL;

u_int32_t g_trans_timer_counter = 0;

pthread_mutex_t g_timer_mutex;

u_int32_t g_execute_timer=1;

//extern pthread_cond_t  g_ad_msg_thread_cond;

/*****************************************************************************+
* Function: trans_timer_compare()
* Description: compare the 2 timer
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_compare(const struct timeval *tv1, const struct timeval *tv2)
{
    u_int32_t  uw_comp = 0;

    if (tv1->tv_sec > tv2->tv_sec)
    {
        uw_comp = TRANS_TIMER_COMPARE_TV1;

    }     
    else if (tv1->tv_sec < tv2->tv_sec)
    {
        uw_comp = TRANS_TIMER_COMPARE_TV2;

    } 
    else if (tv1->tv_usec > tv2->tv_usec)
    {

        uw_comp = TRANS_TIMER_COMPARE_TV1;

    } 
    else if (tv1->tv_usec < tv2->tv_usec)
    {

        uw_comp = TRANS_TIMER_COMPARE_TV2;

    } 
    else
    {
        uw_comp = TRANS_TIMER_COMPARE_EQUAL;

    } 
    
    FLOG_DEBUG("Exit  uw_comp = %d\n", uw_comp);

    return uw_comp;

}

/*****************************************************************************+
* Function: trans_timer_subtracte()
* Description: Calculate the difference between the two time
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-14
* 
+*****************************************************************************/
void trans_timer_subtracte(const struct timeval *tv1, const struct timeval *tv2, 
                struct timeval *tvres)
{
    const struct timeval *tmptv1, *tmptv2;
    u_int32_t  uw_cmpres = 0;

    /*compare tv1 and tv2, set the larger to tmptv1 ,or..*/
    uw_cmpres = trans_timer_compare(tv1, tv2);

    if (TRANS_TIMER_COMPARE_TV1 == uw_cmpres)
    {

        tmptv1 = tv1;

        tmptv2 = tv2;
    } 
    else
    {

        tmptv1 = tv2;

        tmptv2 = tv1;   
    }


    /*  */

    if (tmptv1->tv_usec < tmptv2->tv_usec) 
    {
        /* */
        tvres->tv_sec = tmptv1->tv_sec - tmptv2->tv_sec - 1;

        /*1 second == 1000000 microsecond*/
        tvres->tv_usec = tmptv1->tv_usec + 1000000 - tmptv2->tv_usec;

    } 
    else 
    {
        tvres->tv_sec = tmptv1->tv_sec - tmptv2->tv_sec;

        tvres->tv_usec = tmptv1->tv_usec - tmptv2->tv_usec;
    }

    trans_timer_print_timeval("Subtracte", tvres);  

    return;
}

/*****************************************************************************+
* Function: trans_timer_window()
* Description: Calculate the difference between the two time if within a certain window
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_window(const struct timeval *tv1, const struct timeval *tv2, 
                u_int32_t uw_window_len)
{
    struct timeval tvres;

    u_int32_t uw_Flag = 0;

    trans_timer_subtracte(tv1, tv2, &tvres);

    if (tvres.tv_sec > 0)
    {
        uw_Flag = TRANS_TIMER_OUT_WINDOW;

    }    
    else if (tvres.tv_sec < 0)
    {
        uw_Flag = TRANS_TIMER_WINDOW_ERROR;

    }
    else
    {
        if (tvres.tv_usec > uw_window_len)
        {
            uw_Flag = TRANS_TIMER_OUT_WINDOW;

        }
        /* within 100 micro seconds */
        else
        {
            uw_Flag = TRANS_TIMER_IN_WINDOW;

        }        
    }
    
    FLOG_DEBUG("Exit  uw_Flag = %d\n", uw_Flag);

    return uw_Flag;

}

/*****************************************************************************+
* Function: trans_timer_print_timeval()
* Description: print timeval
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_print_timeval(const char *str, const struct timeval *tv)
{
    FLOG_DEBUG("%s = %ld sec %ld usec\n", str, tv->tv_sec, tv->tv_usec);

    (void)str;
    (void)tv;

    return TRANS_SUCCESS; 
}

/*****************************************************************************+
* Function: trans_timer_find_by_serial_num()
* Description: Find
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
*
+******************************************************************************/
u_int32_t trans_timer_find_by_serial_num(u_int16_t us_s_num, 
                            u_int8_t *p_msg_info, 
                            u_int8_t *p_find_flag)
{   
    
    struct trans_ordered_list *p_ordered_list = g_p_timer_list; // let us use one global list for now

    u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;
    struct trans_timer *p_timer = NULL;
    struct trans_list *p_crru = NULL;

    *p_find_flag = 0;

    //(*pp_data) = NULL;
    
    if (NULL == p_find_flag)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    uw_num_elems = p_ordered_list->uw_node_num;
    
    pthread_mutex_lock(&(p_ordered_list->qmutex));

    p_crru = p_ordered_list->p_head;

    FLOG_DEBUG("p_crru = %p \n", p_crru);

    while ((0 != uw_num_elems) || (NULL != p_crru))
    {
        p_timer = (struct trans_timer *)p_crru->p_data;
        
        FLOG_DEBUG("uw_num_elems = %d.  p_timer = %p, serial_number = %d\n", 
                    uw_num_elems, p_timer, p_timer->st_msg_info.us_serial_number);

         /*Check serial number*/
        if (us_s_num == p_timer->st_msg_info.us_serial_number)
        {
            if (TRANS_TIMER_DELETE != p_timer->uc_deleted)
            {
                //(*pp_data) = p_timer;
                memcpy((u_int8_t *)p_msg_info, (u_int8_t *)(&(p_timer->st_msg_info)), SIZEOF_TRANS_TIMER_MSG_INFO);
                
                *p_find_flag = 1;     
                
                /*Delete Timer*/
                uw_ret = trans_timer_delete(p_timer);
                if (TRANS_SUCCESS != uw_ret) 
                {   
                    FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
                    return TRANS_FAILD;     
                } 
            }

            break;
            
        }
        else
        {

            p_crru = p_crru->p_next;
            uw_num_elems--;
        }  
    }
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    
    FLOG_DEBUG("Exit *p_find_flag = %d \n", *p_find_flag);   
    
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
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_add(const struct timeval *tv,
            trans_timeout_action f_ptr,
            void* p_data,
            size_t len,
            struct trans_timer_msg_info *p_msg_info,
            void** pp_timer_id)
{
    FLOG_DEBUG("Enter \r\n"); 
   
    struct trans_timer * p_timer = NULL;     
    (*pp_timer_id) = NULL;

    u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;

    if ((NULL == tv) || (NULL == f_ptr) || (NULL == p_data) || (NULL == p_msg_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    /* Allocate a memory.  */
    p_timer = (struct trans_timer *)malloc(SIZEOF_TRANS_TIMER);
    if (NULL == p_timer)
    {
        FLOG_ERROR("1 malloc p_timer error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_timer, 0, SIZEOF_TRANS_TIMER);

    /* Allocate a memory.  */
    p_timer->p_msg = (u_int8_t *)malloc(len);
    if (NULL == p_timer->p_msg)
    {
        FLOG_ERROR("2 malloc p_timer->p_msg error! \r\n");
        return TRANS_FAILD;   
    }
   
    memset((u_int8_t*)p_timer->p_msg, 0, len);
    
    //determine the ordered list for this thread
    struct trans_ordered_list *p_ordered_list = g_p_timer_list; // let us use one global list for now

    // determine timer id
    #if (defined TRANS_BS_COMPILE) || (defined TRANS_RRH_COMPILE)
    p_timer->ll_timer_id = (long long int)((void*)p_timer);
    #endif

    #ifdef TRANS_MS_COMPILE
    p_timer->ll_timer_id = (int)((void*)p_timer);
    #endif

    *pp_timer_id = (void*)p_timer;

    // determine absolute time of timeout
    p_timer->st_expired.tv_sec = tv->tv_sec;
    p_timer->st_expired.tv_usec = tv->tv_usec;

    p_timer->uc_deleted = TRANS_TIMER_NO_DELETE;
    //p_timer->st_msg_info.uw_src_moudle = p_msg_info->uw_src_moudle;
    //p_timer->st_msg_info.us_serial_number = p_msg_info->us_serial_number;

    p_timer->len = len;

    //FLOG_DEBUG(" %p %p\n",&(p_timer->st_msg_info), p_msg_info);
    //FLOG_DEBUG(" len = %d \n",len);
    memcpy(&(p_timer->st_msg_info), p_msg_info, SIZEOF_TRANS_TIMER_MSG_INFO);

    p_timer->st_msg_info.f_callback = f_ptr;

    memcpy(p_timer->p_msg, (u_int8_t *)p_data, len); 

    //trans_timer_print_timeval("trans_timer_add", tv);

    // insert it into to timer/event queue

    //FLOG_ERROR("user info %p! \r\n", p_timer->st_msg_info.p_user_info);
    
    uw_ret = trans_ordered_list_insert(p_ordered_list, p_timer, &(p_timer->st_expired), SIZEOF_TRANS_TIMER, &uw_num_elems);
    if(TRANS_SUCCESS != uw_ret) 
    {
        //TRACE(4, "Error: inserting a timer in dll ordered lsit\n");
        FLOG_ERROR("3 Inserting a timer in dll ordered lsit error! \r\n");
        return uw_ret;
    }

    //TRACE3(6, "Completed app_timer_add timer id:%p absolute time:%lld abstime:%lld\n", (*timer_id), a_t->absolute_time, abstime);
    FLOG_DEBUG("Completed trans_timer_add timer id:%p\n", p_timer);

    FLOG_DEBUG("Exit \r\n"); 

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
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_delete(void* p_timer_id)
{

    if (NULL == p_timer_id)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    ((struct trans_timer *)p_timer_id)->uc_deleted = TRANS_TIMER_DELETE;    

    FLOG_DEBUG("Exit p_timer_id = %p\r\n", p_timer_id); 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_timer_delete_by_src()
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
u_int32_t trans_timer_delete_by_src(u_int32_t uw_src_moudle)
{
    struct trans_ordered_list *p_ordered_list = g_p_timer_list; // let us use one global list for now
    
    u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;
    struct trans_timer *p_timer = NULL;
    struct trans_list *p_crru = NULL;
    
    struct trans_resp_msg_header *p_resp_msg;
    
    p_resp_msg = (struct trans_resp_msg_header *)malloc(SIZEOF_TRANS_RESP_MSG_HEADER);
    if (NULL == p_resp_msg)
    {
        FLOG_ERROR("malloc p_resp_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_num_elems = p_ordered_list->uw_node_num;
    
    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    p_crru = p_ordered_list->p_head;
    
    FLOG_DEBUG("p_crru = %p \n", p_crru);
    
    while ((0 != uw_num_elems) || (NULL != p_crru))
    {
        p_timer = (struct trans_timer *)p_crru->p_data;
        
        FLOG_INFO("uw_num_elems = %d.  p_timer = %p, src_moudle = %d\n", 
                    uw_num_elems, p_timer, p_timer->st_msg_info.uw_src_moudle);
    
         /*Check serial number*/
        if ((uw_src_moudle == p_timer->st_msg_info.uw_src_moudle)
            && (TRANS_TIMER_DELETE != p_timer->uc_deleted))
        {
            /*execute the function callback*/
            switch (p_timer->st_msg_info.uw_src_moudle)
            {
                /*MONITOR*/  
                case TRANS_MOUDLE_MONITOR:
                /*ACTION*/  
                case TRANS_MOUDLE_ACTION:
           
                    p_resp_msg->uc_result = TRANS_ACK_FLAG_CLEAN_OPERATION;
                    p_resp_msg->uw_len = 0;
                    p_resp_msg->p_buf = NULL;
                    
                    (*(p_timer->st_msg_info.f_callback))((p_timer->st_msg_info.p_user_info), 
                                                SIZEOF_TRANS_RESP_MSG_HEADER, 
                                                p_resp_msg);
            
                    break;
                    
                /*BS*/ 
                case TRANS_MOUDLE_BS:                     
                /*LOCAL*/
                case TRANS_MOUDLE_LOCAL:
                /*AGENT*/
                case TRANS_MOUDLE_AGENT:
                                  
                default:
                    
                    FLOG_ERROR("Unknow source module! src_moudle = %d\r\n", p_timer->st_msg_info.uw_src_moudle);
                    uw_ret = TRANS_FAILD;
            
            }    

            /*Delete Timer*/
            uw_ret = trans_timer_delete(p_timer);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
                //return TRANS_FAILD;     
            } 
        }
        else
        {
    
            p_crru = p_crru->p_next;
            uw_num_elems--;
        }  
    }

    free(p_resp_msg);
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_timer_fun_exe()
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
u_int32_t trans_timer_fun_exe(struct trans_timer *p_timer)
{
    struct trans_resp_msg_header  st_response;
    if (NULL == p_timer)
    {
        FLOG_ERROR("NULL PTR!p_action \r\n");
        return TRANS_FAILD;
    }
    
    //cosider only the non deleted timers
    FLOG_DEBUG("Timer out : call  f_callback.\r\n"); 
    
    // On timer expiry, either a message can be enqueued in the
    // a function can be called
    if (NULL != p_timer->st_msg_info.f_callback)
    {
        // Call the function. This shouldn't stall the main 
        // timer_handler thread, else spawn a separate thread?
        if ((NULL == p_timer->p_msg) || (0 == p_timer->len))
        {
            FLOG_ERROR("NULL PTR!p_msg.  msg_len =  %d.\r\n", p_timer->len);
            return TRANS_FAILD;
        }
        else
        {
            /*Print Info*/
            
            FLOG_DEBUG("len = %ld, src_moudle = %d! \r\n", 
                p_timer->len, p_timer->st_msg_info.uw_src_moudle);

            switch (p_timer->st_msg_info.uw_src_moudle)
            {
                /*BS*/ 
                case TRANS_MOUDLE_BS:                     
                /*LOCAL*/
                case TRANS_MOUDLE_LOCAL:
                    (*(p_timer->st_msg_info.f_callback))(p_timer->p_msg, p_timer->len, &(p_timer->st_msg_info));
                    break;

                /*ACTION*/  
                case TRANS_MOUDLE_ACTION:
                /*AGENT*/
                case TRANS_MOUDLE_AGENT:

                    st_response.uc_result = TRANS_ACK_FLAG_RRH_TIMEOUT;
                    st_response.uw_len = 0;
                    st_response.p_buf = NULL;
                    
                    (*(p_timer->st_msg_info.f_callback))((p_timer->st_msg_info.p_user_info), 
                                                SIZEOF_TRANS_RESP_MSG_HEADER, 
                                                &st_response);

                    break;
                    
                case TRANS_MOUDLE_MONITOR:

                    //break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow source module! src_moudle = %d\r\n", p_timer->st_msg_info.uw_src_moudle);
                    return TRANS_FAILD;
            
            }       
   
        }                     
    
        FLOG_DEBUG("Timer out : call  f_callback OK . \r\n");
    }
    else
    {
        FLOG_DEBUG("Timer out : No  f_callback . src_moudle =%d. \r\n", p_timer->st_msg_info.uw_src_moudle);
        return TRANS_FAILD;
    }

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
*  Data:    2011-04-14
* 
+*****************************************************************************/
void trans_timer_handler(void* p_timer_list)
{
    struct trans_ordered_list *p_ordered_list = NULL;
    int32_t w_ret = 0;
    struct timespec ts;
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

    //pthread_cond_signal(&g_ad_msg_thread_cond);

    p_ordered_list = (struct trans_ordered_list *)p_timer_list;
    
    while (g_execute_timer) 
    {
        u_int32_t uw_num_elems = 0;
        struct trans_timer *p_timer = NULL;
        void * pp_timer = NULL;
        w_ret = 0;
        int32_t w_min_timer = 0;
    
        if (trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
        {
            //TRACE(4, "Unable to peek timer Q\n");
            FLOG_ERROR("2 Call trans_ordered_list_peek_head error! \r\n");
        }
        p_timer = (struct trans_timer *)pp_timer;
        //remove all deleted timers
        while ((p_timer != NULL)&&(TRANS_TIMER_DELETE == p_timer->uc_deleted)) 
        {
            FLOG_DEBUG("Delete node \r\n"); 
            
            if(trans_ordered_list_pop_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
            {
                //TRACE(4, "Unable to pop timer Q\n");
                FLOG_ERROR("3 Call trans_ordered_list_pop_head error! \r\n");
            }
            p_timer = (struct trans_timer *)pp_timer;

            if ((NULL == p_timer->p_msg) || (NULL == p_timer) || (TRANS_TIMER_DELETE != p_timer->uc_deleted))
            {
                FLOG_ERROR("4 NULL PTR! \r\n");
                break;
            }
            
            //free the expired app_timer
            free(p_timer->p_msg);
            free(p_timer);
            
            if(trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
            {
                //TRACE(4, "Unable to peek timer Q\n");
                FLOG_ERROR("5 Call trans_ordered_list_peek_head error! \r\n");
            }
            p_timer = (struct trans_timer *)pp_timer;
        }
        
    
        pthread_mutex_lock (&(p_ordered_list->qmutex));

        //if(dol is empty)
        // block and wait for 500 ms
        if (0 == uw_num_elems) 
        {
            gettimeofday(&tv, NULL);
            
            tv.tv_sec +=1;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = (tv.tv_usec) * 1000;
            //wait for 1 sec if timer q is empty
            //TRACE(6, "Timer Q empty, wait for 1 sec\n");
            FLOG_DEBUG("Timer Q empty, wait for 1 sec \r\n"); 
        }
        else 
        {
            // wait on the earliest event
            //timed wait on earliest event from dol
            ts.tv_sec = p_timer->st_expired.tv_sec;
            ts.tv_nsec = (p_timer->st_expired.tv_usec)*1000;

            //TRACE2(6, "Timer Q Not empty, timer_id:%p wait for:%lld\n", p_timer->timer_id, p_timer->absolute_time);
            FLOG_DEBUG("Timer Q Not empty, wait for next node \r\n"); 
            //adapt_timer_print_timeval("adapt_timer_handler", &(p_timer->st_expired));
        }
    
        while ((0 == p_ordered_list->uc_head_chgd)&&(0 == w_ret)) 
        {
            //TRACE1(6, "Timer About to enter timed wait, time:%lld\n", readtsc());
            w_ret = pthread_cond_timedwait( &(p_ordered_list->checkq), &(p_ordered_list->qmutex), &ts);

            //TRACE1(6, "Timer Exited timed wait, time:%lld\n", readtsc());
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
            //TRACE(6, "Timer Q Head Changed\n");
            FLOG_DEBUG("Head changed \r\n"); 

            continue;
        }
    
        //Remove all the expired timers
        
        if(ETIMEDOUT == w_ret) 
        {
            FLOG_DEBUG("Timer out \r\n"); 
            gettimeofday(&tv, NULL);
            trans_timer_print_timeval("present time:", &tv);

            // timed wait expired
            //remove timers from dol and enqueue to appropriate q
            //also remove all timers within 100 micro second window
            if (trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
            {
                //TRACE(4, "Unable to peek timer Q\n");
                FLOG_ERROR("6 Call trans_ordered_list_peek_head error! \r\n");
            }
            p_timer = (struct trans_timer *)pp_timer;

            struct timeval expr_tv;

            if (NULL != p_timer) 
            {
                expr_tv.tv_sec = p_timer->st_expired.tv_sec;
                expr_tv.tv_usec = p_timer->st_expired.tv_usec;
            }    

            //Remove all closely expiring timers (within 100 micro seconds
            while ((NULL != p_timer) 
                && (TRANS_TIMER_IN_WINDOW == trans_timer_window(&expr_tv, &(p_timer->st_expired), TRANS_TIMER_WINDOW))) 
            {
                if(trans_ordered_list_pop_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
                {
                    //TRACE(4, "Unable to pop timer Q\n");
                    FLOG_ERROR("7 Call trans_ordered_list_pop_head error! \r\n");
                }
                p_timer = (struct trans_timer *)pp_timer;

                if (NULL == p_timer)
                {
                    FLOG_ERROR("8 NULL PTR! \r\n");
                }
                
                if (TRANS_TIMER_DELETE != p_timer->uc_deleted) 
                { 
                    //FLOG_DEBUG("8 Time Out start . \r\n");
                    w_ret = trans_timer_fun_exe(p_timer);

                    if(TRANS_SUCCESS != w_ret) 
                    {
                        FLOG_ERROR("Call trans_timer_fun_exe error!w_ret = %d. \r\n", w_ret);
                        continue;
                    }
                    
                    //FLOG_DEBUG("9 Time Out end . \r\n");

                    /*TRACE3(6, "Timer: Added to Event Q timer_id:%p expiry time:%lld current time:%lld\n", \
                    p_timer->timer_id, p_timer->absolute_time, readtsc());*/
                }
                
                //free the expired app_timer
                free(p_timer->p_msg);
                free(p_timer);
                
                if(trans_ordered_list_peek_head(p_ordered_list, (void**)&pp_timer, &uw_num_elems)) 
                {
                    //TRACE(4, "Unable to peek timer Q\n");
                    FLOG_ERROR("9 Call trans_ordered_list_peek_head error! \r\n");
                }
                p_timer = (struct trans_timer *)pp_timer;
            }
        }
        
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
*  Data:    2011-04-14
* 
+*****************************************************************************/
u_int32_t trans_timer_init(void)
{
    u_int32_t uw_ret = 0;
    //FLOG_DEBUG("Enter \r\n"); 
    
    uw_ret = trans_ordered_list_init(&g_p_timer_list);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_ordered_list_init error! \r\n");
    
        return uw_ret;        
    }  
    
    pthread_mutex_init(&g_timer_mutex, NULL);
        
    g_trans_timer_counter++;
    g_execute_timer=1;
    
   
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
*  Data:    2011-05-30
* 
+*****************************************************************************/
u_int32_t trans_timer_release(void)
{
    struct trans_ordered_list * p_ordered_list = NULL;
    struct trans_list  *p_list = NULL;
    struct trans_list  *p_tmp_cntr = NULL;
    struct trans_timer *p_timer = NULL;

    p_ordered_list = g_p_timer_list;

    //cleaup the List
    //pthread_mutex_lock(&(p_ordered_list->qmutex));

    p_list = p_ordered_list->p_head;
    
    while(p_list!=NULL)
    {
        p_tmp_cntr = p_list;
        p_timer = p_tmp_cntr->p_data;

        //FLOG_DEBUG("p_timer = %p \r\n", p_timer); 
        
        /*Free Msg*/
        free(p_timer->p_msg);     
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
    free(g_p_timer_list);
    g_p_timer_list = NULL;
    
    g_trans_timer_counter = 0;
 
    return TRANS_SUCCESS;
}



