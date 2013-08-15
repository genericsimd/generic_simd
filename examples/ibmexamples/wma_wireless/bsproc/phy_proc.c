/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: phy_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include <sys/types.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "queue_util.h"
#include "global.h"
#include "flog.h"
#include "phy_proc.h"
#include "bs_cfg.h"

#include "prephy_proc.h"

#include "phy_dl_tx_interface.h"
#include "phy_ul_rx_interface.h"

#include "phy_ul_rx_ranging.h"


pthread_mutex_t mutex_tx_phy_en_flag;
int tx_phy_en_flag = 0;

pthread_mutex_t mutex_ranging_en_flag;
int ranging_en_flag = 0;

pthread_mutex_t mutex_phy_frame_info;
struct phy_frame_info gs_phy_frame_info;


static int phy_dl_thdflag = 0;
static int phy_ul_thdflag = 0;

static pthread_t phy_dl_bs_thread = 0;
static pthread_t phy_ul_bs_thread = 0;
static pthread_t phy_ranging_bs_thread = 0;

struct phy_dl_tx_syspara *phy_dl_para = NULL;
struct phy_ul_rx_syspara *phy_ul_para = NULL;

static struct phy_dts_info *phy_dts_para = NULL;

static struct phy_global_param *phy_g_para = NULL;

static struct phy_ranging_para *ranging_para = NULL;
static struct ranging_rru_para *ranging_rru_para = NULL;


//static struct phy_ranging_para *ranging_para = NULL;

static int get_phy_config (struct phy_global_param * phy_config,
        struct phy_dts_info *dts_config);

static int get_ranging_config ( struct ranging_rru_para * ranging_rrh_config,
        struct phy_ranging_para * ranging_config );


void * phy_dl_bs (void * arg __attribute__ ((unused)));

void * phy_ul_bs (void * arg __attribute__ ((unused)));

void * phy_dl_bs (void * arg __attribute__ ((unused)))
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

    phy_dl_tx (phy_dl_para, ANTENNA_NUM, (u_int32_t *) phy_de_id, ANTENNA_NUM,
            (u_int32_t *) phy_en_id);

    return NULL;
}

void * phy_ul_bs (void * arg __attribute__ ((unused)))
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

    phy_ul_rx( phy_ul_para, ANTENNA_NUM, (u_int32_t *)phy_ul_de_id, ANTENNA_NUM,
               (u_int32_t *) phy_ul_en_id );

    return NULL;
}

void * phy_ul_ranging_bs (void * arg __attribute__ ((unused)))
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

    phy_ul_ranging ( phy_ul_para,
                     ranging_rru_para,
                     ranging_para,
                     phy_ul_de_id[2],
                     phy_ul_de_id[3],
                     phy_ul_en_id[2]);

    return NULL;
}

