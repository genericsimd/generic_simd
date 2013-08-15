/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_list.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Mar.2012      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_timer.h>
#include <trans_list.h>
#include <trans_debug.h>

#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>

/*****************************************************************************+
* Function: trans_unordered_list_init()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-01
* 
+*****************************************************************************/
u_int32_t trans_unordered_list_init(struct trans_unordered_list ** pp_unordered_list) 
{
    /* Allocate a memory.  */
    *pp_unordered_list = (struct trans_unordered_list *)malloc(SIZEOF_TRANS_UNORDERED_LIST);
    
    if (NULL == *pp_unordered_list)
    {
        FLOG_ERROR("malloc pp_unordered_list error! \r\n");

        return TRANS_FAILD;   
    }

    (*pp_unordered_list)->p_head = NULL;
    (*pp_unordered_list)->p_tail = NULL;
    (*pp_unordered_list)->uw_node_num = 0;

    if(pthread_mutex_init(&((*pp_unordered_list)->qmutex), NULL)) 
    {
        FLOG_ERROR("Initializing pp_unordered_list mutex error! \r\n");
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_unordered_list_add()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-01
* 
+*****************************************************************************/
u_int32_t trans_unordered_list_add(struct trans_unordered_list * p_unordered_list, 
                            size_t len,
                            void* p_data) 
{
    struct trans_list  *p_list = NULL;
    struct trans_list  *p_curr = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Enter \r\n");    

    if ((NULL == p_unordered_list) ||(NULL == p_data))
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

    pthread_mutex_lock(&(p_unordered_list->qmutex));
    
    //If the list is empty add this rightway
    if(p_unordered_list->uw_node_num == 0) 
    {
        FLOG_DEBUG_TRANS(g_trans_debug_list, "empty \r\n");  
        
        p_unordered_list->p_head=p_list;
        p_unordered_list->p_tail=p_list;

    }
    else 
    {
        FLOG_DEBUG_TRANS(g_trans_debug_list, "insert \r\n");  
        
        p_curr = p_unordered_list->p_tail;

        if (NULL == p_curr)
        {
            FLOG_ERROR("NULL PTR, p_curr error! \r\n");
            return TRANS_FAILD;   
        }

        p_list->p_prev=p_curr;
        p_curr->p_next = p_list;

        p_unordered_list->p_tail = p_list;
       
    }
    
    p_unordered_list->uw_node_num +=1;

    /*print list*/
    FLOG_DEBUG_TRANS(g_trans_debug_list, "p_head = %p, p_tail = %p, p_list = %p \r\n",
            p_unordered_list->p_head, p_unordered_list->p_tail, p_list);  
    FLOG_DEBUG_TRANS(g_trans_debug_list, "uw_node_num = %d\r\n",
            p_unordered_list->uw_node_num);      
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Exit \r\n");  
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_unordered_list_get_in()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-01
* 
+*****************************************************************************/
u_int32_t trans_unordered_list_get_in(struct trans_unordered_list * p_unordered_list, 
                            void * p_key,
                            cmp_function f_cmp_func,
                            void ** pp_data)
{
    struct trans_list  *p_curr = NULL;
    u_int8_t  uc_result = 0;
    
    *pp_data = NULL;
    
    if ((NULL == p_unordered_list) ||(NULL == p_key) || (NULL == f_cmp_func))
    {
        FLOG_ERROR("NULL PTR! \r\n");        
        return TRANS_FAILD;  
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Enter \r\n");    

    pthread_mutex_lock(&(p_unordered_list->qmutex));
   
    //If the list is empty add this rightway
    if (p_unordered_list->uw_node_num == 0)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_list, "The list is empty! \r\n");
        //return TRANS_SUCCESS;   

    }
    else
    {
        for (p_curr = p_unordered_list->p_head; NULL != p_curr; p_curr = p_curr->p_next)
        {
            f_cmp_func(p_curr->p_data, p_key, &uc_result);
            
            if (1 == uc_result)
            {   
                *pp_data = p_curr->p_data;
                break;
            } 
        }
    }

    if (0 == uc_result)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_list, "Do not find the target node! \r\n");
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Exit uc_result = %d\r\n", uc_result);  

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_unordered_list_get_out()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-01
* 
+*****************************************************************************/
u_int32_t trans_unordered_list_get_out(struct trans_unordered_list * p_unordered_list, 
                            void * p_key,
                            cmp_function f_cmp_func,
                            void ** pp_data)
{
    struct trans_list  *p_curr = NULL;
    //u_int8_t  uc_flag = 0;
    u_int8_t  uc_result = 0;

    *pp_data = NULL;
    
    if ((NULL == p_unordered_list) ||(NULL == p_key) || (NULL == f_cmp_func))
    {
        FLOG_ERROR("NULL PTR! \r\n");        
        return TRANS_FAILD;  
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Enter \r\n");    

    pthread_mutex_lock(&(p_unordered_list->qmutex));
   
    //If the list is empty add this rightway
    if (p_unordered_list->uw_node_num == 0)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_list, "The list is empty! \r\n");
        //return TRANS_FAILD;   

    }
    else
    {
        for (p_curr = p_unordered_list->p_head; NULL != p_curr; p_curr = p_curr->p_next)
        {
            f_cmp_func(p_curr->p_data, p_key, &uc_result);
            
            if (1 == uc_result)
            {   
                *pp_data = p_curr->p_data;
            
                p_unordered_list->uw_node_num --; 
                
                if (p_unordered_list->p_head == p_curr)
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete head  %p \r\n", p_curr);
                    p_unordered_list->p_head = p_curr->p_next;
                    //p_curr->p_next->p_prev = NULL;
                    
                }
                else if (p_unordered_list->p_tail == p_curr)
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete tail  %p \r\n", p_curr);
                    p_unordered_list->p_tail = p_curr->p_prev;
                    p_curr->p_prev->p_next = NULL;
                }
                else
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete node  %p \r\n", p_curr);
                    p_curr->p_prev->p_next = p_curr->p_next;
                    p_curr->p_next->p_prev = p_curr->p_prev;
                }

                break;
            } 
        }

        if (NULL != p_curr)
        {
            free(p_curr);
        }
        else
        {
            FLOG_DEBUG_TRANS(g_trans_debug_list, "Do not find the node: %p! \r\n");
        }
    }

    if (0 == p_unordered_list->uw_node_num)
    {
        p_unordered_list->p_head=NULL;
        p_unordered_list->p_tail=NULL;
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Exit \r\n");  

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_unordered_list_delete()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-01
* 
+*****************************************************************************/
u_int32_t trans_unordered_list_delete(struct trans_unordered_list * p_unordered_list, 
                            void* p_data)
{
    struct trans_list  *p_curr = NULL;
    //u_int8_t  uc_flag = 0;
    
    if ((NULL == p_unordered_list) ||(NULL == p_data))
    {
        FLOG_ERROR("NULL PTR! \r\n");        
        return TRANS_FAILD;  
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Enter \r\n");    

    pthread_mutex_lock(&(p_unordered_list->qmutex));
   
    //If the list is empty add this rightway
    if (p_unordered_list->uw_node_num == 0)
    {
        FLOG_ERROR("The list is empty! \r\n");
        return TRANS_FAILD;   

    }
    else
    {
        for (p_curr = p_unordered_list->p_head; NULL != p_curr; p_curr = p_curr->p_next)
        {
            if (p_data == p_curr->p_data)
            {   
                p_unordered_list->uw_node_num --; 
                
                if (p_unordered_list->p_head == p_curr)
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete head  %p \r\n", p_curr);
                    p_unordered_list->p_head = p_curr->p_next;
                    //p_curr->p_next->p_prev = NULL;
                    
                }
                else if (p_unordered_list->p_tail == p_curr)
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete tail  %p \r\n", p_curr);
                    p_unordered_list->p_tail = p_curr->p_prev;
                    p_curr->p_prev->p_next = NULL;
                }
                else
                {
                    FLOG_DEBUG_TRANS(g_trans_debug_list, "1!delete node  %p \r\n", p_curr);
                    p_curr->p_prev->p_next = p_curr->p_next;
                    p_curr->p_next->p_prev = p_curr->p_prev;
                }
                
                break;
            } 
        }

        if (NULL != p_curr)
        {
            free(p_curr);
        }
        else
        {
            FLOG_DEBUG_TRANS(g_trans_debug_list, "Do not find the node: %p! \r\n", p_data);
        }
    }

    if (0 == p_unordered_list->uw_node_num)
    {
        p_unordered_list->p_head=NULL;
        p_unordered_list->p_tail=NULL;
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_list, "Exit \r\n");  

    return TRANS_SUCCESS;
}


