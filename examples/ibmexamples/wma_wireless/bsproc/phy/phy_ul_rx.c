/* ----------------------------------------------------------------------------

 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: phy_ul_rx.c

 Function: The BS receiver processing main thread



 Change Activity:



 Date             Description of Change                            By

 -----------      ---------------------                            --------


 ----------------------------------------------------------------------------

 PROLOG END TAG zYx                                                           */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "flog.h"
#include "bs_cfg.h"
#include "queue_util.h"
#include "phy_ul_rx_interface.h"
#include "phy_ul_rx_common.h"
#include "phy_ul_rx.h"

#include "phy_ul_power_adjust.h"
#include "phy_ul_zonepermutation.h"
#include "phy_ul_wk_generator.h"
#include "phy_ul_ofdmademodul.h"
#include "phy_ul_subcarderandom.h"
#include "phy_ul_channelest.h"
#include "phy_ul_demuxofdmsym.h"
#include "phy_ul_demodulation.h"
#include "phy_ul_fec_decoding.h"
#include "viterbicore.h"
#include "phy_ul_enum.h"
#include "phy_periodic_sensing.h"

#include "prephy_proc.h"
#include "adapter_bs_ul_map_interface.h"
#include "adapter_bs_interface_phy.h"
#include "adapter_bs_ul_transform_mac.h"
#include "adapter_ul_stube_test.h"
#include "mac_frame.h"
#include "bs_debug.h"
#include "monitor_proc.h"
#include "mac_amc.h"

/* turn on/off dump according to DUMP_PHY_UL_RX setting */
#ifndef DUMP_PHY_UL_RX

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"
#include "prof_util.h"

//#define BROADCAST_CID  0

//#define _DUMP_BER_ERROR_FRAME_

#define BROADCAST_CID_FRAG  0

#define BERSUM_THD 50000
int phy_enable_dump = 0;

//#define _BER_QPSK_TEST_
//#define _BER_16QAM_TEST_
//#define _BER_64QAM_TEST_

//#define _MY_PER_TEST_
#define PRE_FRAME (500)

#ifdef _BS_MONITOR_
#define DTS_NUM (23)
u_int32_t temp_len = 0;
#endif

#define CREATELINKNODE(phead, ppreviousnode, pcurrentnode)  if(phead == NULL)\
{\
    phead = pcurrentnode;\
}  if(ppreviousnode != NULL)\
    ppreviousnode->next = pcurrentnode;\
    ppreviousnode = pcurrentnode

#define MAX_SLOTSYMBOL_SIZE (1152*3) 

#define MAX_DEMUX_SIZE (1680) 


#define _DEBUG_

#ifdef _DEBUG_

#include "phy_ul_rx_utility.h"

#endif


int32_t phy_ul_deinit_rrusymbol (struct phy_ul_rru_symbol *p_rru_symbol)

{

    if (p_rru_symbol != NULL)
    {
        if (p_rru_symbol->symbol_i != NULL)
        {
            free (p_rru_symbol->symbol_i);
            p_rru_symbol->symbol_i = NULL;
        }

        if (p_rru_symbol->symbol_q != NULL)
        {
            free (p_rru_symbol->symbol_q);
            p_rru_symbol->symbol_q = NULL;
        }

        free (p_rru_symbol);
        p_rru_symbol = NULL;

    }

    return SUCCESS_CODE;

}

/**----------------------------------------------------------------------------

 Function:    phy_ul_rx_div1()



 Description: To receive and decode dl sub frame by slot_symbol for the case

 of one receiver antenna.



 Parameters:

 Input-  [struct phy_ul_rx_syspara *para]  The pointer refer to the

 struct of system parameters.

 [const uint32_t in_que_id] The id of input queue.



 Output- [const uint32_t out_que_id] The id of output queue.



 Return Value:

 0       Success

 150     Error



 ----------------------------------------------------------------------------

 LOG END TAG zYx

 */

int32_t phy_ul_rx_div1 (struct phy_ul_rx_syspara *para,
        const u_int32_t in_que_id, const u_int32_t out_que_id)

