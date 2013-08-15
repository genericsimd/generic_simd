/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: dump_utils.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef __DUMP_UTILS_H_
#define __DUMP_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define DUMP_MAX_KEY_LEN 64



/** API for dump */
#ifdef _DUMP_UTIL_ENABLE_
#define DO_DUMP(id, flag, p_buf, len ) \
        { \
            do_dump(id, flag, len, p_buf ); \
        }

#else
#define DO_DUMP(id, flag, p_buf, len)
#endif


#ifdef _DUMP_UTIL_ENABLE_
#define RESET_DUMP(id, key, count, max_count) \
        { \
            reset_dump(id, key, count, max_count); \
        }

#else
#define DO_DUMP(id, flag, p_buf, len)
#endif


//#define GLOBAL_DUMP_PATH "/dev/shm/"


/** Basic operation define */

#define OPEN_DUMP_FILE(FD, NAME, PREFIX, MODE)    \
        do    \
        {    \
            char open_dump_file_path[128];    \
            sprintf(open_dump_file_path, "%s%s", NAME, PREFIX);    \
            FD = fopen(open_dump_file_path, "w");    \
            if (FD != NULL)    \
            {    \
                fclose(FD);    \
                FD = fopen(open_dump_file_path, MODE);    \
            }    \
        }while(0)

#define CLOSE_DUMP_FILE(FD)    \
        do    \
        {    \
            fclose(FD);    \
        }while(0)

#define DUMP_RAW_DATA(IDX, NAME, FD, LEN, BUF)    \
        do    \
        {    \
            char * dump_raw_data_buf = (char *)BUF;    \
            int dump_raw_data_idx = 0;    \
            fprintf(FD, "Index: %d; item %s; len %d\n", IDX, NAME, LEN);    \
            for (dump_raw_data_idx = 0; dump_raw_data_idx < LEN; dump_raw_data_idx ++)    \
            {    \
                fprintf(FD, "%02x ", dump_raw_data_buf[dump_raw_data_idx]);    \
            }    \
            fprintf(FD, "\n");    \
        }while(0)


/** Struct defines */


