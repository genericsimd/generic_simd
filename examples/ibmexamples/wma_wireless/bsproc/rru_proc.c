/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: rru_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "global.h"
#include "queue_util.h"
#include "flog.h"
#include "sem_util.h"
#include "bs_cfg.h"

#include "rru_proc.h"
#include "rru_adapter.h"
#include "trans.h"

#include "mac_proc.h"

#include "phy_dl_tx_interface.h"
#include "phy_proc.h"

#include "dump_util.h"

/* Agent code */
#include "trans.h"
#include "trans_agent.h"
#include "trans_rrh.h"
#include "trans_debug.h"

//#define _DUMP_RRU_RX_
void rru_dump_pkt( void *p_rx_buf );


struct rru_data_config g_rru_data_param;

struct sem_handle * cmd_s_handle;

struct sem_handle * tx_s_handle;

pthread_mutex_t g_resync_lock;
int g_resync_count = 0;

pthread_t ethrru_thread_tx = 0;
pthread_t ethrru_thread_rx = 0;

void *process_ethrru_tx (void *arg __attribute__ ((unused)));
void *process_ethrru_rx (void *arg __attribute__ ((unused)));

static int get_rru_data_config (struct rru_data_config * rru_config);
static void frm_dump_rrh_carrier_reset( int sample_per_frm, int frm_num );
static void frm_dump_rrh_carrier(int ant_num, int carr_num, int sample_num, short *rx_buf_short, int max);

static short * p_tx_buf = NULL;

unsigned short g_cmd_frame_num = 0;
unsigned short g_init_rruframe_num = 0;

int handle_repeat_err( int id )
{
	static int curr_err = ERR_NOERROR;
        static int count = 0;

        if( curr_err == id )
	{

               	count++;
//                if( count % 1 == 1 )
                {
		        return 1;
                }
 //               else
//		        return 0;
	}
	else
	{
                if( count > 1 )
                {
        		FLOG_WARNING ("Last error repeat %d times.", count);
                }
                
                curr_err = id;
                count = 0;
		return 1;
	}
      
}

int rrh_tdd_tx_process (void)
{
    pthread_attr_t tattr;
    int ret;

    if (get_rru_data_config (&g_rru_data_param) != 0)
    {
        FLOG_ERROR ("Configure TRANS parameters failed");
        return 1;
    }

    if ( create_sem (&cmd_s_handle) != 0 )
    {
        FLOG_ERROR ("Enable CMD sem error");
    }

    if ( create_sem (&tx_s_handle) != 0 )
    {
        FLOG_ERROR ("Enable TX sem error");
    }


    int carrier_num;
    ret = get_global_param ("RRU_CARRIER_NUM", &carrier_num);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_CARRIER_NUM error\n");
    }

//    int frm_num;
//    ret = get_global_param ("FAKE_TX_TRACE_FRM_NUM", &frm_num);
//    if (ret != 0)
//    {
//        FLOG_WARNING ("get parameters FAKE_TX_TRACE_FRM_NUM error\n");
//    }

    int frm_len;
    ret = get_global_param ("TX_FRAME_LEN", &frm_len);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TX_FRAME_LEN error\n");
    }

    int sample_rate;
    ret = get_global_param ("RRU_SAMPLE_RATE", &sample_rate);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_SAMPLE_RATE error\n");
    }
    sample_rate = 1 << (sample_rate ); 

    int eth_pkt_size;
    ret = get_global_param ("RRU_ETH_PKT_LEN", &eth_pkt_size);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_ETH_PKT_LEN error\n");
    }


    int sample_per_carrier = frm_len * sample_rate * 4 ;  //frame tx time lengh unit * sample_rate *  4chip
    int frm_point_len = sample_per_carrier * carrier_num;  //frame tx time lengh unit * sample_rate *  4chip
    int frm_data_size = frm_point_len * 4 * g_rru_data_param.tx_ant_num;  //in bytes, i/q total 4 bytes per point
//    int sample_per_pkt = eth_pkt_size / 4;
//    int sample_last_pkt = sample_per_carrier % sample_per_pkt;

    p_tx_buf = my_malloc ( frm_data_size + DATA_HDR_LEN);
//    p_tx_buf = my_malloc (FIX_PAYLOAD_LEN * 2
//            * g_rru_data_param.tx_symbol_num * sizeof(short));

    if (p_tx_buf == NULL)
    {
        FLOG_ERROR ("Malloc Tx buffer error\n");
        return 1;
    }

    p_tx_buf[0] = 0;
    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&ethrru_thread_tx, &tattr, process_ethrru_tx, NULL);
    pthread_attr_destroy (&tattr);

    return 0;
}

int rrh_tdd_tx_release (void)
{
    if (ethrru_thread_tx != 0)
    {
        pthread_cancel (ethrru_thread_tx);
        pthread_join (ethrru_thread_tx, NULL);
    }

  
    if (p_tx_buf != NULL)
    {
        free(p_tx_buf);
        p_tx_buf = NULL;
    }


    if ( delete_sem(cmd_s_handle) != 0)
    {
        FLOG_ERROR ("Disable CMD sem error");
    }

    if ( delete_sem(tx_s_handle) != 0)
    {
        FLOG_ERROR ("Disable TX sem error");
    }

    FLOG_INFO ("RRH Tx thread Release");

    return 0;
}

int rrh_tdd_rx_process (void)
{
    pthread_mutex_init(&g_resync_lock, NULL);
    g_resync_count = 0;

    pthread_attr_t tattr;
    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&ethrru_thread_rx, NULL, process_ethrru_rx, NULL);
    pthread_attr_destroy (&tattr);

    return 0;

}

int rrh_tdd_rx_release (void)
{
    if (ethrru_thread_rx != 0)
    {
        pthread_cancel (ethrru_thread_rx);
        pthread_join (ethrru_thread_rx, NULL);
    }

    FLOG_INFO ("RRH Rx thread Release");

    return 0;
}

void *process_ethrru_tx (void *arg __attribute__ ((unused)))
{
    FLOG_INFO ("ETHRRU TX thread Started");

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
    }

    struct queue_msg * p_msg = my_malloc (sizeof(struct queue_msg));

    int i_buf_tmp = 0;
    int j_buf_tmp = 0;
    int i = 0;
    int j = 0;
    int k = 0;

    int idx_symbol = 0;
    int idx_carrier = 0;
    int idx_pkt = 0;
    int idx_sample = 0;

    unsigned int tx_symbol_num = 0;

    struct rru_data_param param_hdr;

    struct phy_dl_rru_symbol * p_tmp_phy_symbol = NULL;

    struct timeval start_t, end_t;
    unsigned int count_t = 0;
    unsigned int total = 0;

    struct phy_frame_info tmp_tx_info;

    int skip_tx_flag = 0;

//    int ulmap_widx = UL_MAP_DELAY;
    int ulmap_widx = 0;


    int carrier_num;
    int ret;
    ret = get_global_param ("RRU_CARRIER_NUM", &carrier_num);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_CARRIER_NUM error\n");
    }