{
    float a_r[756 * 3], a_i[756 * 3];
    float b_r[756 * 3], b_i[756 * 3];

    u_int32_t rotation_posindex[31 * 5];//155
    u_int32_t pilot_allocation[31 * 24 * 5];//3720
    u_int32_t data_allocation[31 * 48 * 5];//6720
    u_int32_t ranging_allocation[72 * 6]; //432
    int32_t wk[900];
    u_int32_t ava_subch[1];
    u_int8_t ul_perscan_flag = 0;

    float *subcarderandom_r, *subcarderandom_i;
    float *ofdmdemod_r, *ofdmdemod_i;
    float *h_r, *h_i;
    float demuxdata_r[756 * 3], demuxdata_i[756 * 3];
    float demuxchn_r[756 * 3], demuxchn_i[756 * 3];
    float demuxpilot_r[756 * 3], demuxpilot_i[756 * 3];
    float pilotchn_r[756 * 3], pilotchn_i[756 * 3];
    float snr_est[35 * 5], noise_est[35 * 5];
#ifndef ENABLE_VSX
    float softbit[48 * 31 * 6 * 5];
#else
    float softbit[48 * 31 * 6 * 5 * 2]; //On power, 2 ANTENNA's demodulation is combined,
                                        //the bottom half of this array is used for 2nd ANTENNA
#endif

    float temp_power[PS_NFFT], average_power[PS_NFFT];

    float temp_buffer_r[6][48 * 31 * 5], temp_buffer_i[6][48 * 31 * 5];

    float dgain[2], out_pwr_r[1088 * 3], out_pwr_i[1088 * 3];

    struct phy_ul_rru_symbol *p_rrusymbol_ant1 = NULL;
    struct ul_frame_ie *p_map = NULL;
    struct block_data_ie *block_cur = NULL;
    struct slot_symbol_ie *slotsym_cur = NULL;
    struct union_burst_ie * p_burst_cur = NULL;
    struct phy_ul_fec_para *fecpara;
    fecpara = (struct phy_ul_fec_para *) &para->fec_para;

    struct phy_dts_info *dtspara;
    dtspara = (struct phy_dts_info *) calloc (1, sizeof(struct phy_dts_info));//should chnage to static memory
    if(dtspara == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_rx_div1\n");
        return -1;
    }
    u_int32_t err_code;
    u_int32_t i, ul_num;
    u_int32_t slotsym_num[1];
    u_int32_t length_data, length_pilot;
    u_int32_t count = 0;
    u_int32_t counter[1];
    u_int16_t block_length = 0;
    u_int32_t tmp_len = 0;

    int cinr_avg = 0;
    float snr_avg;

    struct queue_msg *p_msg_in = my_malloc (sizeof(struct queue_msg));
    struct queue_msg *p_msg_out = my_malloc (sizeof(struct queue_msg));

    if (p_msg_in == NULL || p_msg_out == NULL) 
    {
        FLOG_ERROR("malloc failed in phy_ul_rx_div1\n");
        return -1;
    }
    if (para == NULL)
    {
        FLOG_FATAL ("phy_ul_rx: The pointer refer to sys para is null!\n");
        return ERROR_CODE;
    }

    p_msg_in->my_type = in_que_id;
    p_msg_out->my_type = out_que_id;

    ul_num = para->symbolnum_per_ul_frame;
    slotsym_num[0] = 0;

    /* getting ul_map message */

    counter[0] = 0;
    
    while (1)
    {
        if (wmrt_dequeue (in_que_id, p_msg_in, sizeof(struct queue_msg)) == -1)
        {
            FLOG_FATAL ("phy_ul_rx: In PHY layer DEQUEUE ERROR\n");
            return ERROR_CODE;
        }
        p_rrusymbol_ant1 = (struct phy_ul_rru_symbol *) p_msg_in->p_buf;

        if (p_rrusymbol_ant1 == NULL || p_rrusymbol_ant1->symbol_i == NULL
                || p_rrusymbol_ant1->symbol_q == NULL)
        {
            FLOG_FATAL (
                    "phy_ul_rx: Error in received frm rru symbol, antenna1!\n");
            return ERROR_CODE;
        }

        ul_perscan_flag = p_rrusymbol_ant1->ul_perscan_flag;
        dgain[0] = p_rrusymbol_ant1->dgain;

        /* enter intro perodic sensing */
        if (ul_perscan_flag == 1)
        {

            if (p_rrusymbol_ant1->ul_subframe_endflag == 1)
            {
                if (p_rrusymbol_ant1->ul_map != NULL)
                {
                    free (p_rrusymbol_ant1->ul_map);
                }
            }

            if (p_rrusymbol_ant1->ul_subframe_endflag != 3)
            {
                /* power adjust with Dgain */
                err_code = phy_ul_power_adjust_single (para,
                        p_rrusymbol_ant1->symbol_i, p_rrusymbol_ant1->symbol_q,
                        dgain, out_pwr_r, out_pwr_i);
                if (err_code)
                {
                    FLOG_WARNING (
                            " spectrum_per_scan: Error in power adjustment!\n");
                }

                err_code = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);

                if (err_code)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }

                /* spectrum sensing */
                err_code = spectrum_per_scan (para, out_pwr_r, out_pwr_i,
                        temp_power);
                if (err_code)
                {
                    FLOG_WARNING (
                            " spectrum_per_scan: Error in perodical sensing!\n");
                }
                
                /* accumulation */
                for (i = 0; i < PS_NFFT; i++)
                {
                    average_power[i] = average_power[i] + temp_power[i];
                }
                /*
                 err_code = phy_ul_deinit_rrusymbol(p_rrusymbol_ant1);

                 if(err_code)
                 {
                 FLOG_FATAL("phy_ul_rx: Error in rru symbol deinit!\n");
                 return ERROR_CODE;
                 }
                 */
            }
            else
            {

                /* power adjustment with Dgain */
                err_code = phy_ul_power_adjust_single (para,
                        p_rrusymbol_ant1->symbol_i, p_rrusymbol_ant1->symbol_q,
                        dgain, out_pwr_r, out_pwr_i);
                if (err_code)
                {
                    FLOG_WARNING (
                            " spectrum_per_scan: Error in power adjustment!\n");
                }

                /* spectrum sensing */
                err_code = spectrum_per_scan (para, out_pwr_r, out_pwr_i,
                        temp_power);

                if (err_code)
                {
                    FLOG_WARNING (
                            " spectrum_per_scan: Error in perodical sensing!\n");
                }

                /* accumulation */
                for (i = 0; i < PS_NFFT; i++)
                {
                    average_power[i] = average_power[i] + temp_power[i];
                }

                err_code = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);

                if (err_code)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }

                for (i = 0; i < PS_NFFT; i++)
                {
                    average_power[i] = average_power[i] * 0.2;//supposed the slotsymbol number is 5
                }

                p_map = (struct ul_frame_ie *) malloc (
                        sizeof(struct ul_frame_ie));
                if (p_map == NULL ) 
                {
                     FLOG_ERROR("malloc failed in phy_ul_rx_div1\n");
                     return -1;
                }

                memset (p_map, 0, sizeof(struct ul_frame_ie));

                p_map->p_total_buffer_out = (void *) malloc (sizeof(float)
                        * PS_NFFT);
                if (p_map->p_total_buffer_out == NULL ) 
                {
                     FLOG_ERROR("malloc failed in phy_ul_rx_div1\n");
                     return -1;
                }
                memcpy (p_map->p_total_buffer_out, average_power, sizeof(float)
                        * PS_NFFT);

                /* calculate the updated dts information */
                err_code = per_dts_update (para, average_power, dtspara);

                if (err_code)
                {
                    FLOG_ERROR (
                            " per_dts_update: Error in interference update!\n");
                }

                p_map->dts_info = (void *) calloc (1,
                        sizeof(struct phy_dts_info));
                if(p_map->dts_info == NULL)
                {
                    FLOG_ERROR("malloc faled in phy_ul_rx_div1\n");
                    return -1;
                }
                memcpy (p_map->dts_info, dtspara, sizeof(struct phy_dts_info));

                p_map->pscan_resflag = 1; //1--perodical sensing result; 0--normal data
                /* enque the Pscan results and updated dts_info */
                p_msg_out->p_buf = p_map;

                /* enqueue to MAC */
                if (wmrt_enqueue (out_que_id, p_msg_out,
                        sizeof(struct queue_msg)) == -1)
                {
                    FLOG_ERROR (
                            "periodical spectrum sensing: Error in write UL sub_frame!\n");
                    return ERROR_CODE;
                }

                memset (average_power, 0, sizeof(float) * PS_NFFT);
                p_map = NULL;
            }
        }
        else
        {
            /* getting ul_map */
            if (p_map == NULL)
            {
                parse_ul_map (p_rrusymbol_ant1->ul_map,
                        p_rrusymbol_ant1->ul_map_len, &p_map);
            }
            if (slotsym_cur == NULL)
            {
                slotsym_cur = p_map->p_slot_header;
            }
            else
            {
                slotsym_cur = slotsym_cur->next;
            }
            if (slotsym_cur == NULL || slotsym_cur->slot_offset
                    != p_rrusymbol_ant1->symbol_offset)
            {
                FLOG_WARNING ("skip frame symbol_offset %d \n",
                        p_rrusymbol_ant1->symbol_offset);
                //free (p_map);
                p_map = NULL;
                slotsym_cur = NULL;
                continue;
            }

            /* Update system parameters */
            para->frame_index = p_rrusymbol_ant1->frame_index;
            para->symbol_offset = p_rrusymbol_ant1->symbol_offset;
            para->last_slotsymbol = p_rrusymbol_ant1->ul_subframe_endflag;
            //  dgain[0] = p_rrusymbol_ant1->dgain;


            if (p_rrusymbol_ant1->symbol_num != para->symbol_per_slot)

            /* Now only support the case where the number of symbols in
             one rru symbol is the same as that in one slot symbol */
            {
                FLOG_ERROR (
                        "phy_ul_rx: The number of ofdma symbols in one rru symbol is not correct\n");
                return ERROR_CODE;
            }

            err_code = phy_ul_wk_generator (para, wk);
            /*
             rru_phydts = (struct phy_dts_info *)p_rrusymbol_ant1->p_dts_info;

             if(rru_phydts != NULL)
             {
             memcpy(para->active_band, rru_phydts->active_band, 21*sizeof(int8_t));
             para->ul_unused_subch = rru_phydts->ul_unused_subch;
             }
             */
            /* zone permutation */
            err_code = phy_ul_zonepermutation (para, para->active_band,
                    ava_subch, rotation_posindex, pilot_allocation,
                    data_allocation, ranging_allocation);

            para->avasubch_num = ava_subch[0];
            length_data = para->avasubch_num * 48;
            length_pilot = para->avasubch_num * 24;

            while (1) //process
            {
                /* Receive one rru symbol from framework */
                if (para->symbol_offset != 0)
                {
                    if (wmrt_dequeue (in_que_id, p_msg_in,
                            sizeof(struct queue_msg)) == -1)
                    {
                        FLOG_FATAL ("phy_ul_rx: In PHY layer DEQUEUE ERROR\n");
                        return ERROR_CODE;
                    }

                    p_rrusymbol_ant1
                            = (struct phy_ul_rru_symbol *) p_msg_in->p_buf;
                    if (p_map == NULL)
                    {
                        parse_ul_map (p_rrusymbol_ant1->ul_map,
                                p_rrusymbol_ant1->ul_map_len, &p_map);
                    }
                    if (slotsym_cur == NULL)
                    {
                        slotsym_cur = p_map->p_slot_header;
                    }
                    else
                    {
                        slotsym_cur = slotsym_cur->next;
                    }
                    if (slotsym_cur->slot_offset
                            != p_rrusymbol_ant1->symbol_offset || slotsym_cur
                            == NULL)
                    {
                        p_map = NULL;
                        slotsym_cur = NULL;
                    }

                    if (p_rrusymbol_ant1 == NULL || p_rrusymbol_ant1->symbol_i
                            == NULL || p_rrusymbol_ant1->symbol_q == NULL)
                    {
                        FLOG_FATAL (
                                "phy_ul_rx: Error in received frm rru symbol!\n");
                        return ERROR_CODE;
                    }

                    /* Update system parameters */
                    para->frame_index = p_rrusymbol_ant1->frame_index;
                    para->symbol_offset = p_rrusymbol_ant1->symbol_offset;
                    para->last_slotsymbol
                            = p_rrusymbol_ant1->ul_subframe_endflag;
                    //     dgain[0] = p_rrusymbol_ant1->dgain;

                    if (p_rrusymbol_ant1->symbol_num != para->symbol_per_slot)

                    /* Now only support the case where the number of symbols in
                     one rru symbol is the same as that in one slot symbol */
                    {
                        FLOG_ERROR (
                                "phy_ul_rx: The number of ofdma symbols in one rru symbol is not correct\n");
                        return ERROR_CODE;
                    }
                }

                /* power adjustment with dgain */
                err_code = phy_ul_power_adjust_single (para,
                        p_rrusymbol_ant1->symbol_i, p_rrusymbol_ant1->symbol_q,
                        dgain, out_pwr_r, out_pwr_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_power_adjust_single: Error in power adjustment for div1!\n");
                    return err_code;
                }

                err_code = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);
                if (err_code)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return err_code;
                }

                /* Compensation according to CFO estimated by sync thread */
                //printf("begin ofdma demodulation!\n");
                ofdmdemod_r = a_r;
                ofdmdemod_i = a_i;
                err_code = phy_ul_ofdmademodul (para, out_pwr_r, out_pwr_i,
                        ofdmdemod_r, ofdmdemod_i);

                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_ofdmademodul: Error in ofdm_demodulation!\n");
                    return err_code;
                }


				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT1_R_ID, 0, ofdmdemod_r, 756*3);
				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT1_I_ID, 0, ofdmdemod_i, 756*3);

                /*
                 err_code = phy_ul_deinit_rrusymbol(p_rrusymbol_ant1);
                 if(err_code)
                 {
                 FLOG_FATAL("phy_ul_rx: Error in rru symbol deinit!\n");
                 return err_code;
                 }
                 */
                subcarderandom_r = b_r;
                subcarderandom_i = b_i;
                err_code = phy_ul_subcarderandom (para, wk, ofdmdemod_r,
                        ofdmdemod_i, subcarderandom_r, subcarderandom_i);
                if (err_code)
                {
                    FLOG_ERROR ("E008_rx: Error in subcarderandom!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT1_R_ID, 0, subcarderandom_r, 756*3);
				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT1_I_ID, 0, subcarderandom_i, 756*3);

                /* begin demuxofdm */
                err_code = phy_ul_demuxofdmsym (para, subcarderandom_r,
                        subcarderandom_i,
                        data_allocation + length_data * count,
                        rotation_posindex, slotsym_num, demuxdata_r,
                        demuxdata_i);
                if (err_code)
                {
                    FLOG_ERROR ("phy_ul_demuxofdmsym: Error in demuxofdm!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT1_R_ID, 0, demuxdata_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT1_I_ID, 0, demuxdata_i, 48*para->usedsubch_num);

                /* begin channel estimation */
                h_r = a_r;
                h_i = a_i;
                err_code = phy_ul_single_chanlest (para, subcarderandom_r,
                        subcarderandom_i, h_r, h_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_single_channelest: Error in channel_estimation!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT1_R_ID, 0, h_r, 756*3);
				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT1_I_ID, 0, h_i, 756*3);

                /* demuxofdm channel_est result*/
                err_code = phy_ul_demuxofdmsym (para, h_r, h_i, data_allocation
                        + length_data * count, rotation_posindex, slotsym_num,
                        demuxchn_r, demuxchn_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxofdmsym: Error in demux channel estimation results!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT1_R_ID, 0, demuxchn_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT1_I_ID, 0, demuxchn_i, 48*para->usedsubch_num);

                /* demux pilot */
                length_pilot = para->avasubch_num * 24;
                err_code = phy_ul_demuxpilot (para, subcarderandom_r,
                        subcarderandom_i, pilot_allocation + length_pilot
                                * count, rotation_posindex, slotsym_num,
                        demuxpilot_r, demuxpilot_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxpilot: Error in demux channel estimation results!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT1_R_ID, 0, demuxpilot_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT1_I_ID, 0, demuxpilot_i, 24*para->usedsubch_num);

                /* demux channel_est result on pilot */
                err_code = phy_ul_demuxpilot (para, h_r, h_i, pilot_allocation
                        + length_pilot * count, rotation_posindex, slotsym_num,
                        pilotchn_r, pilotchn_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxpilot: Error in demux channel estimation results on pilot!\n");
                    return err_code;
                }

				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_R_ID, 0, pilotchn_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_I_ID, 0, pilotchn_i, 24*para->usedsubch_num);

                block_cur = slotsym_cur->p_block_header;
                while (block_cur != NULL)
                {
                    if (block_cur->is_used == 1)
                    {
                        /* insert data subcarrier related data */
                        block_length = block_cur->subchannel_num * 48;
                        //rintf("subchannel_offset = %d\n", block_cur->subchannel_offset);
                        for (i = 0; i < block_length; i++)
                        {
                            temp_buffer_r[0][i] = demuxdata_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_i[0][i] = demuxdata_i[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_r[1][i] = demuxchn_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_i[1][i] = demuxchn_i[48
                                    * block_cur->subchannel_offset + i];
                        }
                        memcpy (temp_buffer_r[2] + block_cur->data_offset,
                                temp_buffer_r[0], sizeof(float) * block_length);
                        memcpy (temp_buffer_i[2] + block_cur->data_offset,
                                temp_buffer_i[0], sizeof(float) * block_length);
                        memcpy (temp_buffer_r[3] + block_cur->data_offset,
                                temp_buffer_r[1], sizeof(float) * block_length);
                        memcpy (temp_buffer_i[3] + block_cur->data_offset,
                                temp_buffer_i[1], sizeof(float) * block_length);

                        /* insert pilot related data */
                        block_length = block_cur->subchannel_num * 24;
                        for (i = 0; i < block_length; i++)
                        {
                            temp_buffer_r[0][i] = demuxpilot_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_i[0][i] = demuxpilot_i[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_r[1][i] = pilotchn_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_i[1][i] = pilotchn_i[24
                                    * block_cur->subchannel_offset + i];

                        }
                        memcpy (temp_buffer_r[4] + block_cur->pilot_offset,
                                temp_buffer_r[0], sizeof(float) * block_length);
                        memcpy (temp_buffer_i[4] + block_cur->pilot_offset,
                                temp_buffer_i[0], sizeof(float) * block_length);
                        memcpy (temp_buffer_r[5] + block_cur->pilot_offset,
                                temp_buffer_r[1], sizeof(float) * block_length);
                        memcpy (temp_buffer_i[5] + block_cur->pilot_offset,
                                temp_buffer_i[1], sizeof(float) * block_length);
                    }

                    block_cur = block_cur->next;
                }

                slotsym_num[0]++;
                count++;
                para->symbol_offset = para->symbol_offset
                        + para->symbol_per_slot;
                counter[0] = count;
                if (slotsym_cur->next == NULL)
                {
                    slotsym_num[0] = 0;
                    count = 0;
                    counter[0] = 0;
                    break;
                }
            }

            p_burst_cur = p_map->p_burst_header;

            while (p_burst_cur != NULL)
            {

                err_code = phy_ul_noise_est (p_burst_cur, temp_buffer_r[4]
                        + p_burst_cur->pilot_offset, temp_buffer_i[4]
                        + p_burst_cur->pilot_offset, temp_buffer_r[5]
                        + p_burst_cur->pilot_offset, temp_buffer_i[5]
                        + p_burst_cur->pilot_offset, noise_est, snr_est);

                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_noise_est: Error in noise estimation processing!\n");
                    return ERROR_CODE;
                }


				DO_DUMP(DUMP_PHY_NOISE_ESTI_ANT1_ID, 0, noise_est, 50);
				DO_DUMP(DUMP_PHY_SNR_ESTI_ANT1_ID, 0, snr_est, 50);

				DO_DUMP(DUMP_PHY_BURST_DATA_ANT1_R_ID, 0, p_burst_cur->data_offset+temp_buffer_r[2], p_burst_cur->slots_num*48);
				DO_DUMP(DUMP_PHY_PILOT_ANT1_R_ID, 0, p_burst_cur->pilot_offset+temp_buffer_r[4], p_burst_cur->slots_num*24);
				DO_DUMP(DUMP_PHY_BURST_PILOT_CHANNEL_ANT1_R_ID, 0, p_burst_cur->pilot_offset+temp_buffer_r[5], p_burst_cur->slots_num*24);

#ifdef _AMC_
                snr_avg = 0.0;
                for (i=0; i<p_burst_cur->slots_num;i++)
                {
                    snr_avg = snr_avg + snr_est[i];
                }

                cinr_avg = round( snr_avg/p_burst_cur->slots_num );
		cinr_avg = (int)(10 * log10f((float)cinr_avg));
		p_burst_cur->cinr = cinr_avg;
#endif




                err_code = phy_ul_demodulation_single (p_burst_cur,
                        temp_buffer_r[2] + p_burst_cur->data_offset,
                        temp_buffer_i[2] + p_burst_cur->data_offset,
                        temp_buffer_r[3] + p_burst_cur->data_offset,
                        temp_buffer_i[3] + p_burst_cur->data_offset, noise_est,
                        softbit);

                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_demodulation_single: Error in demodulation!\n");
                    return ERROR_CODE;
                }

                if (p_burst_cur->code_id == 0)
                    tmp_len = 2;
                else
                    if (p_burst_cur->code_id == 2)
                        tmp_len = 4;
                    else
                        tmp_len = 6;

				DO_DUMP(DUMP_PHY_DEMODULATION_ID, 0, softbit, p_burst_cur->slots_num*48*tmp_len);

                err_code = phy_ul_fec_decoding (para, p_burst_cur, softbit);

                if (err_code)
                {
                    FLOG_ERROR ("phy_ul_rx_div1: Error in data fec decoding!\n");
                    return ERROR_CODE;
                }

				DO_DUMP(DUMP_PHY_FECDECODE_DERAND_ID, 0, p_map->p_total_buffer_out, p_burst_cur->slots_num*48*tmp_len/2);


                p_burst_cur = p_burst_cur->next;

            }


            p_msg_out->p_buf = p_map;
            /* enqueue to MAC */
            if (wmrt_enqueue (out_que_id, p_msg_out, sizeof(struct queue_msg))
                    == -1)
            {
                FLOG_FATAL ("phy_ul_rx_div1: Error in write UL sub_frame!\n");
                return ERROR_CODE;
            }

            p_map = NULL;
            slotsym_cur = NULL;

        }
    }
    return SUCCESS_CODE;

}

