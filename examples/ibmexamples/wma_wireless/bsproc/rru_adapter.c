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
#include <pthread.h>
#include <math.h>

#include "trans_proc.h"
#include "rru_proc.h"
#include "rru_adapter.h"
#include "sem_util.h"
#include "dump_util.h"

#include "flog.h"
#include "bs_cfg.h"

static int g_tx_sock_fd;
static int g_rx_sock_fd;

static struct sockaddr_in g_sin_tx;
static struct sockaddr_in g_sin_rx;

static unsigned int g_tx_seq_no = 0;
//static unsigned int g_tx_frm_no = 0;

pthread_mutex_t mutex_rru_frm_idx;
unsigned short g_rruframe_num = 0;


extern int errno;

void rru_dump_pkt( void *p_rx_buf )
{

	struct rru_data_hdr * p_tmp_hdr = NULL;
	p_tmp_hdr = (struct rru_data_hdr *) p_rx_buf;

	FLOG_WARNING("rru_id %d\n", p_tmp_hdr->rru_id);
	FLOG_WARNING("data_type %d\n", p_tmp_hdr->data_type);
	FLOG_WARNING("length %d\n", p_tmp_hdr->length);
	FLOG_WARNING("chan %d\n", p_tmp_hdr->chan);
	FLOG_WARNING("frm_num %d\n", p_tmp_hdr->frm_num);
	FLOG_WARNING("seq no %d\n", p_tmp_hdr->seq_no);
	FLOG_WARNING("frg_flag %d\n\n", p_tmp_hdr->frg_flag);
	FLOG_WARNING("dgain %d\n\n", p_tmp_hdr->dgain);
	FLOG_WARNING("freq %d\n\n", p_tmp_hdr->freq);
	FLOG_WARNING("carrier %d\n\n", p_tmp_hdr->carrier);
	FLOG_WARNING("sample_rate %d\n\n", p_tmp_hdr->sample_rate);

}

/*
 * PacketInit - Socket initialization.
 */
static int rru_init_socket (void)
{
    struct sockaddr_in sin;
    int recv_buf_len = RECV_BUFFER_LEN;

    memset (&sin, 0, sizeof(struct sockaddr_in));

    g_rx_sock_fd = socket (AF_INET, SOCK_DGRAM, 0);

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
    sin.sin_addr.s_addr = RRU_ADAPTER_HTONL(g_trans_control_param.src_ip);
    sin.sin_port = RRU_ADAPTER_HTONS(g_trans_control_param.src_port);

    if (bind (g_rx_sock_fd, (struct sockaddr *) &sin, sizeof ( sin )) < 0)
    {
        FLOG_FATAL ("bind socket failed : [%s]\n", strerror (errno));
        return 1;
    }

    g_tx_sock_fd = g_rx_sock_fd;

    memset (&g_sin_tx, 0, sizeof(struct sockaddr_in));
    memset (&g_sin_rx, 0, sizeof(struct sockaddr_in));

    g_sin_tx.sin_family = AF_INET;
    g_sin_tx.sin_addr.s_addr = RRU_ADAPTER_HTONL(g_trans_control_param.dst_ip);
    g_sin_tx.sin_port = RRU_ADAPTER_HTONS(g_trans_control_param.dst_port);

    return 0;
}

/** application interface */
int ethrru_init (void)
{
    if (pthread_mutex_init (&mutex_rru_frm_idx, NULL) != 0 )
    {
        FLOG_FATAL ("initial lock error\n");
        return RET_ERROR;
    }

    if (rru_init_socket () != 0)
    {
        FLOG_FATAL ("initial RRU NIC error\n");
        return 1;
    }

    return 0;
}

int ethrru_close (void)
{
    FLOG_INFO ("close data socket\n");

    close (g_tx_sock_fd);

    close (g_rx_sock_fd);

    return 0;
}

