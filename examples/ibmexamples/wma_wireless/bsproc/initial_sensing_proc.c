/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: initial_sensing_proc.c

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "flog.h"
#include "bs_cfg.h"
#include "global.h"

#include "initial_sensing_proc.h"
#include "phy_initial_sensing.h"
#include "trans.h"
#include "rru_adapter.h"
#include "rru_proc.h"
#include "trans_proc.h"

#include "queue_util.h"
#include "dump_util.h"


struct sensing_node
{
    struct spectrum_para * param;
    struct init_scan_result * spectrum_result;
    int freq_num;
    int dgain[2];
    short buf[FIX_DATA_LEN * 2 * ANTENNA_NUM * 2 * PACKET_PER_SYMBOL * 4];
};

static pthread_t phy_is_thread = 0;

float exp10f(float x);
struct spectrum_para g_spectrum;
struct spectrum_config_para g_spectrum_config;

int32_t init_spectrum_sensing(struct spectrum_para * param, struct init_scan_result *spectrum_result, int freq_num);
int32_t generate_result(struct init_scan_result *spectrum_result, float *p_sensing);
int32_t generate_dtsinfo(struct spectrum_para *spec_para, struct spectrum_config_para *g_spec_conf);

void * phy_is_process (void * arg __attribute__ ((unused)));


static short * pkt_buf = NULL;
static float * sum_power = NULL;
static struct spectrum_scan_state * p_spec_conf = NULL;
static float * input_r[2];
static float * input_i[2];
static float * sum_intf;

#ifndef _SIM_RRH_

static u_int8_t  g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
static u_int8_t  g_rrh_state_exe_num = 5;


#ifdef _NEW_TRANS_ENABLE_

#ifdef _NEW_TRANS_GREN_ENABLE_

static int set_rrh_config_carrier(void)
{
    u_int32_t uw_index = 0;
    u_int32_t uw_ret = 0;

    int ret = 0;
    
    struct trans_rrh_carrier_info st_carrier_info;
    
    for (uw_index = 0; uw_index < 4; uw_index++)
    {
        if (0 == g_trans_control_param.carrier_info[uw_index].carrier_enable)
        {
    
            continue;
        }

        st_carrier_info.uc_carr_no = uw_index;

        if (1 == g_spectrum.initial_sensing_enable)
        {
            ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(g_spectrum_config.working_freq));
            if (ret != 0)
            {
                FLOG_WARNING ("get parameters DEFAULT_WORKING_FREQ error\n");
                //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
                g_spectrum_config.working_freq = 230000;
            }

            st_carrier_info.uw_carr_freq =  g_spectrum_config.working_freq;
        }
        else
        {
            st_carrier_info.uw_carr_freq = g_trans_control_param.carrier_info[uw_index].carrier_freq;
        }

        st_carrier_info.uw_carr_bw = g_trans_control_param.carrier_info[uw_index].carrier_bw;
        st_carrier_info.uc_carr_pwr = g_trans_control_param.carrier_info[uw_index].carrier_pwr;
                
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CARR_INFO, 1, &st_carrier_info);
        if (0 != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_CARR_INFO error! uw_ret = %d\r\n", uw_ret);
            return 1;
        }
        
        break;
    
    }
    
    return 0;    

}


static int set_rrh_config_tdd(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    int ret = 0;
    
    FLOG_INFO("Config TDD start...\r\n");
    
    /*CHANNEL_MODE*/
    a_value[0] = 1;
    a_value[1] = 1;
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_MODE, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_MODE error");
        return 1;
    } 
    
    #if 0
    /*CFG_RX_LEN*/
    a_value[0] = g_trans_control_param.rru_rx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_RX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        return 1;
    }
    
   
    /*CFG_TX_LEN*/
    a_value[0] = g_trans_control_param.rru_tx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_TX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);

        return 1;
    
    }
    #endif
    
    /*CARRIER_INFO*/
    ret = set_rrh_config_carrier();
    if (0 != ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! ret = %d\r\n", ret);

        return 1;
    }
    
    /*QUERY_POWER*/
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for QUERY_POWER error! uw_ret = %d\r\n", uw_ret);

        return 1;
    }
    
    FLOG_INFO("Config TDD OK \r\n");
    
    return 0;

}

#else
static int set_rrh_config_tdd(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    int ad_cmd_time = 5000;
    int ret = 0;
    
    /** change back to TDD */
    a_value[0] = 1;
    a_value[1] = 1;
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_MODE, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_MODE error");
        return 1;
    } 
    
    /** set working freq */
    ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(g_spectrum_config.working_freq));
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters DEFAULT_WORKING_FREQ error\n");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        g_spectrum_config.working_freq = 230000;
    }
    
    a_value[0] = g_spectrum_config.working_freq;
    a_value[1] = g_spectrum_config.working_freq;
      
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_FREQ, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_FREQ error");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }
    
    FLOG_INFO("Frequency point Set to %d\n", g_spectrum_config.working_freq);