enum{
	DUMP_TEST_ID = 0, 
	DUMP_RX_ALL_POOL_ID,
	DUMP_RX_SELECT_POOL_ID,
	DUMP_RX_ALL_RAW_ANTO_ID,
	DUMP_RX_ALL_RAW_ANTI_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR0_I_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR0_Q_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR1_I_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR1_Q_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR2_I_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR2_Q_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR3_I_ID,
	DUMP_RX_ALL_RAW_ANT0_CARR3_Q_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR0_I_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR0_Q_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR1_I_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR1_Q_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR2_I_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR2_Q_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR3_I_ID,
	DUMP_RX_ALL_RAW_ANT1_CARR3_Q_ID,
	DUMP_RANGING_POWER_ID,
	DUMP_RANGING_RESULT_ID,
	DUMP_CHN_DUMP_ID,
	DUMP_RANGING_DEBUG_ID,
	DUMP_IR_RX_ALL_RAW_ID,
	DUMP_PR_RX_ALL_RAW_ID,
	DUMP_PHY_TX_INTERFACE_WK_ID,
	DUMP_PHY_AVER_POWER_ID,
	DUMP_PHY_ROTATE_POS_ID,
	DUMP_PHY_PILOT_ALLOC_ID,
	DUMP_PHY_DATA_ALLOC_ID,
	DUMP_PHY_RNG_ALLOC_ID,
	DUMP_PHY_INPUT_ANT1_R_ID,
	DUMP_PHY_INPUT_ANT1_I_ID,
	DUMP_PHY_INPUT_ANT2_R_ID,
	DUMP_PHY_INPUT_ANT2_I_ID,
	DUMP_PHY_OFDM_DEMOD_ANT1_R_ID,
	DUMP_PHY_OFDM_DEMOD_ANT1_I_ID,
	DUMP_PHY_OFDM_DEMOD_ANT2_R_ID,
	DUMP_PHY_OFDM_DEMOD_ANT2_I_ID,
	DUMP_PHY_SUBCAR_DERANDOM_ANT1_R_ID,
	DUMP_PHY_SUBCAR_DERANDOM_ANT1_I_ID,
	DUMP_PHY_SUBCAR_DERANDOM_ANT2_R_ID,
	DUMP_PHY_SUBCAR_DERANDOM_ANT2_I_ID,
	DUMP_PHY_OFDM_DEMUX_ANT1_R_ID,
	DUMP_PHY_OFDM_DEMUX_ANT1_I_ID,
	DUMP_PHY_OFDM_DEMUX_ANT2_R_ID,
	DUMP_PHY_OFDM_DEMUX_ANT2_I_ID,
	DUMP_PHY_CHANNEL_ESTI_ANT1_R_ID,
	DUMP_PHY_CHANNEL_ESTI_ANT1_I_ID,
	DUMP_PHY_CHANNEL_ESTI_ANT2_R_ID,
	DUMP_PHY_CHANNEL_ESTI_ANT2_I_ID,
	DUMP_PHY_DEMUX_CHANNEL_ANT1_R_ID,
	DUMP_PHY_DEMUX_CHANNEL_ANT1_I_ID,
	DUMP_PHY_DEMUX_CHANNEL_ANT2_R_ID,
	DUMP_PHY_DEMUX_CHANNEL_ANT2_I_ID,
	DUMP_PHY_DEMUX_PILOT_ANT1_R_ID,
	DUMP_PHY_DEMUX_PILOT_ANT1_I_ID,
	DUMP_PHY_DEMUX_PILOT_ANT2_R_ID,
	DUMP_PHY_DEMUX_PILOT_ANT2_I_ID,
	DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_R_ID,
	DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT1_I_ID,
	DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT2_R_ID,
	DUMP_PHY_DEMUX_CHANNEL_PILOT_ANT2_I_ID,
	DUMP_PHY_NOISE_ESTI_ANT1_ID,
	DUMP_PHY_NOISE_ESTI_ANT2_ID,
	DUMP_PHY_SNR_ESTI_ANT1_ID,
	DUMP_PHY_SNR_ESTI_ANT2_ID,
	DUMP_PHY_BURST_DATA_ANT1_R_ID,
	DUMP_PHY_PILOT_ANT1_R_ID,
	DUMP_PHY_BURST_PILOT_CHANNEL_ANT1_R_ID,
	DUMP_PHY_DEMODULATION_ID,
	DUMP_PHY_FECDECODE_DERAND_ID,
	DUMP_PHY_UL_RX_ACTIVEBAND_ID,
	DUMP_PHY_OFDM_MOD_SHIFT_R_ID,
	DUMP_PHY_OFDM_MOD_SHIFT_I_ID,
	DUMP_PHY_OFDM_MOD_IFFT_R_ID,
	DUMP_PHY_OFDM_MOD_IFFT_I_ID,
	DUMP_PHY_OFDM_DEMOD_FFT_R_ID,
	DUMP_PHY_OFDM_DEMOD_FFT_I_ID,
	DUMP_PHY_TX_PREAMBLE_ANT1_I_ID,
	DUMP_PHY_TX_PREAMBLE_ANT1_Q_ID,
	DUMP_PHY_TX_PREAMBLE_ANT2_I_ID,
	DUMP_PHY_TX_PREAMBLE_ANT2_Q_ID,
	DUMP_PHY_TX_FEC_ANT1_IN_ID,
	DUMP_PHY_TX5_FEC_ANT1_I_ID,
	DUMP_PHY_TX5_FEC_ANT1_Q_ID,
	DUMP_PHY_TX_PILOT_ANT1_ID,
	DUMP_PHY_TX_DATA_ALLOC_ANT1_ID,
	DUMP_PHY_TX4_MULTIPLEX_ANT1_I_ID,
	DUMP_PHY_TX4_MULTIPLEX_ANT1_Q_ID,
	DUMP_PHY_TX2_SUBCARAND_ANT1_I_ID,
	DUMP_PHY_TX2_SUBCARAND_ANT1_Q_ID,
	DUMP_PHY_TX1_OFDMAMOD_ANT1_I_ID,
	DUMP_PHY_TX1_OFDMAMOD_ANT1_Q_ID,
	DUMP_PHY_TX1_OFDMAMOD_ANT2_I_ID,
	DUMP_PHY_TX1_OFDMAMOD_ANT2_Q_ID,
	DUMP_PHY_TX_SLOTSYMBOL_ANT1_IN_ID,
	DUMP_PHY_TX_SLOTSYMBOL_ANT2_IN_ID,
	DUMP_PHY_TX_SLOTSDATA_ANT1_IN_ID,
	DUMP_PHY_TX_SLOTSDATA_ANT2_IN_ID,
	DUMP_PHY_TX_RRUSYMBOL_ANT1_ID,
	DUMP_PHY_TX_RRUSYMBOL_ANT2_ID,
	DUMP_PHY_TX_I_ANT1_ID,
	DUMP_PHY_TX_Q_ANT1_ID,
	DUMP_PHY_TX_I_ANT2_ID,
	DUMP_PHY_TX_Q_ANT2_ID,
	DUMP_PHY_TX_PREAMBLE_UNSHIFT_ID,
	DUMP_PHY_TX_PREAMBLE_TEMP_ID,
	DUMP_PHY_TX_PREAMBLE_FDOMAIN_ID,
	DUMP_PHY_TX_FEC9_FCHIN_ID,
	DUMP_PHY_TX_FEC8_FCH_TAILBITING_ID,
	DUMP_PHY_TX_FEC7_FCH_PUNCTURE_ID,
	DUMP_PHY_TX_FEC6_FCH_INTERLEAVER_ID,
	DUMP_PHY_TX_FEC5_FCH_REPETITION_ID,
	DUMP_PHY_TX_FEC4_FCH_MOD_I_ID,
	DUMP_PHY_TX_FEC4_FCH_MOD_Q_ID,
	DUMP_PHY_TX_FEC10_MAC_IN_ID,
	DUMP_PHY_TX_FEC9_RANDOMIZER_ID,
	DUMP_PHY_TX_FEC8_TAILBITING_ID,
	DUMP_PHY_TX_FEC7_PUNCTURE_ID,
	DUMP_PHY_TX_FEC6_INTERLEAVER_ID,
	DUMP_PHY_TX_FEC5_REPETITION_ID,
	DUMP_PHY_TX_FEC4_MOD_I_ID,
	DUMP_PHY_TX_FEC4_MOD_Q_ID,
	DUMP_PHY_FEC_DEREPETITION_ID,
	DUMP_PHY_FEC_DEINTERLEAVER_ID,
	DUMP_PHY_FEC_DEPUNCTURE_ID,
	DUMP_PHY_FEC_CLIP_ID,
	DUMP_PHY_FEC_DECODER_ID,
	DUMP_PHY_FEC_DERAND_ID,
	DUMP_PHY_TX_FREQ_PREAMBLE_FGEN_ID,
	DUMP_PHY_DL_ZONEPERM_PILOT_TMP_ID,
	DUMP_PHY_DL_ZONEPERM_PILOT_USED_SLOTSYM_ID,
	DUMP_PHY_DL_ZONEPERM_DATA_TMP_ID,
	DUMP_PHY_DL_ZONEPERM_DATA_PERSYM_ID,
	DUMP_PHY_DL_ZONEPERM_DATA_ZONE_ID,
	DUMP_PHY_DL_ZONEPERM_DATAIDX_1ST_ID,
	DUMP_PHY_DL_ZONEPERM_DATAIDX_OTHER_ID,
	DUMP_PHY_DL_ZONEPERM_DATAROTA_1ST_ID,
	DUMP_PHY_DL_ZONEPERM_DATAROTA_OTHERS_ID,
	DUMP_PHY_UL_ZONEPERM_TILESUM_IDX_ID,
	DUMP_PHY_UL_ZONEPERM_SUBCH_IDX_CON_ID,
	DUMP_PHY_UL_ZONEPERM_ROTA_POSIDX_ID,
	DUMP_PHY_UL_ZONEPERM_ROTA_IDX_ID,
	DUMP_PHY_UL_ZONEPERM_DATAPLT_IDX_ID,
	DUMP_PHY_UL_ZONEPERM_INIT_DATA_IDX_ID,
	DUMP_PHY_UL_ZONEPERM_ROTA_DATA_IDX_ID,
        DUMP_FRM_RX_DURATION_ID,
	DUMP_FRM_TX_DURATION_ID,
        DUMP_MAC_AMC_PARAM_ID,
        DUMP_MAC_DLMAP_ID,
        DUMP_MAC_ULMAP_ID,
	MAX_DUMP_KEYS_NUM
} enum_dump_id;


