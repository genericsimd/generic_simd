/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: prephy_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "bs_cfg.h"
#include "queue_util.h"
#include "flog.h"
#include "phy_proc.h"
#include "sys/time.h"

#include "global.h"

#include "prephy_proc.h"

#include "rru_proc.h"
#include "rru_adapter.h"

#include "dump_util.h"

#include "phy_proc.h"
#include "phy_ul_rx_interface.h"


#include "adapter_bs_interface_phy.h"
#include "adapter_ul_stube_test.h"
#include "mac_headermsg_builder.h"

int g_periodic_sensing_enable = 0;
int g_periodic_sensing_reset = 0;
int g_periodic_sensing_drop = 0;
int g_ber_reset = 0;
int g_fake_ulmap_duration = 0;
int g_fake_ulmap_uiuc = 0;


pthread_t phy_pre_thread = 0;

#if (defined FAKE_ULMAP_DURATION)
int fake_ulmap_duration = FAKE_ULMAP_DURATION;
#else
int fake_ulmap_duration = 0;
#endif

#if (defined FAKE_ULMAP_DURATION)
int fake_ulmap_uiuc = FAKE_ULMAP_UIUC;
#else
int fake_ulmap_uiuc = 0;
#endif


#ifdef _RX_FRAME_POOL_ENABLE_
    struct phy_raw_buf_hdr g_raw_buf_hdr[RX_FRM_POOL_SIZE];
#endif


void *process_pre_phy (void *arg __attribute__ ((unused)));

int pre_phy_process (void)
{
    FLOG_INFO ("PRE PHY thread start");

    pthread_attr_t tattr;
    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&phy_pre_thread, &tattr, process_pre_phy, NULL);
    pthread_attr_destroy (&tattr);

    return 0;
}

int pre_phy_release (void)
{
    FLOG_INFO ("PRE PHY thread Release");

    if (phy_pre_thread != 0)
    {
        pthread_cancel (phy_pre_thread);
        pthread_join (phy_pre_thread, NULL);
    }

    return 0;
}

void *process_pre_phy (void *arg __attribute__ ((unused)))
{
    FLOG_INFO ("PRE PHY process started");

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

//    short tmp_value = 0;

    struct timeval start_t, end_t;
    unsigned int count_t = 0;
    unsigned int total = 0;

    struct queue_msg rx_msg[ANTENNA_NUM];

    struct queue_msg tx_msg;
    //    struct phy_ul_rru_symbol* p_rru_data = NULL;

    int i_buf_tmp = 0;
    int symbol_idx = 0;
//    int end_flag = 0;

    struct phy_frame_info phy_rx_info;

    struct rru_data_param * p_param_hdr[ANTENNA_NUM];
    short * rx_buf_short[ANTENNA_NUM];

    struct phy_ul_rru_symbol * p_rru_symbol[ANTENNA_NUM];
    struct phy_ul_rru_symbol * p_ranging_symbol[ANTENNA_NUM];

    int i, j, k, l;

    int ulmap_ridx = 0;

//    void * p_ulmap = NULL;
//    unsigned int ulmap_len = 0;
//    ul_map_msg *p_ul_map_stube = NULL;

    int skip_rx_flag = 0;

    int ranging_symbol_flag = 0;

    int symbol_offset_ir = 0;
    int symbol_offset_pr = 0;
    int ret;

    int cont_frm_flag = 0;
    int ranging_enable = 1;

//    struct fake_ul_map st_fmap;

#ifdef _RX_FRAME_POOL_ENABLE_
    struct phy_raw_buf_node * p_raw_node = NULL;
    void * tmp_buf = NULL;
    int store_idx = 0;
#endif

    for (i = 0; i < UL_MAP_STORE; i++)
    {
        gs_phy_frame_info.ul_map[i] = NULL;
        gs_phy_frame_info.ul_map_len[i] = 0;
        gs_phy_frame_info.frame_number[i] = 0;
        gs_phy_frame_info.dl_perscan_flag[i] = 0;

        phy_rx_info.ul_map[i] = NULL;
        phy_rx_info.ul_map_len[i] = 0;
        phy_rx_info.frame_number[i] = 0;
        phy_rx_info.dl_perscan_flag[i] = 0;
    }

#ifdef _RX_FRAME_POOL_ENABLE_
    for (i = 0; i < RX_FRM_POOL_SIZE; i++)
    {
        g_raw_buf_hdr[i].ul_map_len = 0;
        g_raw_buf_hdr[i].frame_number = 0;

        g_raw_buf_hdr[i].node = malloc(sizeof (struct phy_raw_buf_node));

        p_raw_node = g_raw_buf_hdr[i].node;
        memset (p_raw_node, 0, sizeof ( struct phy_raw_buf_node ));
        p_raw_node->slot_symbol_idx = 0;
        p_raw_node->data_buf_len = MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot * 2;

        for (j = 1; j < g_rru_data_param.rx_blk_per_frm; j++)
        {
             p_raw_node->next = malloc(sizeof (struct phy_raw_buf_node));
             memset (p_raw_node->next, 0, sizeof ( struct phy_raw_buf_node ));
             p_raw_node->next->slot_symbol_idx = j;
             p_raw_node->next->data_buf_len = MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot * 2;
             p_raw_node = p_raw_node->next;
        }
    }
#endif

#ifdef _FAKE_ULMAP_ENABLE_

    struct fake_ul_map st_fmap;
    void *p_ulmap = malloc (4096);
    unsigned int ulmap_len = 0;
    ul_map_msg *p_ul_map_stube = NULL;

    st_fmap.ss_num = 1;
    st_fmap.ie.bcid = 11;
    st_fmap.ie.next = NULL;

    ret = get_global_param ("FAKE_ULMAP_DURATION", &(st_fmap.ie.duration));
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_DURATION error\n");
    }

    ret = get_global_param ("FAKE_ULMAP_UIUC", &(st_fmap.ie.uiuc));
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_UIUC error\n");
    }


    p_ul_map_stube = (ul_map_msg*) malloc (sizeof(ul_map_msg));
    memset (p_ul_map_stube, 0, sizeof(ul_map_msg));
    adapter_build_ul_map (p_ul_map_stube, &st_fmap);
    memset (p_ulmap, 0, 4096);

    build_ul_map_msg (p_ul_map_stube, p_ulmap, (unsigned int *)&ulmap_len);

