/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: rruif_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __RRU_PROC_H_
#define __RRU_PROC_H_

struct rru_nic_config
{
    unsigned int rru_id;
    unsigned short src_port;
    unsigned short dst_port;
    unsigned int src_ip;
    unsigned int dst_ip;
    int tx_blk_per_frm;
    int rx_blk_per_frm;
    int rx_symbol_num;
    int tx_symbol_num;
};

extern struct rru_nic_config g_rru_nic_param;

int rrh_tx_process (void);

int rrh_rx_process (void);

int rrh_tx_release (void);

int rrh_rx_release (void);

#endif /* __RRU_H_ */