typedef int (*dump_util_cb) (int flag, char * name, FILE * fd, int len, void *p_buf);

#define DUMP_KEYS    \
	CONVERT_DUMP (TEST, "test_dump.dat", "a+", NULL, -1, 0);    \
	CONVERT_DUMP (RX_ALL_POOL, "rx_all_pool_info.dat", "a+", dump_all_rx_pool, 0, 0);    \
	CONVERT_DUMP (RX_SELECT_POOL, "rx_select_pool_info.dat", "a+", dump_one_rx_pool, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANTO, "rx_all_raw_ant0.dat", "a+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANTI, "rx_all_raw_ant1.dat", "a+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR0_I, "rx_all_raw_ant0_carr0_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR0_Q, "rx_all_raw_ant0_carr0_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR1_I, "rx_all_raw_ant0_carr1_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR1_Q, "rx_all_raw_ant0_carr1_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR2_I, "rx_all_raw_ant0_carr2_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR2_Q, "rx_all_raw_ant0_carr2_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR3_I, "rx_all_raw_ant0_carr3_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT0_CARR3_Q, "rx_all_raw_ant0_carr3_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR0_I, "rx_all_raw_ant1_carr0_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR0_Q, "rx_all_raw_ant1_carr0_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR1_I, "rx_all_raw_ant1_carr1_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR1_Q, "rx_all_raw_ant1_carr1_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR2_I, "rx_all_raw_ant1_carr2_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR2_Q, "rx_all_raw_ant1_carr2_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR3_I, "rx_all_raw_ant1_carr3_i.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RX_ALL_RAW_ANT1_CARR3_Q, "rx_all_raw_ant1_carr3_q.dat", "w+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (RANGING_POWER, "dump_ranging_power.dat", "a+", dump_ranging_power, 0, 0);    \
	CONVERT_DUMP (RANGING_RESULT, "dump_ranging_result.dat", "a+", dump_ranging_result, 0, 0);    \
	CONVERT_DUMP (CHN_DUMP, "dump_channel_dump.dat", "a+", dump_channel_quality, 0, 0);    \
	CONVERT_DUMP (RANGING_DEBUG, "ranging_error_frame.dat", "a+", dump_one_rx_pool, 0, 0);    \
	CONVERT_DUMP (IR_RX_ALL_RAW, "ir_all_raw.dat", "a+", dump_short_sample, 0, 0);    \
	CONVERT_DUMP (PR_RX_ALL_RAW, "pr_all_raw.dat", "a+", dump_short_sample, 0, 0); \
	CONVERT_DUMP (PHY_TX_INTERFACE_WK, "phy_wk_x86.dat", "w+", dump_phy_tx_interface_wk, 1, 0); \
	CONVERT_DUMP (PHY_AVER_POWER, "aver_power.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_ROTATE_POS, "rotation_posindex.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_PILOT_ALLOC, "pilot_allocation.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DATA_ALLOC, "data_allocation.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_RNG_ALLOC, "ranging_allocation.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_INPUT_ANT1_R, "input_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_INPUT_ANT1_I, "input_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_INPUT_ANT2_R, "input_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_INPUT_ANT2_I, "input_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_ANT1_R, "ofdmdemod_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_ANT1_I, "ofdmdemod_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_ANT2_R, "ofdmdemod_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_ANT2_I, "ofdmdemod_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SUBCAR_DERANDOM_ANT1_R, "subcarderandom_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SUBCAR_DERANDOM_ANT1_I, "subcarderandom_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SUBCAR_DERANDOM_ANT2_R, "subcarderandom_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SUBCAR_DERANDOM_ANT2_I, "subcarderandom_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMUX_ANT1_R, "demuxofdm_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMUX_ANT1_I, "demuxofdm_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMUX_ANT2_R, "demuxofdm_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMUX_ANT2_I, "demuxofdm_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_CHANNEL_ESTI_ANT1_R, "chest_ant1_r.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_CHANNEL_ESTI_ANT1_I, "chest_ant1_i.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_CHANNEL_ESTI_ANT2_R, "chest_ant2_r.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_CHANNEL_ESTI_ANT2_I, "chest_ant2_i.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_ANT1_R, "demuxchn_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_ANT1_I, "demuxchn_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_ANT2_R, "demuxchn_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_ANT2_I, "demuxchn_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_PILOT_ANT1_R, "demuxpilot_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_PILOT_ANT1_I, "demuxpilot_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_PILOT_ANT2_R, "demuxpilot_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_PILOT_ANT2_I, "demuxpilot_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_PILOT_ANT1_R, "pilotchn_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_PILOT_ANT1_I, "pilotchn_ant1_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_PILOT_ANT2_R, "pilotchn_ant2_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMUX_CHANNEL_PILOT_ANT2_I, "pilotchn_ant2_i.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_NOISE_ESTI_ANT1, "noise_est_ant1.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_NOISE_ESTI_ANT2, "noise_est_ant2.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SNR_ESTI_ANT1, "snr_est_ant1.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_SNR_ESTI_ANT2, "snr_est_ant2.dat", "w+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_BURST_DATA_ANT1_R, "burstdata_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_PILOT_ANT1_R, "pilot_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_BURST_PILOT_CHANNEL_ANT1_R, "burstpilotch_ant1_r.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_DEMODULATION, "demodulation.dat", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_FECDECODE_DERAND, "15_fecdecoding_derand.out", "a+", dump_phy_ul_rx_uchar, 1, 0); \
	CONVERT_DUMP (PHY_UL_RX_ACTIVEBAND, "ul_rx_activeband.out", "w+", dump_phy_ul_rx_activeband, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_MOD_SHIFT_R, "shift_r.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_MOD_SHIFT_I, "shift_i.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_MOD_IFFT_R, "ifft_r.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_MOD_IFFT_I, "ifft_i.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_FFT_R, "fft_output_r.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_OFDM_DEMOD_FFT_I, "fft_output_i.dat", "a+", dump_phy_dl_ofdm_mod, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_ANT1_I, "tx_preamble_ant1_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_ANT1_Q, "tx_preamble_ant1_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_ANT2_I, "tx_preamble_ant2_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_ANT2_Q, "tx_preamble_ant2_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC_ANT1_IN, "tx_fec_ant1_in.dat", "a+", dump_phy_dl_oneslot_data, 1, 0); \
	CONVERT_DUMP (PHY_TX5_FEC_ANT1_I, "tx5_fec_ant1_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX5_FEC_ANT1_Q, "tx5_fec_ant1_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PILOT_ANT1, "tx_pilot_ant1.dat", "a+", dump_phy_dl_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_TX_DATA_ALLOC_ANT1, "tx_dataallocation_ant1.dat", "a+", dump_phy_dl_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_TX4_MULTIPLEX_ANT1_I, "tx4_multiplex_ant1_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX4_MULTIPLEX_ANT1_Q, "tx4_multiplex_ant1_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX2_SUBCARAND_ANT1_I, "tx2_subcarand_ant1_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX2_SUBCARAND_ANT1_Q, "tx2_subcarand_ant1_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX1_OFDMAMOD_ANT1_I, "tx1_ofdmamod_ant1_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX1_OFDMAMOD_ANT1_Q, "tx1_ofdmamod_ant1_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX1_OFDMAMOD_ANT2_I, "tx1_ofdmamod_ant2_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX1_OFDMAMOD_ANT2_Q, "tx1_ofdmamod_ant2_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_SLOTSYMBOL_ANT1_IN, "tx_slotsymbol_ant1_in.dat", "a+", dump_phy_dl_slotsymbol, 1, 0); \
	CONVERT_DUMP (PHY_TX_SLOTSYMBOL_ANT2_IN, "tx_slotsymbol_ant2_in.dat", "a+", dump_phy_dl_slotsymbol, 1, 0); \
	CONVERT_DUMP (PHY_TX_SLOTSDATA_ANT1_IN, "tx_slotsdata_ant1_in.dat", "a+", dump_phy_dl_slots_data, 1, 0); \
	CONVERT_DUMP (PHY_TX_SLOTSDATA_ANT2_IN, "tx_slotsdata_ant2_in.dat", "a+", dump_phy_dl_slots_data, 1, 0); \
	CONVERT_DUMP (PHY_TX_RRUSYMBOL_ANT1, "tx_rrusymbol_ant1.dat", "a+", dump_phy_dl_rrusymbol, 1, 0); \
	CONVERT_DUMP (PHY_TX_RRUSYMBOL_ANT2, "tx_rrusymbol_ant2.dat", "a+", dump_phy_dl_rrusymbol, 1, 0); \
	CONVERT_DUMP (PHY_TX_I_ANT1, "tx_i_ant1.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_Q_ANT1, "tx_q_ant1.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_I_ANT2, "tx_i_ant2.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_Q_ANT2, "tx_q_ant2.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_UNSHIFT, "preamble_unshift.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_TEMP, "preamble_temp.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_PREAMBLE_FDOMAIN, "preamble_fdomain.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC9_FCHIN, "tx_fec9_fchin.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC8_FCH_TAILBITING, "tx_fec8_fchtailbiting.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC7_FCH_PUNCTURE, "tx_fec7_fchpuncture.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC6_FCH_INTERLEAVER, "tx_fec6_fchinterleaver.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC5_FCH_REPETITION, "tx_fec5_fchrepetition.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC4_FCH_MOD_I, "tx_fec4_fchmodulation_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC4_FCH_MOD_Q, "tx_fec4_fchmodulation_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC10_MAC_IN, "tx_fec10_macin.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC9_RANDOMIZER, "tx_fec9_randomizer.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC8_TAILBITING, "tx_fec8_tailbiting.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC7_PUNCTURE, "tx_fec7_puncture.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC6_INTERLEAVER, "tx_fec6_interleaver.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC5_REPETITION, "tx_fec5_repetition.dat", "a+", dump_phy_dl_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC4_MOD_I, "tx_fec4_modulation_i.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_TX_FEC4_MOD_Q, "tx_fec4_modulation_q.dat", "a+", dump_phy_dl_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_FEC_DEREPETITION, "9_fecdecoding_derepetition.out", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_FEC_DEINTERLEAVER, "10_fecdecoding_deinterleaver.out", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_FEC_DEPUNCTURE, "11_fecdecoding_depuncture.out", "a+", dump_phy_ul_rx_ffloat, 1, 0); \
	CONVERT_DUMP (PHY_FEC_CLIP, "12_fecdecoding_clip.out", "a+", dump_phy_ul_rx_uchar, 1, 0); \
	CONVERT_DUMP (PHY_FEC_DECODER, "13_fecdecoding_decoder.out", "a+", dump_phy_ul_rx_uchar, 1, 0); \
	CONVERT_DUMP (PHY_FEC_DERAND, "14_fecdecoding_derand.out", "a+", dump_phy_ul_rx_uchar, 1, 0); \
	CONVERT_DUMP (PHY_TX_FREQ_PREAMBLE_FGEN, "tx_freq_preamble_fgen.dat", "a+", dump_phy_tx_freq_preamble_fgen, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_PILOT_TMP, "pilot_temp.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_PILOT_USED_SLOTSYM, "pilot_used_slotsym.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATA_TMP, "data_temp.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATA_PERSYM, "data_persym.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATA_ZONE, "data_zone.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATAIDX_1ST, "dataindex_1st.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATAIDX_OTHER, "dataindex_other.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATAROTA_1ST, "datarota_1st.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_DL_ZONEPERM_DATAROTA_OTHERS, "datarota_others.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_TILESUM_IDX, "tilesub_index.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_SUBCH_IDX_CON, "subch_index_con.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_ROTA_POSIDX, "rotation_posindex.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_ROTA_IDX, "rotation_index.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_DATAPLT_IDX, "dataplt_index.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_INIT_DATA_IDX, "init_data_index.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
	CONVERT_DUMP (PHY_UL_ZONEPERM_ROTA_DATA_IDX, "data_roation_index.dat", "w+", dump_phy_ul_rx_uinteger, 1, 0); \
        CONVERT_DUMP (FRM_RX_DURATION, "frm_tx_duration.dat", "w+", dump_frm_duration, -1, 0); \
        CONVERT_DUMP (FRM_TX_DURATION, "frm_rx_duration.dat", "w+", dump_frm_duration, -1, 0); \
        CONVERT_DUMP (MAC_AMC_PARAM, "mac_amc_param.dat", "w+", dump_mac_amc_param, -1, 0); \
        CONVERT_DUMP (MAC_DLMAP, "mac_dlmap.dat", "w+", dump_mac_dlmap_param, -1, 0); \
        CONVERT_DUMP (MAC_ULMAP, "mac_ulmap.dat", "w+", dump_mac_ulmap_param, -1, 0); \


struct dump_key_node
{
	char key_name[DUMP_MAX_KEY_LEN];
	dump_util_cb f_dump_cb;
	FILE * fd;
	int max_count;
	int count;
	pthread_mutex_t dump_mutex;
};

int init_dump( void );

int release_dump( void );

int do_dump( int id, int flag, int len, void *p_buf );

int reset_dump(int id, char * key, int count, int max_count);


//declare the function
int dump_mac_amc_param(int flag, char * name, FILE * fd, int len, void *p_buf);

int dump_mac_dlmap_param(int flag, char * name, FILE * fd, int len, void * p_buf);
int dump_mac_ulmap_param(int flag, char * name, FILE * fd, int len, void * p_buf);

#endif /* __DUMP_UTILS_H_ */