/**----------------------------------------------------------------------------

 Function:    phy_ul_rx_div2()


 Description: To receive and decode dl sub frame by slot_symbol for the case

 of two receiver antenna.


 Parameters:

 Input-  [struct phy_ul_rx_syspara *para]  The pointer refer to the

 struct of system parameters.

 [const u_int32_t in_que_id1] The id of the first input

 queue.

 [const u_int32_t in_que_id2] The id of the second input

 queue.



 Output- [const uint32_t out_que_id1] The id of the first

 output queue.

 [const uint32_t out_que_id2] The id of the second

 output queue.



 Return Value:

 0       Success

 150     Error



 ----------------------------------------------------------------------------

 LOG END TAG zYx                                                            */

int32_t phy_ul_rx_div2 (struct phy_ul_rx_syspara *para,
        const u_int32_t in_que_id1, const u_int32_t in_que_id2,
        const u_int32_t out_que_id1, const u_int32_t out_que_id2)

{
    u_int32_t err_code;
    if (para == NULL)
    {
        FLOG_FATAL ("phy_ul_rx: The pointer refer to sys para is null!\n");
        return ERROR_CODE;
    }

    switch (para->mimo_mode)
    {
        case CDD: /* CDD Mode */

            err_code = phy_ul_rx_cdd (para,
            //  dts_para,
                    in_que_id1, in_que_id2, out_que_id1);

            if (err_code)
            {
                FLOG_ERROR ("phy_ul_rx_cdd: Error in CDD receiving!\n");
                return ERROR_CODE;
            }
            break;
        case STCA: /*STC MatrixA Mode */
            err_code = phy_ul_rx_stca (para,
            // dts_para,
                    in_que_id1, in_que_id2, out_que_id1);

            if (err_code)
            {
                FLOG_ERROR ("phy_ul_rx_stca: Error in stca receiving!\n");
                return ERROR_CODE;
            }
            break;
        case STCB: /*STC MatrixB Mode */
            err_code = phy_ul_rx_stcb (para,
            // dts_para,
                    in_que_id1, in_que_id2, out_que_id1, out_que_id2);

            if (err_code)
            {
                FLOG_ERROR ("phy_ul_rx_stcb: Error in stcb receiving!\n");
                return ERROR_CODE;
            }
            break;
        default:
            FLOG_WARNING ("phy_ul_rx_div2: Unsupported mimo mode!\n");
            return ERROR_CODE;

    }

    return SUCCESS_CODE;
}

/*--------------------------------------------------------------------------------------------------

 Function: phy_ul_rx_cdd

 Description: single antenna 1x1 ofdma symbol receiver processing for one slotsymbol

 Parameters:

 Input-  [const phy_ul_rx_syspara *para]  The system parameters



 Return Value:

 0       Success

 150     Error

 ---------------------------------------------------------------------------------------------------

 LOG END TAG zYx                                                                                   */

int32_t phy_ul_rx_cdd (struct phy_ul_rx_syspara *para,
        const u_int32_t in_que_id1, const u_int32_t in_que_id2,
        const u_int32_t out_que_id1)

