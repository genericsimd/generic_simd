/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: bs_debug.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <pthread.h>

#include "global.h"
#include "bs_debug.h"
#include "flog.h"

struct debug_hook_unit * gp_global_hook = NULL;
struct config_hook_unit * gp_hook_config_st = NULL;

//pthread_mutex_t g_hook_mutex;

#define COMPARE_MAC(DST, SRC, STATUS)    \
    do    \
    {    \
        int compare_mac_i;    \
        STATUS = 0;    \
        for (compare_mac_i = 0; compare_mac_i < 6; compare_mac_i++)    \
        {    \
            if (DST[compare_mac_i] != SRC[compare_mac_i])    \
            {    \
                STATUS = 1;    \
                break;    \
            }    \
        }    \
    }while(0)

#define FIND_HOOK_BY_NAME(NAME, NUM, STATUS)    \
    do    \
    {    \
        int find_hook_by_name_i;    \
        STATUS = NUM;    \
        for (find_hook_by_name_i = 0; find_hook_by_name_i < NUM; find_hook_by_name_i++) \
        {    \
            if (strcmp (gp_global_hook[find_hook_by_name_i].hook_name, NAME) == 0)    \
            {     \
                STATUS = find_hook_by_name_i;    \
                break;    \
            }     \
        }    \
    }while(0)

#define FIND_HOOK_BY_IDX(IDX, NUM, STATUS)    \
    do    \
    {    \
        int find_hook_by_idx_i;    \
        STATUS = NUM;    \
        for (find_hook_by_idx_i = 0; find_hook_by_idx_i < NUM; find_hook_by_idx_i++) \
        {    \
            if (gp_global_hook[find_hook_by_idx_i].index == IDX)    \
            {     \
                STATUS = find_hook_by_idx_i;    \
                break;    \
            }     \
        }    \
    }while(0)

#define ADD_HOOK_DST(HEADER, DEV, SAMPLING, ONOFF, STATUS)    \
    do    \
    {    \
        struct hook_unit_dst * add_hook_dst_p_dst = NULL;    \
        struct debug_hook_unit * add_hook_dst_p_hdr = HEADER;    \
        struct hook_dst_device * add_hook_dst_p_dev = DEV;    \
        int add_hook_dst_status;    \
        add_hook_dst_p_dst = add_hook_dst_p_hdr->next;    \
        STATUS = RET_ERROR;    \
        while (add_hook_dst_p_dst != NULL)    \
        {    \
            COMPARE_MAC(add_hook_dst_p_dst->dev.mac, add_hook_dst_p_dev->mac, add_hook_dst_status);    \
            if (add_hook_dst_status == 0)    \
            {    \
                add_hook_dst_p_dst->sampling = SAMPLING;    \
                add_hook_dst_p_dst->dev.trans_id = add_hook_dst_p_dev->trans_id;    \
                STATUS = 0;    \
                break;    \
            }    \
            add_hook_dst_p_dst = add_hook_dst_p_dst->next;    \
        }    \
        \
        if ((add_hook_dst_p_dst == NULL) && (ONOFF == 1) )    \
        {    \
            add_hook_dst_p_dst = add_hook_dst_p_hdr->next;    \
            add_hook_dst_p_hdr->next = (struct hook_unit_dst *) malloc(sizeof(struct hook_unit_dst));    \
            memset(add_hook_dst_p_hdr->next, 0, sizeof(struct hook_unit_dst));    \
            memcpy(&(add_hook_dst_p_hdr->next->dev), add_hook_dst_p_dev, sizeof(struct hook_dst_device));    \
            \
            if (SAMPLING == -1)    \
            {    \
                add_hook_dst_p_hdr->next->sampling = add_hook_dst_p_hdr->default_sampling;    \
            }else    \
            {    \
                add_hook_dst_p_hdr->next->sampling = SAMPLING;    \
            }    \
            add_hook_dst_p_hdr->next->next = add_hook_dst_p_dst;    \
            add_hook_dst_p_hdr->dst_count ++;    \
            STATUS = 0;    \
        }    \
    }while(0)