#endif

    ret = get_global_param ("IR_SYMBOL_OFFSET", & ( symbol_offset_ir ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IR_SYMBOL_OFFSET error\n");
    }


    ret = get_global_param ("PR_SYMBOL_OFFSET", & ( symbol_offset_pr ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PR_SYMBOL_OFFSET error\n");
    }

    ret = get_global_param ("CONTINUE_FRM_NUM", & ( cont_frm_flag ));

    if (ret != 0)
    {
        FLOG_WARNING ("get CONTINUE_FRM_NUM error\n");
    }

    ret = get_global_param ("RANGING_ENABLE", & ( ranging_enable ));

    if (ret != 0)
    {
        FLOG_WARNING ("get RANGING_ENABLE error\n");
    }

    ret = get_global_param ("PERIODIC_SENSING_ENABLE", & ( g_periodic_sensing_enable ));

    if (ret != 0)
    {
        FLOG_WARNING ("get PERIODIC_SENSING_ENABLE error\n");
    }
    
    ret = get_global_param ("BER_RESET", & ( g_ber_reset ));

    if (ret != 0)
    {
        FLOG_WARNING ("get BER_RESET error\n");
    }


    while (1)
    {
        i_buf_tmp = 0;

        for (i = 0; i < ANTENNA_NUM; i++)
        {
            rx_msg[i].my_type = pre_ul_de_id[i];

            if (wmrt_dequeue (pre_ul_de_id[i], &rx_msg[i], sizeof(struct queue_msg))
                    == -1)
            {
                FLOG_ERROR ("DEQUEUE ERROR\n");
            }

            p_param_hdr[i] = (struct rru_data_param *)rx_msg[i].p_buf;
            rx_buf_short[i] = (short *)( (void *)rx_msg[i].p_buf + sizeof (struct rru_data_param));

        }
/*
        printf("prephy: frame number %d, endflag %d, dgain %f %f\n", p_param_hdr[0]->frm_num,
                                                                     p_param_hdr[0]->end_flag,
                                                                     p_param_hdr[0]->dgain[0],
                                                                     p_param_hdr[1]->dgain[0] );
*/
        if ( (p_param_hdr[0]->frm_num != p_param_hdr[1]->frm_num) ||
             (p_param_hdr[0]->end_flag != p_param_hdr[1]->end_flag) )
        {
            FLOG_WARNING("frame number mismatch!\n");
/*
            pthread_mutex_lock(&g_resync_lock);
            g_resync_count ++;
            pthread_mutex_unlock(&g_resync_lock);
*/
            free(rx_msg[0].p_buf);
            free(rx_msg[1].p_buf);

            continue;

//            skip_rx_flag = 1;
            /** handle it in future! */
        }

        if (p_param_hdr[0]->end_flag == HEAD_FLAG)
        {
            ulmap_ridx = (p_param_hdr[0]->frm_num) % UL_MAP_STORE;

            pthread_mutex_lock (&mutex_phy_frame_info);
/*
            if (gs_phy_frame_info.info_flag != 0)
            {
                FLOG_WARNING ("Rx FRAME info read error!\n");
            }
*/
            phy_rx_info.ul_map[0] = gs_phy_frame_info.ul_map[ulmap_ridx];
            gs_phy_frame_info.ul_map[ulmap_ridx] = NULL;

            phy_rx_info.ul_map_len[0] = gs_phy_frame_info.ul_map_len[ulmap_ridx];
            gs_phy_frame_info.ul_map_len[ulmap_ridx] = 0;

//            phy_rx_info.dts_info[0] = gs_phy_frame_info.dts_info[ulmap_ridx];
//            gs_phy_frame_info.dts_info[ulmap_ridx] = NULL;

            phy_rx_info.dl_perscan_flag[0] = gs_phy_frame_info.dl_perscan_flag[ulmap_ridx];
            gs_phy_frame_info.dl_perscan_flag[ulmap_ridx] = 0;

            phy_rx_info.frame_number[0] = gs_phy_frame_info.frame_number[ulmap_ridx] - FRM_NUM_ADJ;
            gs_phy_frame_info.frame_number[ulmap_ridx] = 0;

//            gs_phy_frame_info.info_flag = 2;

            pthread_mutex_unlock (&mutex_phy_frame_info);

            symbol_idx = 0;

            if ( (phy_rx_info.ul_map[0] == NULL) ||
                 ( (unsigned short)(phy_rx_info.frame_number[0] + g_init_rruframe_num + FRM_NUM_ADJ)
                     != p_param_hdr[0]->frm_num ) )
            {
                FLOG_WARNING ("Rx Skip frame! Wrong UL MAP\n");

                pthread_mutex_lock(&g_resync_lock);
                g_resync_count ++;
                pthread_mutex_unlock(&g_resync_lock);

                skip_rx_flag = 1;

            }else
            {
                skip_rx_flag = 0;

#ifdef _FAKE_ULMAP_ENABLE_
                free(phy_rx_info.ul_map[0]);
                phy_rx_info.ul_map[0] = malloc(4096);
                memcpy (phy_rx_info.ul_map[0], p_ulmap, ulmap_len);
                phy_rx_info.ul_map_len[0] = ulmap_len;
#endif
            }
#ifdef _RX_FRAME_POOL_ENABLE_
            g_raw_buf_hdr[store_idx].frame_number = phy_rx_info.frame_number[0];
            g_raw_buf_hdr[store_idx].ul_map_len = phy_rx_info.ul_map_len[0];
            memcpy( g_raw_buf_hdr[store_idx].ul_map,
                    phy_rx_info.ul_map[0],
                    phy_rx_info.ul_map_len[0] );

            p_raw_node = g_raw_buf_hdr[store_idx].node;

            store_idx ++;
            if (store_idx >= RX_FRM_POOL_SIZE)
            {
                store_idx = 0;
            }
#endif
/*
            ulmap_ridx ++;
            if (ulmap_ridx >= UL_MAP_STORE)
            {
                ulmap_ridx = 0;
            }
*/
/** for dump */
//          DO_DUMP(DUMP_RX_ALL_RAW_ANTI_ID, phy_rx_info.frame_number[0], "frame num", 0);
 //         DO_DUMP(DUMP_RX_ALL_RAW_ANTO_ID, phy_rx_info.frame_number[0], "frame num", 0);
        }

        if ( phy_rx_info.dl_perscan_flag[0] == 0 &&
             ( (symbol_idx == symbol_offset_ir) || (symbol_idx == symbol_offset_pr) ) )
        {
            ranging_symbol_flag = 1;
        }else
        {
            ranging_symbol_flag = 0;
        }

/*********************************************************************/
//if (phy_rx_info.dl_perscan_flag[0] == 1)
//printf("start sensing %d\n", phy_rx_info.dl_perscan_flag[0]);

        if (cont_frm_flag == 0)
        {
            phy_rx_info.frame_number[0] = 0;
        }

        if (g_periodic_sensing_enable == 0)
        {
            phy_rx_info.dl_perscan_flag[0] = 0;
        }

        if (ranging_enable == 0)
        {
            ranging_symbol_flag = 0;
        }

/*********************************************************************/
/*
        printf("prephy: frame number %d\n", phy_rx_info.frame_number);
*/
        if (skip_rx_flag == 0)
        {
            for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
            {
                p_rru_symbol[i] = phy_ul_init_rrusymbol (phy_rx_info.frame_number[0],
                        symbol_idx,
                        g_rru_data_param.rx_symbol_per_slot,
                        MAX_SYMBOL_LEN,
                        p_param_hdr[0]->end_flag );

                /** set the UL_MAP only in the first symbol */
                if (symbol_idx == 0)
                {
                    p_rru_symbol[i]->ul_map = (void *)(phy_rx_info.ul_map[0]);
                    p_rru_symbol[i]->ul_map_len = phy_rx_info.ul_map_len[0];
                }

                p_rru_symbol[i]->ul_perscan_flag = phy_rx_info.dl_perscan_flag[0];
                p_rru_symbol[i]->dgain = p_param_hdr[i]->dgain[0];


                /** for ranging! */
                if ( ranging_symbol_flag == 1 )
                {
                    p_ranging_symbol[i] = phy_ul_init_rrusymbol (phy_rx_info.frame_number[0],
                            symbol_idx,
                            g_rru_data_param.rx_symbol_per_slot,
                            MAX_SYMBOL_LEN,
                            p_param_hdr[0]->end_flag );

                    p_ranging_symbol[i]->dgain = p_param_hdr[i]->dgain[0];
                }
            }

            for (j = 0; j < g_rru_data_param.rx_symbol_per_slot; j++)
            {
                for (l = 0; l < PACKET_PER_SYMBOL; l += g_rru_data_param.rx_ant_num)
                {
                    for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
                    {
                        for (k = 0; k < ( SAMPLE_PER_PACKET << 1 ); k += 2)
                        {

                            p_rru_symbol[i]->symbol_i[i_buf_tmp + ( k >> 1 )]
                                    = (float) rx_buf_short[i][k];
                            p_rru_symbol[i]->symbol_q[i_buf_tmp + ( k >> 1 )]
                                    = (float) rx_buf_short[i][k + 1];

                            if (ranging_symbol_flag == 1)
                            {
                                p_ranging_symbol[i]->symbol_i[i_buf_tmp + ( k >> 1 )]
                                        = (float) rx_buf_short[i][k];
                                p_ranging_symbol[i]->symbol_q[i_buf_tmp + ( k >> 1 )]
                                        = (float) rx_buf_short[i][k + 1];
                            }
                        }

                        rx_buf_short[i] += ( SAMPLE_PER_PACKET << 1 );
                    }

                    i_buf_tmp += SAMPLE_PER_PACKET;
                }
            }

            /** finish the using of RX memory */
            for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
            {
                tx_msg.my_type = pre_ul_en_id[i];

                tx_msg.p_buf = (void *) p_rru_symbol[i];

                if (wmrt_enqueue (pre_ul_en_id[i], &tx_msg,
                        sizeof(struct queue_msg)) == -1)
                {
                    FLOG_WARNING ("ENQUEUE ERROR\n");
                }
            }

            /** Ranging part */

            if ( ranging_symbol_flag == 1 )
            {
                for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
                {
                    tx_msg.my_type = pre_ul_en_id[i + g_rru_data_param.rx_ant_num];

                    tx_msg.p_buf = (void *) p_ranging_symbol[i];

                    if (wmrt_enqueue (pre_ul_en_id[i + g_rru_data_param.rx_ant_num], &tx_msg,
                            sizeof(struct queue_msg)) == -1)
                    {
                        FLOG_WARNING ("ENQUEUE ERROR\n");
                    }
                }
            }

            symbol_idx += g_rru_data_param.rx_symbol_per_slot;

/** for DUMP */
#ifndef _DUMP_BIN_RAW_DATA_
            rx_buf_short[0] = (short *)( (void *)rx_msg[0].p_buf + sizeof (struct rru_data_param));
            rx_buf_short[1] = (short *)( (void *)rx_msg[1].p_buf + sizeof (struct rru_data_param));

            for (i = 0; i < MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot; i++)
            {
                DO_DUMP(DUMP_RX_ALL_RAW_ANTO_ID, -1, rx_buf_short[0], 2);
                DO_DUMP(DUMP_RX_ALL_RAW_ANTI_ID, -1, rx_buf_short[1], 2);
                rx_buf_short[0] += 2;
                rx_buf_short[1] += 2;
            }
#else
            rx_buf_short[0] = (short *)( (void *)rx_msg[0].p_buf + sizeof (struct rru_data_param));
            rx_buf_short[1] = (short *)( (void *)rx_msg[1].p_buf + sizeof (struct rru_data_param));

            DO_DUMP(DUMP_RX_ALL_RAW_ANTO_ID, -1, rx_buf_short[0], MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot);
            DO_DUMP(DUMP_RX_ALL_RAW_ANTI_ID, -1, rx_buf_short[1], MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot);
#endif

            if (phy_rx_info.dl_perscan_flag[0] == 1)
            {
                DO_DUMP(DUMP_PR_RX_ALL_RAW_ID, -1, rx_buf_short[0], MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot);
                DO_DUMP(DUMP_PR_RX_ALL_RAW_ID, -1, rx_buf_short[1], MAX_SYMBOL_LEN * g_rru_data_param.rx_symbol_per_slot);
            }
        }

        for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
        {
#ifdef _RX_FRAME_POOL_ENABLE_
            tmp_buf = p_raw_node->memory_hdr[i];

            p_raw_node->memory_hdr[i] = rx_msg[i].p_buf;
            p_raw_node->data_buf[i] = (void *)rx_msg[i].p_buf + sizeof (struct rru_data_param);

            if (tmp_buf != NULL)
            {
                free(tmp_buf);
            }
#else
            free (rx_msg[i].p_buf);
#endif
        }

#ifdef _RX_FRAME_POOL_ENABLE_
        p_raw_node = p_raw_node->next;
#endif

        if (count_t == 5)
        {
            gettimeofday (&end_t, NULL);

            total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                    - start_t.tv_usec );

            gettimeofday (&start_t, NULL);

            if (total > THREADHOLD)
            {
                FLOG_INFO ("Rx time %d at frame %d\n", total, phy_rx_info.frame_number[0]);
            }

            DO_DUMP(DUMP_FRM_RX_DURATION_ID, phy_rx_info.frame_number[0], NULL, total);

            count_t = 0;
        }

        count_t++;
    }

    return NULL;
}