{


#if defined(SSE2OPT) || defined (VSXOPT)
    float a_ant1_r[756 * 3] __attribute__ ((aligned (128))), a_ant1_i[756 * 3] __attribute__ ((aligned (128)));
    float a_ant2_r[756 * 3] __attribute__ ((aligned (128))), a_ant2_i[756 * 3] __attribute__ ((aligned (128)));
    float b_ant1_r[756 * 3] __attribute__ ((aligned (128))), b_ant1_i[756 * 3] __attribute__ ((aligned (128)));
    float b_ant2_r[756 * 3] __attribute__ ((aligned (128))), b_ant2_i[756 * 3] __attribute__ ((aligned (128)));
#else
    float a_ant1_r[756 * 3], a_ant1_i[756 * 3];
    float a_ant2_r[756 * 3], a_ant2_i[756 * 3];
    float b_ant1_r[756 * 3], b_ant1_i[756 * 3];
    float b_ant2_r[756 * 3], b_ant2_i[756 * 3];
#endif

    u_int32_t rotation_posindex[31 * 5];//155
    u_int32_t pilot_allocation[31 * 24 * 5];//3720
    u_int32_t data_allocation[31 * 48 * 5];//6720
    u_int32_t ranging_allocation[72 * 6]; //432
    int32_t wk[900];
    u_int32_t ava_subch[1];
    u_int8_t ul_perscan_flag = 0;
    
    float *subcarderandom_ant1_r, *subcarderandom_ant1_i;
    float *subcarderandom_ant2_r, *subcarderandom_ant2_i;
    float *ofdmdemod_ant1_r, *ofdmdemod_ant1_i;
    float *ofdmdemod_ant2_r, *ofdmdemod_ant2_i;
    float *h_ant1_r, *h_ant1_i;
    float *h_ant2_r, *h_ant2_i;
    float demuxdata_ant1_r[756 * 3] =
    { 0 };
    float demuxdata_ant1_i[756 * 3] =
    { 0 };
    float demuxdata_ant2_r[756 * 3], demuxdata_ant2_i[756 * 3];
    float demuxchn_ant1_r[756 * 3], demuxchn_ant1_i[756 * 3];
    float demuxchn_ant2_r[756 * 3], demuxchn_ant2_i[756 * 3];
    float demuxpilot_ant1_r[756 * 3], demuxpilot_ant1_i[756 * 3];
    float demuxpilot_ant2_r[756 * 3], demuxpilot_ant2_i[756 * 3];
    float pilotchn_ant1_r[756 * 3], pilotchn_ant1_i[756 * 3];
    float pilotchn_ant2_r[756 * 3], pilotchn_ant2_i[756 * 3];
    float snr_ant1_est[35 * 5], noise_ant1_est[35 * 5];
    float snr_ant2_est[35 * 5], noise_ant2_est[35 * 5];
    float softbit[48 * 31 * 6 * 5];

    float dgain[2];
#ifdef SSE2OPT
    float out_pwr_ant0_r[1088 * 3]  __attribute__ ((aligned (128))), out_pwr_ant0_i[1088 * 3]  __attribute__ ((aligned (128)));
    float out_pwr_ant1_r[1088 * 3]  __attribute__ ((aligned (128))), out_pwr_ant1_i[1088 * 3]  __attribute__ ((aligned (128)));
#else
    float out_pwr_ant0_r[1088 * 3], out_pwr_ant0_i[1088 * 3];
    float out_pwr_ant1_r[1088 * 3], out_pwr_ant1_i[1088 * 3];
#endif

    float temp_power[PS_NFFT], average_power[PS_NFFT];

    float temp_buffer_ant1_r[6][48 * 31 * 6],
            temp_buffer_ant1_i[6][48 * 31 * 6];
    float temp_buffer_ant2_r[6][48 * 31 * 6],
            temp_buffer_ant2_i[6][48 * 31 * 6];

    struct phy_ul_rru_symbol *p_rrusymbol_ant1, *p_rrusymbol_ant2;
    void * p_map_org = NULL;
    struct ul_frame_ie *p_map = NULL;
    struct block_data_ie *block_cur = NULL;
    struct slot_symbol_ie *slotsym_cur = NULL;
    struct union_burst_ie * p_burst_cur = NULL;
    struct phy_ul_fec_para *fecpara;
    fecpara = (struct phy_ul_fec_para *) &para->fec_para;

    //    struct phy_dts_info *rru_phydts;
    struct phy_dts_info *dtspara;
    dtspara = (struct phy_dts_info *) calloc (1, sizeof(struct phy_dts_info));//should chnage to static memory
    if(dtspara == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_rx_add\n");
        return -1;
    }

    u_int32_t err_code, err_code_ant1, err_code_ant2;
    u_int32_t i;
    u_int32_t slotsym_num[1];
    u_int32_t length_data, length_pilot;
    u_int32_t count = 0;
    u_int16_t block_length = 0;
    u_int32_t tmp_len = 0;

    int total_burst = 0;
    int burst_idx = 0;

    int sync_for_new_frame = 0;
    int cinr_avg = 0;
    float snr_ant1_avg, snr_ant2_avg;

    struct queue_msg *p_msg_in1 = my_malloc (sizeof(struct queue_msg));
    struct queue_msg *p_msg_in2 = my_malloc (sizeof(struct queue_msg));
    struct queue_msg *p_msg_out1 = my_malloc (sizeof(struct queue_msg));
    if(p_msg_in1 == NULL || p_msg_in2 == NULL || p_msg_out1 == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_rx_add\n");
        return -1;
    }
    if (para == NULL)
    {
        FLOG_FATAL ("phy_ul_rx: The pointer refer to sys para is null!\n");
        return ERROR_CODE;
    }

    p_msg_in1->my_type = in_que_id1;
    p_msg_in2->my_type = in_que_id2;
    p_msg_out1->my_type = out_que_id1;
    
    memset (average_power, 0, sizeof(float) * PS_NFFT);

    slotsym_num[0] = 0;

#if 0
    int32_t max_frmnum = 500000;
    int32_t mix_errfrm = 200;
    float ber_thd = 0.1;
    int32_t bersum = 0;
    int32_t loop_count = 0;
    int32_t ber_count = 0;
    int32_t total_bit = 0;
    int32_t err_frm = 0;

    double ber;
    int source[2400];
    FILE *fp_rxin;
    fp_rxin = fopen("source.dat","r+t");
    for (ber_count = 0; ber_count<2400; ber_count++)
    {
        fscanf(fp_rxin, "%d\n", source+ber_count);
    }
    fclose(fp_rxin);

    bersum =0;
    loop_count = 0;

#endif

#ifdef _BER_TEST_

#define DISCARD_WINDOW (99)

    u_int32_t bersum = 0;
    u_int32_t total_bersum = 0;
    u_int32_t loop_count = 0;
    u_int32_t ber_count = 0;
    u_int32_t total_bit = 0;
    u_int32_t err_frm = 0;
    u_int32_t total_err_frm = 0;
    u_int32_t total_err_blk = 0;

    u_int32_t berfrm_loop = 0;

    u_int32_t blk_num = 0;
    u_int32_t total_blk_num = 0;

    double ber;
    double bler;

    int fake_ulmap_uiuc = 0;
    int fake_ulmap_duration = 0;

    int ret;
/*
    ret = get_global_param ("FAKE_ULMAP_DURATION", &fake_ulmap_duration);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_DURATION error\n");
    }

    ret = get_global_param ("FAKE_ULMAP_UIUC", &fake_ulmap_uiuc);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_UIUC error\n");
    }
*/

    fake_ulmap_duration = g_fake_ulmap_duration;
    fake_ulmap_uiuc = g_fake_ulmap_uiuc;


    int framebit_num = 0;
    int max_frm_num = 0;
    int min_frm_num = 0;

    if (fake_ulmap_uiuc == 1)
    {
        framebit_num = fake_ulmap_duration * 48;
        max_frm_num = 15000;
        min_frm_num = 500;
    }else if (fake_ulmap_uiuc == 2)
    {
        framebit_num = fake_ulmap_duration * 72;
        max_frm_num = 7500;
        min_frm_num = 500;
    }else if (fake_ulmap_uiuc == 3)
    {
        framebit_num = fake_ulmap_duration * 96;
        max_frm_num = 7500;
        min_frm_num = 500;
    }else if (fake_ulmap_uiuc == 5||fake_ulmap_uiuc == 4)
    {
        framebit_num = fake_ulmap_duration * 144;
        max_frm_num = 5000;
        min_frm_num = 500;
    }else if (fake_ulmap_uiuc == 6)
    {
        framebit_num = fake_ulmap_duration * 192;
        max_frm_num = 5000;
        min_frm_num = 500;
    }else if (fake_ulmap_uiuc == 7)
    {
        framebit_num = fake_ulmap_duration * 216;
        max_frm_num = 5000;
        min_frm_num = 500;
    } else
    {
        FLOG_WARNING("unsupported UIUC %d, use QPSK1/2n", fake_ulmap_uiuc);
        framebit_num = fake_ulmap_duration * 48;
        max_frm_num = 15000;
        min_frm_num = 200;
    }
    printf("framebit_num = %d\n", framebit_num);
    printf("fake_ulmap_uiuc = %d\n", fake_ulmap_uiuc);

#endif

#ifdef _BS_MONITOR_

    struct phy_hook_ps_result hook_ps_result;
    struct phy_hook_chan_quality_result hook_chan;

    int32_t temp_loop;

    int32_t chanbuf_len = 0;
//    float temp_slotnum[1];

    int32_t constellationbuf_len = 0;
    struct phy_hook_const_result hook_constellation;

#ifdef _BER_TEST_
    int32_t berbuf_len = 40;
    u_int32_t hook_ber[10];
#endif

#endif

    while (1)
    {

RE_SYNC_TAG:

        if (wmrt_dequeue (in_que_id1, p_msg_in1, sizeof(struct queue_msg))
                == -1)
        {
            FLOG_FATAL ("phy_ul_rx: In PHY layer DEQUEUE ERROR\n");
            return ERROR_CODE;
        }

        p_rrusymbol_ant1 = (struct phy_ul_rru_symbol *) p_msg_in1->p_buf;

        /* receive the rru symbol from 2nd antenna */
        if (wmrt_dequeue (in_que_id2, p_msg_in2, sizeof(struct queue_msg))
                == -1)
        {
            FLOG_FATAL ("phy_ul_rx: In PHY layer DEQUEUE ERROR, 2nd antenna\n");
            return ERROR_CODE;
        }
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_NO_DEQUE ));
        p_rrusymbol_ant2 = (struct phy_ul_rru_symbol *) p_msg_in2->p_buf;

        if (p_rrusymbol_ant1 == NULL || p_rrusymbol_ant1->symbol_i == NULL
                || p_rrusymbol_ant1->symbol_q == NULL)
        {
            FLOG_FATAL (
                    "phy_ul_rx: Error in received frm rru symbol, antenna1!\n");
            return ERROR_CODE;
        }

        if (p_rrusymbol_ant2 == NULL || p_rrusymbol_ant2->symbol_i == NULL
                || p_rrusymbol_ant2->symbol_q == NULL)
        {
            FLOG_FATAL (
                    "phy_ul_rx: Error in received frm rru symbol, antenna2!\n");
            return ERROR_CODE;
        }

        if (sync_for_new_frame == 1)
        {
            if (p_rrusymbol_ant1->ul_subframe_endflag != 1)
            {
                err_code = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);

                if (err_code)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }

                err_code = phy_ul_deinit_rrusymbol (p_rrusymbol_ant2);

                if (err_code)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }

                continue;
            }else
            {
                sync_for_new_frame = 0;
            }
        }

        ul_perscan_flag = p_rrusymbol_ant1->ul_perscan_flag;
        dgain[0] = p_rrusymbol_ant1->dgain;
        dgain[1] = p_rrusymbol_ant2->dgain;
        para->dgain0 = p_rrusymbol_ant1->dgain;
        para->dgain1 = p_rrusymbol_ant2->dgain;
      //  FLOG_DEBUG("Trace dgain[0]= %f\n", dgain[0]);
        //  FLOG_DEBUG("dgain[1]= %f\n", dgain[1]);

	    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_PREPROC ));
        if (ul_perscan_flag == 1)
        {

            if (p_rrusymbol_ant1->ul_subframe_endflag == 1)
            {
                if (p_rrusymbol_ant1->ul_map != NULL)
                {
                    free (p_rrusymbol_ant1->ul_map);

                    p_rrusymbol_ant1->ul_map = NULL;
                }

                err_code_ant1 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);
                err_code_ant2 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant2);
                
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }

            }
            else
                if (p_rrusymbol_ant1->ul_subframe_endflag == 2)
                {
                    /* power adjustment with Dgain */
                    err_code = phy_ul_power_adjust (para,
                            p_rrusymbol_ant1->symbol_i,
                            p_rrusymbol_ant1->symbol_q,
                            p_rrusymbol_ant2->symbol_i,
                            p_rrusymbol_ant2->symbol_q, dgain, out_pwr_ant0_r,
                            out_pwr_ant0_i, out_pwr_ant1_r, out_pwr_ant1_i);

                    if (err_code)
                    {
                        FLOG_WARNING (
                                " spectrum_per_scan: Error in power adjustment!\n");
                    }

                    err_code_ant1 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);
                    err_code_ant2 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant2);

                    if (err_code_ant1 || err_code_ant2)
                    {
                        FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                        return ERROR_CODE;
                    }

                    /* spectrum sensing */
                    err_code = spectrum_per_scan (para, out_pwr_ant0_r,
                            out_pwr_ant0_i, temp_power);
                    if (err_code)
                    {
                        FLOG_WARNING (
                                " spectrum_per_scan: Error in perodical sensing!\n");
                    }

                    /* accumulation */
                    for (i = 0; i < PS_NFFT; i++)
                    {
                        average_power[i] = average_power[i] + temp_power[i];
                    }
                }
                else
                    if (p_rrusymbol_ant1->ul_subframe_endflag == 3)
                    {

                        /* power adjustment with Dgain */
                        err_code = phy_ul_power_adjust (para,
                                p_rrusymbol_ant1->symbol_i,
                                p_rrusymbol_ant1->symbol_q,
                                p_rrusymbol_ant2->symbol_i,
                                p_rrusymbol_ant2->symbol_q, dgain,
                                out_pwr_ant0_r, out_pwr_ant0_i, out_pwr_ant1_r,
                                out_pwr_ant1_i);

                        if (err_code)
                        {
                            FLOG_WARNING (
                                    " spectrum_per_scan: Error in power adjustment!\n");
                        }

                        /* spectrum sensing */
                        err_code = spectrum_per_scan (para, out_pwr_ant0_r,
                                out_pwr_ant0_i, temp_power);
                        if (err_code)
                        {
                            FLOG_WARNING (
                                    " spectrum_per_scan: Error in perodical sensing!\n");
                        }

                        /* accumulation */
                        for (i = 0; i < PS_NFFT; i++)
                        {
                            average_power[i] = average_power[i] + temp_power[i];
                        }

                        err_code_ant1 = phy_ul_deinit_rrusymbol (
                                p_rrusymbol_ant1);
                        err_code_ant2 = phy_ul_deinit_rrusymbol (
                                p_rrusymbol_ant2);

                        if (err_code_ant1 || err_code_ant2)
                        {
                            FLOG_FATAL (
                                    "phy_ul_rx: Error in rru symbol deinit!\n");
                            return ERROR_CODE;
                        }

                        for (i = 0; i < PS_NFFT; i++)
                        {
                            average_power[i] = average_power[i] / 12.0;//supposed the slotsymbol number is 5
                        }

                        p_map_org = (struct ul_frame_ie *) malloc (
                                sizeof(struct ul_frame_ie));
                        if(p_map_org == NULL )
                        { 
                            FLOG_ERROR("malloc failed in phy_ul_rx_add\n");
                            return -1;
                        }

                        memset (p_map_org, 0, sizeof(struct ul_frame_ie));

                        p_map = (struct ul_frame_ie *)p_map_org;

                        p_map->p_total_buffer_out = (void *) malloc (
                                sizeof(float) * PS_NFFT);

                        if(p_map->p_total_buffer_out == NULL)
                        {
                            FLOG_ERROR("malloc failed in phy_ul_rx_add\n");
                            return -1;
                        }
                        memcpy (p_map->p_total_buffer_out, average_power,
                                sizeof(float) * PS_NFFT);

						DO_DUMP(DUMP_PHY_AVER_POWER_ID, 0, average_power, PS_NFFT);


                        /* calculate the updated dts information */
                        err_code
                                = per_dts_update (para, average_power, dtspara);

                        if (err_code)
                        {
                            FLOG_ERROR (
                                    " per_dts_update: Error in interference update!\n");
                        }

						DO_DUMP(DUMP_PHY_UL_RX_ACTIVEBAND_ID, 0, dtspara, 0);


                        p_map->dts_info = (void *) calloc (1,
                                sizeof(struct phy_dts_info));
                        if(p_map->dts_info == NULL)
                        {
                            FLOG_ERROR("malloc failed in phy_ul_rx_add\n");
                            return -1;
                        }
                        memcpy (p_map->dts_info, dtspara,
                                sizeof(struct phy_dts_info));

#ifdef _BS_MONITOR_
                        /*
                         int32_t psbuf_len = (PS_NFFT + DTS_NUM)*4;//PS_NFFT*4+23;
                         float temp_dst[23];
                         float hook_ps[(PS_NFFT + DTS_NUM)];
                         int32_t temp_loop;
                         */
                        /*
                         float *hook_ps;
                         hook_ps = (float *)malloc(sizeof(float) *(PS_NFFT+ DTS_NUM));
                         */

                        /* convert char to float type */
                        hook_ps_result.frm_num = (float)para->frame_index;
                        for(temp_loop =0; temp_loop<21; temp_loop++)
                        {
                            hook_ps_result.active_band[temp_loop] = (float)dtspara->active_band[temp_loop];
                        }

                        hook_ps_result.dl_unused_subch = (float)dtspara->dl_unused_subch;
                        hook_ps_result.ul_unused_subch = (float)dtspara->ul_unused_subch;
                        memcpy(hook_ps_result.power, average_power, PS_NFFT * sizeof(float));
                        hook_ps_result.noise_figure = para->noise_figure;
                        hook_ps_result.noise_maxhold = para->noise_maxhold;

                        hook_debug_trace(HOOK_PS_INFO_IDX, &hook_ps_result, sizeof(struct phy_hook_ps_result), 1);

#ifdef _HOOK_BS_DUMP_
                        FILE *fp_hookps;
                        fp_hookps = fopen("hook_ps.dat","w+t");
                        dump_ulrx_ffloat(fp_hookps, hook_ps, PS_NFFT+DTS_NUM);
                        fclose(fp_hookps);
#endif
                        //    free(hook_ps);
#endif

                        p_map->pscan_resflag = 1; //1--perodical sensing result; 0--normal data

                        /* enque the Pscan results and updated dts_info */
                        p_msg_out1->p_buf = p_map;

                        /* enqueue to MAC */
                        if (wmrt_enqueue (out_que_id1, p_msg_out1,
                                sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR (
                                    "periodical spectrum sensing: Error in write UL sub_frame!\n");
                            return ERROR_CODE;
                        }
                        memset (average_power, 0, sizeof(float) * PS_NFFT);
                        p_map = NULL;
                        p_map_org = NULL;
                    }
                    else
                    {
                        err_code_ant1 = phy_ul_deinit_rrusymbol (
                                p_rrusymbol_ant1);
                        err_code_ant2 = phy_ul_deinit_rrusymbol (
                                p_rrusymbol_ant2);

                        if (err_code_ant1 || err_code_ant2)
                        {
                            FLOG_FATAL (
                                    "phy_ul_rx: Error in rru symbol deinit!\n");
                            return ERROR_CODE;
                        }

                        FLOG_FATAL ("Wrong ul_subframe_endflag in PHY UL Rx\n");
                    }
        }
        else
        {
            /* getting ul_map */
            if (p_map == NULL)
            {
                parse_ul_map (p_rrusymbol_ant1->ul_map,
                        p_rrusymbol_ant1->ul_map_len, &p_map);
            }
            if (slotsym_cur == NULL)
            {
                slotsym_cur = p_map->p_slot_header;
            }
            else
            {
                slotsym_cur = slotsym_cur->next;
            }

            if (slotsym_cur == NULL || slotsym_cur->slot_offset
                    != p_rrusymbol_ant1->symbol_offset)
            {
                FLOG_WARNING ("skip frame\n");

                FLOG_DEBUG ("skip %p, %p, %d, %d \n", p_map, slotsym_cur,
                        p_rrusymbol_ant1->symbol_offset,
                        slotsym_cur->slot_offset);

                p_map_org = p_map;
                free_adapter_frame (&p_map_org);
                p_map = NULL;
                p_map_org = NULL;
                slotsym_cur = NULL;

                slotsym_num[0] = 0;
                count = 0;
                sync_for_new_frame = 1;

//                continue;
                goto RE_SYNC_TAG;
                //                 goto  new_frame;
            }

            /* Update system parameters */
            para->frame_index = p_rrusymbol_ant1->frame_index;
            para->symbol_offset = p_rrusymbol_ant1->symbol_offset;
            para->last_slotsymbol = p_rrusymbol_ant1->ul_subframe_endflag;

            /*if (p_rrusymbol_ant1->frame_index != p_rrusymbol_ant2->frame_index)
             {
             printf("phy_ul_rx_div2: Lost Syn for the two antennas \n");
             return ERROR_CODE;
             }*/

            if (p_rrusymbol_ant1->symbol_num != para->symbol_per_slot)

            /* Now only support the case where the number of symbols in
             one rru symbol is the same as that in one slot symbol */
            {
                FLOG_ERROR (
                        "phy_ul_rx: The number of ofdma symbols in one rru symbol is not correct!\n");
                return ERROR_CODE;
            }

            err_code = phy_ul_wk_generator (para, wk);
            /*
             rru_phydts = (struct phy_dts_info *)p_rrusymbol_ant1->p_dts_info;
             if(rru_phydts != NULL)
             {
             memcpy(para->active_band, rru_phydts->active_band, 21*sizeof(int8_t));
             para->ul_unused_subch = rru_phydts->ul_unused_subch;
             }
             */
            /* zone permutation */
            err_code = phy_ul_zonepermutation (para, para->active_band,
                    ava_subch, rotation_posindex, pilot_allocation,
                    data_allocation, ranging_allocation);
            para->avasubch_num = ava_subch[0];
            // printf("para->avasubch_num = %d\n", para->avasubch_num);
            length_data = para->avasubch_num * 48;
            length_pilot = para->avasubch_num * 24;


			DO_DUMP(DUMP_PHY_ROTATE_POS_ID, 0, rotation_posindex, para->usedsubch_num*5);
			DO_DUMP(DUMP_PHY_PILOT_ALLOC_ID, 0, pilot_allocation, para->avasubch_num*24*5);
			DO_DUMP(DUMP_PHY_DATA_ALLOC_ID, 0, data_allocation, para->avasubch_num*48*5);
			DO_DUMP(DUMP_PHY_RNG_ALLOC_ID, 0, ranging_allocation, para->ranging_subch*72);
			
	    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_PREPROC ));

            while (1) //process
            {
                /* Receive one rru symbol from framework */
	    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_BURST_DEQUE ));

                if (para->symbol_offset != 0)
                {
                    if (wmrt_dequeue (in_que_id1, p_msg_in1,
                            sizeof(struct queue_msg)) == -1)
                    {
                        FLOG_FATAL ("phy_ul_rx: In PHY layer DEQUEUE ERROR\n");
                        return ERROR_CODE;
                    }
                    p_rrusymbol_ant1
                            = (struct phy_ul_rru_symbol *) p_msg_in1->p_buf;

                    /* receive the rru symbol from 2nd antenna */
                    if (wmrt_dequeue (in_que_id2, p_msg_in2,
                            sizeof(struct queue_msg)) == -1)
                    {
                        FLOG_FATAL (
                                "phy_ul_rx: In PHY layer DEQUEUE ERROR, 2nd antenna\n");
                        return ERROR_CODE;
                    }
                    p_rrusymbol_ant2
                            = (struct phy_ul_rru_symbol *) p_msg_in2->p_buf;

                    /* getting ul_map */
                    if (p_map == NULL)
                    {
                        parse_ul_map (p_rrusymbol_ant1->ul_map,
                                p_rrusymbol_ant1->ul_map_len, &p_map);
                    }
                    if (slotsym_cur == NULL)
                    {
                        slotsym_cur = p_map->p_slot_header;
                    }
                    else
                    {
                        slotsym_cur = slotsym_cur->next;
                    }
                    if (slotsym_cur == NULL || slotsym_cur->slot_offset
                            != p_rrusymbol_ant1->symbol_offset)
                    {
                        FLOG_WARNING ("skip frame\n");

                        FLOG_DEBUG ("skip %p, %p, %d, %d \n", p_map, slotsym_cur,
                                       p_rrusymbol_ant1->symbol_offset,
                                       slotsym_cur->slot_offset);

                        p_map_org = p_map;
                        free_adapter_frame (&p_map_org);
                        //free (p_map);
                        p_map = NULL;
                        p_map_org = NULL;
                        slotsym_cur = NULL;

                        slotsym_num[0] = 0;
                        count = 0;
                        sync_for_new_frame = 1;

                        goto RE_SYNC_TAG;
                    }

                    if (p_rrusymbol_ant1 == NULL || p_rrusymbol_ant1->symbol_i
                            == NULL || p_rrusymbol_ant1->symbol_q == NULL)
                    {
                        FLOG_FATAL (
                                "phy_ul_rx: Error in received frm rru symbol, antenna1!\n");
                        exit (0);
                        //return ERROR_CODE;
                    }

                    if (p_rrusymbol_ant2 == NULL || p_rrusymbol_ant2->symbol_i
                            == NULL || p_rrusymbol_ant2->symbol_q == NULL)
                    {
                        FLOG_FATAL (
                                "phy_ul_rx: Error in received frm rru symbol, antenna2!\n");
                        exit (0);
                        //return ERROR_CODE;
                    }

                    /* Update system parameters */
                    para->frame_index = p_rrusymbol_ant1->frame_index;
                    para->symbol_offset = p_rrusymbol_ant1->symbol_offset;
                    para->last_slotsymbol
                            = p_rrusymbol_ant1->ul_subframe_endflag;

                    //  dgain[0] =  p_rrusymbol_ant1->dgain;
                    //  dgain[1] =  p_rrusymbol_ant2->dgain;

                    if (p_rrusymbol_ant1->frame_index
                            != p_rrusymbol_ant2->frame_index)
                    {
                        FLOG_ERROR (
                                "phy_ul_rx_div2: Lost Syn for the two antennas \n");
                        return ERROR_CODE;
                    }

                    if (p_rrusymbol_ant1->symbol_num != para->symbol_per_slot)

                    /* Now only support the case where the number of symbols in

                     one rru symbol is the same as that in one slot symbol */
                    {
                        FLOG_ERROR (
                                "phy_ul_rx: The number of ofdma symbols in one rru symbol is wrong!\n");
                        return ERROR_CODE;
                    }
                }
	    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_BURST_DEQUE ));
	    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_PWR_ADJUST ));
                /* preprocessing for the 2nd antenna for Power adjustment */
                //just for UT, should be removed later //

                err_code = phy_ul_power_adjust (para,
                        p_rrusymbol_ant1->symbol_i, p_rrusymbol_ant1->symbol_q,
                        p_rrusymbol_ant2->symbol_i, p_rrusymbol_ant2->symbol_q,
                        dgain, out_pwr_ant0_r, out_pwr_ant0_i, out_pwr_ant1_r,
                        out_pwr_ant1_i);
                if (err_code)
                {
                    FLOG_ERROR (
                            "phy_ul_power_adjust: Error in power adjustment for antenna 2!\n");
                    return err_code;
                }

                err_code_ant1 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant1);
                err_code_ant2 = phy_ul_deinit_rrusymbol (p_rrusymbol_ant2);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_FATAL ("phy_ul_rx: Error in rru symbol deinit!\n");
                    return ERROR_CODE;
                }


				DO_DUMP(DUMP_PHY_INPUT_ANT1_R_ID, 0, out_pwr_ant0_r, 1088*3);
				DO_DUMP(DUMP_PHY_INPUT_ANT1_I_ID, 0, out_pwr_ant0_i, 1088*3);
				DO_DUMP(DUMP_PHY_INPUT_ANT2_R_ID, 0, out_pwr_ant1_r, 1088*3);
				DO_DUMP(DUMP_PHY_INPUT_ANT2_I_ID, 0, out_pwr_ant1_i, 1088*3);
				


	    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_PWR_ADJUST ));
	    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_OFDM_DEMOD ));
                /* OFDMA Demodulation */
                // printf("begin ofdma demodulation!\n");
                ofdmdemod_ant1_r = a_ant1_r;
                ofdmdemod_ant1_i = a_ant1_i;
                err_code_ant1 = phy_ul_ofdmademodul (para, out_pwr_ant0_r,
                        out_pwr_ant0_i, ofdmdemod_ant1_r, ofdmdemod_ant1_i);

                /*
                 err_code_ant1 = phy_ul_deinit_rrusymbol(p_rrusymbol_ant1);
                 err_code_ant2 = phy_ul_deinit_rrusymbol(p_rrusymbol_ant2);

                 if(err_code_ant1 || err_code_ant2)
                 {
                 FLOG_FATAL("phy_ul_rx: Error in rru symbol deinit!\n");
                 return ERROR_CODE;
                 }
                 */

                ofdmdemod_ant2_r = a_ant2_r;
                ofdmdemod_ant2_i = a_ant2_i;
                err_code_ant2 = phy_ul_ofdmademodul (para, out_pwr_ant1_r,
                        out_pwr_ant1_i, ofdmdemod_ant2_r, ofdmdemod_ant2_i);

                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_ofdmademodul: Error in ofdm_demodulation!\n");
                    return ERROR_CODE;
                }
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_OFDM_DEMOD ));


				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT1_R_ID, 0, ofdmdemod_ant1_r, 756*3);
				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT1_I_ID, 0, ofdmdemod_ant1_i, 756*3);
				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT2_R_ID, 0, ofdmdemod_ant2_r, 756*3);
				DO_DUMP(DUMP_PHY_OFDM_DEMOD_ANT2_I_ID, 0, ofdmdemod_ant2_i, 756*3);
				
				

		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_SUBCAR_DEMOD ));
                /* subcar derandom for two antennas */
                subcarderandom_ant1_r = b_ant1_r;
                subcarderandom_ant1_i = b_ant1_i;
                err_code_ant1 = phy_ul_subcarderandom (para, wk,
                        ofdmdemod_ant1_r, ofdmdemod_ant1_i,
                        subcarderandom_ant1_r, subcarderandom_ant1_i);

                subcarderandom_ant2_r = b_ant2_r;
                subcarderandom_ant2_i = b_ant2_i;
                err_code_ant2 = phy_ul_subcarderandom (para, wk,
                        ofdmdemod_ant2_r, ofdmdemod_ant2_i,
                        subcarderandom_ant2_r, subcarderandom_ant2_i);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_subcarderandom: Error in subcarderandom!\n");
                    return ERROR_CODE;
                }

		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_SUBCAR_DEMOD ));
				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT1_R_ID, 0, subcarderandom_ant1_r, 756*3);
				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT1_I_ID, 0, subcarderandom_ant1_i, 756*3);
				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT2_R_ID, 0, subcarderandom_ant2_r, 756*3);
				DO_DUMP(DUMP_PHY_SUBCAR_DERANDOM_ANT2_I_ID, 0, subcarderandom_ant2_i, 756*3);
				


		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMAPPING ));
                /* begin demuxofdm */
                err_code_ant1 = phy_ul_demuxofdmsym (para,
                        subcarderandom_ant1_r, subcarderandom_ant1_i,
                        data_allocation + length_data * count,
                        rotation_posindex, slotsym_num, demuxdata_ant1_r,
                        demuxdata_ant1_i);
                err_code_ant2 = phy_ul_demuxofdmsym (para,
                        subcarderandom_ant2_r, subcarderandom_ant2_i,
                        data_allocation + length_data * count,
                        rotation_posindex, slotsym_num, demuxdata_ant2_r,
                        demuxdata_ant2_i);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR ("phy_ul_demuxofdmsym: Error in demuxofdm!\n");
                    return ERROR_CODE;
                }
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMAPPING ));


				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT1_R_ID, 0, demuxdata_ant1_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT1_I_ID, 0, demuxdata_ant1_i, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT2_R_ID, 0, demuxdata_ant2_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_OFDM_DEMUX_ANT2_I_ID, 0, demuxdata_ant2_i, 48*para->usedsubch_num);
				

                /* begin channel estimation */
		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_CHAN_EST ));
                h_ant1_r = a_ant1_r;
                h_ant1_i = a_ant1_i;
                err_code_ant1 = phy_ul_single_chanlest (para,
                        subcarderandom_ant1_r, subcarderandom_ant1_i, h_ant1_r,
                        h_ant1_i);
                h_ant2_r = a_ant2_r;
                h_ant2_i = a_ant2_i;
                err_code_ant2 = phy_ul_single_chanlest (para,
                        subcarderandom_ant2_r, subcarderandom_ant2_i, h_ant2_r,
                        h_ant2_i);

                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_channel_est_single: Error in channel_estimation!\n");
                    return ERROR_CODE;
                }
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_CHAN_EST ));
				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT1_R_ID, 0, h_ant1_r, 756*3);
				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT1_I_ID, 0, h_ant1_i, 756*3);
				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT2_R_ID, 0, h_ant2_r, 756*3);
				DO_DUMP(DUMP_PHY_CHANNEL_ESTI_ANT2_I_ID, 0, h_ant2_i, 756*3);
				

                /* demuxofdm channel_est result*/
		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMAPPING ));
                err_code_ant1 = phy_ul_demuxofdmsym (para, h_ant1_r, h_ant1_i,
                        data_allocation + length_data * count,
                        rotation_posindex, slotsym_num, demuxchn_ant1_r,
                        demuxchn_ant1_i);

                err_code_ant2 = phy_ul_demuxofdmsym (para, h_ant2_r, h_ant2_i,
                        data_allocation + length_data * count,
                        rotation_posindex, slotsym_num, demuxchn_ant2_r,
                        demuxchn_ant2_i);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxofdmsym: Error in demux channel estimation results!\n");
                    return ERROR_CODE;
                }

				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT1_R_ID, 0, demuxchn_ant1_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT1_I_ID, 0, demuxchn_ant1_i, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT2_R_ID, 0, demuxchn_ant2_r, 48*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_ANT2_I_ID, 0, demuxchn_ant2_i, 48*para->usedsubch_num);
				

                /* demux pilot */
                length_pilot = para->avasubch_num * 24;
                err_code_ant1 = phy_ul_demuxpilot (para, subcarderandom_ant1_r,
                        subcarderandom_ant1_i, pilot_allocation + length_pilot
                                * count, rotation_posindex, slotsym_num,
                        demuxpilot_ant1_r, demuxpilot_ant1_i);

                err_code_ant2 = phy_ul_demuxpilot (para, subcarderandom_ant2_r,
                        subcarderandom_ant2_i, pilot_allocation + length_pilot
                                * count, rotation_posindex, slotsym_num,
                        demuxpilot_ant2_r, demuxpilot_ant2_i);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxpilot: Error in demux channel estimation results!\n");
                    return ERROR_CODE;
                }

				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT1_R_ID, 0, demuxpilot_ant1_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT1_I_ID, 0, demuxpilot_ant1_i, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT2_R_ID, 0, demuxpilot_ant2_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_PILOT_ANT2_I_ID, 0, demuxpilot_ant2_i, 24*para->usedsubch_num);
				

                /* demux channel_est result on pilot */
                err_code_ant1 = phy_ul_demuxpilot (para, h_ant1_r, h_ant1_i,
                        pilot_allocation + length_pilot * count,
                        rotation_posindex, slotsym_num, pilotchn_ant1_r,
                        pilotchn_ant1_i);

                err_code_ant2 = phy_ul_demuxpilot (para, h_ant2_r, h_ant2_i,
                        pilot_allocation + length_pilot * count,
                        rotation_posindex, slotsym_num, pilotchn_ant2_r,
                        pilotchn_ant2_i);
                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_demuxpilot: Error in demux channel estimation results on pilot!\n");
                    return ERROR_CODE;
                }
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMAPPING ));

				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_R_ID, 0, pilotchn_ant1_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_I_ID, 0, pilotchn_ant1_i, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT2_R_ID, 0, pilotchn_ant2_r, 24*para->usedsubch_num);
				DO_DUMP(DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT2_I_ID, 0, pilotchn_ant2_i, 24*para->usedsubch_num);
				

		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_BURST_FORMING ));
                block_cur = slotsym_cur->p_block_header;
                while (block_cur != NULL)
                {
                    if (block_cur->is_used == 1)
                    {
                        /* insert data subcarrier related data */
                        block_length = block_cur->subchannel_num * 48;
                        //     printf("subchannel_offset = %d\n", block_cur->subchannel_offset);
#if 0
                        if(para->symbol_offset == 0)
                        {
                            block_cur->subchannel_offset = block_cur->subchannel_offset - para->ranging_subch;
                            printf("block_cur->subchannel_offset = %d\n", block_cur->subchannel_offset);
                        }
#endif

                        for (i = 0; i < block_length; i++)
                        {
                            temp_buffer_ant1_r[0][i] = demuxdata_ant1_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_i[0][i] = demuxdata_ant1_i[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_r[1][i] = demuxchn_ant1_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_i[1][i] = demuxchn_ant1_i[48
                                    * block_cur->subchannel_offset + i];

                            temp_buffer_ant2_r[0][i] = demuxdata_ant2_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_i[0][i] = demuxdata_ant2_i[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_r[1][i] = demuxchn_ant2_r[48
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_i[1][i] = demuxchn_ant2_i[48
                                    * block_cur->subchannel_offset + i];
                        }
                        memcpy (temp_buffer_ant1_r[2] + block_cur->data_offset,
                                temp_buffer_ant1_r[0], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant1_i[2] + block_cur->data_offset,
                                temp_buffer_ant1_i[0], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant1_r[3] + block_cur->data_offset,
                                temp_buffer_ant1_r[1], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant1_i[3] + block_cur->data_offset,
                                temp_buffer_ant1_i[1], sizeof(float)
                                        * block_length);

                        memcpy (temp_buffer_ant2_r[2] + block_cur->data_offset,
                                temp_buffer_ant2_r[0], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant2_i[2] + block_cur->data_offset,
                                temp_buffer_ant2_i[0], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant2_r[3] + block_cur->data_offset,
                                temp_buffer_ant2_r[1], sizeof(float)
                                        * block_length);
                        memcpy (temp_buffer_ant2_i[3] + block_cur->data_offset,
                                temp_buffer_ant2_i[1], sizeof(float)
                                        * block_length);

                        /* insert pilot related data */
                        block_length = block_cur->subchannel_num * 24;
                        //  printf("subchannel_offset = %d\n", block_cur->subchannel_offset);
                        /*
                         if(para->symbol_offset == 0)
                         {
                         block_cur->subchannel_offset = block_cur->subchannel_offset - para->ranging_subch ;
                         }
                         */
                        for (i = 0; i < block_length; i++)
                        {
                            temp_buffer_ant1_r[0][i] = demuxpilot_ant1_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_i[0][i] = demuxpilot_ant1_i[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_r[1][i] = pilotchn_ant1_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant1_i[1][i] = pilotchn_ant1_i[24
                                    * block_cur->subchannel_offset + i];

                            temp_buffer_ant2_r[0][i] = demuxpilot_ant2_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_i[0][i] = demuxpilot_ant2_i[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_r[1][i] = pilotchn_ant2_r[24
                                    * block_cur->subchannel_offset + i];
                            temp_buffer_ant2_i[1][i] = pilotchn_ant2_i[24
                                    * block_cur->subchannel_offset + i];
                        }
                        /*
                         printf("print before deemodulation\n");

                         for (i=456; i<480; i++)
                         {
                         printf("pilot_data = %f, pilot_est = %f\n",
                         temp_buffer_ant1_r[0][i],temp_buffer_ant1_r[1][i]);
                         }*/

                        memcpy (
                                temp_buffer_ant1_r[4] + block_cur->pilot_offset,
                                temp_buffer_ant1_r[0], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant1_i[4] + block_cur->pilot_offset,
                                temp_buffer_ant1_i[0], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant1_r[5] + block_cur->pilot_offset,
                                temp_buffer_ant1_r[1], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant1_i[5] + block_cur->pilot_offset,
                                temp_buffer_ant1_i[1], sizeof(float)
                                        * block_length);

                        memcpy (
                                temp_buffer_ant2_r[4] + block_cur->pilot_offset,
                                temp_buffer_ant2_r[0], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant2_i[4] + block_cur->pilot_offset,
                                temp_buffer_ant2_i[0], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant2_r[5] + block_cur->pilot_offset,
                                temp_buffer_ant2_r[1], sizeof(float)
                                        * block_length);
                        memcpy (
                                temp_buffer_ant2_i[5] + block_cur->pilot_offset,
                                temp_buffer_ant2_i[1], sizeof(float)
                                        * block_length);
                    }

                    block_cur = block_cur->next;
                }

                slotsym_num[0]++;
                count++;
                para->symbol_offset = para->symbol_offset
                        + para->symbol_per_slot;
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_BURST_FORMING ));

                if (slotsym_cur->next == NULL)
                {
                    slotsym_num[0] = 0;
                    count = 0;
                    break;
                }
            }

	    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_OTHER ));
            p_burst_cur = p_map->p_burst_header;

            total_burst = 0;
            burst_idx = 0;

            while (p_burst_cur != NULL)
            {
                total_burst++;
                p_burst_cur = p_burst_cur->next;
            }

            p_burst_cur = p_map->p_burst_header;

            while (p_burst_cur != NULL)
            {
                err_code_ant1 = phy_ul_noise_est (p_burst_cur,
                        temp_buffer_ant1_r[4] + p_burst_cur->pilot_offset,
                        temp_buffer_ant1_i[4] + p_burst_cur->pilot_offset,
                        temp_buffer_ant1_r[5] + p_burst_cur->pilot_offset,
                        temp_buffer_ant1_i[5] + p_burst_cur->pilot_offset,
                        noise_ant1_est, snr_ant1_est);

                err_code_ant2 = phy_ul_noise_est (p_burst_cur,
                        temp_buffer_ant2_r[4] + p_burst_cur->pilot_offset,
                        temp_buffer_ant2_i[4] + p_burst_cur->pilot_offset,
                        temp_buffer_ant2_r[5] + p_burst_cur->pilot_offset,
                        temp_buffer_ant2_i[5] + p_burst_cur->pilot_offset,
                        noise_ant2_est, snr_ant2_est);

                if (err_code_ant1 || err_code_ant2)
                {
                    FLOG_ERROR (
                            "phy_ul_noise_est: Error in noise estimation processing!\n");
                    return ERROR_CODE;
                }
#ifdef _AMC_   
#if 1 
                snr_ant1_avg = 0.0;
                snr_ant2_avg = 0.0;
#if 0
                for (i=0; i<5; i++)
                {
                    printf("the slot SNR = %f \n", snr_ant1_est[i]);
                }       
#endif         

                for (i=0; i<p_burst_cur->slots_num; i++)
                {
                    snr_ant1_avg = snr_ant1_avg + snr_ant1_est[i];
                    snr_ant2_avg = snr_ant2_avg + snr_ant2_est[i];
                }


                if ( snr_ant1_avg > snr_ant2_avg)
		{
			cinr_avg = round(10*log10(snr_ant1_avg/p_burst_cur->slots_num)) - 3;
		}
                else
		{ 
			cinr_avg = round(10*log10(snr_ant2_avg/p_burst_cur->slots_num)) - 3;
		}             

//cinr_avg = (int)(10 * log10f((float)cinr_avg));
		p_burst_cur->cinr = cinr_avg; 

//               printf("phy cinr report: max->%ddBm, Ant1->%f, And2->%f\n",  cinr_avg, snr_ant1_avg/p_burst_cur->slots_num, snr_ant2_avg/p_burst_cur->slots_num);
#endif
#endif


#ifdef _BS_MONITOR_
                /*
                 int32_t chanbuf_len = p_burst_cur->slots_num*4*4;
                 //    void *hook_chan;
                 //    hook_chan = (float *)malloc(sizeof(float) * p_burst_cur->slots_num*4);
                 float hook_chan[MAX_SLOTS_NUM*4];
                 */
                chanbuf_len = (p_burst_cur->slots_num * 4 + 2 ) * 4;

                hook_chan.frm_num = (float)para->frame_index;
                hook_chan.slots_num = (float)p_burst_cur->slots_num;

                memcpy(&(hook_chan.buf[0]), noise_ant1_est, p_burst_cur->slots_num*sizeof(float));
                memcpy(&(hook_chan.buf[p_burst_cur->slots_num]), noise_ant2_est, p_burst_cur->slots_num*sizeof(float));
                memcpy(&(hook_chan.buf[p_burst_cur->slots_num * 2]), snr_ant1_est, p_burst_cur->slots_num*sizeof(float));
                memcpy(&(hook_chan.buf[p_burst_cur->slots_num * 3]), snr_ant2_est, p_burst_cur->slots_num*sizeof(float));

                hook_debug_trace(HOOK_CHAN_QUALITY_IDX, &hook_chan, chanbuf_len, 1);
                DO_DUMP(DUMP_CHN_DUMP_ID, 0, &hook_chan, chanbuf_len);

                //free(hook_chan);

#ifdef _HOOK_BS_DUMP_
                FILE *fp_hookchan;
                fp_hookchan = fopen("hook_chan.dat","w+t");
                dump_ulrx_ffloat(fp_hookchan, hook_chan, p_burst_cur->slots_num*4+1);
                fclose(fp_hookchan);
                //    printf("finished saving data!\n");
#endif

#endif



				DO_DUMP(DUMP_PHY_NOISE_ESTI_ANT1_ID, 0, noise_ant1_est, p_burst_cur->slots_num);
				DO_DUMP(DUMP_PHY_SNR_ESTI_ANT1_ID, 0, snr_ant1_est, p_burst_cur->slots_num);
				DO_DUMP(DUMP_PHY_NOISE_ESTI_ANT2_ID, 0, noise_ant2_est, p_burst_cur->slots_num);
				DO_DUMP(DUMP_PHY_SNR_ESTI_ANT2_ID, 0, snr_ant2_est, p_burst_cur->slots_num);



#ifdef _BS_MONITOR_

                /*
                 int32_t constellationbuf_len = p_burst_cur->slots_num*48*4*4;
                 // void *hook_constellation;
                 // hook_constellation = (float *)malloc(sizeof(float) * p_burst_cur->slots_num*48*4);
                 float hook_constellation[MAX_SLOTS_NUM*48*4];
                 */
                if (hook_debug_trace(HOOK_CONSTELLATION_IDX, NULL, 0, 0) > 0)
                {
                    temp_len = p_burst_cur->slots_num * 48;

                    constellationbuf_len = (temp_len * 8 + 5) * 4;

                    hook_constellation.frm_num = (float)para->frame_index;
                    hook_constellation.slot_num = (float)p_burst_cur->slots_num;
                    hook_constellation.code_id = (float)p_burst_cur->code_id;
                    hook_constellation.total_burst = (float)total_burst;
                    hook_constellation.burst_idx = (float)burst_idx;

                    memcpy(&(hook_constellation.buf[0]), p_burst_cur->data_offset+temp_buffer_ant1_r[2],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len]), p_burst_cur->data_offset+temp_buffer_ant1_i[2],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 2]), p_burst_cur->data_offset+temp_buffer_ant2_r[2],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 3]), p_burst_cur->data_offset+temp_buffer_ant2_i[2],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 4]), p_burst_cur->data_offset+temp_buffer_ant1_r[3],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 5]), p_burst_cur->data_offset+temp_buffer_ant1_i[3],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 6]), p_burst_cur->data_offset+temp_buffer_ant2_r[3],
                            temp_len * sizeof(float));

                    memcpy(&(hook_constellation.buf[temp_len * 7]), p_burst_cur->data_offset+temp_buffer_ant2_i[3],
                            temp_len * sizeof(float));

                    hook_debug_trace(HOOK_CONSTELLATION_IDX, &hook_constellation, constellationbuf_len, 0);
                }

                burst_idx ++;

                if (burst_idx == total_burst)
                {
                    hook_debug_trace(HOOK_CONSTELLATION_IDX, NULL, -1, 1);
                }
                
                //free(hook_constellation);
