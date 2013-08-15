/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: dump_utils.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


/** OS */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include "bs_cfg.h"
#include "flog.h"
#include "dump_util.h"

/** DUMP cbs APIs */
#include "prephy_proc.h"
#include "rru_proc.h"
#include "phy_ul_rx_ranging.h"
#include "phy_ul_rx.h"
#include "phy_dl_tx_interface.h"
#include "phy_ul_rx_utility.h"
#include "phy_dl_ofdmamodul.h"
#include "phy_dl_preamble.h"
#include "phy_dl_utility.h"

/*dump mac related*/
#include "dlmap.h"
#include "dlmap_builder.h"
#include "ulmap.h"
#include "ul_scheduler.h"

struct dump_key_node * dump_key[MAX_DUMP_KEYS_NUM];
int g_dump_key_count = 0;

static int dump_all_rx_pool (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_one_rx_pool (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_short_sample (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_ranging_power (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_ranging_result (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_channel_quality (int flag, char * name, FILE * fd, int len, void *p_buf);
static int dump_frm_duration (int flag, char * name, FILE * fd, int len, void *p_buf);


int init_dump( void )
{
    int bbu_id = 0;
    char path[64];
    char prefix[64];
    int ret = 0;

    ret = get_global_param ("BBU_SERVER_ID", & ( bbu_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_SERVER_ID error\n");
    }

    ret = get_global_param ("DUMP_UTIL_PATH", path);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters DUMP_UTIL_PATH error\n");
    }

    sprintf(prefix, "%s/bbu%d_", path, bbu_id);

    memset(dump_key, 0, sizeof(dump_key));

#define CONVERT_DUMP(NAME, PATH, MODE, FUN, MAX, ACTIVE) \
        do{ \
            if (ACTIVE == 1) \
            { \
                int dump_idx = DUMP_##NAME##_ID; \
                dump_key[dump_idx] = (struct dump_key_node *)malloc(sizeof(struct dump_key_node)); \
                if (dump_key[dump_idx] == NULL) \
                { \
                    FLOG_ERROR("error malloc\n"); \
                    return 1; \
                } \
                memset(dump_key[dump_idx], 0, sizeof (struct dump_key_node)); \
                strcpy(dump_key[dump_idx]->key_name, #NAME); \
                dump_key[dump_idx]->f_dump_cb = FUN; \
                dump_key[dump_idx]->max_count = MAX; \
                dump_key[dump_idx]->count = 0; \
                OPEN_DUMP_FILE(dump_key[dump_idx]->fd, prefix, PATH, MODE);    \
                pthread_mutex_init( &(dump_key[dump_idx]->dump_mutex), NULL ); \
                g_dump_key_count++; \
            } \
        }while(0)
    DUMP_KEYS;
#undef CONVERT_DUMP

    return 0;
}

int release_dump( void )
{
    int i = 0;

    for (i = 0; i < MAX_DUMP_KEYS_NUM; i++)
    {
        if (dump_key[i] != NULL)
        {
            if (dump_key[i]->fd != 0)
            {
                CLOSE_DUMP_FILE(dump_key[i]->fd);
            }

            pthread_mutex_destroy( &(dump_key[i]->dump_mutex) );

            free (dump_key[i]);
            dump_key[i] = NULL;
        }
    }

    return 0;
}

int do_dump( int id, int flag, int len, void *p_buf )
{
#ifdef _DUMP_UTIL_ENABLE_
    int max_count = 0;

    if (dump_key[id] == NULL)
    {
        return 1;
    }

    if (dump_key[id]->fd == 0)
    {
        return 1;
    }

    max_count = dump_key[id]->max_count;

//    pthread_mutex_lock ( &(dump_key[id]->dump_mutex) );

    if (( max_count >= 0 ) && ( (dump_key[id]->count) >= (max_count) ))
    {
        return 1;
    }

//    pthread_mutex_unlock ( &(dump_key[id]->dump_mutex) );
    if (dump_key[id]->f_dump_cb != NULL)
    {
        (*(dump_key[id]->f_dump_cb))(flag, dump_key[id]->key_name, dump_key[id]->fd, len, p_buf);
    }
    else
    {
        DUMP_RAW_DATA(flag, dump_key[id]->key_name, dump_key[id]->fd, len, p_buf);    \
    }

   
    dump_key[id]->count ++;

    if (dump_key[id]->count < 0)
    {
        dump_key[id]->count = 0;
    }

#else
    (void)id;
    (void)flag;
    (void)len;
    (void)p_buf;
#endif

    return 0;
}

int reset_dump(int id, char * key, int count, int max_count)
{
#ifdef _DUMP_UTIL_ENABLE_
    int key_id = 0;
    int i;

    if (key != NULL)
    {
        for (i = 0; i < MAX_DUMP_KEYS_NUM; i++)
        {
            if (dump_key[i] != NULL)
            {
                if (strcmp (dump_key[i]->key_name, key) == 0)
                {
                    key_id  = i;
                    break;
                }
            }
        }
        if (i == MAX_DUMP_KEYS_NUM)
        {
            FLOG_ERROR("DUMP key did not exist %s\n", key);
            return 1;
        }
    }else
    {
        key_id = id;
    }

    if (dump_key[key_id] == NULL)
    {
        FLOG_ERROR("DUMP key did not exist\n");
        return 1;
    }

    if (dump_key[key_id]->fd == 0)
    {
        FLOG_ERROR("file was not opened\n");
        return 1;
    }

    dump_key[key_id]->max_count = max_count;
    dump_key[key_id]->count = count;
#else
    (void)id;
    (void)key;
    (void)count;
    (void)max_count;
#endif
    return 0;
}

static int dump_all_rx_pool (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;
    (void) len;
    (void) p_buf;

    return ( frm_dump_all_rx(flag, fd) );
}

static int dump_one_rx_pool (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;
    (void) len;
    (void) p_buf;

    return ( frm_dump_select_rx(flag, fd) );
}

static int dump_short_sample (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;

    return (frm_dump_short_sample (flag, fd, len, p_buf));
}

static int dump_ranging_power (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;

    return (frm_dump_ranging_power (flag, fd, len, p_buf));
}

static int dump_ranging_result (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;

    return (frm_dump_ranging_result (flag, fd, len, p_buf));
}

static int dump_channel_quality (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;

    return (frm_dump_channel_quality (flag, fd, len, p_buf));
}


static int dump_frm_duration (int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void) name;
    (void) p_buf;

    return (frm_dump_duration (flag, fd, len));
}