int frm_dump_all_rx(unsigned int frame_num, FILE * f_p)
{
#ifdef _RX_FRAME_POOL_ENABLE_
    int i = 0;
    int j = 0;
    int k = 0;
    struct phy_raw_buf_node * p_raw_node = NULL;

    short * rx_buf_short[ANTENNA_NUM];

    for (i = 0; i < RX_FRM_POOL_SIZE; i++)
    {
        if (g_raw_buf_hdr[i].frame_number == frame_num)
        {
            fprintf(f_p, "------ frame %d ------\n", g_raw_buf_hdr[i].frame_number);
        }else
        {
            fprintf(f_p, "------ Match frame %d ------\n", g_raw_buf_hdr[i].frame_number);
        }

        fprintf(f_p, "ULMAP len %d\n", g_raw_buf_hdr[i].ul_map_len);
        fprintf(f_p, "ULMAP:\n");

        for (j = 0; j < (int)g_raw_buf_hdr[i].ul_map_len; j++)
        {
            fprintf(f_p, "%02x ", ( (unsigned char *)(g_raw_buf_hdr[i].ul_map) )[j]);
        }

        fprintf(f_p, "\n");

        p_raw_node = g_raw_buf_hdr[i].node;

        fprintf(f_p, "Rx RAW data (in format I0 Q0 I1 Q1...)\n");

        for (k = 0; k < g_rru_data_param.rx_blk_per_frm; k++)
        {
//          fprintf(f_p, "slot_symbol: %d\n", p_raw_node->slot_symbol_idx);

            rx_buf_short[0] = (short *)(p_raw_node->data_buf[0]);
            rx_buf_short[1] = (short *)(p_raw_node->data_buf[1]);

            for (j = 0; j < p_raw_node->data_buf_len; j += 2)
            {
                fprintf(f_p, "%d\n", rx_buf_short[0][j]);
                fprintf(f_p, "%d\n", rx_buf_short[0][j + 1]);
                fprintf(f_p, "%d\n", rx_buf_short[1][j]);
                fprintf(f_p, "%d\n", rx_buf_short[1][j + 1]);
            }

            p_raw_node = p_raw_node->next;
        }
    }

    return 0;
#else
    (void)frame_num;
    (void)f_p;

    return 1;
#endif
}

