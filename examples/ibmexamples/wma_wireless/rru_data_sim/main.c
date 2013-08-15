/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: main.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>

#include <unistd.h>

/** from base support lib */
#include "witm_info.h"

#include "rru_adapter.h"
#include "rru_proc.h"
#include "flog.h"

#include "config.h"

struct rru_nic_config g_rru_nic_param;

pthread_t rx_thread = 0;

FILE * r_i[ANTENNA_NUM];
FILE * r_q[ANTENNA_NUM];

FILE * w_i[ANTENNA_NUM];
FILE * w_q[ANTENNA_NUM];

struct timeval g_start_t0;
struct timeval g_end_t0;

unsigned int g_total = 0;
unsigned int g_delay = 0;
unsigned int g_delay_count = 0;
unsigned int g_avg_delay = 0;

#ifdef _DUMP_TX_
/** test code */
FILE * w0_i[ANTENNA_NUM];
FILE * w0_q[ANTENNA_NUM];
/** end of test code */
#endif

void *process_rx (void *arg __attribute__ ((unused)))
{
    short * rx_buf;
    short * p_rx_buf;

    int ret = 0;

#ifdef _DUMP_RX_
    int i, j, k, l;
#endif

    struct timeval start_t, end_t;
    unsigned int count_t = 0;
    unsigned int total = 0;
    unsigned int frame_count = 0;

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");
        return NULL;
    }

    rx_buf = malloc(RX_SYMBOL_NUM * FIX_DATA_LEN * 2 * ANTENNA_NUM * 2);
    p_rx_buf = rx_buf;

    while (1)
    {
        p_rx_buf = rx_buf;

        ret = ethrru_data_rx ((char *) p_rx_buf, RX_SYMBOL_NUM);

        if (ret != 0)
        {
            FLOG_WARNING("Rx data error\n");
        }

        if (count_t == 1)
        {
            gettimeofday (&end_t, NULL);

            total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec - start_t.tv_usec );
            gettimeofday (&start_t, NULL);

            count_t = 0;

            if (total > THREAD_HOLD)
            {
                FLOG_INFO ("Rx time  %d, frame number %d\n", total, frame_count);
            }
        }

        count_t ++;
        frame_count ++;

#ifdef _DUMP_RX_
        for (j = 0; j < g_rru_nic_param.rx_symbol_num; j++)
        {
            for (l = 0; l < PACKET_PER_SYMBOL; l += ANTENNA_NUM)
            {
                for (i = 0; i < ANTENNA_NUM; i++)
                {
                    for (k = 0; k < (FIX_DATA_LEN >> 1); k += 2)
                    {
                        fprintf (w_i[i], "%d\n", p_rx_buf[k]);
                        fprintf (w_q[i], "%d\n", p_rx_buf[k + 1]);
                    }

                    p_rx_buf += (FIX_DATA_LEN >> 1);
                }
            }
        }
#endif

#ifdef _DETETC_DELAY_
        gettimeofday (&g_end_t0, NULL);

        g_delay = ( g_end_t0.tv_sec - g_start_t0.tv_sec ) * 1000000 + ( g_end_t0.tv_usec
                  - g_start_t0.tv_usec );

        if (g_delay > UL_DURATION)
        {
            g_delay_count ++;
            FLOG_INFO("Rx frame late %d, at frame %d\n", g_total, frame_count);
        }

        g_total += g_delay;
        g_avg_delay = g_total / frame_count;
#endif

    }
    return NULL;
}

static int exit_mbmsproc (int cmd)
{
#ifdef _DUMP_RX_
    int i;
#endif

    FLOG_DEBUG ("Base Station Process exited with %d", cmd);

    (void) cmd;

#ifdef _DETETC_DELAY_

    FLOG_INFO("delay count %d, average delay %d\n", g_delay_count, g_avg_delay);

#endif

#ifdef _DUMP_RX_
    for (i = 0; i < ANTENNA_NUM; i++)
    {
        fclose (w_i[i]);
        fclose (w_q[i]);
    }
#endif

    ethrru_close();

#ifdef _DUMP_TX_
    /** test code */
    fclose(w0_i[0]);
    fclose(w0_q[0]);
    fclose(w0_i[1]);
    fclose(w0_q[1]);
    /** end test code */
#endif

    wmrt_exit ();

    return 0;
}