int ethrru_data_tx (char * p_tx_buf, int symbol_num,
        struct rru_data_param * param)
{

    int len;
    int i = 0;
    int ret = 0;
    int result = 0;
    unsigned int chan_num = g_rru_data_param.tx_ant_num;
    int pkt_len = FIX_DATA_LEN;
    int batch = 0;
    int sample_rate = 1;
    int carrier_num = 1;

    //DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID, -1, p_tx_buf + DATA_HDR_LEN , (len - DATA_HDR_LEN)/2);
        //FLOG_INFO ("tx -------------> symbol %d, flag %d\n", symbol_num, param->end_flag);

    ret = get_global_param ("RRU_ETH_PKT_LEN", &pkt_len);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_ETH_PKT_LEN error\n");
    }

    ret = get_global_param ("RRU_SAMPLE_RATE", &sample_rate);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_SAMPLE_RATE error\n");
    }

    ret = get_global_param ("RRU_CARRIER_NUM", &carrier_num);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_CARRIER_NUM error\n");
    }

    len = DATA_HDR_LEN + PACKET_PER_SYMBOL * symbol_num * pkt_len;;

    struct rru_data_hdr * p_tmp_hdr = NULL;
    int tail_pkt_size = ((len - DATA_HDR_LEN ) / chan_num / carrier_num ) % pkt_len;  
    int pkt_per_carr = ((len - DATA_HDR_LEN ) / chan_num / carrier_num ) / pkt_len + ((tail_pkt_size == 0)?0:1);
    int pkt_total = pkt_per_carr * chan_num * carrier_num ;

    //struct rru_data_hdr * p_tmp_hdr = NULL;
    //int pkt_total = symbol_num * PACKET_PER_SYMBOL;

    if (param->end_flag == HEAD_FLAG)
    {
        g_tx_seq_no = 0;
    }

    for (i = 0; i < pkt_total; i++)
    {
	    batch = g_tx_seq_no / chan_num / carrier_num;
        p_tmp_hdr = (struct rru_data_hdr *) p_tx_buf;

        p_tmp_hdr->rru_id = REVL(g_trans_control_param.rru_id);
        p_tmp_hdr->data_type = DL_TYPE_FLAG;
        p_tmp_hdr->length = REVS(pkt_len);
        p_tmp_hdr->chan = (i % chan_num) + 1;
        p_tmp_hdr->frm_num = REVS(param->frm_num);
        p_tmp_hdr->dgain = 0;
        p_tmp_hdr->seq_no = REVS(batch);
        
#ifdef _NEW_TRANS_GREN_ENABLE_
        p_tmp_hdr->freq = 0;
	    p_tmp_hdr->sample_rate = sample_rate;
	    p_tmp_hdr->carrier = (i / chan_num) % carrier_num;
#else
        p_tmp_hdr->resv = 0;
#endif


	    if ( batch == 0 && ( param->end_flag == HEAD_FLAG ) )
	    {
		    p_tmp_hdr->frg_flag = HEAD_FLAG;
#ifdef RRH_ENDIAN_SWITCH
		        int i;
                short *pIQ = (short *)( p_tx_buf + sizeof (struct rru_data_hdr)) ;
                for(i = 0; i < (pkt_len)/2; i++)
		        {
			            *pIQ = REVS( *pIQ );
                        pIQ++;
                }
#endif
		    result = sendto (g_tx_sock_fd, (void*) p_tx_buf, pkt_len + DATA_HDR_LEN, 0,
				    (struct sockaddr *) &g_sin_tx, sizeof(struct sockaddr_in));
		    p_tx_buf += pkt_len;
	    }
	    else
		    if (  batch == ( pkt_per_carr - 1 ) && ( param->end_flag == TAIL_FLAG ) )
		    {
			    p_tmp_hdr->frg_flag = TAIL_FLAG;
                            tail_pkt_size = (tail_pkt_size)?tail_pkt_size:pkt_len;
#ifdef RRH_ENDIAN_SWITCH
		        int i;
                short *pIQ = (short *)( p_tx_buf + sizeof (struct rru_data_hdr)) ;
                for(i = 0; i < (tail_pkt_size)/2; i++)
		        {
			            *pIQ = REVS( *pIQ );
                        pIQ++;
                }
#endif
			    result = sendto (g_tx_sock_fd, (void*) p_tx_buf, tail_pkt_size + DATA_HDR_LEN, 0,
					    (struct sockaddr *) &g_sin_tx, sizeof(struct sockaddr_in));
			    p_tx_buf += tail_pkt_size;
		    }
		    else
		    {
			    p_tmp_hdr->frg_flag = BODY_FLAG;
#ifdef RRH_ENDIAN_SWITCH
		        int i;
                short *pIQ = (short *)( p_tx_buf + sizeof (struct rru_data_hdr)) ;
                for(i = 0; i < (pkt_len)/2; i++)
		        {
			            *pIQ = REVS( *pIQ );
                        pIQ++;
                }
#endif
			    result = sendto (g_tx_sock_fd, (void*) p_tx_buf, pkt_len + DATA_HDR_LEN, 0,
					    (struct sockaddr *) &g_sin_tx, sizeof(struct sockaddr_in));
			    p_tx_buf += pkt_len;
		    }


        if (result < 0)
        {
            FLOG_WARNING ("tx pkt error\n");
            ret = 1;
        }

        g_tx_seq_no++;

        //FLOG_DEBUG ("Tx one symbol, seqno %d\n", g_tx_seq_no);
    }

    return ret;
}