#ifdef _HOOK_BS_DUMP_
                FILE *fp_hookste;
                fp_hookste = fopen("hook_ste.dat","w+t");
                dump_ulrx_ffloat(fp_hookste, hook_constellation, p_burst_cur->slots_num*48*8+1);
                fclose(fp_hookste);
                //    printf("finished saving data!\n");
#endif

#endif
		PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_OTHER ));
	        PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMOD ));


		DO_DUMP(DUMP_PHY_BURST_DATA_ANT1_R_ID, 0, p_burst_cur->data_offset+temp_buffer_ant1_r[2], p_burst_cur->slots_num*48);
		DO_DUMP(DUMP_PHY_PILOT_ANT1_R_ID, 0, p_burst_cur->pilot_offset+temp_buffer_ant1_r[4], p_burst_cur->slots_num*24);
		DO_DUMP(DUMP_PHY_BURST_PILOT_CHANNEL_ANT1_R_ID, 0, p_burst_cur->pilot_offset+temp_buffer_ant1_r[5], p_burst_cur->slots_num*24);

                err_code = phy_ul_demodulation (p_burst_cur,
                        temp_buffer_ant1_r[2] + p_burst_cur->data_offset,
                        temp_buffer_ant1_i[2] + p_burst_cur->data_offset,
                        temp_buffer_ant2_r[2] + p_burst_cur->data_offset,
                        temp_buffer_ant2_i[2] + p_burst_cur->data_offset,
                        temp_buffer_ant1_r[3] + p_burst_cur->data_offset,
                        temp_buffer_ant1_i[3] + p_burst_cur->data_offset,
                        temp_buffer_ant2_r[3] + p_burst_cur->data_offset,
                        temp_buffer_ant2_i[3] + p_burst_cur->data_offset,
                        noise_ant1_est, noise_ant2_est, softbit);

                if (err_code)
                {
                    FLOG_ERROR ("phy_ul_demodulation: Error in demodulation!\n");
                    return ERROR_CODE;
                }

                if (p_burst_cur->code_id == 0 || p_burst_cur->code_id == 1 )
                    tmp_len = 2;
                else
                    if (p_burst_cur->code_id == 2 || p_burst_cur->code_id == 3)
                        tmp_len = 4;
                    else
                        tmp_len = 6;

                //  printf("p_burst_cur->code_id = %d\n", p_burst_cur->code_id);
		    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_DEMOD ));

		PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DECODE ));

		DO_DUMP(DUMP_PHY_DEMODULATION_ID, 0, softbit, p_burst_cur->slots_num*48*tmp_len);

                err_code = phy_ul_fec_decoding (para, p_burst_cur, softbit);
                 //printf("p_burst_cur->slots_num= %d\n", p_burst_cur->slots_num);
               // printf("p_burst_cur->code_id = %d\n", p_burst_cur->code_id);


                if (err_code)
                {
                    FLOG_ERROR ("phy_ul_rx_div1: Error in data fec decoding!\n");
                    return ERROR_CODE;
                }
		PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_FEC_DECODE ));
		    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_UL_OTHER ));

                switch ( p_burst_cur->code_id)
                {   
                    case CC_QPSK12:
                         tmp_len = p_burst_cur->slots_num*48;
                         break;
                    case CC_QPSK34:
                         tmp_len = p_burst_cur->slots_num*48*1.5;
                         break;
                    case CC_QAM16_12:
                         tmp_len =  p_burst_cur->slots_num*48*2;
                         break;
                    case CC_QAM16_34:
                         tmp_len =  p_burst_cur->slots_num*48*3;
                    case CC_QAM64_12:
                         tmp_len =  p_burst_cur->slots_num*48*3;
                         break;
                    case CC_QAM64_34:
                         tmp_len =  p_burst_cur->slots_num*48*4.5;
                         break;
                    case CC_QAM64_23:
                         tmp_len =  p_burst_cur->slots_num*48*4;
                         break;

                }