//    int frm_num;
//    ret = get_global_param ("FAKE_TX_TRACE_FRM_NUM", &frm_num);
//    if (ret != 0)
//    {
//        FLOG_WARNING ("get parameters FAKE_TX_TRACE_FRM_NUM error\n");
//    }

    int frm_len;
    ret = get_global_param ("TX_FRAME_LEN", &frm_len);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TX_FRAME_LEN error\n");
    }

    int sample_rate;
    ret = get_global_param ("RRU_SAMPLE_RATE", &sample_rate);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_SAMPLE_RATE error\n");
    }
    sample_rate = 1 << (sample_rate ); 

    int eth_pkt_size;
    ret = get_global_param ("RRU_ETH_PKT_LEN", &eth_pkt_size);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_ETH_PKT_LEN error\n");
    }


    int sample_per_carrier = frm_len * sample_rate * 4 ;  //frame tx time lengh unit * sample_rate *  4chip
    int frm_point_len = sample_per_carrier * carrier_num;  //frame tx time lengh unit * sample_rate *  4chip
    int frm_data_size = frm_point_len * 4 * g_rru_data_param.tx_ant_num;  //in bytes, i/q total 4 bytes per point
    int sample_per_pkt = eth_pkt_size / 4;
    int sample_last_pkt = sample_per_carrier % sample_per_pkt;

#ifdef _DL_TX_FAKE_

    short * p_fake_slotsymbol_i[ANTENNA_NUM][CARRIER_NUM];
    short * p_fake_slotsymbol_q[ANTENNA_NUM][CARRIER_NUM];

    int fake_slotsymbol_i_curr[ANTENNA_NUM][CARRIER_NUM];
    int fake_slotsymbol_q_curr[ANTENNA_NUM][CARRIER_NUM];

    int fake_slotsymbol_i_max[ANTENNA_NUM][CARRIER_NUM];
    int fake_slotsymbol_q_max[ANTENNA_NUM][CARRIER_NUM];

    char file_name[128];
    char file_name1[128];
    FILE *fp_t = NULL;
    FILE *fp_null = NULL;

    memset(p_fake_slotsymbol_i, 0, sizeof(p_fake_slotsymbol_i));
    memset(p_fake_slotsymbol_q, 0, sizeof(p_fake_slotsymbol_q));

    fp_null = fopen("/dev/null", "w");
    if( fp_null == NULL )
    {
    	FLOG_FATAL("open NULL output file failed!\n");
	goto xit;
    }

    struct stat fst;
    for(i = 0; i < g_rru_data_param.tx_ant_num; i++)
    {
	    for(j = 0; j < carrier_num; j++)
	    {
		    ret = get_global_param ("FAKE_TX_TRACE_I0", file_name);
		    if (ret != 0)
		    {
			    FLOG_WARNING ("get parameters FAKE_TX_TRACE_I0 error\n");
		    }

//		    snprintf(file_name1, sizeof(file_name1), "%s_ant%d_carr%d.dat", file_name, i, j);
		    snprintf(file_name1, sizeof(file_name1), "%s", file_name);
		    fp_t = fopen(file_name1, "r");
		    if( fp_t == NULL )
		    {
			    FLOG_FATAL("open FAKE DL file %s failed!\n", file_name1);
			    goto xit;
		    }
                    stat( file_name1, &fst );

		    p_fake_slotsymbol_i[i][j] = (short *)malloc( fst.st_size );

//		    p_fake_slotsymbol_i[i][j] = (short *)malloc(frm_num * sample_per_carrier * sizeof(short));
		    if( p_fake_slotsymbol_i[i][j]  == NULL )
		    {
			    FLOG_FATAL("memory allocation error\n");
			    goto xit;
		    }
		    ret = fseek( fp_t, 0, SEEK_SET);
		    if (ret < 0  )
		    {
			    FLOG_WARNING ("Seek to head of file %s error %d\n", file_name1, ret);
			    goto xit;
		    }

		    ret = fread(  p_fake_slotsymbol_i[i][j], fst.st_size, 1, fp_t);
#ifdef _INCR_SEQ_ 
                    int kk;
                    for(kk = 0; kk < (fst.st_size/2) ; kk++)
                    {
                    	 p_fake_slotsymbol_i[i][j][kk] = kk;
                    }
#endif

		    if (ret < 0 || ret < 1 )
		    {
			    FLOG_WARNING ("Read data from file %s error %d\n", file_name1, ret);
			    goto xit;
		    }
                    fake_slotsymbol_i_curr[i][j] = 0;
                    fake_slotsymbol_i_max[i][j] = fst.st_size/2;
                    fclose(fp_t); 

		    ret = get_global_param ("FAKE_TX_TRACE_Q0", file_name);
		    if (ret != 0)
		    {
			    FLOG_WARNING ("get parameters FAKE_TX_TRACE_Q0 error\n");
		    }

//		    snprintf(file_name1, sizeof(file_name1), "%s_ant%d_carr%d.dat", file_name, i, j);
		    snprintf(file_name1, sizeof(file_name1), "%s", file_name);
		    fp_t = fopen(file_name1, "r");
		    if( fp_t == NULL )
		    {
			    FLOG_FATAL("open FAKE DL file %s failed!\n", file_name1);
			    goto xit;
		    }
                    stat( file_name1, &fst );

		    p_fake_slotsymbol_q[i][j] = (short *)malloc( fst.st_size );
		    if( p_fake_slotsymbol_q[i][j]  == NULL )
		    {
			    FLOG_FATAL("memory allocation error\n");
			    goto xit;
		    }

		    ret = fseek( fp_t, 0, SEEK_SET);
		    if (ret < 0  )
		    {
			    FLOG_WARNING ("Seek to head of file %s error %d\n", file_name1, ret);
			    goto xit;
		    }
		    ret = fread(  p_fake_slotsymbol_q[i][j], fst.st_size, 1, fp_t);
		    if (ret < 0 || ret < 1 )
		    {
			    FLOG_WARNING ("Read data from file %s error %d\n", file_name1, ret);
			    goto xit;
		    }

#ifdef _INCR_SEQ_ 
                    for(kk = 0; kk < (fst.st_size/2) ; kk++)
                    {
                    	 p_fake_slotsymbol_q[i][j][kk] = kk;
                    }
#endif
                    
                    fake_slotsymbol_q_curr[i][j] = 0;
                    fake_slotsymbol_q_max[i][j] = fst.st_size/2;
                    fclose(fp_t); 
	    }
    }

    //DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID, -1, p_fake_slotsymbol_i[0][0],fst.st_size/2);
    //DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_Q_ID, -1, p_fake_slotsymbol_q[0][0],fst.st_size/2);