#ifndef _OLD_RRH_

    a_value[0] = g_trans_control_param.rru_rx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_RX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }
    
    a_value[0] = g_trans_control_param.rru_tx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_TX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    
    }

    a_value[0] = g_trans_control_param.enable_agc;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_AGC_ENABLE, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_AGC error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;

    }
    
    if (g_trans_control_param.enable_agc == 0)
    {
        a_value[0] = g_trans_control_param.chan1_rx_pgc;
        a_value[1] = g_trans_control_param.chan2_rx_pgc;
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CH_RX_PGC, 2, a_value);
        if (0 != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_CH_RX_PGC error! uw_ret = %d\r\n", uw_ret);
            //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
            return 1;
        }
    }

    #endif

    ret = get_global_param ("ADVANCED_CMD_TIME", & ( ad_cmd_time ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
        ad_cmd_time = 5000;
    }
    
    a_value[0] = ad_cmd_time;
    a_value[1] = ad_cmd_time;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_DL_PRESEND_TIME, 1, a_value);
    
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for DL_PRESEND_TIME error");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }

#ifndef _OLD_RRH_

    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for QUERY_POWER error! uw_ret = %d\r\n", uw_ret);
    
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }

#endif

    return 0;

}

#endif


int set_rrh_config(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    int i, j;
    //int ad_cmd_time = 5000;
    int ret = 0;
    float * p_sensing = NULL;
    
    struct init_scan_result ** scan_result = NULL;

    #ifdef _NEW_TRANS_GREN_ENABLE_
    struct trans_rrh_carrier_info st_carrier_info;
    #endif

    if (g_spectrum.initial_sensing_enable == 0)
    {
        ret = connect_agent();

        if (0 != ret)
        {
            FLOG_ERROR("Call wma_agent_process error! uw_ret = %d\r\n", ret);

            return 1;
        }

        g_rrh_config_state = SET_RRH_CONFIG_STEP_3;

    }else
    {

        g_rrh_config_state = SET_RRH_CONFIG_STEP_1;

        sum_intf = (float *) malloc ( sizeof(float) * 21);

        if (sum_intf == NULL)
        {
            FLOG_ERROR("malloc intf buffer memory error\n");
            return 1;
        }

        pkt_buf = (short *) malloc (FIX_DATA_LEN * 2 * ANTENNA_NUM * 2 * PACKET_PER_SYMBOL * 4);

        if (pkt_buf == NULL)
        {
            FLOG_ERROR("malloc packet buffer memory error\n");
            return 1;
        }

        sum_power = (float *) malloc (1024 * 4);

        if (sum_power == NULL)
        {
            FLOG_ERROR("malloc summary power memory error\n");
            return 1;
        }

        p_spec_conf = (struct spectrum_scan_state *) malloc (sizeof (struct spectrum_scan_state));

        if (p_spec_conf == NULL)
        {
            FLOG_ERROR("malloc struct spectrum_scan_state memory error\n");
            return 1;
        }

        for (i = 0; i < 2; i++)
        {
            input_r[i] = (float *)malloc (2048 * 4);
            input_i[i] = (float *)malloc (2048 * 4);

            if ( (input_r[i] == NULL) || (input_i[i]) == NULL)
            {
                FLOG_ERROR("malloc packet buffer memory error\n");
                return 1;
            }
        }

        scan_result = (struct init_scan_result **)malloc(g_spectrum.num_freq * sizeof(void *));

        if (scan_result == NULL)
        {
            FLOG_ERROR("malloc scan");
            return 1;
        }

        for (i = 0; i < g_spectrum.num_freq; i++)
        {
//            printf("g_spectrum.num_freq=%d\n", g_spectrum.num_freq);
            scan_result[i] = (struct init_scan_result *)malloc(sizeof(struct init_scan_result));

            if (scan_result[i] == NULL)
            {
                FLOG_ERROR("malloc scan");
                return 1;
            }

            scan_result[i]->average_power = 999999;
            memset(scan_result[i]->active_band, 999999, sizeof(float) * 21);
        }

        init_spectrum_ed_scan();

        p_sensing = (float *)malloc(TRANS_AGENT_SENSING_NUM * 4);
        
        if (NULL == p_sensing)
        {
            FLOG_ERROR("malloc p_sensing error");
            return 1;
        }

    }

    u_int8_t uc_stop_flag =0;
    while (g_rrh_state_exe_num)
    {         
        switch (g_rrh_config_state)
        {
             case SET_RRH_CONFIG_STEP_1:
                 a_value[0] = 3;
                 a_value[1] = 3;
                 uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_MODE, 2, a_value);
                 if (0 != uw_ret)
                 {
                     FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_MODE error");                     
                     //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                     uc_stop_flag = 1;
                     break;                     
                 }
/*
                a_value[0] = g_trans_control_param.is_rxfrm_len;                
                uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_RX_LEN, 1, a_value);
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);

                    //uc_stop_flag = 1;
                    break;
                }
*/
                g_rrh_config_state = SET_RRH_CONFIG_STEP_2;
                break;

             case SET_RRH_CONFIG_STEP_2:

                pthread_create (&phy_is_thread, NULL, phy_is_process, NULL);

                for (j = 0; j< g_spectrum.num_freq; j++)
                {
                    /** set freq */
                    #ifdef _NEW_TRANS_GREN_ENABLE_

                    st_carrier_info.uc_carr_no = 0;
                    st_carrier_info.uw_carr_freq = g_spectrum.frequency[j];
                    st_carrier_info.uw_carr_bw = g_spectrum.carrier_bw;
                    st_carrier_info.uc_carr_pwr = g_spectrum.carrier_pwr;
                            
                    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CARR_INFO, 1, &st_carrier_info);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_CARR_INFO error! uw_ret = %d\r\n", uw_ret);
                        return 1;
                    }
                    
                    #else
                    a_value[0] = g_spectrum.frequency[j];
                    a_value[1] = g_spectrum.frequency[j];
                    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_FREQ, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_FREQ error");
                        //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                        continue;   
                    }

                    #endif
                    
                    /** open channel */
                    a_value[0] = 1;
                    a_value[1] = 1;
                    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_FLAG, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_OPEN error");
                        //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                        continue;  
                    }
                    
                    FLOG_INFO("Scaning %dhz\n", g_spectrum.frequency[j]);

                    uw_ret = init_spectrum_sensing(&g_spectrum, scan_result[j], g_spectrum.frequency[j]);

                    if (uw_ret != 0)
                    {
                        FLOG_ERROR("init_spectrum_sensing failed");
                        continue;  
                    }

                    /** close channel */
                    a_value[0] = 0;
                    a_value[1] = 0;
                    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_FLAG, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_CLOSE error");
                        continue;                        
                    }

                    FLOG_INFO("Scan finish .");

                    ret = generate_result(scan_result[j], p_sensing);
                    if (ret != 0)
                    {
                        FLOG_ERROR("Call generate_result error");
                        continue;
                    }

                }

                if (phy_is_thread != 0)
                {
                    pthread_cancel (phy_is_thread);
                    pthread_join (phy_is_thread, NULL);
                }

                /** send result to NMS server */
                /** wait for result from NMS server and put into g_spectrum_config*/
                /** g_spectrum_config->str_active_band must be a string here! */
                /** Use sprintf if needed */
                /*just send message to agent when connected to Agent*/

                ret = connect_agent();
                if (0 != ret)
                {
                    FLOG_ERROR("Call wma_agent_process error! uw_ret = %d\r\n", ret);
    
                    return 1;
                }

                ret =  wait_agent();
                if (1 == ret)
                {
                    FLOG_INFO("Send msg to agent for SPECTRUM");

                    uw_ret = trans_send_bs_msg_to_agent(TRANS_SEND_AGENT_SPECTRUM, 0, p_sensing);
                    /*Error*/
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_bs_msg_to_agent for AGENT_SPECTRUM error! uw_ret = %d\r\n", uw_ret);
                        //return TRANS_FAILD;
                    }
                    
                    ret = get_global_param("INTERFERENCE_ACTIVE", (void *)(g_spectrum_config.str_active_band));

                    if (ret != 0)
                    {
                        FLOG_ERROR("get initial active error");
                        return 1;
                    }

                    ret = generate_dtsinfo(&g_spectrum, &g_spectrum_config);
                    if (ret != 0)
                    {
                        FLOG_ERROR("generate dtsinfo error");
                        return 1;
                    }
                }
                
                #if 0
                /** change back to TDD */
                a_value[0] = 1;
                a_value[1] = 1;
                uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CHANNEL_MODE, 2, a_value);
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CHANNEL_MODE error");
                    uc_stop_flag = 1;
                    break;
                }
                #endif

                if (SET_RRH_CONFIG_STEP_1 != g_rrh_config_state)
                {
                    g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
                }

                break;
                    
            case SET_RRH_CONFIG_STEP_3:

                ret = set_rrh_config_tdd();
                if (ret != 0)
                {
                    FLOG_ERROR("Config RRH FOR TDD error");
                    g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
                    
                    break;
                }
                
                FLOG_INFO("Config RRH FOR TDD OK .");
                /*Config End ---While End*/
                uc_stop_flag = 1;

                break;

            default:
                
                FLOG_ERROR("g_rrh_config_state = %d error", g_rrh_config_state);
                g_rrh_config_state = 0;

                break;            

        }

        if (0 != uc_stop_flag)
        {
            break;        
        }
        
        g_rrh_state_exe_num--;

    }
    
    if (g_spectrum.initial_sensing_enable != 0)
    {

        if (sum_intf != NULL)
        {
            free(sum_intf);
        }

        if (pkt_buf != NULL)
        {
            free(pkt_buf);
        }

        if (p_spec_conf != NULL)
        {
            free(p_spec_conf);
        }

        if (sum_power != NULL)
        {
            free(sum_power);
        }

        for (i = 0; i < 2; i++)
        {
            if (input_r[i] != NULL)
            {
                free(input_r[i]);
            }

            if (input_i[i] != NULL)
            {
                free(input_i[i]);
            }
        }

        for (i = 0; i < g_spectrum.num_freq; i++)
        {
            if (scan_result[i] != NULL)
            {
                free(scan_result[i]);
            }
        }

        if (scan_result != NULL)
        {
            free(scan_result);
        }

        release_spectrum_ed_scan();

        free(p_sensing);
    }

    return uw_ret;

}