#if 0
                FILE *fp15;
                fp15 = fopen("15_fecdecoding_derand.out","wt+");
                dump_ulrx_uchar(fp15, p_map->p_total_buffer_out, tmp_len);
                fclose(fp15);

#endif

		DO_DUMP(DUMP_PHY_FECDECODE_DERAND_ID, 0, p_map->p_total_buffer_out, tmp_len);

                //printf("p_burst_cur->slots_num = %d\n", p_burst_cur->slots_num);


#ifdef _BER_TEST_
                /* added for Field BER Test */
#ifndef _MY_PER_TEST_
                //    if (loop_count >= PRE_FRAME)

                {
                    ber_count = 0;
                    for (i=0; i< (unsigned int)framebit_num; i++)
                    {
                        if (*((char *)(p_map->p_total_buffer_out+i)) != 1)
                        ber_count++;
                    }

                    if (ber_count != 0)
                    {
//                        err_frm ++;
//                        total_err_frm ++;

#ifdef _DUMP_BER_ERROR_FRAME_
                        if (phy_enable_dump != 0)
                        {
                            FLOG_INFO ("error frame NO %d, error bits %d, symbol offset %d\n", para->frame_index, ber_count, para->symbol_offset);

                            if (dump_rx_raw_info (para->frame_index, 1) != 0)
                            {
                                FLOG_FATAL ("Dump RAW INFO error\n");
                            }

                            exit_bsproc();
                            while(1)
                            {
                                sleep(1);
                            }
                        }
#endif
                    }
//                    break;
                }