#endif

    memset(&tmp_tx_info, 0, sizeof (struct phy_frame_info));
 
    while (1)
    {
/*
        if (frame_flag == 0)
        {
            if (wait_sem (cmd_s_handle) != 0 )
            {
                FLOG_ERROR ("get SEM error\n");
            }

            rruframe_idx = g_rruframe_num;
            if ( ( rruframe_idx != (unsigned short)(pre_rruframe_idx + 1) ) &&
                 ( rruframe_idx != pre_rruframe_idx) )
            {
                skip_tx_flag = 1;
                FLOG_INFO("late FRMAE!, CMD frame num %d, pre Tx frame num %d\n, ",
                            rruframe_idx, pre_rruframe_idx);
            }else
            {
                skip_tx_flag = 0;
            }
            pre_rruframe_idx = rruframe_idx;
        }
*/
        /*
         if (tx_phy_en_flag >= g_rru_data_param.tx_blk_per_frm)
         {

         for (idx_symbol = 0;
         idx_symbol < g_rru_data_param.tx_blk_per_frm;
         idx_symbol++)
         {
         */

#ifdef _DL_TX_FAKE_
	    //FLOG_FATAL("start data tx loop!\n");
	    int i_buf_tmp = (DATA_HDR_LEN >> 1);


	    // FLOG_ERROR ("idx_pkt  = %d, %d, %d, %d \n", idx_pkt, idx_carrier, idx_sample, i_buf_tmp);
	    for (idx_pkt= 0; idx_pkt < sample_per_carrier - sample_last_pkt; idx_pkt += sample_per_pkt)
	    {
		    for (idx_carrier= 0; idx_carrier < carrier_num; idx_carrier++)
		    {
			    for (i = 0; i < g_rru_data_param.tx_ant_num; i++)
			    {
				    for (idx_sample = 0; idx_sample < sample_per_pkt; idx_sample++)
				    {
					    p_tx_buf[i_buf_tmp + ( idx_sample << 1 )]
						    = p_fake_slotsymbol_i[i][idx_carrier][fake_slotsymbol_i_curr[i][idx_carrier]++];

					    fake_slotsymbol_i_curr[i][idx_carrier] %= fake_slotsymbol_i_max[i][idx_carrier];

					    p_tx_buf[i_buf_tmp + ( idx_sample << 1 ) + 1]
						    = p_fake_slotsymbol_q[i][idx_carrier][fake_slotsymbol_q_curr[i][idx_carrier]++];

					    fake_slotsymbol_q_curr[i][idx_carrier] %= fake_slotsymbol_q_max[i][idx_carrier];

				    }
#if 0
		                     FLOG_WARNING ("tx buf = %d, source_buf = %d source_char %d\n", 
                                                    i_buf_tmp + ( idx_sample << 1 ),
                                                    fake_slotsymbol_i_curr[i][idx_carrier],
                                                    p_fake_slotsymbol_i[i][idx_carrier][fake_slotsymbol_i_curr[i][idx_carrier]]
                                                  );
#endif              
				    i_buf_tmp += sample_per_pkt << 1;

#if 0
				    static int count = 0; 
				    if((count ++ % 1000) == 0 )
				    {
					    fprintf ( fp_null, "idx_pkt  = %d, %d, %d, %d \n", idx_pkt, idx_carrier, idx_sample, i_buf_tmp);
				    }
#endif              
			    }

		    }

	    }
//	    FLOG_FATAL("after data tx loop! sample_last_pkt = %d\n", sample_last_pkt);
	    for (idx_carrier= 0; idx_carrier < carrier_num; idx_carrier++)
	    {
		    for (i = 0; i < g_rru_data_param.tx_ant_num; i++)
		    {
			    for (idx_sample = 0; idx_sample < sample_last_pkt; idx_sample++)
			    {
				    p_tx_buf[i_buf_tmp + ( idx_sample << 1 )]
					    = p_fake_slotsymbol_i[i][idx_carrier][fake_slotsymbol_q_curr[i][idx_carrier]++];

				    fake_slotsymbol_q_curr[i][idx_carrier] %= fake_slotsymbol_q_max[i][idx_carrier];

				    p_tx_buf[i_buf_tmp + ( idx_sample << 1 ) + 1]
					    = p_fake_slotsymbol_q[i][idx_carrier][fake_slotsymbol_i_curr[i][idx_carrier]++];

				    fake_slotsymbol_i_curr[i][idx_carrier] %= fake_slotsymbol_i_max[i][idx_carrier];

			    }
			    i_buf_tmp += sample_last_pkt << 1;
		    }
	    }
#else


        for (i = 0; i < g_rru_data_param.tx_ant_num; i++)
        {
            /* we keep this for zero-copy in Tx, this is idx for short  */

            i_buf_tmp = ( SAMPLE_PER_PACKET << 1 ) * i + (DATA_HDR_LEN >> 1);
            j_buf_tmp = 0;

            p_msg->my_type = frm_de_id[i];

            if (wmrt_dequeue (frm_de_id[i], p_msg, sizeof(struct queue_msg))
                    == -1)
            {
                FLOG_ERROR ("ENQUEUE ERROR\n");
            }

            p_tmp_phy_symbol = (struct phy_dl_rru_symbol *) p_msg->p_buf;

            for (idx_symbol = 0; idx_symbol
                    < (int) p_tmp_phy_symbol->symbol_num; idx_symbol++)
            {
                for (idx_pkt = 0; idx_pkt < PACKET_PER_ANT; idx_pkt++)
                {
                    for (idx_sample = 0; idx_sample < SAMPLE_PER_PACKET; idx_sample++)
                    {
                        p_tx_buf[i_buf_tmp + ( idx_sample << 1 )]
                                = (short) ( p_tmp_phy_symbol->symbol_i[j_buf_tmp + idx_sample] * g_rru_data_param.gain_tx);

                        p_tx_buf[i_buf_tmp + ( idx_sample << 1 ) + 1]
                                = (short) ( p_tmp_phy_symbol->symbol_q[j_buf_tmp + idx_sample] * g_rru_data_param.gain_tx);
                    }

//                    printf( "i_buf_tmp %d\n", i_buf_tmp);

                    i_buf_tmp += (SAMPLE_PER_PACKET << 1) * g_rru_data_param.tx_ant_num;
                    j_buf_tmp += SAMPLE_PER_PACKET;
                }
            }

            tx_symbol_num
                    = ( tx_symbol_num > p_tmp_phy_symbol->symbol_num ) ? tx_symbol_num
                            : p_tmp_phy_symbol->symbol_num;

//            printf("tx_symbol_num %d index %d\n", tx_symbol_num, i);

            tmp_tx_info.dl_perscan_flag[0] = p_tmp_phy_symbol->dl_perscan_flag;
            tmp_tx_info.ul_map[0] = p_tmp_phy_symbol->p_ulmap;
            tmp_tx_info.ul_map_len[0] = p_tmp_phy_symbol->ul_map_len;
//            tmp_tx_info.dts_info[0] = p_tmp_phy_symbol->p_dts_info;
            tmp_tx_info.frame_number[0] = p_tmp_phy_symbol->frame_index;

            /** need to check if different symbol_offset has the same frame_index */

//            param_hdr.frm_num = p_tmp_phy_symbol->frame_index;
//            param_hdr.frm_num = rruframe_idx;

            if (p_tmp_phy_symbol->symbol_offset == 0)
            {
                param_hdr.end_flag = HEAD_FLAG;
            }else if ( (int)(p_tmp_phy_symbol->symbol_offset)
                    == (g_rru_data_param.tx_symbol_num
                          - g_rru_data_param.tx_symbol_per_slot))
            {
                param_hdr.end_flag = TAIL_FLAG;
            }else
            {
                param_hdr.end_flag = BODY_FLAG;
            }

            phy_dl_deinit_rrusymbol (p_tmp_phy_symbol);
        }

#endif

        if (param_hdr.end_flag == HEAD_FLAG)
        {
            if (wait_sem (cmd_s_handle) != 0 )
            {
                FLOG_ERROR ("get SEM error\n");
            }

            if ( (unsigned short)(tmp_tx_info.frame_number[0] + g_init_rruframe_num) < g_cmd_frame_num )
            {
                skip_tx_flag = 1;

                FLOG_WARNING("late FRMAE! CMD frame num %d, pre Tx frame num %d, %d\n",
                        g_cmd_frame_num, (unsigned short)(tmp_tx_info.frame_number[0] + g_init_rruframe_num),
                        g_init_rruframe_num );
/*
                if ( post_sem (tx_s_handle) )
                {
                    FLOG_ERROR("post CMD SEM error\n");
                }
*/
            }
            else if ((unsigned short)(tmp_tx_info.frame_number[0] + g_init_rruframe_num) > g_cmd_frame_num )
            {
                skip_tx_flag = 1;

                FLOG_WARNING("late FRMAE! CMD frame num %d, pre Tx frame num %d\n",
                        g_cmd_frame_num, (unsigned short)(tmp_tx_info.frame_number[0] + g_init_rruframe_num),
                        g_init_rruframe_num );
/*
                if ( post_sem (cmd_s_handle) )
                {
                    FLOG_ERROR("post CMD SEM error\n");
                }
*/
            }
            else
            {
                param_hdr.frm_num = g_cmd_frame_num;
                skip_tx_flag = 0;
            }
        }

        if (skip_tx_flag == 0)
        {
            if (ethrru_data_tx ((char *) p_tx_buf, tx_symbol_num, &param_hdr) != 0)
            {
                FLOG_WARNING ("Tx packet to RRH failed.\n");
            }
        }

        i_buf_tmp = 0;
        tx_symbol_num = 0;

        if (param_hdr.end_flag == HEAD_FLAG)
        {
            ulmap_widx = (tmp_tx_info.frame_number[0] + g_init_rruframe_num + UL_MAP_DELAY)%UL_MAP_STORE;

            pthread_mutex_lock (&mutex_phy_frame_info);
/*
            if (gs_phy_frame_info.info_flag != 2)
            {
                FLOG_ERROR("Tx FRAME info read error! [unset]\n");
            }
*/
            if (gs_phy_frame_info.ul_map[ulmap_widx] != NULL)
            {
                free(gs_phy_frame_info.ul_map[ulmap_widx]);

//                FLOG_WARNING("Tx FRAME info read error! [wrong frm num] %d\n", ulmap_widx);
            }

            gs_phy_frame_info.ul_map[ulmap_widx] = tmp_tx_info.ul_map[0];
            gs_phy_frame_info.ul_map_len[ulmap_widx] = tmp_tx_info.ul_map_len[0];
//            gs_phy_frame_info.dts_info[ulmap_widx] = tmp_tx_info.dts_info[0];
            gs_phy_frame_info.frame_number[ulmap_widx] = tmp_tx_info.frame_number[0] + UL_MAP_DELAY;
            gs_phy_frame_info.dl_perscan_flag[ulmap_widx] = tmp_tx_info.dl_perscan_flag[0];

//            gs_phy_frame_info.info_flag = 0;

            pthread_mutex_unlock (&mutex_phy_frame_info);
/*
            ulmap_widx ++;
            if (ulmap_widx >= UL_MAP_STORE)
            {
                ulmap_widx = 0;
            }
*/
        }

        pthread_mutex_lock (&mutex_tx_phy_en_flag);
        //        tx_phy_en_flag -= g_rru_data_param.tx_blk_per_frm;
        tx_phy_en_flag = tx_phy_en_flag - 1;
        pthread_mutex_unlock (&mutex_tx_phy_en_flag);

        if (count_t == 3)
        {
            gettimeofday (&end_t, NULL);
            total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                    - start_t.tv_usec );

            gettimeofday (&start_t, NULL);

            if (total > THREADHOLD)
            {
                FLOG_INFO ("Tx time  %d, at frame %d\n", total, tmp_tx_info.frame_number[0]);
            }

            DO_DUMP(DUMP_FRM_TX_DURATION_ID, tmp_tx_info.frame_number[0], NULL, total);

            count_t = 0;
        }

        count_t++;
        //printf("ISSUE to ETH_RRU %d bytes\n", len);
    }