int phy_bs_process (void)
{
    int ret = 0;

    if (pthread_mutex_init(&mutex_tx_phy_en_flag, NULL) != 0 )
    {
        FLOG_FATAL ("initial lock error\n");
        return RET_ERROR;
    }

    if (pthread_mutex_init(&mutex_phy_frame_info, NULL) != 0 )
    {
        FLOG_FATAL ("initial lock error\n");
        return RET_ERROR;
    }

    if (pthread_mutex_init(&mutex_ranging_en_flag, NULL) != 0 )
    {
        FLOG_FATAL ("initial lock error\n");
        return RET_ERROR;
    }

    memset (&gs_phy_frame_info, 0, sizeof (struct phy_frame_info));

    phy_dl_para = (struct phy_dl_tx_syspara *) malloc (
            sizeof(struct phy_dl_tx_syspara));

    if (phy_dl_para == NULL)
    {
        FLOG_FATAL ("malloc memory error\n");
        return RET_ERROR;
    }

    memset (phy_dl_para, 0, sizeof(struct phy_dl_tx_syspara));


    phy_ul_para = (struct phy_ul_rx_syspara *) malloc (
            sizeof(struct phy_ul_rx_syspara));

    if (phy_ul_para == NULL)
    {
        FLOG_FATAL ("malloc memory error\n");
        return RET_ERROR;
    }

    memset (phy_ul_para, 0, sizeof(struct phy_ul_rx_syspara));


    phy_dts_para = (struct phy_dts_info *) malloc (sizeof(struct phy_dts_info));

    if (phy_dts_para == NULL)
    {
        FLOG_FATAL ("malloc memory error\n");
        return RET_ERROR;
    }

    memset (phy_dts_para, 0, sizeof(struct phy_dts_info));

    phy_g_para = (struct phy_global_param *) malloc (
            sizeof(struct phy_global_param));

    if (phy_g_para == NULL)
    {
        FLOG_FATAL ("malloc memory error\n");
        return RET_ERROR;
    }

    memset (phy_g_para, 0, sizeof(struct phy_global_param));


/*
    ranging_para = (struct phy_ranging_para *)
            malloc( sizeof (struct phy_ranging_para) );
*/
    ret = get_phy_config (phy_g_para, phy_dts_para);

    if (ret != 0)
    {
        FLOG_FATAL ("get_phy_config error");
    }

    ret = phy_dl_tx_init (phy_g_para, phy_dts_para, phy_dl_para);

    if (ret != 0)
    {
        FLOG_FATAL ("phy_dl_tx_init error");
    }

    ret = phy_ul_rx_init (phy_g_para, phy_dts_para, phy_ul_para);

    if (ret != 0)
    {
        FLOG_FATAL ("phy_dl_tx_init error");
    }


    ranging_para = (struct phy_ranging_para *)malloc(sizeof(struct phy_ranging_para));

    if (ranging_para == NULL)
    {
        FLOG_FATAL ("ranging_para malloc memory error");
    }

    memset (ranging_para, 0, sizeof(struct phy_ranging_para));


    ranging_rru_para = (struct ranging_rru_para *)malloc(sizeof(struct ranging_rru_para));

    if (ranging_rru_para == NULL)
    {
        FLOG_FATAL ("ranging_rru_para malloc memory error");
    }

    memset (ranging_rru_para, 0, sizeof(struct ranging_rru_para));


    ret = get_ranging_config ( ranging_rru_para, ranging_para );

    if (ret != 0)
    {
        FLOG_FATAL ("get_ranging_config error");
    }

    ret = phy_ul_ranging_init ( phy_ul_para,  ranging_para, ranging_rru_para);

    if (ret != 0)
    {
        FLOG_FATAL ("initial ranging error");
    }


    pthread_create (&phy_dl_bs_thread, NULL, phy_dl_bs, NULL);
    pthread_create (&phy_ul_bs_thread, NULL, phy_ul_bs, NULL);

    pthread_create (&phy_ranging_bs_thread, NULL, phy_ul_ranging_bs, NULL);

    phy_dl_thdflag = 1;

    return 0;

}

int phy_bs_release (void)
{
    int ret = 0;

    if (phy_dl_thdflag == 0)
    {
        return 0;
    }

    pthread_cancel (phy_dl_bs_thread);
    pthread_join (phy_dl_bs_thread, NULL);

    pthread_cancel (phy_ul_bs_thread);
    pthread_join (phy_ul_bs_thread, NULL);

    pthread_cancel (phy_ranging_bs_thread);
    pthread_join (phy_ranging_bs_thread, NULL);


    ret = phy_ul_ranging_deinit ( ranging_para, ranging_rru_para);

    if (ret != 0)
    {
        FLOG_FATAL ("phy_ul_ranging_deinit error");
    }


    ret = phy_dl_tx_deinit (phy_dl_para);

    if (ret != 0)
    {
        FLOG_FATAL ("phy_dl_tx_deinit error");
    }

    ret = phy_ul_rx_deinit (phy_ul_para);

    if (ret != 0)
    {
        FLOG_FATAL ("phy_dl_tx_deinit error");
    }

    if (phy_dl_para != NULL)
    {
        free (phy_dl_para);
        phy_dl_para = NULL;
    }

    if (phy_ul_para != NULL)
    {
        free (phy_ul_para);
        phy_ul_para = NULL;
    }

    if (phy_dts_para != NULL)
    {
        free (phy_dts_para);
        phy_dts_para = NULL;
    }

    if (phy_g_para != NULL)
    {
        free (phy_g_para);
        phy_g_para = NULL;
    }

    if (ranging_para != NULL)
    {
        free (ranging_para);
        ranging_para = NULL;
    }

    if (ranging_rru_para != NULL)
    {
        free(ranging_rru_para);
        ranging_rru_para = NULL;
    }

    phy_dl_thdflag = 0;
    phy_ul_thdflag = 0;

    FLOG_INFO("PHY layer released");

    return 0;
}