#else
                ber_count = 0;
                for (i=0; i< framebit_num; i++)
                {
                    if (*((char *)(p_map->p_total_buffer_out+i)) != 1)
                    ber_count++;
                }
                // printf("ber_count per frame = %d\n", ber_count);

                if (ber_count != 0)
                {   err_frm++;}
                break;

#endif //Endif of _MY_PER_TEST_
#endif

                p_burst_cur = p_burst_cur->next;

            }


            p_map->pscan_resflag = 0;

            p_map->frame_num = para->frame_index;

            p_msg_out1->p_buf = p_map;

            /** end by zzb for integration */
            if (wmrt_enqueue (out_que_id1, p_msg_out1, sizeof(struct queue_msg))
                    == -1)
            {
                FLOG_FATAL (
                        "phy_ul_rx_div2: Error in send processedblock queue to adapter!\n");
                return ERROR_CODE;
            }


#ifdef _BER_TEST_
            
             if (g_ber_reset == 1)
            {
                printf("------------------------ber reset--------------------------------\n");
                switch (g_fake_ulmap_uiuc)
                {
                   case 1:
                       framebit_num = g_fake_ulmap_duration *48;
                       break;
                   case 2:
                       framebit_num = g_fake_ulmap_duration *72;
                       break;
                   case 3:
                       framebit_num = g_fake_ulmap_duration *96;
                       break;
                   case 4:
                       framebit_num = g_fake_ulmap_duration *144;
                       break;
                   case 5:
                       framebit_num = g_fake_ulmap_duration *144;
                       break;
                   case 6:
                       framebit_num = g_fake_ulmap_duration *192;
                       break;
                   case 7:
                       framebit_num = g_fake_ulmap_duration *216;
                       break;
                 }
                printf("framebit_num = %d\n", framebit_num);
                phy_enable_dump = 1;
                total_bersum = 0;
                total_err_frm = 0;
                loop_count = 0;
                total_err_blk = 0;
                total_blk_num = 0;
                berfrm_loop = 0;
                g_ber_reset = 0;
            }
            else
            {

            if ( berfrm_loop > DISCARD_WINDOW)
            {
                if (ber_count != 0)
                {
                    err_frm ++;
                    total_err_frm ++;
                }

                loop_count ++;
                bersum = bersum + ber_count;
                total_bersum = total_bersum + ber_count;

                blk_num = blk_num + para->blk_per_frame;
                total_blk_num = total_blk_num + para->blk_per_frame;


                if ( (ber_count != 0) && (phy_enable_dump != 0) )
               {
                    DO_DUMP(DUMP_RX_SELECT_POOL_ID, para->frame_index, NULL, 0);
               }

                if(loop_count % PRE_FRAME == 0)
               {
                    total_bit = framebit_num * PRE_FRAME;
                    ber = (double)bersum /(double)total_bit;
                    bler = (double)para->blkerr_num/(double)blk_num;

                    FLOG_INFO("window size = %d, window bersum = %d, window total_bit = %d, ber= %e, err_frm = %d\n",
                        PRE_FRAME, bersum, total_bit, ber, err_frm);

                    FLOG_INFO("window block size = %d, window blkerr = %d, bler = %e\n", blk_num, para->blkerr_num, bler);

                    total_err_blk += para->blkerr_num;

                    total_bit = framebit_num * loop_count;

                    ber = (double)total_bersum /(double)total_bit;
                    bler = (double)total_err_blk/(double)total_blk_num;

                    FLOG_INFO("total frame_num = %d, bersum=%d, total_bit = %d, ber= %e, err_frm = %d\n",
                               loop_count, total_bersum, total_bit, ber, total_err_frm);

                    FLOG_INFO("total block size = %d, total blkerr = %d, bler = %e\n", total_blk_num, total_err_blk, bler);


#ifdef _BS_MONITOR_
                    hook_ber[0] = bersum;
                    hook_ber[1] = framebit_num * PRE_FRAME;
                    hook_ber[2] = para->blkerr_num;
                    hook_ber[3] = blk_num;
                    hook_ber[4] = total_bersum;
                    hook_ber[5] = framebit_num * loop_count;
                    hook_ber[6] = total_err_blk;
                    hook_ber[7] = total_blk_num;
                    hook_ber[8] = total_err_frm;
                    hook_ber[9] = loop_count;

                    hook_debug_trace(HOOK_BER_IDX, hook_ber, berbuf_len, 1);
#ifdef _HOOK_BS_DUMP_
                    FILE *fp_hookber;
                    fp_hookber = fopen("hook_ber.dat","w+t");
                    dump_ulrx_uinteger(fp_hookber, hook_ber, 7);
                    fclose(fp_hookber);
#endif /* _HOOK_BS_DUMP_ */

#endif /* _BS_MONITOR_ */

#if 1
                     printf("total frame_num = %d, bersum=%d, total_bit = %d, ber= %e, err_frm = %d\n",
                             loop_count, total_bersum, total_bit, ber, total_err_frm);
                     printf("total block size = %d, total blkerr = %d, bler = %e\n", total_blk_num, total_err_blk, bler);
#endif
                     bersum = 0;
                     err_frm = 0;
                     blk_num = 0;
                     para->blkerr_num = 0;
                 }
             }
            else
            {
                berfrm_loop ++;
                para->blkerr_num = 0;

            }
           }