#else

static int set_rrh_config_tdd(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    int ad_cmd_time = 5000;
    int ret = 0;
    
    /** change back to TDD */
    a_value[0] = 1;
    a_value[1] = 1;
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_MODE, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_MODE error");
        return 1;
    } 

    /** set working freq */
    ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(g_spectrum_config.working_freq));
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters DEFAULT_WORKING_FREQ error\n");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        g_spectrum_config.working_freq = 230000;
    }
    
    a_value[0] = g_spectrum_config.working_freq;
    a_value[1] = g_spectrum_config.working_freq;
    
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_FREQ, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_FREQ error");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }
    
    FLOG_INFO("Frequency point Set to %d\n", g_spectrum_config.working_freq);

#ifndef _OLD_RRH_

    a_value[0] = g_trans_control_param.rru_rx_len;                
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_RX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }
    
    a_value[0] = g_trans_control_param.rru_tx_len;                
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_TX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    
    }

    a_value[0] = g_trans_control_param.enable_agc;                
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_AGC, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_AGC error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;

    }
    
    if (g_trans_control_param.enable_agc == 0)
    {
        a_value[0] = g_trans_control_param.chan1_rx_pgc;
        a_value[1] = g_trans_control_param.chan2_rx_pgc;
        uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CH_RX_PGC, 2, a_value);
        if (0 != uw_ret)
        {
            FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_CH_RX_PGC error! uw_ret = %d\r\n", uw_ret);
            //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
            return 1;
        }
    }

