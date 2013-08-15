/*****************************************************************************+
*
*  File Name: trans_list.h
*
*  Function: List
*
*  
*  Data:    2011-4-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_LIST_H_
#define TRANS_LIST_H_


#include <sys/types.h>
#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/



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
    struct timeval st_kye;/*order kye */
    size_t len;         //length of the list element
    void* p_data;    //data of the list element
    //int w_data_type; //type of the data
};

#define SIZEOF_TRANS_LIST   sizeof(struct trans_list)


struct trans_ordered_list
{
    struct trans_list* p_head;
    struct trans_list* p_tail;
    struct trans_list* p_last_visited;
    u_int32_t   uw_node_num;
    //u_int32_t   uw_num_bytes;
    pthread_mutex_t   qmutex;
    pthread_cond_t     checkq;
    u_int8_t uc_head_chgd;
};

#define SIZEOF_TRANS_ORDERED_LIST   sizeof(struct trans_ordered_list)


/*****************************************************************************+
*extern
+*****************************************************************************/

extern u_int32_t trans_ordered_list_init(struct trans_ordered_list ** pp_ordered_list) ;

extern u_int32_t trans_ordered_list_insert(struct trans_ordered_list * p_ordered_list, 
                            void* p_data, 
                            struct timeval *tv,
                            size_t len, 
                            u_int32_t *p_num_elems); 

extern u_int32_t trans_ordered_list_delete(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            struct timeval *key_tv,
                            u_int32_t *p_num_elems);

extern u_int32_t trans_ordered_list_pop_head(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            u_int32_t *p_num_elems);


extern u_int32_t trans_ordered_list_peek_head(struct trans_ordered_list * p_ordered_list, 
                            void** pp_data, 
                            u_int32_t *p_num_elems);
#if 0

                            
extern u_int32_t trans_ordered_list_cleanup(struct trans_ordered_list * p_ordered_list)
#endif
#endif /* TRANS_LIST_H_ */