int ethrru_data_rx_pkt(char * p_rx_buf, int pkt_len)
{
    int len = 0;
    unsigned int sin_len = sizeof (struct sockaddr);

    while (1)
    {

        len = recvfrom (g_rx_sock_fd, p_rx_buf, 2048, 0,
                (struct sockaddr *) &g_sin_rx, &sin_len);
        //FLOG_INFO ("recv %d\n", len);

/*
        len = recvfrom (g_rx_sock_fd, p_rx_buf, 2048, 0,
                NULL, NULL);
*/
/*
        len = recv (g_rx_sock_fd, p_rx_buf, 2048, 0);
*/

        if ( (len == pkt_len) || (len == CMD_PAYLOAD_LEN) )
        {


#ifdef RRH_ENDIAN_SWITCH
            struct rru_data_hdr * p_tmp_hdr = NULL;

            p_tmp_hdr = (struct rru_data_hdr *) p_rx_buf;
            p_tmp_hdr->rru_id = REVL(p_tmp_hdr->rru_id);
            p_tmp_hdr->length = REVS(p_tmp_hdr->length);
            p_tmp_hdr->frm_num = REVS(p_tmp_hdr->frm_num);
            p_tmp_hdr->seq_no = REVS(p_tmp_hdr->seq_no);
            
        if ( len == CMD_PAYLOAD_LEN )
        {
                struct rru_cmd_payload *p_cur_cmd_state = NULL;
                p_cur_cmd_state = (struct rru_cmd_payload *) ( p_tmp_hdr + sizeof (struct rru_data_hdr));

                p_cur_cmd_state->pkt_crc_count = REVS(p_cur_cmd_state->pkt_crc_count);
                p_cur_cmd_state->pkt_lost_count = REVS(p_cur_cmd_state->pkt_lost_count);
                p_cur_cmd_state->pkt_late_count = REVS(p_cur_cmd_state->pkt_late_count);
                p_cur_cmd_state->pkt_start_time = REVL(p_cur_cmd_state->pkt_start_time);
                p_cur_cmd_state->pkt_end_time = REVL(p_cur_cmd_state->pkt_end_time);
        }
        else
        {
		int i;
                short *pIQ = (short *)( p_rx_buf + sizeof (struct rru_data_hdr)) ;
                for(i = 0; i < (FIX_PAYLOAD_LEN - sizeof (struct rru_data_hdr))/2; i++)
		{
			*pIQ = REVS( *pIQ );
                        pIQ++;
                }

        }
#endif
/*
            printf("seq no %d\n", p_tmp_hdr->seq_no);
            printf("length %d\n", p_tmp_hdr->length);
            printf("chan %d\n", p_tmp_hdr->chan);
            printf("frm_num %d\n", p_tmp_hdr->frm_num);
            printf("frg_flag %d\n\n", p_tmp_hdr->frg_flag);
*/
            return 0;
        }

        else if ( len < pkt_len  )
	{
		struct rru_data_hdr * p_tmp_hdr = NULL;

		p_tmp_hdr = (struct rru_data_hdr *) p_rx_buf;
		if(p_tmp_hdr->frg_flag == TAIL_FLAG) 
		{
			return 0;
		}
		else
		{
			FLOG_WARNING ("small not tail packet receive %d \n", len);
                        rru_dump_pkt( p_rx_buf ); 
		}

	}
	else
        {
            FLOG_WARNING ("big packet receive %d \n", len);
            rru_dump_pkt( p_rx_buf ); 
        }
    }

    return 1;

}
#if 0
int ethrru_data_rx (char * p_rx_buf, int symbol_num,
        struct rru_data_param * param, int flag, int tdd)
{
    int len;
    int j;

