/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: rru_adapter.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>  
#include <net/if.h>  
#include <sys/types.h>  
#include <arpa/inet.h>  
#include <errno.h>

#include "rru_proc.h"
#include "rru_adapter.h"
#include "config.h"

#include "flog.h"

static int g_tx_sock_fd;
static int g_rx_sock_fd;

static struct sockaddr_in g_sin_tx;
static struct sockaddr_in g_sin_rx;

static unsigned short g_tx_seq_no = 0;
static unsigned short g_tx_frm_no = TX_FRAME_NUM_START;

/** for cmd packet */
char cmd_pkt[FIX_PAYLOAD_LEN];
static unsigned short dgain[2];

extern int errno;

/*
 * PacketInit - Socket initialization.
 */
static int rru_init_socket (void)
{
    struct rrh_data_hdr * p_tmp_hdr;

    struct sockaddr_in sin;
    int recv_buf_len = RECV_BUFFER_LEN;

    memset (&sin, 0, sizeof(struct sockaddr_in));

    g_rx_sock_fd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (g_rx_sock_fd < 0)
    {
        FLOG_FATAL ("set socket failed : [%s]\n", strerror (errno));
        return 1;
    }

    if (setsockopt (g_rx_sock_fd, SOL_SOCKET, SO_RCVBUF,
            (const char *) &recv_buf_len, sizeof(int)) != 0)
    {
        FLOG_FATAL ("set socket failed : [%s]\n", strerror (errno));
        close (g_rx_sock_fd);
        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons (g_rru_nic_param.src_port);

    if (bind (g_rx_sock_fd, (struct sockaddr *) &sin, sizeof ( sin )) < 0)
    {
        FLOG_FATAL ("bind socket failed : [%s]\n", strerror (errno));
        return 1;
    }

    g_tx_sock_fd = g_rx_sock_fd;

    memset (&g_sin_tx, 0, sizeof(struct sockaddr_in));
    memset (&g_sin_rx, 0, sizeof(struct sockaddr_in));

    g_sin_tx.sin_family = AF_INET;
    g_sin_tx.sin_addr.s_addr = htonl (g_rru_nic_param.dst_ip);
    g_sin_tx.sin_port = htons (g_rru_nic_param.dst_port);

    /** for cmd packet */
    memset (cmd_pkt, 0, FIX_PAYLOAD_LEN);

    p_tmp_hdr = (struct rrh_data_hdr *) cmd_pkt;

    p_tmp_hdr->rru_id = g_rru_nic_param.rru_id;
    p_tmp_hdr->data_type = CMD_TYPE_FLAG;
    p_tmp_hdr->length = FIX_DATA_LEN;
    p_tmp_hdr->chan = CHANNEL_TYPE;
    p_tmp_hdr->frm_num = 0;
    p_tmp_hdr->seq_no = 0;
    p_tmp_hdr->dgain = 0;
    p_tmp_hdr->frg_flag = 0;

    dgain[0] = DGAIN0;
    dgain[1] = DGAIN1;

    return 0;
}

/** application interface */
int ethrru_init (void)
{
    if (rru_init_socket () != 0)
    {
        FLOG_FATAL ("initial RRU NIC error\n");
        return 1;
    }

    return 0;
}

int ethrru_close (void)
{

    close (g_tx_sock_fd);

    close (g_rx_sock_fd);

    return 0;
}

int ethrru_data_tx (void * p_tx_buf, int symbol_num, unsigned int cmd_time)
{
    int i = 0;
    int j = 0;
//    int k = 0;
    int ret = 0;
    int result = 0;
    int pkt_count = 0;

    struct rrh_data_hdr * p_tmp_hdr = NULL;

    struct timeval start_t, end_t;
//    unsigned int count_t = 0;
    unsigned int total = 0;

    char p_tmp_tx_buf[2048];
    g_tx_seq_no = 0;

    gettimeofday (&start_t, NULL);

    for (i = 0; i < symbol_num; i++)
    {
        for (j = 0; j < PACKET_PER_SYMBOL; j++)
        {
            memcpy(p_tmp_tx_buf, p_tx_buf, FIX_PAYLOAD_LEN);

            p_tmp_hdr = (struct rrh_data_hdr *) p_tmp_tx_buf;

            p_tmp_hdr->rru_id = g_rru_nic_param.rru_id;
            p_tmp_hdr->data_type = UL_TYPE_FLAG;
            p_tmp_hdr->length = FIX_DATA_LEN;
            p_tmp_hdr->chan = (j%2) + 1;
            p_tmp_hdr->frm_num = g_tx_frm_no;
            p_tmp_hdr->dgain = dgain[j%2];
            p_tmp_hdr->seq_no = g_tx_seq_no >> 1;

            if ( (pkt_count >> 1) == 0 )
            {
                p_tmp_hdr->frg_flag = 1;
            }else if ( (pkt_count >> 1) == (((symbol_num * PACKET_PER_SYMBOL) >> 1) - 1) )
            {
                p_tmp_hdr->frg_flag = 3;
            }else
            {
                p_tmp_hdr->frg_flag = 2;
            }

            result = sendto (g_tx_sock_fd, (void*) p_tmp_tx_buf, FIX_PAYLOAD_LEN,
                    0, (struct sockaddr *) &g_sin_tx,
                    sizeof(struct sockaddr_in));

            if (result < 0)
            {
                FLOG_WARNING ("tx pkt error\n");
                ret = 1;
            }

            p_tx_buf += FIX_DATA_LEN;
            g_tx_seq_no++;
            pkt_count ++;
        }

//        FLOG_DEBUG ("Tx one symbol, seqno %d, flag %d\n", g_tx_seq_no, p_tmp_hdr->frg_flag);

        /** check if transmit CMD */
        gettimeofday (&end_t, NULL);

        total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                - start_t.tv_usec );

        if (total > cmd_time)
        {
//            FLOG_INFO ("Tx CMD pkt\n");

            p_tmp_hdr = (struct rrh_data_hdr *) cmd_pkt;

            p_tmp_hdr->frm_num = g_tx_frm_no;

            result = sendto (g_tx_sock_fd, (void*) cmd_pkt, FIX_PAYLOAD_LEN, 0,
                    (struct sockaddr *) &g_sin_tx, sizeof(struct sockaddr_in));

            if (result < 0)
            {
                FLOG_WARNING ("tx CMD pkt error\n");
                ret = 1;
            }

        }
    }

    while (1)
    {
        /** check if transmit CMD */
        gettimeofday (&end_t, NULL);

        total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                - start_t.tv_usec );

        if (total > cmd_time)
        {
//            FLOG_INFO ("Tx CMD pkt\n");

            p_tmp_hdr = (struct rrh_data_hdr *) cmd_pkt;

            p_tmp_hdr->frm_num = g_tx_frm_no;

            result = sendto (g_tx_sock_fd, (void*) cmd_pkt, FIX_PAYLOAD_LEN, 0,
                    (struct sockaddr *) &g_sin_tx, sizeof(struct sockaddr_in));

            if (result < 0)
            {
                FLOG_WARNING ("tx CMD pkt error\n");
                ret = 1;
            }

            break;
        }else
        {
//            usleep(1000);
        }
    }

    g_tx_frm_no ++;

    return ret;
}