#endif

    ret = get_global_param ("ADVANCED_CMD_TIME", & ( ad_cmd_time ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
        ad_cmd_time = 5000;
    }
    
    a_value[0] = ad_cmd_time;
    a_value[1] = ad_cmd_time;
    
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_DL_PRESEND_TIME, 1, a_value);
    
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for DL_PRESEND_TIME error");
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }

#ifndef _OLD_RRH_

    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_QUERY_POWER, 0, NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for QUERY_POWER error! uw_ret = %d\r\n", uw_ret);
    
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    }

#endif

    return 0;

}



int set_rrh_config(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    int i, j;
    //int ad_cmd_time = 5000;
    int ret = 0;
    float * p_sensing = NULL;
    
    struct init_scan_result ** scan_result = NULL;

    if (g_spectrum.initial_sensing_enable == 0)
    {
        ret = connect_agent();

        if (0 != ret)
        {
            FLOG_ERROR("Call wma_agent_process error! uw_ret = %d\r\n", ret);

            return 1;
        }

        g_rrh_config_state = SET_RRH_CONFIG_STEP_3;

    }else
    {

        g_rrh_config_state = SET_RRH_CONFIG_STEP_1;

        sum_intf = (float *) malloc ( sizeof(float) * 21);

        if (sum_intf == NULL)
        {
            FLOG_ERROR("malloc intf buffer memory error\n");
            return 1;
        }

        pkt_buf = (short *) malloc (FIX_DATA_LEN * 2 * ANTENNA_NUM * 2 * PACKET_PER_SYMBOL * 4);

        if (pkt_buf == NULL)
        {
            FLOG_ERROR("malloc packet buffer memory error\n");
            return 1;
        }

        sum_power = (float *) malloc (1024 * 4);

        if (sum_power == NULL)
        {
            FLOG_ERROR("malloc summary power memory error\n");
            return 1;
        }

        p_spec_conf = (struct spectrum_scan_state *) malloc (sizeof (struct spectrum_scan_state));

        if (p_spec_conf == NULL)
        {
            FLOG_ERROR("malloc struct spectrum_scan_state memory error\n");
            return 1;
        }

        for (i = 0; i < 2; i++)
        {
            input_r[i] = (float *)malloc (2048 * 4);
            input_i[i] = (float *)malloc (2048 * 4);

            if ( (input_r[i] == NULL) || (input_i[i]) == NULL)
            {
                FLOG_ERROR("malloc packet buffer memory error\n");
                return 1;
            }
        }

        scan_result = (struct init_scan_result **)malloc(g_spectrum.num_freq * sizeof(void *));

        if (scan_result == NULL)
        {
            FLOG_ERROR("malloc scan");
            return 1;
        }

        for (i = 0; i < g_spectrum.num_freq; i++)
        {
//            printf("g_spectrum.num_freq=%d\n", g_spectrum.num_freq);
            scan_result[i] = (struct init_scan_result *)malloc(sizeof(struct init_scan_result));

            if (scan_result[i] == NULL)
            {
                FLOG_ERROR("malloc scan");
                return 1;
            }

            scan_result[i]->average_power = 999999;
            memset(scan_result[i]->active_band, 999999, sizeof(float) * 21);
        }

        init_spectrum_ed_scan();

        p_sensing = (float *)malloc(TRANS_AGENT_SENSING_NUM * 4);
        
        if (NULL == p_sensing)
        {
            FLOG_ERROR("malloc p_sensing error");
            return 1;
        }

    }

    u_int8_t uc_stop_flag =0;
    while (g_rrh_state_exe_num)
    {         
        switch (g_rrh_config_state)
        {
             case SET_RRH_CONFIG_STEP_1:
                 a_value[0] = 3;
                 a_value[1] = 3;
                 uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_MODE, 2, a_value);
                 if (0 != uw_ret)
                 {
                     FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_MODE error");                     
                     //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                     uc_stop_flag = 1;
                     break;                     
                 }
/*
                a_value[0] = g_trans_control_param.is_rxfrm_len;                
                uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_RX_LEN, 1, a_value);
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);

                    //uc_stop_flag = 1;
                    break;
                }
*/
                g_rrh_config_state = SET_RRH_CONFIG_STEP_2;
                break;

             case SET_RRH_CONFIG_STEP_2:

                pthread_create (&phy_is_thread, NULL, phy_is_process, NULL);

                for (j = 0; j< g_spectrum.num_freq; j++)
                {
                    /** set freq */
                    a_value[0] = g_spectrum.frequency[j];
                    a_value[1] = g_spectrum.frequency[j];
                    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_FREQ, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_FREQ error");
                        //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                        continue;   
                    }
                    
                    /** open channel */
                    a_value[0] = 1;
                    a_value[1] = 1;
                    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_FLAG, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_OPEN error");
                        //g_rrh_config_state = SET_RRH_CONFIG_STEP_1;
                        continue;  
                    }
                    
                    FLOG_INFO("Scaning %dhz\n", g_spectrum.frequency[j]);

#ifdef ENABLE_NEW_TRANS_GREN
                    ethrru_symbol_rx_reset();
#endif
                    uw_ret = init_spectrum_sensing(&g_spectrum, scan_result[j], g_spectrum.frequency[j]);

                    if (uw_ret != 0)
                    {
                        FLOG_ERROR("init_spectrum_sensing failed");
                        continue;  
                    }

                    /** close channel */
                    a_value[0] = 0;
                    a_value[1] = 0;
                    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_FLAG, 2, a_value);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_CLOSE error");
                        continue;                        
                    }

                    FLOG_INFO("Scan finish .");

                    ret = generate_result(scan_result[j], p_sensing);
                    if (ret != 0)
                    {
                        FLOG_ERROR("Call generate_result error");
                        continue;
                    }

                }

                if (phy_is_thread != 0)
                {
                    pthread_cancel (phy_is_thread);
                    pthread_join (phy_is_thread, NULL);
                }

                /** send result to NMS server */
                /** wait for result from NMS server and put into g_spectrum_config*/
                /** g_spectrum_config->str_active_band must be a string here! */
                /** Use sprintf if needed */
                /*just send message to agent when connected to Agent*/

                ret = connect_agent();
                if (0 != ret)
                {
                    FLOG_ERROR("Call wma_agent_process error! uw_ret = %d\r\n", ret);
    
                    return 1;
                }

                ret =  wait_agent();
                if (1 == ret)
                {
                    FLOG_INFO("Send msg to agent for SPECTRUM");

                    struct trans_send_msg_to_agent st_msg_info;
                    
                    st_msg_info.f_callback = NULL;
                    st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
                    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
                    st_msg_info.uw_resp_len = TRANS_AGENT_SENSING_NUM * 4;
                    st_msg_info.p_resp_msg = p_sensing;
                    st_msg_info.p_reqs_msg = "spectrum";
                    
                    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
                    /*Error*/
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
                        //return TRANS_FAILD;
                    }
                    
                    #if 0
                    uw_ret = trans_send_msg_to_agent(TRANS_SEND_SPECTRUM_TO_AGENT, p_sensing);
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call trans_send_msg_to_agent for SPECTRUM error");

                    }  
                    #endif

                    ret = get_global_param("INTERFERENCE_ACTIVE", (void *)(g_spectrum_config.str_active_band));

                    if (ret != 0)
                    {
                        FLOG_ERROR("get initial active error");
                        return 1;
                    }

                    ret = generate_dtsinfo(&g_spectrum, &g_spectrum_config);
                    if (ret != 0)
                    {
                        FLOG_ERROR("generate dtsinfo error");
                        return 1;
                    }
                }
                
                #if 0
                /** change back to TDD */
                a_value[0] = 1;
                a_value[1] = 1;
                uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_CHANNEL_MODE, 2, a_value);
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_msg_to_rrh for CHANNEL_MODE error");
                    uc_stop_flag = 1;
                    break;
                }
                #endif

                if (SET_RRH_CONFIG_STEP_1 != g_rrh_config_state)
                {
                    g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
                }

                break;
                    
            case SET_RRH_CONFIG_STEP_3:

                ret = set_rrh_config_tdd();
                if (ret != 0)
                {
                    FLOG_ERROR("Config RRH FOR TDD error");
                    g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
                    
                    break;
                }
                
                FLOG_INFO("Config RRH FOR TDD OK .");
                /*Config End ---While End*/
                uc_stop_flag = 1;

                break;

            default:
                
                FLOG_ERROR("g_rrh_config_state = %d error", g_rrh_config_state);
                g_rrh_config_state = 0;

                break;            

        }

        if (0 != uc_stop_flag)
        {
            break;        
        }
        
        g_rrh_state_exe_num--;

    }
    
    if (g_spectrum.initial_sensing_enable != 0)
    {

        if (sum_intf != NULL)
        {
            free(sum_intf);
        }

        if (pkt_buf != NULL)
        {
            free(pkt_buf);
        }

        if (p_spec_conf != NULL)
        {
            free(p_spec_conf);
        }

        if (sum_power != NULL)
        {
            free(sum_power);
        }

        for (i = 0; i < 2; i++)
        {
            if (input_r[i] != NULL)
            {
                free(input_r[i]);
            }

            if (input_i[i] != NULL)
            {
                free(input_i[i]);
            }
        }

        for (i = 0; i < g_spectrum.num_freq; i++)
        {
            if (scan_result[i] != NULL)
            {
                free(scan_result[i]);
            }
        }

        if (scan_result != NULL)
        {
            free(scan_result);
        }

        release_spectrum_ed_scan();

        free(p_sensing);
    }

    return uw_ret;

}