xit:
#ifdef _DL_TX_FAKE_
    for(i = 0; i < ANTENNA_NUM; i++)
    {
	    for(j = 0; j < CARRIER_NUM; j++)
	    {

		    if(p_fake_slotsymbol_i[i][j])
			    free(p_fake_slotsymbol_i[i][j]);

		    if(p_fake_slotsymbol_q[i][j])
			    free(p_fake_slotsymbol_q[i][j]);
	    }
    }

    if (fp_t != NULL)
    {
        fclose(fp_t);
    }
#endif

    return NULL;

}
void *process_ethrru_rx (void *arg __attribute__ ((unused)))
{
    FLOG_INFO ("ETHRRU RX thread Started");

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

//    struct rru_data_param rru_param;
/*
    struct timeval start_t, end_t;
    unsigned int count_t = 0;
    unsigned int total = 0;
*/
    char * p_rx_buf[ANTENNA_NUM][CARRIER_NUM];
    char * p_rx_rbuf[ANTENNA_NUM][CARRIER_NUM];
    char * p_rx_wbuf[ANTENNA_NUM][CARRIER_NUM];
    int idx_carrier;
    int idx_chan;

    char pkt_buf[2048];
    struct rru_data_hdr * p_tmp_hdr = NULL;

    struct queue_msg rx_msg;

    struct rru_data_param * p_param_hdr[ANTENNA_NUM][CARRIER_NUM];
    char lock[ANTENNA_NUM][CARRIER_NUM];
    unsigned int frm_pkt_count[ANTENNA_NUM][CARRIER_NUM];

    int i, j;
    int ret;

    memset(&p_rx_buf, 0, sizeof (p_rx_buf));
    memset(&p_rx_rbuf, 0, sizeof (p_rx_rbuf));
    memset(&p_rx_wbuf, 0, sizeof (p_rx_wbuf));

    int frm_len;
    ret = get_global_param ("RX_FRAME_LEN", &frm_len);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RX_FRAME_LEN error\n");
    }
    int sample_rate;
    ret = get_global_param ("RRU_SAMPLE_RATE", &sample_rate);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_SAMPLE_RATE error\n");
    }
    sample_rate = 1 <<  (sample_rate);

    int eth_pkt_size;
    ret = get_global_param ("RRU_ETH_PKT_LEN", &eth_pkt_size);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_ETH_PKT_LEN error\n");
    }

    int carrier_num;
    ret = get_global_param ("RRU_CARRIER_NUM", &carrier_num);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_CARRIER_NUM error\n");
    }

    int frm_num;
    ret = get_global_param ("FAKE_RX_TRACE_FRM_NUM", &frm_num);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_RX_TRACE_FRM_NUM error\n");
    }

    int test_tx_only;
    ret = get_global_param ("TEST_TX_ONLY", &test_tx_only);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TEST_TX_ONLY error\n");
    }

    int frm_point_len = frm_len * sample_rate * 4;  //frame rx time lengh unit is 4chip
    int frm_data_size = frm_point_len * 4 * carrier_num * g_rru_data_param.rx_ant_num;  //i/q total 4 bytes per point
    int sample_per_pkt = eth_pkt_size / 4;
    int sample_last_pkt =  frm_point_len % sample_per_pkt;
    int total_pkt_per_carrier = frm_point_len / sample_per_pkt + ((sample_last_pkt)?1:0);  