int main (int argc, char *argv[])
{
    int i, j, k;

    short * tx_buf;
    short * tmp_tx_buf;
    float i_buf[ANTENNA_NUM][TX_SYMBOL_NUM * FIX_PAYLOAD_LEN];
    float q_buf[ANTENNA_NUM][TX_SYMBOL_NUM * FIX_PAYLOAD_LEN];

    char infile_i[128];
    char infile_q[128];

    int ante_idx = 0;

    int i_buf_tmp = 0;
    int j_buf_tmp = 0;

    int idx_sample = 0;
    int idx_symbol = 0;
    int idx_pkt = 0;

    struct timeval start_t, end_t;
    unsigned int count_t = 0;
    unsigned int total = 0;
    unsigned int frame_count = 0;

    (void) argc;
    (void) argv;

    LOG_INIT_CONSOLE_ONLY ("RRU Data simulator");

    if (wminfo_init (WMI_ROLEBS) != 0)
    {
        FLOG_FATAL ("Failed to initialize Managerial Runtime!");
        return -1;
    }

    if (wmrt_setsysexitfunc (&exit_mbmsproc) != 0)
    {
        FLOG_FATAL ("Failed to set sys-exit");
        return -1;
    }

#ifdef _DUMP_TX_
    /** test code */
    w0_i[0] = fopen ("TX_OUT_I_0.dat", "wb");
    w0_q[0] = fopen ("TX_OUT_Q_0.dat", "wb");
    w0_i[1] = fopen ("TX_OUT_I_1.dat", "wb");
    w0_q[1] = fopen ("TX_OUT_Q_1.dat", "wb");
    /** end test code */
#endif

    tx_buf = malloc (TX_SYMBOL_NUM * FIX_PAYLOAD_LEN * 2 * ANTENNA_NUM * FRAME_NUM * 2);
    tmp_tx_buf = tx_buf;

    for (ante_idx = 0; ante_idx < ANTENNA_NUM; ante_idx++)
    {
        sprintf (infile_i, "IN_I_%d.dat", ante_idx);
        sprintf (infile_q, "IN_Q_%d.dat", ante_idx);

        r_i[ante_idx] = fopen (infile_i, "rb+");
        r_q[ante_idx] = fopen (infile_q, "rb+");

        if ( ( r_i[ante_idx] == NULL ) || ( r_q[ante_idx] == NULL ))
        {
            FLOG_FATAL ("NO INPUT I/Q file found (OUT_I_x.dat, OUT_Q_x.dat)");
            return 0;
        }
    }

#ifdef _DUMP_RX_
    for (ante_idx = 0; ante_idx < ANTENNA_NUM; ante_idx++)
    {
        sprintf (infile_i, "OUT_I_%d.dat", ante_idx);
        sprintf (infile_q, "OUT_Q_%d.dat", ante_idx);

        w_i[ante_idx] = fopen (infile_i, "wb");
        w_q[ante_idx] = fopen (infile_q, "wb");

        if ( ( w_i[ante_idx] == NULL ) || ( w_q[ante_idx] == NULL ))
        {
            FLOG_FATAL ("NO INPUT I/Q file found (OUT_I_x.dat, OUT_Q_x.dat)");
            return 0;
        }
    }
#endif

    g_rru_nic_param.dst_ip = ntohl (inet_addr (DST_IP));
    g_rru_nic_param.src_ip = ntohl (inet_addr (SRC_IP));

    g_rru_nic_param.dst_port = DST_PORT;
    g_rru_nic_param.src_port = SRC_PORT;

    g_rru_nic_param.rru_id = RRU_ID;
    g_rru_nic_param.rx_symbol_num = RX_SYMBOL_NUM;
    g_rru_nic_param.tx_symbol_num = TX_SYMBOL_NUM;

    if (ethrru_init() != 0)
    {
        FLOG_FATAL ("Failed to init NIC");
        return -1;
    }

    for (k = 0; k < FRAME_NUM; k++)
    {

        for (ante_idx = 0; ante_idx < ANTENNA_NUM; ante_idx++)
        {
            for (j = 0; j < TX_SYMBOL_NUM * FIX_DATA_LEN; j++)
            {
                fscanf (r_i[ante_idx], "%f\n", & ( i_buf[ante_idx][j] ));

                fscanf (r_q[ante_idx], "%f\n", & ( q_buf[ante_idx][j] ));
            }
        }

        /* we keep this for zero-copy in Tx */
        for (i = 0; i < ANTENNA_NUM; i++)
        {
            i_buf_tmp = ( SAMPLE_PER_PACKET << 1 ) * i + (DATA_HDR_LEN >> 1);
            j_buf_tmp = 0;

            for (idx_symbol = 0; idx_symbol < TX_SYMBOL_NUM; idx_symbol++)
            {
                for (idx_pkt = 0; idx_pkt < PACKET_PER_ANT; idx_pkt++)
                {
                    for (idx_sample = 0; idx_sample < SAMPLE_PER_PACKET; idx_sample++)
                    {
                        tmp_tx_buf[i_buf_tmp + ( idx_sample << 1 )]
                                = (short) ( i_buf[i][j_buf_tmp + idx_sample] * G_TX);

                        tmp_tx_buf[i_buf_tmp + ( idx_sample << 1 ) + 1]
                                = (short) ( q_buf[i][j_buf_tmp + idx_sample] * G_TX);

#ifdef _DUMP_TX_
                        /** test code */
                        fprintf (w0_i[i], "%d\n", tmp_tx_buf[i_buf_tmp + ( idx_sample << 1 )]);
                        fprintf (w0_q[i], "%d\n", tmp_tx_buf[i_buf_tmp + ( idx_sample << 1 ) + 1]);
                        /** end of test code */
#endif
                    }

//               printf( "i_buf_tmp %d\n", i_buf_tmp);
                    i_buf_tmp += (SAMPLE_PER_PACKET << 1) * ANTENNA_NUM;
                    j_buf_tmp += SAMPLE_PER_PACKET;
                }
            }
        }

        tmp_tx_buf += TX_SYMBOL_NUM * FIX_PAYLOAD_LEN * 2 * ANTENNA_NUM;
    }

    for (i = 0; i < ANTENNA_NUM; i++)
    {
        fclose (r_i[i]);
        fclose (r_q[i]);
    }

    /** start the RX thread here */
    pthread_create (&rx_thread, NULL, process_rx, NULL);
/*
    while(1)
    {
        sleep(1);
    }
*/
    while (1)
    {
        tmp_tx_buf = tx_buf;

        for (k = 0; k < FRAME_NUM; k++)
        {

#ifdef _DETETC_DELAY_
            gettimeofday (&g_start_t0, NULL);
#endif
            if (ethrru_data_tx ((char *) tmp_tx_buf, g_rru_nic_param.tx_symbol_num,
                    CMD_DELAT) != 0)
            {
                FLOG_WARNING ("Tx packet to RRH failed.\n");
            }

            usleep(FRAME_DURATION - CMD_DELAT - 600);

            if (count_t == 1)
            {
                gettimeofday (&end_t, NULL);

                total += ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                        - start_t.tv_usec );

                gettimeofday (&start_t, NULL);

                if (total > THREAD_HOLD)
                {
                    FLOG_INFO ("Tx time  %d, frame number %d\n", total, frame_count);
                }

                total = 0;
                count_t = 0;
            }

            count_t ++;
            frame_count ++;
//        printf("%d\n", total);
        //printf("ISSUE to ETH_RRU %d bytes\n", len);

            i_buf_tmp = 0;
            tmp_tx_buf += TX_SYMBOL_NUM * FIX_PAYLOAD_LEN * 2 * ANTENNA_NUM;
        }
    }

    exit_mbmsproc (0);

    return 0;
}