int frm_dump_select_rx(unsigned int frame_num, FILE * f_p)
{
#ifdef _RX_FRAME_POOL_ENABLE_
    int i = 0;
    int j = 0;
    int k = 0;
    struct phy_raw_buf_node * p_raw_node = NULL;

    short * rx_buf_short[ANTENNA_NUM];

    for (i = 0; i < RX_FRM_POOL_SIZE; i++)
    {
        if (g_raw_buf_hdr[i].frame_number == frame_num)
        {
            fprintf(f_p, "------ Match frame %d ------\n", g_raw_buf_hdr[i].frame_number);

            fprintf(f_p, "ULMAP len %d\n", g_raw_buf_hdr[i].ul_map_len);
            fprintf(f_p, "ULMAP:\n");

            for (j = 0; j < (int)g_raw_buf_hdr[i].ul_map_len; j++)
            {
                fprintf(f_p, "%02x ", ( (unsigned char *)(g_raw_buf_hdr[i].ul_map) )[j]);
            }

            fprintf(f_p, "\n");

            p_raw_node = g_raw_buf_hdr[i].node;

            fprintf(f_p, "Rx RAW data (in format I0 Q0 I1 Q1...)\n");

            for (k = 0; k < g_rru_data_param.rx_blk_per_frm; k++)
            {
//              fprintf(f_p, "slot_symbol: %d\n", p_raw_node->slot_symbol_idx);

                rx_buf_short[0] = (short *)(p_raw_node->data_buf[0]);
                rx_buf_short[1] = (short *)(p_raw_node->data_buf[1]);

                for (j = 0; j < p_raw_node->data_buf_len; j += 2)
                {
                    fprintf(f_p, "%d\n", rx_buf_short[0][j]);
                    fprintf(f_p, "%d\n", rx_buf_short[0][j + 1]);
                    fprintf(f_p, "%d\n", rx_buf_short[1][j]);
                    fprintf(f_p, "%d\n", rx_buf_short[1][j + 1]);
                }

                p_raw_node = p_raw_node->next;
            }

//            fflush(f_p);
            return 0;
        }
    }

    fprintf(f_p, "------ No matched frame to frame number %d ------\n", frame_num);
//    fflush(f_p);

    return 0;
#else
    (void)frame_num;
    (void)f_p;
    return 1;
#endif
}