#ifdef _DUMP_RRU_RX_
    frm_dump_rrh_carrier_reset( frm_point_len, frm_num );
#endif

#ifndef _SIM_RRH_

    struct rru_cmd_payload pre_cmd_state;
    struct rru_cmd_payload * p_cur_cmd_state = NULL;

    memset(&pre_cmd_state, 0, sizeof (struct rru_cmd_payload));
#endif
    

    for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
    {
	    for (j = 0; j < carrier_num; j++)
	    {
		    p_rx_rbuf[i][j] = my_malloc ( frm_point_len * 4  + sizeof(struct rru_data_param));
		    if( p_rx_rbuf[i][j]  == NULL )
		    {
			    FLOG_FATAL("memory allocation error\n");
			    goto xit;
		    }

		    p_param_hdr[i][j] = (struct rru_data_param *)p_rx_rbuf[i][j];

		    p_param_hdr[i][j]->dgain[i] = 0.0F;
		    p_param_hdr[i][j]->frm_num = 0;
		    p_param_hdr[i][j]->end_flag = BODY_FLAG;
		    p_param_hdr[i][j]->pkt_count = 0;
		    p_param_hdr[i][j]->carrier = j;

		    frm_pkt_count[i][j] = 0;

		    p_rx_buf[i][j] = p_rx_rbuf[i][j] + sizeof (struct rru_data_param);
		    p_rx_wbuf[i][j] = p_rx_buf[i][j];

		    lock[i][j] = 1;
	    }
    }

#ifndef _SIM_RRH_

    /** Sync to the set UDP stream */
    FLOG_INFO("start to set UDP in data path\r\n");

    int a_value[2];


    a_value[0] = 1;
    a_value[1] = 1;

    #ifdef _NEW_TRANS_ENABLE_

    ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_FLAG, 2, a_value);
    if (ret != 0)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_OPEN error");
    }

    #else

    ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_FLAG, 2, a_value);
    if (ret != 0)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_OPEN error");
    }

    #endif

    /** Sync to the data UDP stream */
    FLOG_INFO("Syncing to RRH in data path \r\n");

    for (i = 0; i < INITIAL_DELAY; i++)
    {
        ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);
    }

#endif

//RE_SYNC_TAG:
#if 0
    while (1)
    {
        ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);

        p_tmp_hdr = (struct rru_data_hdr *) pkt_buf;

        if ( ( p_tmp_hdr->data_type == UL_TYPE_FLAG )
                && ( p_tmp_hdr->frg_flag == HEAD_FLAG ) )
        {
        ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);
        ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);

            break;
        }
    }