#endif

#endif

int get_sensing_param (struct spectrum_para * param, struct spectrum_config_para * config)
{
    int ret = 0;
    int i = 0;
    char tmp_string[4096];

    char search[] = ",";
    char *token = NULL;

    ret = get_global_param ("INITIAL_SENSING_ENABLE", & (param->initial_sensing_enable));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters INITIAL_SENSING_ENABLE error\n");
        return 1;
    }

    ret = get_global_param ("INITIAL_SENSING_POLICY", & (param->initial_sensing_policy));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters INITIAL_SENSING_POLICY error\n");
        return 1;
    }

    ret = get_global_param ("INITIAL_SENSING_THD", & (param->initial_sensing_thd));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters INITIAL_SENSING_THD error\n");
        return 1;
    }

    ret = get_global_param ("PERIODIC_SENSING_DURATION", & (param->periodic_duration));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PERIODIC_SENSING_DURATION error\n");
        return 1;
    }

    ret = get_global_param ("SCAN_STEP", & (param->loop_step));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SCAN_STEP error\n");
        return 1;
    }

    ret = get_global_param ("INIT_SCAN_FREQ_NUM", & (param->num_freq));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters INIT_SCAN_FREQ_NUM error\n");
        return 1;
    }

    if ( param->num_freq > MAX_FREQ_SET_NUM )
    {
        FLOG_WARNING ("INIT_SCAN_FREQ_NUM to large, set to %d\n", MAX_FREQ_SET_NUM);
        param->num_freq = MAX_FREQ_SET_NUM;
    }

    ret = get_global_param ("CALIBRATE_DIGITAL_POWER0", & (param->cali_digi_pwr[0]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER0 error\n");
        return 1;
    }

    ret = get_global_param ("CALIBRATE_DIGITAL_POWER1", & (param->cali_digi_pwr[1]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER1 error\n");
        return 1;
    }

    ret = get_global_param ("IS_NFFT", & (param->is_nfft));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SCAN_STEP error\n");
        return 1;
    }

    ret = get_global_param ("FCH_BITMAP", & (param->fch_bitmap));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FCH_BITMAP error\n");
        return 1;
    }

    ret = get_global_param ("UL_BITMAP", & (param->ul_bitmap));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters UL_BITMAP error\n");
        return 1;
    }

    if (param->num_freq > 0)
    {
        param->frequency = (unsigned int *) malloc(param->num_freq * sizeof(unsigned int));

        ret = get_global_param ("SCAN_CENTER_FREQ", tmp_string);

        if (ret != 0)
        {
            FLOG_WARNING ("get parameters SCAN_CENTER_FREQ error\n");
            return 1;
        }

        token = strtok(tmp_string, search);

        if (token != NULL)
        {
            param->frequency[0] = atoi(token);
        }else
        {
            FLOG_WARNING ("get parameters SCAN_CENTER_FREQ error\n");
            return 1;
        }

        for(i = 1; i < param->num_freq; i++)
        {
            token = strtok(NULL, search);
            if (token != NULL)
            {
                param->frequency[i] = atoi(token);
            }else
            {
                FLOG_WARNING ("get parameters SCAN_CENTER_FREQ error\n");
                return 1;
            }
        }
    }

    /** get default configuration parameters */
    ret = get_global_param ("DEFAULT_WORKING_FREQ", & (config->working_freq));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters DEFAULT_WORKING_FREQ error\n");
        return 1;
    }

    ret = get_global_param ("SCAN_BW", & (param->carrier_bw));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SCAN_BW error\n");
        return 1;
    }

    ret = get_global_param ("SCAN_POWER", & (param->carrier_pwr));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SCAN_POWER error\n");
        return 1;
    }


    return 0;
}

