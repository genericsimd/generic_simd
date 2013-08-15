/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: bs_debug.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __BS_DEBUG_H_
#define __BS_DEBUG_H_

#include <sys/types.h>

#define MAX_HOOK_NUM 64
#define MAX_HOOK_COMPONENT_NUM 8

#define MAX_KEY_LEN 128
#define MAX_VALUE_SIZE 128 /* value size in byte */

/** Use MACRO define for these, do not like enum */

/* COMPONENT ID (unused) */
#define COMM_HOOK_ID 0
#define MAC_HOOK_ID 1
#define PHY_HOOK_ID 2
#define RRU_HOOK_ID 3


/* HOOK Index */
#define HOOK_FCH_IDX 0
#define HOOK_CONSTELLATION_IDX 1
#define HOOK_CHAN_QUALITY_IDX 2
#define HOOK_BER_IDX 3
#define HOOK_PS_INFO_IDX 4
#define HOOK_RANGING_POWER_IDX 5
#define HOOK_RANGING_RESULT_IDX 6
#define HOOK_CRC_COUNT_IDX 7


/* hooks are list here with their args. */
/* hook name, component, default sampling, hook idx, buffer length in byte) */

#define HOOK_KEYS    \
    CONVERT_HOOK (FCH, PHY, 500, HOOK_FCH_IDX, 256)    \
    CONVERT_HOOK (Constellation, PHY, 500, HOOK_CONSTELLATION_IDX, 327680)    \
    CONVERT_HOOK (Channel_quality, PHY, 500, HOOK_CHAN_QUALITY_IDX, 4096)    \
    CONVERT_HOOK (BER, PHY, 500, HOOK_BER_IDX, 256)    \
    CONVERT_HOOK (PS_info, PHY, 50, HOOK_PS_INFO_IDX, 8192)    \
    CONVERT_HOOK (Ranging_power, PHY, 500, HOOK_RANGING_POWER_IDX, 256)    \
    CONVERT_HOOK (Ranging_result, PHY, 500, HOOK_RANGING_RESULT_IDX, 256)    \
    CONVERT_HOOK (CRC_count, PHY, 1, HOOK_CRC_COUNT_IDX, 256)

struct hook_dst_device
{
    char mac[6];
    unsigned int trans_id;
};

struct hook_unit_dst
{
    struct hook_dst_device dev;
    unsigned int on_off;                   /* 0 - off; 1 - on */
    unsigned int sampling;
    struct hook_unit_dst * next;
};

struct debug_hook_unit
{
    pthread_mutex_t hook_mutex;
    char hook_name[MAX_KEY_LEN];           /* key name. */
    char component_name[MAX_KEY_LEN];     /* component name */
    unsigned int index;
    unsigned int default_sampling;
    unsigned int buf_len;
    unsigned int count;
    unsigned int dst_count;
    unsigned int active; /* if active (:1) or not (:0) */
    struct hook_unit_dst * next;
};

struct config_hook_unit
{
    int total_hook_num;           /* total parameters. */
};


int32_t init_global_hook (void);

int32_t deinit_global_hook (void);

int32_t key_set_global_hook (char * hook_name, struct hook_dst_device * dev, int on_off, int sampling);

int32_t idx_set_global_hook (int index, struct hook_dst_device * dev, int on_off, int sampling);

int32_t key_get_global_hook (char * hook_name, struct debug_hook_unit ** p_hook_node);

int32_t idx_get_global_hook (int index, struct debug_hook_unit ** p_hook_node);

int32_t key_find_global_hook (char * hook_name, struct hook_dst_device * dev);

int32_t cleanall_dst(void);

#endif /* __FRM_CFG_H_ */