    char * p_rx_buf_hdr = p_rx_buf;

    char tmp_rx_buf[2048];
    struct rru_data_hdr * p_tmp_hdr = NULL;

    int pkt_total = PACKET_PER_SYMBOL * symbol_num;
    unsigned int sin_len = sizeof (struct sockaddr);
    unsigned int pkt_count = 0;

    struct rru_data_param * rru_param = (struct rru_data_param *) param;

    rru_param->end_flag = BODY_FLAG;

    if (flag == 1)
    {
        while (1)
        {
            len = recvfrom (g_rx_sock_fd, tmp_rx_buf, 2048, 0,
                    (struct sockaddr *) &g_sin_rx, &sin_len);

            if (len == FIX_PAYLOAD_LEN)
            {
                p_tmp_hdr = (struct rru_data_hdr *) tmp_rx_buf;

                if ( ( p_tmp_hdr->data_type == UL_TYPE_FLAG )
                        && ( p_tmp_hdr->frg_flag == HEAD_FLAG ))
                {
                    FLOG_INFO("Get frame sync from Ethernet link \n");

                    if (tdd == 1)
                    {
                        if ( post_sem (tx_s_handle) )
                        {
                            FLOG_ERROR("Tx TDD SEM error\n");
                        }
                    }
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

                    rru_param->dgain[0] = (float) p_tmp_hdr->dgain;
                    rru_param->frm_num = p_tmp_hdr->frm_num;

                    pkt_total--;
                    pkt_count++;
                    break;
                }
            }
        }

        rru_param->end_flag = HEAD_FLAG;
    }

