/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: rru_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __RRU_PROC_H_
#define __RRU_PROC_H_

#include <stdio.h>
#include <pthread.h>

extern pthread_mutex_t g_resync_lock;
extern int g_resync_count;

struct rru_data_config
{
    int tx_blk_per_frm;
    int rx_blk_per_frm;
    int rx_symbol_num;
    int tx_symbol_num;
    int tx_ant_num;
    int rx_ant_num;
    int tx_symbol_per_slot;
    int rx_symbol_per_slot;
    float gain_tx;
    int advanced_cmd_time;
};

enum
{
        ERR_NOERROR,
        ERR_CARR_IDX,
        ERR_CHAN_IDX,
        ERR_CMD,
        ERR_BODY_FRM_NO_MISMATCH,
        ERR_FRM_MISHEAD,
        ERR_FRM_MISTAIL,
        ERR_TAIL_FRM_NO_MISMATCH,
        ERR_MISS_PIECE,
        ERR_TOO_MANY_BODY
}enum_err_id;

extern struct rru_data_config g_rru_data_param;

extern struct sem_handle * cmd_s_handle;

extern struct sem_handle * tx_s_handle;

extern unsigned short g_cmd_frame_num;

extern unsigned short g_init_rruframe_num;

int rrh_tdd_tx_process (void);

int rrh_tdd_rx_process (void);

int rrh_tdd_tx_release (void);

int rrh_tdd_rx_release (void);

int frm_dump_short_sample (int flag, FILE * f_p, int len, void * buf);
int frm_dump_duration(unsigned int frame_num, FILE * f_p, int duration);

#endif /* __RRU_H_ */