static int get_phy_config (struct phy_global_param * phy_config,
        struct phy_dts_info *dts_config)
{
    int ret;
    int i;
    char tmp_string[128];

    /* DL system parameters */
    ret = get_global_param ("PHY_BANDWIDTH", & ( phy_config->bandwidth ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_BANDWIDTH error\n");
    }

    ret = get_global_param ("PHY_OFDM_FS", & ( phy_config->ofdma_fs ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_OFDM_FS error\n");
    }

    ret = get_global_param ("PHY_OVERSAMPLING", & ( phy_config->oversampling ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_OVERSAMPLING error\n");
    }

    ret = get_global_param ("PHY_MULTIPLE_ZONE",
                    & ( phy_config->multiple_zone ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_MULTIPLE_ZONE error\n");
    }

    ret = get_global_param ("TX_ANTENNA_NUM", & ( phy_config->tx_div ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters TX_ANTENNA_NUM error\n");
    }

    ret = get_global_param ("PHY_DL_FRAME_IDX",
                    & ( phy_config->dl_frame_index ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_FRAME_IDX error\n");
    }

    ret = get_global_param ("PHY_DL_SYMBOL_OFFSET",
            & ( phy_config->dl_symbol_offset ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_SYMBOL_OFFSET error\n");
    }

    ret = get_global_param ("PHY_DL_FIRST_ZONE",
                    & ( phy_config->dl_first_zone ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_FIRST_ZONE error\n");
    }
    
    ret = get_global_param ("PHY_CCD_NUM", & ( phy_config->cdd_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_CCD_NUM error\n");
    }

    ret = get_global_param ("PHY_DL_MIMO_MODE", & ( phy_config->dl_mimo_mode ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_MIMO_MODE error\n");
    }

    ret = get_global_param ("SYMBOL_NUM_IN_DL_FRAME",
            & ( phy_config->symbolnum_per_dl_frame ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_DL_FRAME error\n");
    }

    phy_config->symbolnum_per_dl_frame -= 1;

    ret = get_global_param ("PHY_DL_ZONE_LEN", & ( phy_config->dl_zone_len ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_ZONE_LEN error\n");
    }

    ret = get_global_param ("PHY_DL_PERMUTATION_TYPE",
            & ( phy_config->dl_permutation_type ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_PERMUTATION_TYPE error\n");
    }

    ret = get_global_param ("PHY_OFDMA_NFFT", & ( phy_config->ofdma_nfft ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_OFDMA_NFFT error\n");
    }

    ret = get_global_param ("PHY_OFDMA_G", & ( phy_config->ofdma_g ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_OFDMA_G error\n");
    }

    ret = get_global_param ("PHY_DL_PERMBASE", & ( phy_config->dl_permbase ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_DL_PERMBASE error\n");
    }

    ret = get_global_param ("PHY_MAX_CELLNUM", & ( phy_config->max_cellnum ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_MAX_CELLNUM error\n");
     }

    ret = get_global_param ("PHY_MAX_SEGNUM", & ( phy_config->max_segnum ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_MAX_SEGNUM error\n");
    }

    ret = get_global_param ("PHY_PRBS_ID", & ( phy_config->prbs_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_PRBS_ID error\n");
    }

    ret = get_global_param ("PHY_PILOT_INSERT", & ( phy_config->pilot_insert ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_PILOT_INSERT error\n");
    }

    ret = get_global_param ("PHY_PREAMBLE_IDX",
                    & ( phy_config->preamble_index ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_PREAMBLE_IDX error\n");
    }

    ret = get_global_param ("PHY_PREAMBLE_GUARD_BAND",
            & ( phy_config->preamble_guard_band ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_PREAMBLE_GUARD_BAND error\n");
    }

    ret = get_global_param ("PHY_PREAMBLE_MAX_RUN_IDX",
            & ( phy_config->preamble_max_running_index ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_PREAMBLE_MAX_RUN_IDX error\n");
    }

    ret = get_global_param ("FCH_BITMAP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FCH_BITMAP error\n");
    }

    tmp_string[6] = 0;

    for (i = 5; i >= 0; i--)
    {
        phy_config->fch_bitmap[i] = atoi(&tmp_string[i]);
        tmp_string[i] = 0;
    }

    ret = get_global_param ("PHY_STC_RATE", & ( phy_config->stc_rate ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_STC_RATE error\n");
    }

    ret = get_global_param ("PHY_STCLAYER_NUM",
            & ( phy_config->stclayer_num ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_STCLAYER_NUM error\n");
    }

    ret = get_global_param ("PHY_CODING_TYPE",
            & ( phy_config->coding_type ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_CODING_TYPE error\n");
    }

    ret = get_global_param ("RX_ANTENNA_NUM", & ( phy_config->rx_div ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RX_ANTENNA_NUM error\n");
    }

    ret = get_global_param ("BS_ID", & ( phy_config->bs_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BS_ID error\n");
    }

    ret = get_global_param ("PHY_FRAME_INC",
            & ( phy_config->frame_increased ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_FRAME_INC error\n");
    }

    ret = get_global_param ("PHY_UL_ZONE_BEGIN",
            & ( phy_config->ul_zone_begin ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_ZONE_BEGIN error\n");
    }

    ret = get_global_param ("PHY_UL_ZONE_LEN",
            & ( phy_config->ul_zone_len ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_ZONE_LEN error\n");
    }

    ret = get_global_param ("PHY_UL_PERMUTATION_TYPE",
            & ( phy_config->ul_permutation_type ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_PERMUTATION_TYPE error\n");
    }

    ret = get_global_param ("SYMBOL_NUM_IN_UL_FRAME",
            & ( phy_config->symbolnum_per_ul_frame ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
    }

    ret = get_global_param ("PHY_UL_PERMBASE",
            & ( phy_config->ul_permbase ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_PERMBASE error\n");
    }


    ret = get_global_param ("UL_BITMAP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters UL_BITMAP error\n");
    }

    tmp_string[35] = 0;

    for (i = 34; i >= 0; i--)
    {
        phy_config->ul_bitmap[i] = atoi(&tmp_string[i]);
        tmp_string[i] = 0;
    }

    ret = get_global_param ("PHY_RANGING_SYMOFFSET",
            & ( phy_config->ranging_symoffset ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_RANGING_SYMOFFSET error\n");
    }

    ret = get_global_param ("PHY_RANGING_SUBCHOFFSET",
            & ( phy_config->ranging_subchoffset ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_RANGING_SUBCHOFFSET error\n");
    }

    ret = get_global_param ("PHY_RANGING_SYM",
            & ( phy_config->ranging_sym ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_RANGING_SYM error\n");
    }


    ret = get_global_param ("PHY_RANGING_SUBCH",
            & ( phy_config->ranging_subch ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_RANGING_SUBCH error\n");
    }

    ret = get_global_param ("PHY_UL_MIMO_MODE",
            & ( phy_config->ul_mimo_mode ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_MIMO_MODE error\n");
    }

    ret = get_global_param ("PHY_UL_CODING_TYPE",
            & ( phy_config->ul_coding_type ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PHY_UL_CODING_TYPE error\n");
    }

    ret = get_global_param ("CALIBRATE_DIGITAL_POWER0", & (phy_config->cali_digi_pwr[0]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER0 error\n");
        return 1;
    }

    ret = get_global_param ("CALIBRATE_DIGITAL_POWER1", & (phy_config->cali_digi_pwr[1]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER1 error\n");
        return 1;
    }

     ret = get_global_param ("CALIBRATE_ANALOG_POWER0", & ( phy_config->cali_ana_pwr[0]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_ANALOG_POWER0 error\n");
    }


    ret = get_global_param ("CALIBRATE_ANALOG_POWER1", & ( phy_config->cali_ana_pwr[1] ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_ANALOG_POWER1 error\n");
    }


    ret = get_global_param ("CALIBRATE_DIGTAL_GAIN0", & ( phy_config->cali_dgain[0] ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGTAL_GAIN0 error\n");
    }


    ret = get_global_param ("CALIBRATE_DIGTAL_GAIN1", & ( phy_config->cali_dgain[1]));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGTAL_GAIN1 error\n");
    }


    if (get_global_param ("INTERFERENCE_ACTIVE", tmp_string) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }else
    {
        tmp_string[21] = 0;

        for (i = 20; i >= 0; i--)
        {
            dts_config->active_band[i] = atoi (&(tmp_string[i]));
            tmp_string[i] = 0;
        }
    }

    if (get_global_param ("NUM_DL_INTERFERENCE",
            & ( dts_config->dl_unused_subch )) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }

    if (get_global_param ("NUM_UL_INTERFERENCE",
            & ( dts_config->ul_unused_subch )) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }

     ret = get_global_param ("SOFTBIT_THRESHOLD",
            & ( phy_config->softbit_threshold ));

    if (ret != 0)
    {
        FLOG_WARNING ("get softbit threshold factor error\n");
    }

    ret = get_global_param ("SOFTBIT_SHIFT",
            & ( phy_config->softbit_shift ));

    if (ret != 0)
    {
        FLOG_WARNING ("get softbit shift error\n");
    }

    ret = get_global_param ("PERIODIC_SENSING_THD",
            & ( phy_config->ps_thd ));

    if (ret != 0)
    {
        FLOG_WARNING ("get PS threshold factor error\n");
    }

    ret = get_global_param ("DEFAULT_WORKING_FREQ",
            & ( phy_config->fre_central ));

    if (ret != 0)
    {
        FLOG_WARNING ("get central frequency error\n");
    }
    
    return 0;

}

static int get_ranging_config ( struct ranging_rru_para * ranging_rrh_config,
        struct phy_ranging_para * ranging_config )
{
    int ret;
    int i;
    char tmp_string[128];
    unsigned int value_ui = 0;
    float value_f = 0.0f;
    char *search = ",";
    char *token = NULL;


    ret = get_global_param ("IR_HO_CODE_SET_NUM", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IR_HO_CODE_SET_NUM error\n");
    }

    ranging_config->ir_ranging_codeset_num = value_ui;


    ret = get_global_param ( "IR_HO_CODE_SET", tmp_string );

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IR_HO_CODE_SET error\n");
    }

    token = strtok(tmp_string, search);

    if (token != NULL)
    {
        ranging_config->ir_codeset_ID_array[0] = atoi(token);

    }else
    {
        FLOG_WARNING ("get parameters IR_HO_CODE_SET error\n");
    }

    for(i = 1; i < (int)ranging_config->ir_ranging_codeset_num; i++)
    {
        token = strtok(NULL, search);

        if (token != NULL)
        {
            ranging_config->ir_codeset_ID_array[i] = atoi(token);
        }else
        {
            FLOG_WARNING ("get parameters IR_HO_CODE_SET error\n");
        }
    }


    ret = get_global_param ("PR_BR_CODE_SET_NUM", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PR_BR_CODE_SET_NUM error\n");
    }

    ranging_config->pr_ranging_codeset_num = value_ui;


    ret = get_global_param ( "PR_BR_CODE_SET", tmp_string );

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PR_BR_CODE_SET error\n");
    }

    token = strtok(tmp_string, search);

    if (token != NULL)
    {
        ranging_config->pr_codeset_ID_array[0] = atoi(token);
    }else
    {
        FLOG_WARNING ("get parameters PR_BR_CODE_SET_NUM error\n");
    }

    for(i = 1; i < (int)ranging_config->pr_ranging_codeset_num; i++)
    {
        token = strtok(NULL, search);

        if (token != NULL)
        {
            ranging_config->pr_codeset_ID_array[i] = atoi(token);
        }else
        {
            FLOG_WARNING ("get parameters PR_BR_CODE_SET_NUM error\n");
        }
    }


    ret = get_global_param ("IR_SYMBOL_OFFSET", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IR_SYMBOL_OFFSET error\n");
    }

    ranging_config->symbol_offset_ir = value_ui;


    ret = get_global_param ("PR_SYMBOL_OFFSET", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PR_SYMBOL_OFFSET error\n");
    }

    ranging_config->symbol_offset_pr = value_ui;


    ret = get_global_param ("IR_SUBCH_OFFSET", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IR_SUBCH_OFFSET error\n");
    }

    ranging_config->subch_offset_ir = value_ui;


    ret = get_global_param ("PR_SUBCH_OFFSET", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters PR_SUBCH_OFFSET error\n");
    }

    ranging_config->subch_offset_pr = value_ui;


    ret = get_global_param ("RANGING_THRESHOLD", & ( value_f ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RANGING_SNR_THRESHOLD error\n");
    }

    ranging_config->ranging_threshold = value_f;

    ret = get_global_param ("RANGING_SNR_THRESHOLD", & ( value_f ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RANGING_SNR_THRESHOLD error\n");
    }

    ranging_config->ranging_snr_threshold = value_f;

    ret = get_global_param ("CALIBRATE_ANALOG_POWER0", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_ANALOG_POWER0 error\n");
    }

    ranging_rrh_config->cali_ana_pwr0 = value_ui;


    ret = get_global_param ("CALIBRATE_DIGITAL_POWER0", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER0 error\n");
    }

    ranging_rrh_config->cali_digi_pwr0 = value_ui;


    ret = get_global_param ("CALIBRATE_ANALOG_POWER1", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_ANALOG_POWER1 error\n");
    }

    ranging_rrh_config->cali_ana_pwr1 = value_ui;


    ret = get_global_param ("CALIBRATE_DIGITAL_POWER1", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGITAL_POWER1 error\n");
    }

    ranging_rrh_config->cali_digi_pwr1 = value_ui;


    ret = get_global_param ("EXPECT_ANALOG_POWER", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters EXPECT_ANALOG_POWER error\n");
    }

    sscanf(tmp_string, "%d", &(ranging_rrh_config->expect_ana_pwr));

    ret = get_global_param ("CALIBRATE_DIGTAL_GAIN0", & ( value_f ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGTAL_GAIN0 error\n");
    }

    ranging_rrh_config->cali_dgain0 = value_f;


    ret = get_global_param ("CALIBRATE_DIGTAL_GAIN1", & ( value_f ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CALIBRATE_DIGTAL_GAIN1 error\n");
    }

    ranging_rrh_config->cali_dgain1 = value_f;

    ret = get_global_param ("RANGING_ALGORITHMS", & ( value_ui ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RANGING_ALGORITHMS error\n");
    }

    ranging_config->ranging_algorithms = value_ui;

    return 0;

}

int cfg_update_phy_interference(int type, int len, void * buf)
{
    char * tmp_string = (char *)buf;
    int i;

    tmp_string[21] = 0;

    for (i = 20; i >= 0; i--)
    {
        phy_dl_para->fixed_active_band[i] = atoi (&(tmp_string[i]));
        phy_ul_para->fixed_active_band[i] = atoi (&(tmp_string[i]));

        tmp_string[i] = 0;
    }

    g_periodic_sensing_reset = 1;
    g_periodic_sensing_enable = 1;

    (void) type;
    (void) len;

    return 0;
}

int cfg_update_ranging_exp_power(int type, int len, void * buf)
{
//    ranging_rru_para->expect_ana_pwr = *((float *)buf);

    sscanf((char *)buf, "%d", &(ranging_rru_para->expect_ana_pwr));

    (void) type;
    (void) len;

    return 0;
}

int cfg_update_ranging_algorithms(int type, int len, void * buf)
{
    ranging_para->ranging_algorithms = *((int *)buf);

    (void) type;
    (void) len;

    return 0;
}