int init_sensing_process (void)
{
    int ret = 0;

    ret = get_sensing_param(&g_spectrum, &g_spectrum_config);

    if (ret != 0)
    {
        FLOG_ERROR("Get sensing parameters error");
        return 1;
    }

#ifndef _SIM_RRH_

    ret = set_rrh_config();

    if (ret != 0)
    {
        FLOG_ERROR("set rrh config error");
        return 1;
    }
    
#endif

    return 0;
}

int init_sensing_release (void)
{

    if (g_spectrum.frequency != NULL)
    {
        free(g_spectrum.frequency);
    }

    return 0;
}

int32_t init_spectrum_sensing(struct spectrum_para * param, struct init_scan_result *spectrum_result, int freq_num)
{
    struct sensing_node * node = NULL;
    int loop_step = 0;
    struct rru_data_param rru_param;
    struct queue_msg node_msg;
    float temp = 0;
    int count = 0;

    node_msg.my_type = dump_en_id;

    memset(sum_power, 0, sizeof(float) * 1024);
    memset(sum_intf, 0, sizeof(float) * 21);
    memset(p_spec_conf, 0, sizeof (struct spectrum_scan_state));

    p_spec_conf->cali_ana_pwr[0] = param->cali_ana_pwr[0];
    p_spec_conf->cali_ana_pwr[1] = param->cali_ana_pwr[1];

    p_spec_conf->cali_digi_pwr[0] = param->cali_digi_pwr[0];
    p_spec_conf->cali_digi_pwr[1] = param->cali_digi_pwr[1];

    p_spec_conf->is_nfft = param->is_nfft;

    for (loop_step = 0; loop_step < param->loop_step; loop_step++)
    {
        node = malloc(sizeof (struct sensing_node));

        if (node == NULL)
        {
            FLOG_ERROR("malloc node memory error\n");
            continue;
        }

        memset( &rru_param, 0, sizeof(struct rru_data_param) );
        ethrru_symbol_rx((char *)(node->buf), 1, &rru_param);

        node->dgain[0] = rru_param.dgain[1];
        node->dgain[1] = rru_param.dgain[2];
        node->param = param;
        node->freq_num = freq_num;
        node->spectrum_result = spectrum_result;

        node_msg.p_buf = (void *) node;

        if (wmrt_enqueue (dump_en_id, &node_msg,
                sizeof(struct queue_msg)) == -1)
        {
            FLOG_WARNING ("ENQUEUE ERROR\n");
        }

    }

    /* average the power and interefrence */
    for (count =0; count < 1024; count++)
    {
        temp = temp + sum_power[count];
    }

    spectrum_result->average_power = (temp/1024)/loop_step;

    for (count=0; count < 21; count++)
    {
        spectrum_result->active_band[count] = sum_intf[count]/loop_step;
    }

    return 0;
}