int cfg_update_ps_enable_cb(int type, int len, void * buf)
{
    int value = *((int *)buf);

    if (value == 1)
    {
        g_periodic_sensing_enable = 1;
        g_periodic_sensing_reset = 0;
    }
    else if (value == 0)
    {
        g_periodic_sensing_reset = 1;
        g_periodic_sensing_enable = 1;
    }else
    {

    }

    FLOG_INFO("change PS enable status %d, %d\n", g_periodic_sensing_enable, g_periodic_sensing_reset);

    (void) len;
    (void) type;

    return 0;
}

int cfg_update_ber_reset(int type, int len, void * buf)
{
    int value = *((int *)buf);

    if (value == 1)
    {
        g_ber_reset = 1;
    }
    else
    {
        g_ber_reset = 0;
    }


    FLOG_INFO("change BER RESET status %d\n", g_ber_reset);

    (void) len;
    (void) type;

    return 0;
}


int cfg_update_fake_ulmap_duration(int type, int len, void * buf)
{
    int value = *((int *)buf);

    if (value > 149 || value < 0)
    {
        FLOG_ERROR("ERROR: FAKE ULMAP DURATIONS Exceed the scope\n");
        g_fake_ulmap_duration = 10;
    }
    else
    {
        g_fake_ulmap_duration = value;
        FLOG_INFO("change FAKE ULMAP DURATIONS %d\n", g_fake_ulmap_duration);
    }

    (void) len;
    (void) type;

    return 0;
}

int cfg_update_fake_ulmap_uiuc(int type, int len, void * buf)
{
    int value = *((int *)buf);


    if (value > 10)
    {
        FLOG_ERROR("ERROR: FAKE ULMAP UIUC not in scope\n");
        g_fake_ulmap_uiuc = 1; //default QPSK 1/2
    }
    else
    {
        g_fake_ulmap_uiuc = value;
        FLOG_INFO("change FAKE ULMAP UIUC %d\n", g_fake_ulmap_uiuc);
    }


    (void) len;
    (void) type;

    return 0;
}