#define FIND_HOOK_DST(HEADER, DEV, STATUS)    \
    do    \
    {    \
        struct hook_unit_dst * find_hook_dst_p_dst = NULL;    \
        struct debug_hook_unit * find_hook_dst_p_hdr = HEADER;    \
        struct hook_dst_device * find_hook_dst_p_dev = DEV;    \
        int find_hook_dst_status;    \
        find_hook_dst_p_dst = find_hook_dst_p_hdr->next;    \
        STATUS = RET_ERROR;    \
        while (find_hook_dst_p_dst != NULL)    \
        {    \
            COMPARE_MAC(find_hook_dst_p_dst->dev.mac, find_hook_dst_p_dev->mac, find_hook_dst_status);    \
            if (find_hook_dst_status == 0)    \
            {    \
                find_hook_dst_p_dev->trans_id = find_hook_dst_p_dst->dev.trans_id;    \
                STATUS = 0;    \
                break;    \
            }    \
            find_hook_dst_p_dst = find_hook_dst_p_dst->next;    \
        }    \
        \
    }while(0)

#define DEL_HOOK_DST(HEADER, DEV, SAMPLING, STATUS)    \
    do    \
    {    \
        struct hook_unit_dst * del_hook_dst_p_dst = NULL;    \
        struct hook_unit_dst * del_hook_dst_p_dst_free = NULL;    \
        struct debug_hook_unit * del_hook_dst_p_hdr = HEADER;    \
        struct hook_dst_device * del_hook_dst_p_dev = DEV;    \
        int del_hook_dst_status;    \
        STATUS = RET_NOTMATCH;    \
        \
        del_hook_dst_p_dst = del_hook_dst_p_hdr->next;    \
        \
        if (del_hook_dst_p_dst != NULL)    \
        {    \
            while (del_hook_dst_p_dst->next != NULL)    \
            {    \
                COMPARE_MAC(del_hook_dst_p_dst->next->dev.mac, del_hook_dst_p_dev->mac, del_hook_dst_status);    \
                if (del_hook_dst_status == 0)    \
                {    \
                    del_hook_dst_p_dst_free = del_hook_dst_p_dst->next;    \
                    del_hook_dst_p_dst->next = del_hook_dst_p_dst->next->next;    \
                    free(del_hook_dst_p_dst_free);    \
                    del_hook_dst_p_hdr->dst_count --;    \
                    STATUS = 0;    \
                }else    \
                {    \
                    del_hook_dst_p_dst = del_hook_dst_p_dst->next;    \
                }    \
            }    \
            \
            COMPARE_MAC(del_hook_dst_p_hdr->next->dev.mac, del_hook_dst_p_dev->mac, del_hook_dst_status);    \
            if (del_hook_dst_status == 0)    \
            {    \
                del_hook_dst_p_dst_free = del_hook_dst_p_hdr->next;    \
                del_hook_dst_p_hdr->next = del_hook_dst_p_hdr->next->next;    \
                free(del_hook_dst_p_dst_free);    \
                del_hook_dst_p_hdr->dst_count --;    \
                STATUS = 0;    \
            }    \
        }    \
    }while(0)

#define CLEAN_HOOK_DST(HEADER)    \
    do    \
    {    \
        struct hook_unit_dst * clean_hook_dst_p_dst = NULL;    \
        struct hook_unit_dst * clean_hook_dst_p_dst_free = NULL;    \
        struct debug_hook_unit * clean_hook_dst_p_hdr = HEADER;    \
        clean_hook_dst_p_dst = clean_hook_dst_p_hdr->next;    \
        \
        while (clean_hook_dst_p_dst != NULL)    \
        {    \
            clean_hook_dst_p_dst_free = clean_hook_dst_p_dst;    \
            clean_hook_dst_p_dst = clean_hook_dst_p_dst->next;    \
            free(clean_hook_dst_p_dst_free);    \
        }    \
        clean_hook_dst_p_hdr->dst_count = 0;    \
        clean_hook_dst_p_hdr->next = NULL;    \
    }while(0)