int ethrru_data_rx (char * rx_buf, int symbol_num)
{
    int len;
    int i;
    int j;

    char tmp_rx_buf[2048];
    struct rrh_data_hdr * p_tmp_hdr = NULL;
    char * p_rx_buf = rx_buf;
    unsigned int sin_len;

    for (i = 0; i < symbol_num; i++)
    {
        for (j = 0; j < PACKET_PER_SYMBOL; j++)
        {
            while (1)
            {
                len = recvfrom (g_rx_sock_fd, tmp_rx_buf, 2048, 0,
                        (struct sockaddr *)&g_sin_rx, &sin_len);

                if (len == FIX_PAYLOAD_LEN)
                {
                    p_tmp_hdr = (struct rrh_data_hdr *) tmp_rx_buf;

                    if (p_tmp_hdr->data_type == DL_TYPE_FLAG)
                    {
/*
                        printf("seq no %d\n", p_tmp_hdr->seq_no);
                        printf("length %d\n", p_tmp_hdr->length);
                        printf("chan %d\n", p_tmp_hdr->chan);
                        printf("frm_num %d\n", p_tmp_hdr->frm_num);
                        printf("frg_flag %d\n", p_tmp_hdr->frg_flag);
*/
                        memcpy (p_rx_buf, ( tmp_rx_buf + DATA_HDR_LEN ),
                                FIX_DATA_LEN);

                        p_rx_buf += FIX_DATA_LEN;
                        break;

                    }
                }
            }
        }

//        FLOG_DEBUG ("read one symbol\n");
    }

    return 0;

}