void * phy_is_process (void * arg __attribute__ ((unused)))
{
    int i, l, k, count;
//    int temp_count;
    int err_code;
    float factor = 0;
    int i_tmp_buf = 0;

    short * p_buf = NULL;

    struct queue_msg node_msg;

    struct sensing_node * node = NULL;

    node_msg.my_type = dump_de_id;

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

    while(1)
    {
        if (wmrt_dequeue (dump_de_id, &node_msg,
                sizeof(struct queue_msg)) == -1)
        {
            FLOG_WARNING ("ENQUEUE ERROR\n");
        }

        node = (struct sensing_node *)(node_msg.p_buf);

        p_buf = (short *)node->buf;
        i_tmp_buf = 0;

        p_spec_conf->dgain[0] = node->dgain[0];
        p_spec_conf->dgain[1] = node->dgain[1];

//        printf("dgain0 = %f\n", p_spec_conf->dgain[0]);
//        printf("dgain1 = %f\n", p_spec_conf->dgain[1]);

        //DO_DUMP(DUMP_IR_RX_ALL_RAW_ID, p_spec_conf->dgain[0], "degain 0", 0);
        //DO_DUMP(DUMP_IR_RX_ALL_RAW_ID, p_spec_conf->dgain[1], "degain 1", 0);
        //DO_DUMP(DUMP_IR_RX_ALL_RAW_ID, node->freq_num, "freq num", 0);
        DO_DUMP(DUMP_IR_RX_ALL_RAW_ID, -1, p_buf, FIX_DATA_LEN * 4);

        for (l = 0; l < PACKET_PER_SYMBOL; l += ANTENNA_NUM)
        {
            for (i = 0; i < ANTENNA_NUM; i++)
            {
                for (k = 0; k < (FIX_DATA_LEN >> 1); k += 2)
                {
                    input_r[i][i_tmp_buf + (k >> 1)] = (float)p_buf[k];
                    input_i[i][i_tmp_buf + (k >> 1)] = (float)p_buf[k + 1];
                }

                p_buf += (FIX_DATA_LEN >> 1);
            }
            i_tmp_buf += SAMPLE_PER_PACKET;
        }

         /* power adjustment processing for antenna 0 */
        factor = exp10f( (-1.0F) * p_spec_conf->dgain[0] / 20.0F);

        for( i=0; i < FIX_DATA_LEN; i++ )
        {
            input_r[0][i] = input_r[0][i]*factor;
            input_i[0][i] = input_i[0][i]*factor;
        }

        /* processing for antenna 1 */
        factor = exp10f( (-1.0F) * p_spec_conf->dgain[1]/20.0F)
                          * sqrt(p_spec_conf->cali_digi_pwr[0] / p_spec_conf->cali_digi_pwr[1]);

        for( i=0; i < FIX_DATA_LEN; i++ )
        {
            input_r[1][i] = input_r[1][i]*factor;
            input_i[1][i] = input_i[1][i]*factor;
        }

        err_code = spectrum_ed_scan(input_r[0], input_i[0], p_spec_conf);

        if (err_code)
        {
           FLOG_ERROR(" spectrum_ed_scan: Error in spectrum scanning accumulation!\n");
        }

        /* accumulated average power */

        for (count =0; count < 1024; count++)
        {
            sum_power[count] = sum_power[count] + p_spec_conf->power[count];
        }
        /* accumulated interference information */

        for (count =0; count < 21; count++)
        {
            sum_intf[count] = sum_intf[count] + p_spec_conf->intf[count];
        }
    }

    return 0;
}