#endif

    int need_resync = 1;
    while ( 1 )
    {

        int skip;
Resync:
        skip = total_pkt_per_carrier * carrier_num;
        while ( skip-- )
        {
            ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);
        }

    for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
    {
	    for (j = 0; j < carrier_num; j++)
	    {

		    p_param_hdr[i][j] = (struct rru_data_param *)p_rx_rbuf[i][j];

		    p_param_hdr[i][j]->dgain[i] = 0.0F;
		    p_param_hdr[i][j]->frm_num = 0;
		    p_param_hdr[i][j]->end_flag = BODY_FLAG;
		    p_param_hdr[i][j]->pkt_count = 0;
		    p_param_hdr[i][j]->carrier = j;

		    frm_pkt_count[i][j] = 0;

		    p_rx_buf[i][j] = p_rx_rbuf[i][j] + sizeof (struct rru_data_param);
		    p_rx_wbuf[i][j] = p_rx_buf[i][j];

		    lock[i][j] = 1;
	    }
    }

    while ( need_resync )
    {
        ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);

        p_tmp_hdr = (struct rru_data_hdr *) pkt_buf;

        if ( (( p_tmp_hdr->data_type == UL_TYPE_FLAG )
                && ( p_tmp_hdr->frg_flag == HEAD_FLAG ))
                || (test_tx_only && ( p_tmp_hdr->data_type == CMD_TYPE_FLAG )) )
        {
            FLOG_INFO("Get frame sync from Ethernet link \n");

            g_rruframe_num = p_tmp_hdr->frm_num;
            g_init_rruframe_num = p_tmp_hdr->frm_num;
            resync_mac();

            if ( post_sem (tx_s_handle) )
            {
                FLOG_ERROR("Tx TDD SEM error\n");
            }
#ifdef GREN_RRH
            if( p_tmp_hdr->carrier >= 0 && p_tmp_hdr->carrier < carrier_num ) 
            {
       	  	idx_carrier = p_tmp_hdr->carrier;
            }
            else
            {
                if(handle_repeat_err(ERR_CARR_IDX))
                {
            		FLOG_ERROR(" Carrier number (%d) in the packet exceed limits(0 ~ %d)\n", p_tmp_hdr->carrier, carrier_num - 1 );
                }
                continue;
            }

#else
            idx_carrier = 0;
#endif
            if( p_tmp_hdr->chan <= g_rru_data_param.rx_ant_num ) 
            {
                idx_chan = (p_tmp_hdr->data_type == CMD_TYPE_FLAG )?0:(p_tmp_hdr->chan -1);
            }
            else
            {
                if(handle_repeat_err(ERR_CHAN_IDX))
                {
            	FLOG_ERROR(" Channel number (%d) in the packet exceed limits(0 ~ %d)\n", p_tmp_hdr->chan, g_rru_data_param.rx_ant_num);
                }
                continue;
            }

//            if ( idx_carrier != 0 || idx_chan != 0)
//            	continue;

            memcpy (p_rx_wbuf[idx_chan][idx_carrier], ( pkt_buf + DATA_HDR_LEN ),
                    eth_pkt_size);

            p_rx_wbuf[idx_chan][idx_carrier] += eth_pkt_size;

            p_param_hdr[idx_chan][idx_carrier]->dgain[0] = (float) p_tmp_hdr->dgain;
            p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;
            p_param_hdr[idx_chan][idx_carrier]->end_flag = HEAD_FLAG;
            p_param_hdr[idx_chan][idx_carrier]->pkt_count = 1;
#ifdef GREN_RRH
            p_param_hdr[idx_chan][idx_carrier]->carrier = p_tmp_hdr->carrier;
#else
            p_param_hdr[idx_chan][idx_carrier]->carrier = 0;
#endif

            frm_pkt_count[idx_chan][idx_carrier] = 0;

            lock[idx_chan][idx_carrier] = 0;
            
            need_resync = 0;
            FLOG_INFO("Synced to RRH in data path, frm_num = %d\r\n", p_param_hdr[idx_chan][idx_carrier]->frm_num );
            break;
        }
    }


    //turn_metric(1);

    /** Start the TDD */

        while (1)
        {
            ethrru_data_rx_pkt(pkt_buf, eth_pkt_size + DATA_HDR_LEN);
            p_tmp_hdr = (struct rru_data_hdr *) pkt_buf;
#ifdef GREN_RRH
            if( p_tmp_hdr->carrier >= 0 && p_tmp_hdr->carrier < carrier_num ) 
            {
       	  	idx_carrier = p_tmp_hdr->carrier;
            }
            else
            {
                if(handle_repeat_err(ERR_CARR_IDX))
                {
            	FLOG_ERROR(" Carrier number (%d) in the packet exceed limits(0 ~ %d)\n", p_tmp_hdr->carrier, carrier_num);
                rru_dump_pkt( p_tmp_hdr );
                }
                continue;
            }

#else
            idx_carrier = 0;
#endif
            if( p_tmp_hdr->chan <= g_rru_data_param.rx_ant_num ) 
            {
                idx_chan = (p_tmp_hdr->data_type == CMD_TYPE_FLAG )?0:(p_tmp_hdr->chan -1);
            }
            else
            {
                if(handle_repeat_err(ERR_CHAN_IDX))
                {
            	FLOG_ERROR(" Channel number (%d) in the packet exceed limits(0 ~ %d)\n", p_tmp_hdr->chan, g_rru_data_param.rx_ant_num);
                rru_dump_pkt( p_tmp_hdr );
                }
                continue;
            }
/*
            printf("seq no %d\n", p_tmp_hdr->seq_no);
            printf("length %d\n", p_tmp_hdr->length);
            printf("chan %d\n", p_tmp_hdr->chan);
            printf("frm_num %d\n", p_tmp_hdr->frm_num);
            printf("frg_flag %d\n\n", p_tmp_hdr->frg_flag);
*/

            if (p_tmp_hdr->data_type == CMD_TYPE_FLAG)
            {

#ifndef _SIM_RRH_
                p_cur_cmd_state = (struct rru_cmd_payload *) (pkt_buf + sizeof (struct rru_data_hdr));


                if ( (pre_cmd_state.pkt_crc_count != p_cur_cmd_state->pkt_crc_count) ||
                     (pre_cmd_state.pkt_lost_count != p_cur_cmd_state->pkt_lost_count) 
                      || (pre_cmd_state.pkt_late_count != p_cur_cmd_state->pkt_late_count)  )
                {
#if 1
                if(handle_repeat_err(ERR_CMD))
                {
                    FLOG_WARNING("\n \
                                     Packet CRC error count %d\n \
                                     Packet lost count %d\n \
                                     Packet late count %d\n \
                                     First packet arrive time %d\n \
                                     Last packet arrive time %d\n",
                                  p_cur_cmd_state->pkt_crc_count,
                                  p_cur_cmd_state->pkt_lost_count,
                                  p_cur_cmd_state->pkt_late_count,
                                  p_cur_cmd_state->pkt_start_time,
                                  p_cur_cmd_state->pkt_end_time);
                }
#endif

                    pre_cmd_state.pkt_crc_count = p_cur_cmd_state->pkt_crc_count;
                    pre_cmd_state.pkt_lost_count = p_cur_cmd_state->pkt_lost_count;
                    pre_cmd_state.pkt_late_count = p_cur_cmd_state->pkt_late_count;
                    pre_cmd_state.pkt_start_time = p_cur_cmd_state->pkt_start_time;
                    pre_cmd_state.pkt_end_time = p_cur_cmd_state->pkt_end_time;
                }
#endif

                g_cmd_frame_num = p_tmp_hdr->frm_num;

                    if ( post_sem (cmd_s_handle) )
                    {
                        FLOG_ERROR("post CMD SEM error\n");
                    }

//                   FLOG_ERROR("post CMD SEM \n");

            }
            else if( test_tx_only) 
		continue;

            if ( p_tmp_hdr->data_type == UL_TYPE_FLAG )
            {
                if ( p_tmp_hdr->frg_flag == BODY_FLAG )
		{
/*                       FLOG_WARNING("BODY: chan %d, frm_num %d seq %d\n",
                         p_tmp_hdr->chan,
                         p_tmp_hdr->frm_num,
                         p_tmp_hdr->seq_no
                         );
*/

			if (p_tmp_hdr->frm_num != p_param_hdr[idx_chan][idx_carrier]->frm_num)
			{
                if(handle_repeat_err(ERR_BODY_FRM_NO_MISMATCH))
                {
				FLOG_WARNING("Body frame number mismatch: hdr frm number %d, current frm number %d, (chan %d, carr %d)\n",
						p_tmp_hdr->frm_num,
						p_param_hdr[idx_chan][idx_carrier]->frm_num,
                                               idx_chan, idx_carrier);

                                rru_dump_pkt( p_tmp_hdr );
                }



				/* trigger this frame for tx */
				if ( post_sem (tx_s_handle) )
				{
					FLOG_ERROR("Tx TDD SEM error\n");
				}

				p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;

				/* lock the packet */
				for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
				{
					for (j = 0; j < carrier_num; j++)
					{

						lock[i][j] = 1;
					}
				}

                                need_resync = 1;
                                goto Resync;
			}
			if (lock[idx_chan][idx_carrier] == 1)
			{
                if(handle_repeat_err(ERR_FRM_MISHEAD))
                {
				FLOG_WARNING("Missed header: chan %d, frm_num %d\n",
						p_tmp_hdr->chan,
						p_tmp_hdr->frm_num);
                                rru_dump_pkt( p_tmp_hdr );
                }


				/* wait until next header\n */
                                need_resync = 1;
                                goto Resync;
			}

			memcpy (p_rx_wbuf[idx_chan][idx_carrier], ( pkt_buf + DATA_HDR_LEN ),
					eth_pkt_size);

			p_rx_wbuf[idx_chan][idx_carrier] += eth_pkt_size;

			p_param_hdr[idx_chan][idx_carrier]->dgain[0] = (float) p_tmp_hdr->dgain;

			p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;

			p_param_hdr[idx_chan][idx_carrier]->pkt_count ++;

			frm_pkt_count[idx_chan][idx_carrier] ++;
		}

                if ( p_tmp_hdr->frg_flag == HEAD_FLAG )
                {

                    if (lock[idx_chan][idx_carrier] == 0)
                    {
                if(handle_repeat_err(ERR_FRM_MISTAIL))
                {
                        FLOG_WARNING("Miss tail: chan %d, frm_num %d\n",
                                p_tmp_hdr->chan,
                                p_tmp_hdr->frm_num);
                                rru_dump_pkt( p_tmp_hdr );
                }

                    }
       /*                 FLOG_WARNING("---------->HEAD: chan %d, frm_num %d seq %d\n",
                         p_tmp_hdr->chan,
                         p_tmp_hdr->frm_num,
                         p_tmp_hdr->seq_no
                         );
       */

                    g_rruframe_num = p_tmp_hdr->frm_num;

                    if (p_tmp_hdr->chan == 1)
                    {
                        if ( post_sem (tx_s_handle) )
                        {
                            FLOG_ERROR("Tx TDD SEM error\n");
                        }
                    }

                    /** header is header */
                    p_rx_buf[idx_chan][idx_carrier] = p_rx_rbuf[idx_chan][idx_carrier] + sizeof (struct rru_data_param);
                    p_rx_wbuf[idx_chan][idx_carrier] = p_rx_buf[idx_chan][idx_carrier];

                    memcpy (p_rx_wbuf[idx_chan][idx_carrier], ( pkt_buf + DATA_HDR_LEN ),
                            eth_pkt_size);

                    p_rx_wbuf[idx_chan][idx_carrier] += eth_pkt_size;

                    p_param_hdr[idx_chan][idx_carrier]->dgain[0] = (float) p_tmp_hdr->dgain;
                    p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;
                    p_param_hdr[idx_chan][idx_carrier]->end_flag = HEAD_FLAG;
                    p_param_hdr[idx_chan][idx_carrier]->pkt_count = 1;

                    frm_pkt_count[idx_chan][idx_carrier] = 1;

                    lock[idx_chan][idx_carrier] = 0;
                }

                if (p_tmp_hdr->frg_flag == TAIL_FLAG)
                {
/*                        FLOG_WARNING("================= TAIL: chan %d, frm_num %d seq %d\n",
                         p_tmp_hdr->chan,
                         p_tmp_hdr->frm_num,
                         p_tmp_hdr->seq_no
                         );
*/

                    if (p_tmp_hdr->frm_num != p_param_hdr[idx_chan][idx_carrier]->frm_num)
		    {
                if(handle_repeat_err(ERR_TAIL_FRM_NO_MISMATCH))
                {
			    FLOG_WARNING("Tail frame number mismatch: hdr frm number %d, current frm number %d (chan %d, carr %d)\n",
					    p_tmp_hdr->frm_num,
					    p_param_hdr[idx_chan][idx_carrier]->frm_num,
                                            idx_chan, idx_carrier);
                                rru_dump_pkt( p_tmp_hdr );
                }



			    /* trigger this frame for tx */
			    if ( post_sem (tx_s_handle) )
			    {
				    FLOG_ERROR("Tx TDD SEM error\n");
			    }

			    p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;

			    /* lock the packet */
			    for (i = 0; i < g_rru_data_param.rx_ant_num; i++)
			    {
				    for (j = 0; j < carrier_num; j++)
				    {

					    lock[i][j] = 1;
				    }
			    }

                            need_resync = 1;
                            goto Resync;
		    }

                    if (lock[idx_chan][idx_carrier] == 1)
                    {
                if(handle_repeat_err(ERR_FRM_MISHEAD))
                {
                        FLOG_WARNING("Missed header: chan %d, frm_num %d\n",
                                p_tmp_hdr->chan,
                                p_tmp_hdr->frm_num);
                                rru_dump_pkt( p_tmp_hdr );
                }


                        /* wait until next header\n */
                        need_resync = 1;
                        goto Resync;
                    }

                    lock[idx_chan][idx_carrier] = 1;
                    p_param_hdr[idx_chan][idx_carrier]->pkt_count ++;
                    frm_pkt_count[idx_chan][idx_carrier] ++;

                    memcpy (p_rx_wbuf[idx_chan][idx_carrier], ( pkt_buf + DATA_HDR_LEN ),
                             (sample_last_pkt) ? sample_last_pkt * 4 : eth_pkt_size );

                    if ( (int)(p_param_hdr[idx_chan][idx_carrier]->pkt_count)
                            < PACKET_PER_ANT * g_rru_data_param.rx_symbol_per_slot )
                    {
                if(handle_repeat_err(ERR_MISS_PIECE))
                {
                        FLOG_WARNING("some pieces missed in last slot symbol, %d ,%d", p_param_hdr[idx_chan][idx_carrier]->pkt_count, total_pkt_per_carrier);
                                rru_dump_pkt( p_tmp_hdr );
                }


                    }else if ((int)( p_param_hdr[idx_chan][idx_carrier]->pkt_count )
                                  > PACKET_PER_ANT * g_rru_data_param.rx_symbol_per_slot )
                    {
                        FLOG_WARNING("Why this happend???");
                                rru_dump_pkt( p_tmp_hdr );
                    }

                    p_param_hdr[idx_chan][idx_carrier]->end_flag = TAIL_FLAG;

                    frm_pkt_count[idx_chan][idx_carrier] = 0;
                }

                if ( (int) (frm_pkt_count[idx_chan][idx_carrier] )
                        > PACKET_PER_ANT * g_rru_data_param.rx_symbol_num )
                {
                if(handle_repeat_err(ERR_TOO_MANY_BODY))
                {
                    FLOG_WARNING("Too many body without tail!%d ,%d", frm_pkt_count[idx_chan][idx_carrier] , total_pkt_per_carrier);
                                rru_dump_pkt( p_tmp_hdr );
                }


                    frm_pkt_count[idx_chan][idx_carrier]  --;

                    /** wait until header */
                    lock[idx_chan][idx_carrier]  = 1;
                    need_resync = 1;
                    goto Resync;
                }else if ( (int)(p_param_hdr[idx_chan][idx_carrier]->pkt_count )
                        >=  PACKET_PER_ANT * g_rru_data_param.rx_symbol_per_slot )
                {
                    rx_msg.p_buf = p_rx_rbuf[idx_chan][idx_carrier];

                    rx_msg.my_type = frm_ul_en_id[idx_carrier * carrier_num + idx_chan];
		  

                    if (wmrt_enqueue (frm_ul_en_id[idx_carrier * carrier_num + idx_chan], &rx_msg, sizeof(struct queue_msg))
                            == -1)
                    {
                        FLOG_ERROR ("ENQUEUE ERROR\n");
                    }

#ifdef _DUMP_RRU_RX_
                   frm_dump_rrh_carrier( idx_chan, idx_carrier, frm_point_len, 
                                        (short *)p_rx_rbuf[idx_chan][idx_carrier] + sizeof(struct rru_data_param)/2,
                                         g_rru_data_param.rx_ant_num * carrier_num );
#endif


                    p_rx_rbuf[idx_chan][idx_carrier] = my_malloc ( frm_point_len * 4  + sizeof(struct rru_data_param));
                    p_param_hdr[idx_chan][idx_carrier]  = (struct rru_data_param *)p_rx_rbuf[idx_chan][idx_carrier];

                    p_param_hdr[idx_chan][idx_carrier]->dgain[0] = 0.0F;
                    p_param_hdr[idx_chan][idx_carrier]->dgain[1] = 0.0F;
                    p_param_hdr[idx_chan][idx_carrier]->frm_num = p_tmp_hdr->frm_num;
                    p_param_hdr[idx_chan][idx_carrier]->end_flag = BODY_FLAG;
                    p_param_hdr[idx_chan][idx_carrier]->pkt_count = 0;
#ifdef GREN_RRH
                    p_param_hdr[idx_chan][idx_carrier]->carrier = idx_carrier;
#else
                    p_param_hdr[idx_chan][idx_carrier]->carrier = 0;
#endif

                    p_rx_buf[idx_chan][idx_carrier] = p_rx_rbuf[idx_chan][idx_carrier] + sizeof (struct rru_data_param);
                    p_rx_wbuf[idx_chan][idx_carrier] = p_rx_buf[idx_chan][idx_carrier];

                    //break;
                }
            }
        }