    for (j = 0; j < pkt_total; j++)
    {
        while (1)
        {
            len = recvfrom (g_rx_sock_fd, tmp_rx_buf, 2048, 0,
                    (struct sockaddr *) &g_sin_rx, &sin_len);

            if (len == FIX_PAYLOAD_LEN)
            {
                p_tmp_hdr = (struct rru_data_hdr *) tmp_rx_buf;

                if (p_tmp_hdr->data_type == UL_TYPE_FLAG)
                {
                    pkt_count++;
/*
                        printf("seq no %d\n", p_tmp_hdr->seq_no);
                        printf("length %d\n", p_tmp_hdr->length);
                        printf("chan %d\n", p_tmp_hdr->chan);
                        printf("frm_num %d\n", p_tmp_hdr->frm_num);
                        printf("frg_flag %d\n", p_tmp_hdr->frg_flag);
                        printf("degain %d\n", p_tmp_hdr->dgain);
*/
                    memcpy (p_rx_buf, ( tmp_rx_buf + DATA_HDR_LEN ),
                            FIX_DATA_LEN);

                    p_rx_buf += FIX_DATA_LEN;

                    rru_param->dgain[j % g_rru_data_param.rx_ant_num] = (float) p_tmp_hdr->dgain;
                    rru_param->frm_num = p_tmp_hdr->frm_num;

                    if (p_tmp_hdr->frg_flag == TAIL_FLAG)
                    {
                        rru_param->end_flag = p_tmp_hdr->frg_flag;
                        rru_param->pkt_count = pkt_count;

                        return 0;
                    }

                    if ( p_tmp_hdr->frg_flag == HEAD_FLAG )
                    {
                        rru_param->end_flag = HEAD_FLAG;

//                        pthread_mutex_lock(&mutex_rru_frm_idx);
                        g_rruframe_num = p_tmp_hdr->frm_num;
//                        pthread_mutex_unlock(&mutex_rru_frm_idx);

                        if (tdd == 1)
                        {
                            if ( post_sem (tx_s_handle) )
                            {
                                FLOG_ERROR("post TDD SEM error\n");
                            }
                        }

                        if (pkt_count != 1)
                        {
                            p_rx_buf = p_rx_buf_hdr;

                            memcpy (p_rx_buf, ( tmp_rx_buf + DATA_HDR_LEN ),
                                    FIX_DATA_LEN);

                            p_rx_buf += FIX_DATA_LEN;

                            rru_param->end_flag = HEAD_FLAG;

                            rru_param->dgain[0] = (float) p_tmp_hdr->dgain;
                            rru_param->frm_num = p_tmp_hdr->frm_num;

                            pkt_count = 1;
                            j = 0;
                        }
                    }

                    break;
                }

                if (p_tmp_hdr->data_type == CMD_TYPE_FLAG)
                {
//                    FLOG_DEBUG ("read one CMD PKT\n");
                    if (tdd == 1)
                    {
                        if ( post_sem (cmd_s_handle) )
                        {
                            FLOG_ERROR("post CMD SEM error\n");
                        }
                    }
                }
            }
        }

        //        FLOG_DEBUG ("read one symbol\n");
    }

    return 0;

}
#endif

#ifdef ENABLE_NEW_TRANS_GREN
#define MAX_BUFFERED 3
static char tmp_rx_buf[2][MAX_BUFFERED][2048];
static int curr_rhead[2];
static int curr_whead[2];
static int curr_len[2];
static short curr_cnt[2];

void ethrru_symbol_rx_reset ()
{
	curr_rhead[0]=0;
	curr_rhead[1]=0;
	curr_whead[0]=0;
	curr_whead[1]=0;
	curr_len[0]=0;
	curr_len[1]=0;
	curr_cnt[0]=0;
	curr_cnt[1]=0;
}