#endif
            p_map = NULL;
            p_map_org = NULL;
            slotsym_cur = NULL;

        }
                                                                                                           

	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_OTHER ));
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_UL_NO_DEQUE ));
    }
    return SUCCESS_CODE;

}

/*--------------------------------------------------------------------------------------------------

 Function: phy_ul_rx_stca

 Description: stc matrixa 2x1 ofdma symbol receiver processing for one slotsymbol

 Parameters:

 Return Value:

 0       Success
 150     Error

 ---------------------------------------------------------------------------------------------------

 LOG END TAG zYx                                                                                   */

int32_t phy_ul_rx_stca (struct phy_ul_rx_syspara *para,
        const u_int32_t in_que_id1, const u_int32_t in_que_id2,
        const u_int32_t out_que_id1)

{
    (void) para;
    (void) in_que_id1;
    (void) in_que_id2;
    (void) out_que_id1;

    return SUCCESS_CODE;

}

/*--------------------------------------------------------------------------------------------------

 Function: phy_ul_rx_stcb

 Description: stc matrixb 2x2 ofdma symbol receiver processing for one slotsymbol

 Parameters:


 Return Value:

 0       Success
 150     Error

 ---------------------------------------------------------------------------------------------------

 LOG END TAG zYx                                                                                   */

int32_t phy_ul_rx_stcb (struct phy_ul_rx_syspara *para,
        const u_int32_t in_que_id1, const u_int32_t in_que_id2,
        const u_int32_t out_que_id1, const u_int32_t out_que_id2)

{
    (void) para;
    (void) in_que_id1;
    (void) in_que_id2;
    (void) out_que_id1;
    (void) out_que_id2;

    return SUCCESS_CODE;

}

int frm_dump_channel_quality (int flag, FILE * f_p, int len, void * buf)
{
    struct phy_hook_chan_quality_result * hook_chan = (struct phy_hook_chan_quality_result *)buf;
    int i;
    int slot_num = 0;
    float * n0;
    float * n1;
    float * s0;
    float * s1;

    if ( (buf == NULL) || (len == 0) )
    {
        FLOG_ERROR("input error");
        return 1;
    }

    slot_num = (int)hook_chan->slots_num;

    fprintf(f_p, "%d,%d\n", (int)hook_chan->frm_num, slot_num);

    n0 = &(hook_chan->buf[0]);
    n1 = &(hook_chan->buf[slot_num]);
    s0 = &(hook_chan->buf[slot_num * 2]);
    s1 = &(hook_chan->buf[slot_num * 3]);

    for (i = 0; i < slot_num; i++)
    {
        fprintf(f_p, "%f,%f,%f,%f\n", n0[i], n1[i], s0[i], s1[i]);
    }

    (void) flag;

    return 0;
}

/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_UL_RX

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif

