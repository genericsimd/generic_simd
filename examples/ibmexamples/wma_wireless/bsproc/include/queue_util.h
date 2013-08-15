/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: queue_util.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __QUEUE_UTIL_H_
#define __QUEUE_UTIL_H_

#include <pthread.h>
#include "global.h"
#include "bsl/witm_rt.h"
#include "bsl/wntk_queue.h"

#define TRANS_ACTION_NUM 2

extern int * mac_en_id;
extern int * phy_de_id;
extern int * phy_en_id;
extern int * frm_de_id;
//extern int * rx_frm_en_id;

extern int * mac_ul_de_id;
extern int * phy_ul_de_id;
extern int * phy_ul_en_id;
extern int * frm_ul_en_id;
extern int * pre_ul_de_id;
extern int * pre_ul_en_id;

extern int   dump_en_id;
extern int   dump_de_id;

extern int bsagent_en_id;
extern int bsagent_de_id;

extern int trans_msg_en_id;
extern int trans_msg_de_id;

extern int * trans_action_en_id;
extern int * trans_action_de_id;

extern int csforw_en_id;
extern int csforw_de_id;

extern int monitor_en_id;
extern int monitor_de_id;

extern int exit_en_id;
extern int exit_de_id;

struct queue_global_param
{
    unsigned int internal_q_base;
};

struct queue_msg
{
    long my_type;
    unsigned int len;
    void * p_buf;
};

struct rx_data_block
{
    long my_type;
    unsigned int preamble_flag;
    unsigned int offset;
    unsigned int tail_flag;
    unsigned int len; // sample number
    float feq_offset;
    float * p_buf_i;
    float * p_buf_q;
};

struct queue_id_node
{
    int qid;
    struct queue_id_node * p_next;
};

struct queue_id_hdr
{
    pthread_mutex_t mutex_hdr;
    struct queue_id_node * p_list;
};


#define QUEUE_KEYS    \
    CONVERT_KEY (mac_en_id[0], phy_de_id[0], 0)    \
    CONVERT_KEY (mac_en_id[1], phy_de_id[1], 0)    \
    CONVERT_KEY (mac_en_id[2], phy_de_id[2], 0)    \
    CONVERT_KEY (mac_en_id[3], phy_de_id[3], 0)    \
    CONVERT_KEY (phy_en_id[0], frm_de_id[0], 0)    \
    CONVERT_KEY (phy_en_id[1], frm_de_id[1], 0)    \
    CONVERT_KEY (phy_en_id[2], frm_de_id[2], 0)    \
    CONVERT_KEY (phy_en_id[3], frm_de_id[3], 0)    \
    CONVERT_KEY (frm_ul_en_id[0], pre_ul_de_id[0], 0)    \
    CONVERT_KEY (frm_ul_en_id[1], pre_ul_de_id[1], 0)    \
    CONVERT_KEY (frm_ul_en_id[2], pre_ul_de_id[2], 0)    \
    CONVERT_KEY (frm_ul_en_id[3], pre_ul_de_id[3], 0)    \
    CONVERT_KEY (pre_ul_en_id[0], phy_ul_de_id[0], 0)    \
    CONVERT_KEY (pre_ul_en_id[1], phy_ul_de_id[1], 0)    \
    CONVERT_KEY (pre_ul_en_id[2], phy_ul_de_id[2], 0)    \
    CONVERT_KEY (pre_ul_en_id[3], phy_ul_de_id[3], 0)    \
    CONVERT_KEY (phy_ul_en_id[0], mac_ul_de_id[0], 0)    \
    CONVERT_KEY (phy_ul_en_id[1], mac_ul_de_id[1], 0)    \
    CONVERT_KEY (phy_ul_en_id[2], mac_ul_de_id[2], 0)    \
    CONVERT_KEY (phy_ul_en_id[3], mac_ul_de_id[3], 0)    \
    CONVERT_KEY (trans_action_en_id[0], trans_action_de_id[0], 0)    \
    CONVERT_KEY (trans_action_en_id[1], trans_action_de_id[1], 0)    \
    CONVERT_KEY (trans_action_en_id[2], trans_action_de_id[2], 0)    \
    CONVERT_KEY (trans_action_en_id[3], trans_action_de_id[3], 0)    \
    CONVERT_KEY (dump_en_id, dump_de_id, 0)    \
    CONVERT_KEY (bsagent_en_id, bsagent_de_id, 0)    \
    CONVERT_KEY (trans_msg_en_id, trans_msg_de_id, 0)    \
    CONVERT_KEY (csforw_en_id, csforw_de_id, 0)    \
    CONVERT_KEY (monitor_en_id, monitor_de_id, 0)    \
    CONVERT_KEY (exit_en_id, exit_de_id, 0)


void * my_malloc (unsigned int size);

int init_queue( void );

int release_queue( void );

int exit_bsproc(void);

#endif /* __UTIL_QUEUE_H_ */