int ethrru_symbol_rx (char * rx_buf, int symbol_num, struct rru_data_param * param)
{
	int len;
	int i;
	int j;
	static int ant = 0;
	static int rx_ant = 0;

	struct rru_data_hdr * p_tmp_hdr = NULL;
	char * p_rx_buf = rx_buf;
	unsigned int sin_len = sizeof (struct sockaddr);

	for (i = 0; i < symbol_num; i++)
	{
		for (j = 0; j < PACKET_PER_SYMBOL; j++)
		{

			while (1)
			{
				if( curr_len[ant] >= FIX_DATA_LEN )
				{
					int cpylen = FIX_DATA_LEN;
					while( cpylen )
					{
						p_tmp_hdr = (struct rru_data_hdr *) tmp_rx_buf[ant][curr_rhead[ant]];
                                                int bytes_left = p_tmp_hdr->length - p_tmp_hdr->seq_no;
						if( cpylen <  bytes_left )
						{
							memcpy (p_rx_buf, tmp_rx_buf[ant][curr_rhead[ant]] + p_tmp_hdr->seq_no + DATA_HDR_LEN, cpylen);
					                p_rx_buf += cpylen; ;
							p_tmp_hdr->seq_no += cpylen;
							cpylen = 0;
						}
						else if ( cpylen >  bytes_left )
						{
							memcpy (p_rx_buf, tmp_rx_buf[ant][curr_rhead[ant]] + p_tmp_hdr->seq_no + DATA_HDR_LEN,
									( cpylen > bytes_left )? bytes_left : cpylen );
					                p_rx_buf += bytes_left; ;

							curr_rhead[ant]++;
							curr_rhead[ant] %= MAX_BUFFERED;
							cpylen -= bytes_left;

						}
						else 
						{
							memcpy (p_rx_buf, tmp_rx_buf[ant][curr_rhead[ant]] + p_tmp_hdr->seq_no + DATA_HDR_LEN,
									( cpylen > bytes_left )? bytes_left : cpylen );
					                p_rx_buf += bytes_left; ;

							curr_rhead[ant]++;
							curr_rhead[ant] %= MAX_BUFFERED;
							cpylen -= bytes_left;

						}
					}
                                        curr_len[ant] -= FIX_DATA_LEN;

					ant = !ant;
					break;
				}
				else
				{

					len = recvfrom (g_rx_sock_fd, tmp_rx_buf[rx_ant][curr_whead[rx_ant]], 2048, 0,
							(struct sockaddr *)&g_sin_rx, &sin_len);

					if(len == FIX_PAYLOAD_LEN || len == (1024 + DATA_HDR_LEN))
					{
						p_tmp_hdr = (struct rru_data_hdr *) tmp_rx_buf[rx_ant][curr_whead[rx_ant]];

#if 0                                               
                                                short *stmp = (short *)p_tmp_hdr;  
                                                stmp += DATA_HDR_LEN/2;
                                                int kk; 
                                                for( kk = 0; kk < (len - DATA_HDR_LEN)/2; kk++ )
                                                {
                                                                    
							stmp[kk] = curr_cnt[rx_ant]++;
							curr_cnt[rx_ant] %= 544;
                                                }
#endif

						if (p_tmp_hdr->data_type == UL_TYPE_FLAG)
						{
							param->dgain[p_tmp_hdr->chan] = p_tmp_hdr->dgain;
                                                        p_tmp_hdr->seq_no = 0; //borraw it to record the byte has been read out
                                                        
                                                        
							/*
							   printf("seq no %d\n", p_tmp_hdr->seq_no);
							   printf("length %d\n", p_tmp_hdr->length);
							   printf("chan %d\n", p_tmp_hdr->chan);
							   printf("frm_num %d\n", p_tmp_hdr->frm_num);
							   printf("frg_flag %d\n", p_tmp_hdr->frg_flag);
							 */
                                                        curr_len[rx_ant] += (len - DATA_HDR_LEN);
							curr_whead[rx_ant]++;
							curr_whead[rx_ant] %= MAX_BUFFERED;
							rx_ant = !rx_ant;


						}
					}
				}
			}
		}
	}

	return 0;
}

#else
int ethrru_symbol_rx (char * rx_buf, int symbol_num, struct rru_data_param * param)
{
    int len;
    int i;
    int j;

    char tmp_rx_buf[2048];
    struct rru_data_hdr * p_tmp_hdr = NULL;
    char * p_rx_buf = rx_buf;
    unsigned int sin_len = sizeof (struct sockaddr);

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
                    p_tmp_hdr = (struct rru_data_hdr *) tmp_rx_buf;

                    if (p_tmp_hdr->data_type == UL_TYPE_FLAG)
                    {
                        param->dgain[p_tmp_hdr->chan] = p_tmp_hdr->dgain;
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
    }

    return 0;
}
#endif
