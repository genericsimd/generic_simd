/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: util_queue.c

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <fdebug.h>

#include "witm_rt.h"
#include "queue_util.h"
#include "bs_cfg.h"
#include "log.h"

/** for flexible usage of Queue in anywhere,
 *  we set it as the Global variable        */

int * mac_en_id;
int * phy_de_id;
int * phy_en_id;
int * frm_de_id;
//int * rx_frm_en_id;

int * mac_ul_de_id;
int * phy_ul_de_id;
int * phy_ul_en_id;
int * frm_ul_en_id;
int * pre_ul_de_id;
int * pre_ul_en_id;

int dump_en_id;
int dump_de_id;

int bsagent_en_id;
int bsagent_de_id;

int trans_msg_en_id;
int trans_msg_de_id;

int * trans_action_en_id;
int * trans_action_de_id;

int csforw_en_id;
int csforw_de_id;

int monitor_en_id;
int monitor_de_id;

int exit_en_id;
int exit_de_id;

static struct queue_id_hdr * p_att_q_hdr = NULL;
static struct queue_id_hdr * p_init_q_hdr = NULL;
static struct queue_id_node * p_att_q_cur = NULL;
static struct queue_id_node * p_init_q_cur = NULL;

static struct queue_global_param * queue_g_para = NULL;

void * my_malloc (unsigned int size)
{
    void * p_tmp = malloc (size);

    if (p_tmp == NULL)
    {
        FERROR("malloc memory error");
        return NULL;
    }

    return p_tmp;
}

static int get_queue_key (struct queue_global_param * q_config)
{
    int ret = 0;
    int bbu_id = 0;
    int value = 0;

    ret = get_global_param ("BBU_SERVER_ID", & ( bbu_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_SERVER_ID error\n");
    }

    ret = get_global_param ("WIRELESS_INTERNAL_Q_IDBASE", & (value));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TX_MAC_PHY_Q_IDBASE error\n");
    }

    q_config->internal_q_base = (unsigned int)(value + bbu_id * 100);

    return 0;
}

static int free_op (const int qid)
{

    if ( wmrt_testqueue (qid) != 0 )
    {
        if ( wmrt_fluqueue (qid) != 0 )
        {
            FERROR("flu queue error");
            return 1;
        }
    }

    wmrt_relqueue (qid);

    return 0;

}

static int alloc_qid (const unsigned int key)
{
    int qid;

    qid = wmrt_reqqueue (key);

    if ( qid == -1 )
    {
        FERROR("req queue error");
        return -1;
    }

    if ( wmrt_testqueue (qid) != 0 )
    {
        if ( wmrt_fluqueue (qid) != 0 )
        {
            FERROR("flu queue error");
            return 1;
        }
    }

    return qid;
}


static int add_qid( const int q_id,
             struct queue_id_hdr ** pp_q_hdr,
             struct queue_id_node ** pp_q_cur )
{
    struct queue_id_node * p_tmp =
        (struct queue_id_node *) my_malloc ( sizeof(struct queue_id_node) );

    pthread_mutex_lock( &((*pp_q_hdr)->mutex_hdr) );

    if ( (*pp_q_cur) == NULL)
    {
        (*pp_q_hdr)->p_list = p_tmp;
        (*pp_q_cur) = p_tmp;
        p_tmp->qid = q_id;
        p_tmp->p_next = NULL;
    }else
    {
        (*pp_q_cur)->p_next = p_tmp;
        p_tmp->qid = q_id;
        (*pp_q_cur) = p_tmp;
        p_tmp->p_next = NULL;
    }

    pthread_mutex_unlock( &((*pp_q_hdr)->mutex_hdr) );

    return 0;
}

static int clean_qid( struct queue_id_hdr ** pp_q_hdr,
               struct queue_id_node ** pp_q_cur,
               int (*p_fun_q)(int qid) )
{
    pthread_mutex_lock( &((*pp_q_hdr)->mutex_hdr) );

    struct queue_id_node * p_tmp;
    struct queue_id_node * p_tmp_q;

    p_tmp = (*pp_q_hdr)->p_list;

    while (p_tmp != NULL)
    {
        p_tmp_q = p_tmp->p_next;
        (*p_fun_q)(p_tmp->qid);
        free (p_tmp);
        p_tmp = p_tmp_q;
    }

    (*pp_q_cur) = NULL;
    (*pp_q_hdr)->p_list = NULL;

    pthread_mutex_unlock( &((*pp_q_hdr)->mutex_hdr) );

    return 0;
}