#if 0
        if (count_t == 5)
        {
            gettimeofday (&end_t, NULL);

            total = ( end_t.tv_sec - start_t.tv_sec ) * 1000000 + ( end_t.tv_usec
                    - start_t.tv_usec );

            gettimeofday (&start_t, NULL);

            if (total > THREADHOLD)
            {
                FLOG_INFO ("Pure rx time %d\n", total);
            }
            count_t = 0;
        }
#endif
    }
xit:

    for(i = 0; i < ANTENNA_NUM; i++)
    {
	    for(j = 0; j < CARRIER_NUM; j++)
	    {

		    if(p_rx_buf[i][j])
			    free(p_rx_buf[i][j]);

	    }
    }
    return NULL;
}


int get_rru_data_config (struct rru_data_config * rru_config)
{
    int ret;


    ret = get_global_param ("TX_ANTENNA_NUM", & ( rru_config->tx_ant_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TX_ANTENNA_NUM error\n");
    }

    ret = get_global_param ("DL_SYMBOL_PER_SLOTSYMBOL", & ( rru_config->tx_symbol_per_slot ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters DL_SYMBOL_PER_SLOTSYMBOL error\n");
    }

    ret = get_global_param ("SYMBOL_NUM_IN_DL_FRAME",
            & ( rru_config->tx_symbol_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_DL_FRAME error\n");
    }

    rru_config->tx_symbol_num = rru_config->tx_symbol_num;

    rru_config->tx_blk_per_frm = (int) ( rru_config->tx_symbol_num / rru_config->tx_symbol_per_slot );

    ret = get_global_param ("RX_ANTENNA_NUM", & ( rru_config->rx_ant_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RX_ANTENNA_NUM error\n");
    }

    ret = get_global_param ("UL_SYMBOL_PER_SLOTSYMBOL", & ( rru_config->rx_symbol_per_slot ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters UL_SYMBOL_PER_SLOTSYMBOL error\n");
    }

    ret = get_global_param ("SYMBOL_NUM_IN_UL_FRAME",
            & ( rru_config->rx_symbol_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
    }

    rru_config->rx_blk_per_frm = (int) ( rru_config->rx_symbol_num / rru_config->rx_symbol_per_slot );

    ret = get_global_param ("GAIN_TX",
            & ( rru_config->gain_tx ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
    }

    ret = get_global_param ("ADVANCED_CMD_TIME",
            & ( rru_config->advanced_cmd_time ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
    }

    return 0;
}

#ifndef _DUMP_BIN_RAW_DATA_
int frm_dump_short_sample (int flag, FILE * f_p, int len, void * buf)
{
    short * p_tmp = (short *)buf;
    int i;

    if (flag >= 0)
    {
        fprintf(f_p, "%s,%d\n", (char *)buf, flag);
    }else
    {
        for (i = 0; i < len; i++)
        {
            fprintf(f_p, "%d,", p_tmp[i]);
        }

        fprintf(f_p, "\n");
    }

    return 0;
}
#else
int frm_dump_short_sample (int flag, FILE * f_p, int len, void * buf)
{
    if (len == 0)
    {
        fwrite(&flag, sizeof(int), 1, f_p);
//        fprintf(f_p, "%s,%d\n", (char *)buf, flag);
    }else
    {
        fwrite(buf, sizeof(short), len, f_p);
    }

    return 0;
}
#endif


int frm_dump_duration(unsigned int frame_num, FILE * f_p, int duration)
{
    fprintf(f_p, "%d,%d\n", frame_num, duration);

    return 0;
}


#ifndef SEPARATE_IQ
static void frm_dump_rrh_carrier_reset( int sample_per_frm, int frm_num )
{
	int total = frm_num;

	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_Q_ID, NULL, 0, total);
        

}
static void frm_dump_rrh_carrier(int ant_num, int carr_num, int sample_num, short *rx_buf_short, int max)
{

	int i=0;
        int total_short = sample_num * 2;
        static int count  = 0;
        //FLOG_WARNING ("total_short = %d\n", total_short);

	if( 0 == ant_num )
	{
		if( 0 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_I_ID, -1, &rx_buf_short[i], total_short);

		} 
		else if( 1 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_I_ID, -1, &rx_buf_short[i], total_short);
		}
		else if( 2 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_I_ID, -1, &rx_buf_short[i], total_short);
		}
		else if( 3 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_I_ID, -1, &rx_buf_short[i], total_short);
		}
	} 
        else if( 1 == ant_num )
        {
		if( 0 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_I_ID, -1, &rx_buf_short[i], total_short);

		} 
		else if( 1 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_I_ID, -1, &rx_buf_short[i], total_short);
		}
		else if( 2 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID, -1, &rx_buf_short[i], total_short);
		}
		else if( 3 == carr_num )
		{
			DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID, -1, &rx_buf_short[i], total_short);
		}

        }
}

