/*****************************************************************************+
*
*  File Name: trans_list.c
*
*  Function:  List
*
*  
*  Data:    2011-04-14
*  Modify:
*
+*****************************************************************************/

#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_timer.h>
#include <trans_list.h>

#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>

/*****************************************************************************+
* Function: trans_ordered_list_init()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_init(struct trans_ordered_list ** pp_ordered_list) 
{
    /* Allocate a memory.  */
    *pp_ordered_list = (struct trans_ordered_list *)malloc(SIZEOF_TRANS_ORDERED_LIST);
    
    if (NULL == *pp_ordered_list)
    {
        FLOG_ERROR("malloc pp_ordered_list error! \r\n");

        return TRANS_FAILD;   
    }

    (*pp_ordered_list)->p_head = NULL;
    (*pp_ordered_list)->p_tail = NULL;
    (*pp_ordered_list)->p_last_visited = NULL;
    (*pp_ordered_list)->uc_head_chgd = 0;
    (*pp_ordered_list)->uw_node_num = 0;

    if(pthread_mutex_init(&((*pp_ordered_list)->qmutex), NULL)) 
    {
        //TRACE(4, "Error Initializing dll_ordered_list mutex \n");
        FLOG_ERROR("Initializing dll_ordered_list mutex error! \r\n");
    }

    if(pthread_cond_init(&((*pp_ordered_list)->checkq), NULL)) 
    {
        //TRACE(4, "Error Initializing dll_ordered_list checkq conditional variable \n");
        FLOG_ERROR("Initializing dll_ordered_list checkq conditional variable error! \r\n");
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_ordered_list_insert()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_insert(struct trans_ordered_list * p_ordered_list, 
                            void* p_data, 
                            struct timeval *tv,
                            size_t len, 
                            u_int32_t *p_num_elems) 
{
    struct trans_list  *p_list = NULL;
    
    FLOG_DEBUG("Enter \r\n");    

    if ((NULL == p_ordered_list) ||(NULL == p_data) || (NULL == p_num_elems) || (NULL == tv))
    {

        FLOG_ERROR("NULL PTR! \r\n");
        
        return TRANS_FAILD;  
    }
     
    /* Allocate a memory.  */
    p_list = (struct trans_list *)malloc(SIZEOF_TRANS_LIST);
    if (NULL == p_list)
    {
        FLOG_ERROR("malloc p_list error! \r\n");

        return TRANS_FAILD;   
    }

    p_list->p_data = p_data;
    p_list->len = len;
    p_list->p_next = NULL;
    p_list->p_prev = NULL;
    p_list->st_kye.tv_sec = tv->tv_sec;
    p_list->st_kye.tv_usec = tv->tv_usec;

    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    //If the list is empty add this rightway
    if(p_ordered_list->uw_node_num == 0) 
    {
        FLOG_DEBUG("empty \r\n");  
        
        p_ordered_list->p_head=p_list;
        p_ordered_list->p_tail=p_list;
        p_ordered_list->p_last_visited=p_list;
        p_ordered_list->uc_head_chgd=1;
    }
    else 
    {
        FLOG_DEBUG("insert \r\n");  
        
        if (NULL == p_ordered_list->p_last_visited)
        {
            FLOG_ERROR("NULL PTR! \r\n");

            return TRANS_FAILD;   
        }
        
        struct trans_list *p_curr = p_ordered_list->p_last_visited;

        if (TRANS_TIMER_COMPARE_TV1 == trans_timer_compare(&(p_ordered_list->p_last_visited->st_kye), tv))
        {
            // walk towards head
            // use prev
            FLOG_DEBUG(" walk towards head \r\n");  
            
            while((p_curr!=NULL) 
                &&(TRANS_TIMER_COMPARE_TV1 == trans_timer_compare(&(p_curr->st_kye), tv)))
            {
                p_curr= p_curr->p_prev;
            }
            //we are here 
            //either curr is NULL
            //or key>=curr->key
            if(p_curr==NULL) 
            {
                // make the new node head
                p_list->p_next=p_ordered_list->p_head;
                p_ordered_list->p_head->p_prev=p_list;
                p_ordered_list->p_head=p_list;
                p_ordered_list->p_last_visited=p_list;
                p_ordered_list->uc_head_chgd=1;
            }            
            else 
            {
                // insert approriately
                if (NULL == p_curr)
                {
                    FLOG_ERROR("NULL PTR! \r\n");

                    return TRANS_FAILD;   
                }
                p_list->p_prev=p_curr;
                p_list->p_next=p_curr->p_next;
                p_curr->p_next=p_list;
                p_list->p_next->p_prev=p_list;
            }
        }
        
        else if (TRANS_TIMER_COMPARE_TV2 == trans_timer_compare(&(p_ordered_list->p_last_visited->st_kye), tv))
        {
            // walk towards tail
            // use next
            FLOG_DEBUG(" walk towards tail \r\n");  
            while((p_curr!=NULL) 
                &&(TRANS_TIMER_COMPARE_TV1 == trans_timer_compare(tv, &(p_curr->st_kye))))
            {
                p_curr= p_curr->p_next;
            }
            //we are here 
            //either curr is NULL
            //or key<=curr->key
            if(p_curr==NULL) 
            {
                // make the new node tail
                p_list->p_prev=p_ordered_list->p_tail;
                p_ordered_list->p_tail->p_next=p_list;
                p_ordered_list->p_tail=p_list;
                p_ordered_list->p_last_visited=p_list;
            }
            else
            {
                // insert approriately
                if (NULL == p_curr)
                {
                    FLOG_ERROR("NULL PTR! \r\n");

                    return TRANS_FAILD;   
                }
                
                p_list->p_next=p_curr;
                p_list->p_prev=p_curr->p_prev;
                p_curr->p_prev=p_list;
                p_list->p_prev->p_next=p_list;
            }
        }
        else 
        { 
            //key==dol->last_visited->key
            // can insert right away
            p_list->p_prev=p_ordered_list->p_last_visited;
            p_list->p_next=p_ordered_list->p_last_visited->p_next;
            p_ordered_list->p_last_visited->p_next=p_list;

            if(p_list->p_next!=NULL)
            {
                p_list->p_next->p_prev=p_list;  
            }
        }
    }
    
    p_ordered_list->uw_node_num +=1;
    p_ordered_list->p_last_visited=p_list;
    *p_num_elems=p_ordered_list->uw_node_num;

        
    if(1 == p_ordered_list->uc_head_chgd) 
    {
        pthread_cond_signal(&(p_ordered_list->checkq));
    }

    /*print list*/
    FLOG_DEBUG("p_head = %p, p_tail = %p, p_last_visited = %p \r\n",
            p_ordered_list->p_head, p_ordered_list->p_tail, p_ordered_list->p_last_visited);  
    FLOG_DEBUG("uw_node_num = %d, uc_head_chgd = %d \r\n",
            p_ordered_list->uw_node_num, p_ordered_list->uc_head_chgd);      
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    FLOG_DEBUG("Exit \r\n");  
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_ordered_list_insert()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_delete(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            struct timeval *key_tv,
                            u_int32_t *p_num_elems)
{
    struct trans_list  *p_list = NULL;
    
    FLOG_DEBUG("Enter \r\n");    

    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    p_list = p_ordered_list->p_last_visited;
    
    //If the list is empty add this rightway
    if (p_ordered_list->uw_node_num == 0)
    {
        *pp_data = NULL;
    }
    else
    {
        if (NULL == p_ordered_list->p_last_visited)
        {
            FLOG_ERROR("NULL PTR! \r\n");

            return TRANS_FAILD;   
        }
        
        struct trans_list *p_curr = p_ordered_list->p_last_visited;
        
        if (TRANS_TIMER_COMPARE_TV1 == trans_timer_compare(&(p_ordered_list->p_last_visited->st_kye), key_tv))
        {
            // walk towards head
            // use prev
            while((p_curr!=NULL) 
                && (TRANS_TIMER_COMPARE_TV1 == trans_timer_compare(&(p_curr->st_kye), key_tv)))
            {
                p_curr = p_curr->p_prev;
            }
            //we are here 
            //either p_curr is NULL
            //or key_tv>=p_curr->st_kye
            if (p_curr == NULL)
            {
                //data not found
                *pp_data = NULL;
            }
            else
            {
                // insert approriately
                if (NULL == p_curr)
                {
                    FLOG_ERROR("NULL PTR! \r\n");

                    return TRANS_FAILD;   
                }
                
                if (TRANS_TIMER_COMPARE_EQUAL == trans_timer_compare(&(p_curr->st_kye), key_tv))
                {
                    if(p_curr->p_prev!=NULL)
                    {
                        p_curr->p_prev->p_next=p_curr->p_next;
                        p_ordered_list->p_last_visited=p_curr->p_prev;
                    }
                    if(p_curr->p_next!=NULL)
                    {
                        p_curr->p_next->p_prev=p_curr->p_prev;
                        p_ordered_list->p_last_visited=p_curr->p_next;
                    }
                    p_ordered_list->uw_node_num--;
                }
                else
                {
                    //data not found
                    *pp_data = NULL;
                }
            }
        }
        else if (TRANS_TIMER_COMPARE_TV2 == trans_timer_compare(&(p_ordered_list->p_last_visited->st_kye), key_tv))
        {
            // walk towards tail
            // use next
            while ((p_curr!=NULL) 
                && (TRANS_TIMER_COMPARE_TV2 == trans_timer_compare(&(p_curr->st_kye), key_tv)))
            {
                p_curr = p_curr->p_next;
            }
            //we are here 
            //either p_curr is NULL
            //or key<=p_curr->st_kye
            if (p_curr == NULL)
            {
                //data not found
                *pp_data = NULL;
            }
            else
            {
                if(TRANS_TIMER_COMPARE_EQUAL == trans_timer_compare(&(p_curr->st_kye), key_tv)) 
                {
                    if(p_curr->p_prev!=NULL)
                    {
                        p_curr->p_prev->p_next=p_curr->p_next;
                        p_ordered_list->p_last_visited=p_curr->p_prev;
                    }
                    if(p_curr->p_next!=NULL)
                    {
                        p_curr->p_next->p_prev=p_curr->p_prev;
                        p_ordered_list->p_last_visited=p_curr->p_next;
                    }
                    p_ordered_list->uw_node_num--;
                }
                else
                {
                    //data not found
                    *pp_data = NULL;
                }
            }
        }
        else
        { 
            //key==p_ordered_list->p_last_visited->key
            p_curr = p_ordered_list->p_last_visited;
            
            if(p_curr->p_prev!=NULL)
            {
                p_curr->p_prev->p_next=p_curr->p_next;
                p_ordered_list->p_last_visited=p_curr->p_prev;
            }
            if(p_curr->p_next!=NULL)
            {
                p_curr->p_next->p_prev=p_curr->p_prev;
                p_ordered_list->p_last_visited=p_curr->p_next;
            }
            p_ordered_list->uw_node_num--;
        }
    }
    
    *p_num_elems = p_ordered_list->uw_node_num;
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    FLOG_DEBUG("Exit \r\n");  

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_ordered_list_pop_head()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_pop_head(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            u_int32_t *p_num_elems)
{
    struct trans_list  *p_list = NULL;
    *pp_data = NULL;
    *p_num_elems = 0;
    
    FLOG_DEBUG("Enter \r\n"); 
    
    if(p_ordered_list->uw_node_num > 0)
    {
        pthread_mutex_lock(&(p_ordered_list->qmutex));

            
        if (NULL == p_ordered_list->p_head)
        {
            FLOG_ERROR("NULL PTR! \r\n");

            return TRANS_FAILD;   
        }   
        
        p_list = p_ordered_list->p_head;
        
        p_ordered_list->p_head = p_ordered_list->p_head->p_next;
    
        if (p_ordered_list->p_head!=NULL)
        {
            p_ordered_list->p_head->p_prev=NULL;
        }
    
        p_ordered_list->uw_node_num--;
    
        if (p_ordered_list->p_last_visited == p_list)
        {
            p_ordered_list->p_last_visited = p_ordered_list->p_head;
        }
    
        *p_num_elems = p_ordered_list->uw_node_num;
        
        pthread_mutex_unlock(&(p_ordered_list->qmutex));
        
        (*pp_data) = p_list->p_data;

        //free the q_cntr
        free(p_list);
        
    }

    FLOG_DEBUG("Exit *pp_data = %p, *p_num_elems = %d\r\n", (*pp_data), *p_num_elems); 
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_ordered_list_peek_head()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_peek_head(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            u_int32_t *p_num_elems)
{
    (*pp_data) = NULL;
    *p_num_elems = 0;
    
    //FLOG_INFO("Enter \r\n"); 
    
    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    if(p_ordered_list->uw_node_num > 0)
    {
        //FLOG_INFO("2 %p %p\n",p_ordered_list->p_head, p_ordered_list->p_head->p_data);
        
        if (NULL == p_ordered_list->p_head)
        {
            FLOG_ERROR("1 NULL PTR! \r\n");
        
            return TRANS_FAILD;   
        } 
        
        if (NULL == p_ordered_list->p_head->p_data)
        {
            FLOG_ERROR("2 NULL PTR! \r\n");
        
            return TRANS_FAILD;   
        }   

        (*pp_data) = p_ordered_list->p_head->p_data;
        
        *p_num_elems = p_ordered_list->uw_node_num;
    }
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    FLOG_DEBUG("Exit *pp_data = %p, *p_num_elems = %d\r\n", (*pp_data), *p_num_elems);    
    
    return TRANS_SUCCESS;
}

#if 0
/*****************************************************************************+
* Function: trans_ordered_list_cleanup()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-20
* 
+*****************************************************************************/
u_int32_t trans_ordered_list_cleanup(struct trans_ordered_list * p_ordered_list)
{
    //cleaup the dol
    pthread_mutex_lock(&(p_ordered_list->qmutex));
    
    struct trans_list  *p_list = NULL;
    p_list = p_ordered_list->p_head;

    while(p_list!=NULL)
    {
        struct trans_list  *p_tmp_cntr = p_list;
        
        p_list = p_list->p_next;
        
        free(p_tmp_cntr->p_data);
        /*?????????*/
        free(p_tmp_cntr->p_data);
        //free the q_cntr
        free(p_tmp_cntr);
    }
    
    p_ordered_list->uc_head_chgd = 1;
    p_ordered_list->uw_node_num = 0;
    p_ordered_list->p_head = NULL;
    
    pthread_mutex_unlock(&(p_ordered_list->qmutex));
    
    return TRANS_SUCCESS;
}
#endif