int init_queue( void )
{
    static int queue_idx = 0;

    queue_g_para = (struct queue_global_param *) malloc (sizeof (struct queue_global_param));

    if (queue_g_para == NULL)
    {
        FLOG_ERROR("malloc memory failed!\n");
        return 1;
    }

    if (get_queue_key (queue_g_para) != 0)
    {
        FLOG_ERROR("Get queue key failed!\n");
        return 1;
    }

    mac_en_id = my_malloc( sizeof (int) * 4 );
    phy_de_id = my_malloc( sizeof (int) * 4 );
    phy_en_id = my_malloc( sizeof (int) * 4 );
    frm_de_id = my_malloc( sizeof (int) * 4 );

    mac_ul_de_id = my_malloc( sizeof (int) * 4 );
    phy_ul_de_id = my_malloc( sizeof (int) * 4 );
    phy_ul_en_id = my_malloc( sizeof (int) * 4 );
    frm_ul_en_id = my_malloc( sizeof (int) * 4 );
    pre_ul_de_id = my_malloc( sizeof (int) * 4 );
    pre_ul_en_id = my_malloc( sizeof (int) * 4 );

    trans_action_en_id = my_malloc( sizeof (int) * 4 );
    trans_action_de_id = my_malloc( sizeof (int) * 4 );

    p_att_q_hdr
        = (struct queue_id_hdr *) my_malloc (sizeof(struct queue_id_hdr));

    p_init_q_hdr
        = (struct queue_id_hdr *) my_malloc (sizeof(struct queue_id_hdr));

    pthread_mutex_init (& ( p_att_q_hdr->mutex_hdr ), NULL);
    pthread_mutex_init (& ( p_init_q_hdr->mutex_hdr ), NULL);


#define CONVERT_KEY(IN_ID, OUT_ID, TYPE)    \
        { \
            int tmp_queue_idx = 0; \
            \
            if (TYPE == 0) \
            { \
                tmp_queue_idx = queue_g_para->internal_q_base + queue_idx; \
                queue_idx ++; \
            }else \
            { \
                tmp_queue_idx = TYPE; \
            } \
            \
            IN_ID = alloc_qid (tmp_queue_idx); \
            if (IN_ID == -1) \
            { \
                FLOG_ERROR("Alloc QUEUE error\n"); \
                return 1; \
            } \
            add_qid (IN_ID, &p_init_q_hdr, &p_init_q_cur); \
            \
            OUT_ID = alloc_qid (tmp_queue_idx); \
            if (OUT_ID == -1) \
            { \
                FLOG_ERROR("Attach QUEUE error\n"); \
                return 1; \
            } \
            add_qid (OUT_ID, &p_att_q_hdr, &p_att_q_cur); \
        }
    QUEUE_KEYS;
#undef CONVERT_KEY

    return 0;
}

int release_queue()
{
    if (clean_qid (&p_att_q_hdr, &p_att_q_cur, &free_op) != 0)
    {
        FERROR ("CLEAN Queue ERROR\n");
        return 1;
    }

    if (clean_qid (&p_init_q_hdr, &p_init_q_cur, &free_op) != 0)
    {
        FERROR ("CLEAN Queue ERROR\n");
        return 1;
    }


    free (p_att_q_hdr);
    free (p_init_q_hdr);

    p_att_q_hdr = NULL;
    p_init_q_hdr = NULL;

    free( mac_en_id );
    free( phy_de_id );
    free( phy_en_id );
    free( frm_de_id );
    free( mac_ul_de_id );
    free( phy_ul_de_id );
    free( phy_ul_en_id );
    free( frm_ul_en_id );
    free( pre_ul_en_id );
    free( pre_ul_de_id );

    return 0;
}

int exit_bsproc()
{
    struct queue_msg p_msg;

    p_msg.my_type = exit_en_id;
    p_msg.p_buf = NULL;

    if (wmrt_enqueue (exit_en_id, &p_msg, sizeof(struct queue_msg)) == -1)
    {
        FLOG_WARNING ("ENQUEUE ERROR in MAC layer\n");
    }

    return 0;
}

