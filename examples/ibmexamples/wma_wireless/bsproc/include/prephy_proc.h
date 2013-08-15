/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: prephy_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PREPHY_PROC_H_
#define __PREPHY_PROC_H_

#include "global.h"

extern int fake_ulmap_duration;
extern int fake_ulmap_uiuc;

extern int g_periodic_sensing_enable;
extern int g_periodic_sensing_reset;
extern int g_periodic_sensing_drop;
extern int g_ber_reset;
extern int g_fake_ulmap_duration;
extern int g_fake_ulmap_uiuc;

struct phy_raw_buf_node
{
    int slot_symbol_idx;
    int data_buf_len;
    void * data_buf[ANTENNA_NUM];    /* for dump use */
    void * memory_hdr[ANTENNA_NUM]; /* for free use */
    struct phy_raw_buf_node * next;
};

struct phy_raw_buf_hdr
{
    u_int32_t frame_number;
    int ul_map_len;
    char ul_map[UL_MAP_MAX_LEN];    /* for dump use */
    struct phy_raw_buf_node * node;
};

int pre_phy_process (void);

int pre_phy_release (void);

int frm_dump_all_rx(unsigned int frame_num, FILE * f_p);
int frm_dump_select_rx(unsigned int frame_num, FILE * f_p);

int cfg_update_ps_enable_cb(int type, int len, void * buf);
int cfg_update_ber_reset(int type, int len, void * buf);
int cfg_update_fake_ulmap_duration(int type, int len, void * buf);
int cfg_update_fake_ulmap_uiuc(int type, int len, void * buf);

#endif /* __PRE_PHY_H_ */
