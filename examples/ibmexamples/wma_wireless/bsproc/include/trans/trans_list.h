/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_list.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Mar.2012      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_LIST_H_
#define TRANS_LIST_H_


#include <sys/types.h>
//#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

typedef int (*cmp_function) (void *p_data, void *p_key, void *p_result);

/*****************************************************************************+
*Enum
+*****************************************************************************/


/*****************************************************************************+
*Data structure
+*****************************************************************************/
struct trans_list 
{
    struct trans_list* p_prev; //prev ptr in list
    struct trans_list* p_next; // next ptr in list
    size_t len;         //length of the list element
    void* p_data;    //data of the list element
};

#define SIZEOF_TRANS_LIST   sizeof(struct trans_list)


struct trans_unordered_list
{
    struct trans_list* p_head;
    struct trans_list* p_tail;
    u_int32_t   uw_node_num;
    pthread_mutex_t   qmutex;
    //pthread_cond_t     checkq;
    //u_int8_t uc_head_chgd;
};

#define SIZEOF_TRANS_UNORDERED_LIST   sizeof(struct trans_unordered_list)

#define TRANS_UNORDERED_LIST_NUM(list)        ((list)->uw_node_num)  

#define TRANS_UNORDERED_LIST_HEAD(list)        ((list)->p_head)  
    
#define TRANS_UNORDERED_LIST_TAIL(list)            NULL
    
#define TRANS_UNORDERED_LIST_EMPTY(list)      (TRANS_UNORDERED_LIST_HEAD(list) == TRANS_UNORDERED_LIST_TAIL(list))  
    
#define TRANS_UNORDERED_LIST_NEXT(field)       (((struct trans_list *)field)->p_next)  
    
#define TRANS_UNORDERED_LIST_DATA(field)       (((struct trans_list *)field)->p_data)  

#define TRANS_UNORDERED_LIST_FOREACH(list, field)    \
                        for((field) = TRANS_UNORDERED_LIST_HEAD(list);     \
                            (field)!= TRANS_UNORDERED_LIST_TAIL(list);    \
                            (field) = TRANS_UNORDERED_LIST_NEXT(field))


#define TRANS_UNORDERED_LIST_DELETE(list, curr)                  \
    do    \
    {    \
        struct trans_unordered_list *p_list = list;    \
        struct trans_list *p_curr = curr;    \
            \
        p_list->uw_node_num --;    \
            \
        if (p_list->p_head == p_curr)    \
        {    \
                \
            p_list->p_head = p_curr->p_next;    \
        }    \
        else if (p_list->p_tail == p_curr)    \
        {    \
                \
            p_list->p_tail = p_curr->p_prev;    \
            p_curr->p_prev->p_next = NULL;    \
        }    \
        else    \
        {    \
                \
            p_curr->p_prev->p_next = p_curr->p_next;    \
            p_curr->p_next->p_prev = p_curr->p_prev;    \
        }    \
            \
        free(p_curr);    \
            \
        if (0 == p_list->uw_node_num)    \
        {    \
            p_list->p_head=NULL;    \
            p_list->p_tail=NULL;    \
        }    \
            \
    }while(0);



/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_unordered_list_init(struct trans_unordered_list ** pp_unordered_list);

extern u_int32_t trans_unordered_list_add(struct trans_unordered_list * p_unordered_list, 
                            size_t len,
                            void* p_data);

extern u_int32_t trans_unordered_list_delete(struct trans_unordered_list * p_unordered_list, 
                            void* p_data);

extern u_int32_t trans_unordered_list_get_in(struct trans_unordered_list * p_unordered_list, 
                            void * p_key,
                            cmp_function f_cmp_func,
                            void ** pp_data);

extern u_int32_t trans_unordered_list_get_out(struct trans_unordered_list * p_unordered_list, 
                            void * p_key,
                            cmp_function f_cmp_func,
                            void ** pp_data);



#endif /* TRANS_LIST_H_ */