int32_t generate_result(struct init_scan_result *spectrum_result, float * p_sensing)
{
    static u_int32_t uw_index = 0;
//    u_int32_t i = 0;
    u_int32_t j = 0;

    p_sensing[uw_index] = spectrum_result->average_power;
    
    uw_index++;
    
    if (TRANS_AGENT_SENSING_NUM <= uw_index)
    {
        FLOG_ERROR("1 uw_index error, p_sensing[%d] is full!\n", uw_index);
    
        return 1;
    }
    
    for (j = 0; j < 24; j++)
    {
        p_sensing[uw_index] = spectrum_result->active_band[j];
    
        uw_index++;
        if (TRANS_AGENT_SENSING_NUM < uw_index)
        {
            FLOG_ERROR("2 uw_index error, p_sensing[%d] is full!\n", uw_index);
            return 1;
        }
    }
/*
    for (i = uw_index - 25; i < uw_index; i++)
    {
        FLOG_DEBUG("p_sensing[%d] = %f.\n", i, p_sensing[i]);    
    }
*/
    //(void) spectrum_result;
    return 0;
}



int32_t generate_dtsinfo(struct spectrum_para *spec_para, struct spectrum_config_para *g_spec_conf)
{
    int32_t i;
    u_int8_t count = 0;
    u_int8_t ul_count = 0;
    u_int8_t dl_num = 0;
    u_int8_t ul_num = 0;
    int32_t fch_bitmap[6];
    int32_t ul_bitmap[35];
    int32_t active_band[21];

/* calculate the DL active forbidden channel number */


    for(i=5; i >=0; i--)
    {
        fch_bitmap[i] = atoi(&spec_para->fch_bitmap[i]);
        spec_para->fch_bitmap[i] = 0;
    }

    for (i= 0; i<3; i++)
    {
        if (fch_bitmap[2*i] == 0)
        {
            dl_num += 12;
        }
        if (fch_bitmap[2*i+1] == 0)
        {
            dl_num += 2;
        }
    }
   /* calculate the interference subband number */
    for(i=21; i >= 0; i--)
    {
       active_band[i] = atoi(&g_spec_conf->str_active_band[i]);
       g_spec_conf->str_active_band[i]=0;
    }

    for (i=0; i<21; i++)
    {
        if (active_band[i] == 1)
        {
            count++;
        }
    }

    /* calculate the DL Unused subchannel */
    g_spec_conf->dl_unused_subch = count*2 + dl_num;
    
    /* calculate UL available subchannel */
    ul_count = (u_int8_t)floor((756-36*count)/(6*4));
    /* calculate the active forbidden channel for UL */
    for(i=35; i>=0; i--)
    {
        ul_bitmap[i] = atoi(&spec_para->ul_bitmap[i]);
        spec_para->ul_bitmap[i]=0;
    }

    for (i=0; i<ul_count; i++)
    {
        if(ul_bitmap[i] == 1)
        {
            ul_num += 1;
        }
    }

    /* calculate the UL Unused subchannel */
    g_spec_conf->ul_unused_subch = 31 - ul_num;

    return 0;
}

int cfg_update_ps_duration(int type, int len, void * p_buf)
{
    g_spectrum.periodic_duration = *( (int *)p_buf);

    (void)type;
    (void)len;

    return 0;
}