#else
static void frm_dump_rrh_carrier_reset( int sample_per_frm, int frm_num )
{
	int total = sample_per_frm * frm_num;

	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_Q_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID, NULL, 0, total);
	RESET_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_Q_ID, NULL, 0, total);
        

}

static void frm_dump_rrh_carrier(int ant_num, int carr_num, int sample_num, short *rx_buf_short, int max)
{

	int i;
        int total_short = sample_num * 2;
        static int count  = 0;

	if( 0 == ant_num )
	{
		if( 0 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR0_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}

		} 
		else if( 1 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR1_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}
		else if( 2 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR2_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}
		else if( 3 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT0_CARR3_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}
	} 
        else if( 1 == ant_num )
        {

		if( 0 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR0_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}

		} 
		else if( 1 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR1_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}
		else if( 2 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR2_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}
		else if( 3 == carr_num )
		{
			for (i = 0; i < total_short ; i+=2)
			{
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID, -1, &rx_buf_short[i], 1);
				DO_DUMP(DUMP_RX_ALL_RAW_ANT1_CARR3_Q_ID, -1, &rx_buf_short[i + 1], 1);

			}
		}


        }
#if 0
        FLOG_WARNING ("dump %d max %d\n", count, max);
        if( ++count >= max )
        {
            flush_dump();
            count = 0;
        }
#endif
}
#endif 