int32_t init_global_hook (void)
{
    int idx_count = 0;

//    pthread_mutex_init (&g_hook_mutex, NULL);

//    pthread_mutex_lock (&g_hook_mutex);

    gp_global_hook = (struct debug_hook_unit *) malloc (MAX_HOOK_NUM
            * sizeof(struct debug_hook_unit));

    gp_hook_config_st = (struct config_hook_unit *) malloc (sizeof(struct config_hook_unit));

    memset (gp_global_hook, 0, MAX_HOOK_NUM
            * sizeof(struct debug_hook_unit));

    memset (gp_hook_config_st, 0, sizeof(struct config_hook_unit));


    /* Initial the key name and component name first */

#define CONVERT_HOOK(NAME, COMPONENT, SAMPLING, IDX, LEN)    \
        strcpy ( gp_global_hook[IDX].hook_name, #NAME );    \
        strcpy ( gp_global_hook[IDX].component_name, #COMPONENT );\
        gp_global_hook[IDX].default_sampling = SAMPLING;    \
        gp_global_hook[IDX].dst_count = 0;    \
        gp_global_hook[IDX].index = IDX;    \
        gp_global_hook[IDX].buf_len = LEN;    \
        gp_global_hook[IDX].active = 1;    \
        gp_global_hook[IDX].count = 0;    \
        gp_global_hook[IDX].next = NULL;    \
        pthread_mutex_init( &(gp_global_hook[IDX].hook_mutex), NULL );    \
        idx_count++;
    HOOK_KEYS;
#undef CONVERT_HOOK

    gp_hook_config_st->total_hook_num = idx_count;

//    pthread_mutex_unlock (&g_hook_mutex);

    return 0;
}

int32_t deinit_global_hook (void)
{
    int i = 0;
/*
    if (pthread_mutex_destroy (&g_hook_mutex) != 0)
    {
        FLOG_ERROR ("destory lock error\n");
    }
*/
    if (gp_global_hook != NULL)
    {
        for (i = 0; i < gp_hook_config_st->total_hook_num; i++)
        {
            pthread_mutex_destroy( &(gp_global_hook[i].hook_mutex) );
            CLEAN_HOOK_DST(&(gp_global_hook[i]));
        }
        free (gp_global_hook);
        gp_global_hook = NULL;
    }

    if (gp_hook_config_st != NULL)
    {
        free (gp_hook_config_st);
        gp_hook_config_st = NULL;
    }

    return 0;

}

int32_t cleanall_dst(void)
{
    int i;

//    pthread_mutex_lock (&g_hook_mutex);

    if (gp_global_hook != NULL)
    {
        for (i = 0; i < gp_hook_config_st->total_hook_num; i++)
        {
            pthread_mutex_lock ( &(gp_global_hook[i].hook_mutex) );
            CLEAN_HOOK_DST(&(gp_global_hook[i]));
            pthread_mutex_unlock ( &(gp_global_hook[i].hook_mutex) );
        }
    }

//    pthread_mutex_unlock (&g_hook_mutex);

    return 0;
}

int32_t key_set_global_hook (char * hook_name, struct hook_dst_device * dev, int on_off, int sampling)
{
    int status;
    int ret;

    if ( ( gp_global_hook == NULL ) || ( gp_hook_config_st == NULL ))
    {
        FLOG_ERROR ("global hook haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ( (hook_name == NULL) || (sampling == 0) )
    {
        FLOG_ERROR ("key name or sampling value error\n");
        return RET_ERROR;
    }

    /** search hook unit */
//    pthread_mutex_lock (&g_hook_mutex);

    FIND_HOOK_BY_NAME(hook_name, MAX_HOOK_NUM, status);

    if (status == MAX_HOOK_NUM)
    {
//        pthread_mutex_unlock (&g_hook_mutex);
        return RET_NOTMATCH;
    }

    /** search dst dev */
    switch(on_off)
    {
        case 0:
            pthread_mutex_lock ( &(gp_global_hook[status].hook_mutex) );
            DEL_HOOK_DST(&(gp_global_hook[status]), dev, sampling, ret);
            pthread_mutex_unlock ( &(gp_global_hook[status].hook_mutex) );
            if (ret != 0)
            {
                FLOG_WARNING("Del hoot dst error\n");
            }
        break;
        case -1:
        case 1:
            pthread_mutex_lock ( &(gp_global_hook[status].hook_mutex) );
            ADD_HOOK_DST(&(gp_global_hook[status]), dev, sampling, on_off, ret);
            pthread_mutex_unlock ( &(gp_global_hook[status].hook_mutex) );
            if (ret != 0)
            {
                FLOG_WARNING("Add hoot dst error\n");
            }
        break;
        default :
            FLOG_INFO("Unknown on-off status to set\n");
            ret = RET_ERROR;
        break;
    }

//    pthread_mutex_unlock (&g_hook_mutex);

    return ret;
}

int32_t key_get_global_hook (char * hook_name, struct debug_hook_unit ** p_hook_node)
{
    int i;

    if ( ( gp_global_hook == NULL ) || ( gp_hook_config_st == NULL ))
    {
        FLOG_ERROR ("global hook haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ( (hook_name == NULL) || (p_hook_node == NULL))
    {
        FLOG_ERROR ("key name or on_off / sampling error\n");
        return RET_ERROR;
    }

    *p_hook_node = NULL;

//    pthread_mutex_lock (&g_hook_mutex);

    for (i = 0; i < MAX_HOOK_NUM; i++)
    {
        if (strcmp (gp_global_hook[i].hook_name, hook_name) == 0)
        {
            *p_hook_node = &(gp_global_hook[i]);

//            pthread_mutex_unlock (&g_hook_mutex);
            return 0;
        }
    }

//    pthread_mutex_unlock (&g_hook_mutex);

    return RET_NOTMATCH;
}

int32_t idx_set_global_hook (int index, struct hook_dst_device * dev, int on_off, int sampling)
{
    int ret = 0;

    if ( ( gp_global_hook == NULL ) || ( gp_hook_config_st == NULL ))
    {
        FLOG_ERROR ("global hook haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ( (index >=  MAX_HOOK_NUM) || (index < 0) || (sampling == 0))
    {
        FLOG_ERROR ("index / sampling did not correct\n");
        return RET_ERROR;
    }

    if ( gp_global_hook[index].active != 1 )
    {
        FLOG_ERROR ("Hook did not exist\n");
        return RET_ERROR;
    }

    /** search hook unit */
//    pthread_mutex_lock (&g_hook_mutex);
/*
    FIND_HOOK_BY_IDX(index, MAX_HOOK_NUM, status);

    if (status == MAX_HOOK_NUM)
    {
        pthread_mutex_unlock (&g_hook_mutex);
        return RET_NOTMATCH;
    }
*/
    /** search dst dev */
    switch(on_off)
    {
        case 0:
            pthread_mutex_lock ( &(gp_global_hook[index].hook_mutex) );
            DEL_HOOK_DST(&(gp_global_hook[index]), dev, sampling, ret);
            pthread_mutex_unlock ( &(gp_global_hook[index].hook_mutex) );
            if (ret != 0)
            {
                FLOG_WARNING("Add hoot dst error\n");
            }
        break;
        case -1:
        case 1:
            pthread_mutex_lock ( &(gp_global_hook[index].hook_mutex) );
            ADD_HOOK_DST(&(gp_global_hook[index]), dev, sampling, on_off, ret);
            pthread_mutex_unlock ( &(gp_global_hook[index].hook_mutex) );
            if (ret != 0)
            {
                FLOG_WARNING("Add hoot dst error\n");
            }
        break;
        default :
            FLOG_INFO("Unknown on-off status to set\n");
            ret = RET_ERROR;
        break;
    }

//    pthread_mutex_unlock (&g_hook_mutex);

    return ret;


}

int32_t idx_get_global_hook (int index, struct debug_hook_unit ** p_hook_node)
{
    if ( ( gp_global_hook == NULL ) || ( gp_hook_config_st == NULL ))
    {
        FLOG_ERROR ("global hook haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ( (index >=  MAX_HOOK_NUM) || (index < 0) )
    {
        FLOG_ERROR ("index did not correct\n");
        return RET_ERROR;
    }

    if ( p_hook_node == NULL )
    {
        FLOG_ERROR ("key name or on_off / sampling error\n");
        return RET_ERROR;
    }

    *p_hook_node = NULL;

    if ( gp_global_hook[index].active != 1 )
    {
        FLOG_ERROR ("Hook did not exist\n");
        return RET_ERROR;
    }

//    pthread_mutex_lock (&g_hook_mutex);
    *p_hook_node = &(gp_global_hook[index]);
//    pthread_mutex_unlock (&g_hook_mutex);

    return 0;
}

int32_t key_find_global_hook (char * hook_name, struct hook_dst_device * dev)
{

    int status;
    int ret;

    if ( ( gp_global_hook == NULL ) || ( gp_hook_config_st == NULL ))
    {
        FLOG_ERROR ("global hook haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ( hook_name == NULL )
    {
        FLOG_ERROR ("key name or sampling value error\n");
        return RET_ERROR;
    }

    /** search hook unit */
//    pthread_mutex_lock (&g_hook_mutex);

    FIND_HOOK_BY_NAME(hook_name, MAX_HOOK_NUM, status);

    if (status == MAX_HOOK_NUM)
    {
//        pthread_mutex_unlock (&g_hook_mutex);
        return RET_NOTMATCH;
    }

    pthread_mutex_lock ( &(gp_global_hook[status].hook_mutex) );
    FIND_HOOK_DST(&(gp_global_hook[status]), dev, ret);
    pthread_mutex_unlock ( &(gp_global_hook[status].hook_mutex) );

    if (ret != 0)
    {
         return RET_NOTMATCH;
    }

    return 0;
}

