/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_rx_ranging.c

   Change Activity:

   Date             	Description of Change                            By
   -----------      ---------------------                 --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#ifdef IPP_OPT_TEMP730
#include "ipp.h"
#include "ipps.h"
#endif

#include "phy_periodic_sensing.h"

#include "phy_ul_rx_interface.h"
#include "phy_ul_rx_ranging.h"
#include "wntk_queue.h"
#include "queue_util.h"
#include "fdebug.h"
#include "fft.h"
#include "phy_ul_zonepermutation.h"
#include "phy_ul_rx.h"
#include "phy_proc.h"

#include "flog.h"
#include "bs_cfg.h"
#include "bs_debug.h"
#include "monitor_proc.h"

/* turn on/off dump according to DUMP_PHY_UL_RX_RANGING setting */
#ifndef DUMP_PHY_UL_RX_RANGING

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"
#include "prof_util.h"

static char ranging_root[128];
static int rf_flag = 0;

static int ranging_method_flag = 0; 
	// = 0: The original method: mean_0 * threshold
	// = 1: The SNR method

static float g_expect_ana_pwr = 0.0F;
static float g_noise_figure = 0.0F;

/**----------------------------------

    Tools for data dump

-----------------------------------*/

int dump_int_array(int *buf, const int start_num, const int end_num)

{

    int i;

    printf("\n \n");

    for (i = start_num-1; i < end_num; i++)

        printf("%d  ", buf[i]);

    printf("\n \n ");

    (void)buf;

    return 0;

   

}



int dump_float_array(float *buf, const int start_num, const int end_num)

{

    int i;

    printf("\n \n");

    for (i = start_num-1; i < end_num; i++)

        printf("%f  ", buf[i]);

    printf("\n \n ");

    return 0;
 

}


int dump_cf_array(float *buf_i, float *buf_q, const int start_num, const int 
end_num)

{

    int i;

    printf("\n \n ");

    for (i = start_num-1; i < end_num; i++)

        printf("%f+%fi  ", buf_i[i], buf_q[i]);

    printf("\n \n ");

    return 0;

}

/**----------------------------------

   It should be provided by adaptor or others

-----------------------------------*/

int32_t get_ranging_symbol_offset_ir(u_int32_t *symbol_offset_ir)
{

    *symbol_offset_ir = DEFAULT_IR_SYMBOL_OFFSET;

    return 0;
}

int32_t get_ranging_symbol_offset_pr(u_int32_t *symbol_offset_pr)
{
    *symbol_offset_pr = DEFAULT_PR_SYMBOL_OFFSET;

    return 0;
}


int32_t get_ranging_subch_offset_ir(u_int32_t *subch_offset_ir)
{
    *subch_offset_ir = DEFAULT_IR_SUBCH_OFFSET;

    return 0;
}

int32_t get_ranging_subch_offset_pr(u_int32_t *subch_offset_pr)
{
    *subch_offset_pr = DEFAULT_PR_SUBCH_OFFSET;

    return 0;
}

#ifdef __RANGING_UT__

int32_t get_ir_codeset_id(u_int32_t *ranging_codeset_id)

{

    int temp_buf[IR_CODE_SET_NUM] = IR_CODE_SET;

    int i;



    for (i = 0; i < IR_CODE_SET_NUM; i++)

    {

        ranging_codeset_id[i] = temp_buf[i];

    }

    return 0;

}


/**----------------------------------

   It should be provided by adaptor or others

-----------------------------------*/

int32_t get_pr_codeset_id(u_int32_t *ranging_codeset_id)

{

    int temp_buf[PR_CODE_SET_NUM] = PR_CODE_SET;

    int i;



    for (i = 0; i < PR_CODE_SET_NUM; i++)

    {

        ranging_codeset_id[i] = temp_buf[i];

    }

    return 0;

}
#endif

int32_t fftshift(float *buf_i, float *buf_q, const u_int32_t fft_len)
{
    float temp;
    u_int32_t i, j;
    
    for(i = 0; i < (fft_len >> 1); i++)
    {
        j = (fft_len >> 1) + i;
        temp = buf_i[j];
        buf_i[j] = buf_i[i];
        buf_i[i] = temp;

        temp = buf_q[j];
        buf_q[j] = buf_q[i];
        buf_q[i] = temp;
       
    }

    return 0;
}

/**----------------------------------------------------------------------------

   Function:    phy_rru_parameter_init()

   Description: To initiate the parameter for RRU.
	1.	To initiate the RRU parameters such as cali_dgain, expect_ana_pwr, etc. (rru_para)


   Parameters:

                Input-  
		

                Output- 

                     [struct phy_ranging_para *ranging_parameter]	
                       		The pointer refers to the structure of ranging parameters.

   Return Value:

                0       Success

                150    Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */
#ifdef __RANGING_UT__
int phy_rru_parameter_init(struct ranging_rru_para *rru_parameter)
{
    rru_parameter->cali_ana_pwr0 = CALIBRATE_ANALOG_POWER0;
	
    rru_parameter->cali_digi_pwr0 = CALIBRATE_DIGITAL_POWER0;

    rru_parameter->expect_ana_pwr = EXPECT_ANALOG_POWER;

    rru_parameter->cali_dgain0 = CALIBRATE_DIGTAL_GAIN0;

    rru_parameter->cali_dgain1 = CALIBRATE_DIGTAL_GAIN1;	

//    rru_parameter->dgain0 = DIGTAL_GAIN0;
//    rru_parameter->dgain1 = DIGTAL_GAIN1;	

    return 0;
}
#endif

/**----------------------------------------------------------------------------

   Function:    phy_ranging_parameter_init()

   Description: To initiate the ranging paramters.

   Parameters:

                Input-  
		

                Output- 

                     [struct phy_ranging_para *ranging_parameter]	
                       		The pointer refers to the structure of ranging parameters.

   Return Value:

                0       Success

                150    Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */
#ifdef __RANGING_UT__
int32_t phy_ranging_parameter_init(const struct phy_ul_rx_syspara *phy_ul_sys_parameter, struct phy_ranging_para *ranging_parameter)
{
    FILE *pf_ranging;

    char tmp_string[256];

    rf_flag = get_global_param ("RANGING_FILE_PATH", ranging_root);

    if (rf_flag != 0)
    {
        FLOG_WARNING ("get parameters RANGING_FILE_PATH error\n");
    }

    ranging_parameter->ir_ranging_codeset_num = IR_CODE_SET_NUM;

    ranging_parameter->pr_ranging_codeset_num = PR_CODE_SET_NUM;

    get_ir_codeset_id(ranging_parameter->ir_codeset_ID_array);

    get_pr_codeset_id(ranging_parameter->pr_codeset_ID_array);

    ranging_parameter->cp_len = phy_ul_sys_parameter->ofdma_ncp;

    ranging_parameter->fft_len = phy_ul_sys_parameter->ofdma_nfft;

    if (rf_flag != 0)
    {
        sprintf(tmp_string, "%s", "ranging_code.dat");
    }else
    {
        sprintf(tmp_string, "%s%s", ranging_root, "ranging_code.dat");
    }

    pf_ranging = fopen(tmp_string, "rb+");

    if (pf_ranging)

    {

        fread(ranging_parameter->ranging_codeset, sizeof(int32_t), TOTAL_CODE_SET_MAX_NUM 
			* RANGE_CODE_LEN, pf_ranging);

        fclose(pf_ranging);

    }

    else

    {

        printf("Can not open the ranging code file: %s !\n", tmp_string);

  	return ERROR_CODE;

    }

    ranging_parameter->ranging_threshold = RANGING_SNR_THRESHOLD;

    return 0;

}
#else
int32_t phy_ranging_parameter_init(const struct phy_ul_rx_syspara *phy_ul_sys_parameter, struct phy_ranging_para *ranging_parameter)
{

    FILE *pf_ranging;

    char tmp_string[256];

    rf_flag = get_global_param ("RANGING_FILE_PATH", ranging_root);

    if (rf_flag != 0)
    {
        FLOG_WARNING ("get parameters RANGING_FILE_PATH error\n");
    }

    ranging_parameter->cp_len = phy_ul_sys_parameter->ofdma_ncp;

    ranging_parameter->fft_len = phy_ul_sys_parameter->ofdma_nfft;

    if (rf_flag != 0)
    {
        sprintf(tmp_string, "%s", "ranging_code.dat");
    }else
    {
        sprintf(tmp_string, "%s%s", ranging_root, "ranging_code.dat");
    }

    pf_ranging = fopen(tmp_string, "rb+");

    if (pf_ranging)

    {

        fread(ranging_parameter->ranging_codeset, sizeof(int32_t), TOTAL_CODE_SET_MAX_NUM
            * RANGE_CODE_LEN, pf_ranging);

        fclose(pf_ranging);
	FLOG_INFO("RANGING CODE SET:\n");
//        dump_int_array(ranging_parameter->ranging_codeset, 1, 20);

    }

    else

    {

        printf("Can not open the ranging code file: %s !\n", tmp_string);

        return ERROR_CODE;

    }


    return 0;

}
#endif

/**----------------------------------------------------------------------------

   Function:    phy_ul_ranging_init()

   Description: System initialization for ranging.
	This is the function to do the initialization for all the ranging preparation, including those for initial ranging (IR) and periodic ranging (PR). It includes
	1.	To get the whole ranging code sets (entire system ranging code set)
	2.	To get the number of ranging codes to be used for IR 
	3.	To get the number of ranging codes to be used for PR
	4.	To get the code indexes of IR ranging codes
	5.	To get the code indexes of PR ranging codes
	6.	To get the ranging sub-carrier index for both IR and PR
	7.	To initiate the RRU parameters such as cali_dgain, expect_ana_pwr, etc. (rru_para)
	8.	To initiate the phy_ir_result buffer and phy_pr_result buffer.

   Parameters:

                Input-  
		

                Output- 

                     [struct phy_ranging_para *ranging_parameter]	
                       		The pointer refers to the structure of ranging parameters.
			[struct rru_para *rru_parameter]		
					The pointer refers to the structure of RRU pa-rameters which will be used in ranging.
			[struct phy_ir_result *ir_result]
					The pointer refers to the IR result buffer, which will be filled by ranging function.
			[struct phy_pr_result *pr_result] 	
					The pointer refers to the PR result buffer, which will be filled by ranging function.

   Return Value:

                0       Success

                150    Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */

int32_t	phy_ul_ranging_init(const struct phy_ul_rx_syspara 
                                   *phy_ul_sys_parameter,
                            struct phy_ranging_para *ranging_parameter, 
                            struct ranging_rru_para *rru_parameter)
{

//    struct phy_ranging_para *phy_ranging_parameter;


    FILE *pf_ranging;
    char tmp_string[256];

    if (ranging_parameter == NULL) 
    {
        printf("E007_ranging: the memory allowcation of ranging_parameter failed!\n");
        return ERROR_CODE;
    }    


    if (rru_parameter == NULL) 
    {
        printf("E006_ranging: the memory allocation of rru_parameter failed!\n");
        return ERROR_CODE;
    }    

    rf_flag = get_global_param ("RANGING_FILE_PATH", ranging_root);

    if (rf_flag != 0)
    {
        FLOG_WARNING ("get parameters RANGING_FILE_PATH error\n");
    }

#ifdef __RANGING_UT__
    phy_rru_parameter_init(rru_parameter);
#endif

    phy_ranging_parameter_init(phy_ul_sys_parameter, ranging_parameter);

    if (rf_flag != 0)
    {
        sprintf(tmp_string, "%s", "ranging_corr_code.dat");
    }else
    {
        sprintf(tmp_string, "%s%s", ranging_root, "ranging_corr_code.dat");
    }
	
    pf_ranging = fopen(tmp_string, "r");

    if (pf_ranging)
    {

        fread(ranging_parameter->ranging_corr_codeset, sizeof(float), 
			TOTAL_CODE_SET_MAX_NUM * (RANGE_CODE_LEN>>2) * 3, pf_ranging);

        fclose(pf_ranging);


    }

    else

    {

        FLOG_ERROR("Can not open the ranging code file: %s !\n", tmp_string);

        return ERROR_CODE;

    }
	
#ifdef IPP_OPT_TEMP730

    ippsFFTInitAlloc_C_32f(&(ranging_parameter->pFFTSpecFwd), (int)(log(
OFDMA_SYMBOL_SIZE)/log(2)), IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);

    ippsFFTInitAlloc_C_32f(&(ranging_parameter->pFFTSpecInv), (int)(log(
OFDMA_SYMBOL_SIZE)/log(2)), IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);

    int BufSizeFwd, BufSizeInv;

    ippsFFTGetBufSize_C_32f(ranging_parameter->pFFTSpecFwd, &BufSizeFwd);

    ippsFFTGetBufSize_C_32f(ranging_parameter->pFFTSpecInv, &BufSizeInv);

    ranging_parameter->BufFwd = malloc(BufSizeFwd*sizeof(Ipp8u));

    ranging_parameter->BufInv = malloc(BufSizeInv*sizeof(Ipp8u));
    if(ranging_parameter->BufFwd == NULL || ranging_parameter->BufInv == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_ranging_init\n");
        return -1;
    } 
#else

    fft_init(OFDMA_SYMBOL_SIZE, &ranging_parameter->XX, &
ranging_parameter->x, &ranging_parameter->X);

#endif

    return 0;
}


/**----------------------------------------------------------------------------

   Function:    phy_ul_ranging_free()

   This is the function to free all the allocated buffer for ranging, 
   which will be allocated in phy_ul_ranging_init()

   Parameters:

                Input-  
                     [struct phy_ranging_para *phy_ranging_parameter]	
                     	The pointer refers to the struc-ture of ranging parameters.
			[struct rru_para *rru_parameter]		
				The pointer refers to the structure of RRU pa-rameters which will be used 
				in ranging.
			[struct phy_ir_result *ir_result]	
				The pointer refers to the IR result buffer, which will be filled by ranging 
				function.
			[struct phy_pr_result *pr_result] 	
				The pointer refers to the PR result buffer, which will be filled by ranging 
				function.


                Output- 


   Return Value:

                0       Success
                150    Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_ul_ranging_deinit (struct phy_ranging_para *phy_ranging_parameter, 
	                             struct ranging_rru_para *rru_parameter)
//	                             struct phy_ranging_result *ranging_result)
{
    FLOG_INFO("ranging deinit\n");

#ifdef IPP_OPT_TEMP730

    ippsFFTFree_C_32f(phy_ranging_parameter->pFFTSpecFwd);
    ippsFFTFree_C_32f(phy_ranging_parameter->pFFTSpecInv);
    free(phy_ranging_parameter->BufFwd);
    free(phy_ranging_parameter->BufInv);
#else /* non IPP_OPT_TEMP730 */
    fft_quit(phy_ranging_parameter->XX, phy_ranging_parameter->x, 
             phy_ranging_parameter->X);
#endif
//    free(phy_ranging_parameter);
    (void)rru_parameter;

//    free(rru_parameter);
//    free(pr_result);
	
    return 0;
}


int32_t ranging_timeoffset_estimation(
	const struct phy_ranging_para *phy_ranging_parameter,
	const float *recv_ranging_data_i,
	const float *recv_ranging_data_q,
	const u_int32_t ranging_code_id,
        int32_t *time_offset,
	const u_int32_t ranging_mode)
{
#ifdef VSXOPT
    float ranging_ch_f_i[MAX_FFT_SIZE] __attribute__ ((aligned (128))), ranging_ch_f_q[MAX_FFT_SIZE] __attribute__ ((aligned (128)));
    float ranging_ch_t_i[MAX_FFT_SIZE] __attribute__ ((aligned (128))), ranging_ch_t_q[MAX_FFT_SIZE] __attribute__ ((aligned (128)));
#else
    float ranging_ch_f_i[MAX_FFT_SIZE], ranging_ch_f_q[MAX_FFT_SIZE];
    float ranging_ch_t_i[MAX_FFT_SIZE], ranging_ch_t_q[MAX_FFT_SIZE];
#endif
    int i, p, max_path, tmp_offset, tmp_offset_1;
    float max_val, temp_val;
    int *p_ranging_codeset;	

    p_ranging_codeset = (int *)(&(phy_ranging_parameter->ranging_codeset[0]));

//    dump_int_array(p_ranging_codeset, 1, 20);

    for (i = 0; i < (int)phy_ranging_parameter->fft_len; i++)
    {
            ranging_ch_f_i[i] = 0;
            ranging_ch_f_q[i] = 0;			
    }

    p = ranging_code_id * RANGE_CODE_LEN;

    if (ranging_mode == PERIODIC_RANGING_FLAG)
    {
        tmp_offset = 2 * 144;
	tmp_offset_1 = 2048;
    }
    else
    {
        tmp_offset = 144;
	tmp_offset_1 = 1024;
    }

    

    for (i = 0; i < RANGE_CODE_LEN; i++)
    {
//        FLOG_INFO("p+i is %d, p_ranging_codeset[] is %d\n", p+i, p_ranging_codeset[p+i]);

        ranging_ch_f_i[phy_ranging_parameter->ranging_allocation[i + tmp_offset] - tmp_offset_1] 
			= recv_ranging_data_i[i] * p_ranging_codeset[p + i];
        ranging_ch_f_q[phy_ranging_parameter->ranging_allocation[i + tmp_offset] - tmp_offset_1] 
			= recv_ranging_data_q[i] * p_ranging_codeset[p + i];
    }
/*     FLOG_INFO("ranging_code:\n");
    dump_int_array(p_ranging_codeset, p+1, p+10);
    dump_int_array(p_ranging_codeset, p+21, p+30);

   
    FLOG_INFO("recv_ranging_data:\n");
    dump_cf_array(recv_ranging_data_i, recv_ranging_data_q, 1, 10);
    dump_cf_array(recv_ranging_data_i, recv_ranging_data_q, 21, 30);

    FLOG_INFO("ranging_ch_f:\n");
    dump_cf_array(ranging_ch_f_i, ranging_ch_f_q, 100, 150);
    dump_cf_array(ranging_ch_f_i, ranging_ch_f_q, 200, 250);
    dump_cf_array(ranging_ch_f_i, ranging_ch_f_q, 300, 350);*/
//    dump_cf_array(recv_ranging_data_i, recv_ranging_data_q, 1, 20);
    

#ifdef IPP_OPT_TEMP730  

    ippsFFTInv_CToC_32f(ranging_ch_f_i, ranging_ch_f_q, ranging_ch_t_i, 
                ranging_ch_t_q,
                phy_ranging_parameter->pFFTSpecInv, 
                phy_ranging_parameter->BufInv);
/*    ippsFFTInv_CToC_32f(ranging_ch_t_i, ranging_ch_t_q, ranging_ch_f_i, 
                ranging_ch_f_q,
                phy_ranging_parameter->pFFTSpecInv, 
                phy_ranging_parameter->BufInv);*/


#else
#ifdef VSXOPT
    ifft_p( phy_ranging_parameter->fft_len, ranging_ch_t_i, ranging_ch_t_q, ranging_ch_f_i, 
                ranging_ch_f_q, 
	        phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                phy_ranging_parameter->X);
#else

    ifft( phy_ranging_parameter->fft_len, ranging_ch_t_i, ranging_ch_t_q, ranging_ch_f_i, 
                ranging_ch_f_q, 
	        phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                phy_ranging_parameter->X);
#endif
#endif

    max_path = 0;

    max_val = 0;

//    dump_cf_array(ranging_ch_t_i, ranging_ch_t_q, 1, 20);

    FLOG_INFO("fft: %d\n", phy_ranging_parameter->fft_len);

    //To find the max path in time domain	
    for (i = 0; i <= (int)phy_ranging_parameter->fft_len; i++)
    {
        temp_val = ranging_ch_t_q[i] * ranging_ch_t_q[i] 
			+ ranging_ch_t_i[i] * ranging_ch_t_i[i];
//	FLOG_INFO("%d=%f   ", i, temp_val);

        if (max_val <= temp_val)
        {
            max_val = temp_val;
	    max_path = i;
	}
    }

    FLOG_INFO("max_path: %d", max_path);

    if (ranging_mode == INITIAL_RANGING_FLAG)
    {
    	if ((max_path >= 0) && ( max_path < 750))
    	{
            max_path = max_path + phy_ranging_parameter->cp_len;
    	}
    	else
    	{
            max_path = max_path + phy_ranging_parameter->cp_len 
                               - phy_ranging_parameter->fft_len; 
    	}
    FLOG_INFO("max_path: %d", max_path);
    }
    else if (ranging_mode == PERIODIC_RANGING_FLAG)
    {
        if (max_path >= 512)
        {
            max_path = max_path -1024;
	}
    }
    else
    {
        return ERROR_CODE;
    }
		
    max_path = -max_path - 5;

    memcpy(time_offset, &max_path, sizeof(u_int32_t));
 
    return 0;
	
}


/**----------------------------------------------------------------------------

   Function:    phy_ul_ir()

   Description: This is the function for initial ranging.

   Parameters:

                Input-  
		  	[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_0]
				The pointer refers to the sampled data and related information from RRU on an-tenna 0.
			[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_1]
				The pointer refers to the sampled data and related information from RRU on an-tenna 1.
			[const struct rru_para *rru_parameter]	The RRU parameters to be used in ranging
			[const struct phy_ranging_para *phy_ranging_parameter]	
				The parameters to be used in ranging
			[const int ant_num]	The number of antennas


                Output- 
			[struct phy_ir_result ir_result]	The pointer to the buffer holding the IR result

   Return Value:

                0       Success
                150    Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_ul_ir (const struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_0, 
	                             const struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_1, 
	                             const struct ranging_rru_para *rru_parameter,
	                             const struct phy_ranging_para *phy_ranging_parameter, 
	                             const u_int32_t ant_num, 
	                             struct phy_ranging_result *ranging_result)
{
#ifdef VSXOPT
    float recv_symbol_data_0_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_0_q[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_1_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_1_q[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
    float recv_symbol_data_0_i[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_0_q[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_1_i[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_1_q[OFDMA_SYMBOL_SIZE];
#endif
    float recv_ranging_data_0_i[RANGE_CODE_LEN];
    float recv_ranging_data_0_q[RANGE_CODE_LEN]; 
    float recv_ranging_data_1_i[RANGE_CODE_LEN]; 
    float recv_ranging_data_1_q[RANGE_CODE_LEN];	
    float *p_recv_ranging_data_i;
    float *p_recv_ranging_data_q;		
    float correlation_array_0_i[RANGE_CODE_LEN];
    float correlation_array_0_q[RANGE_CODE_LEN];
    float correlation_array_1_i[RANGE_CODE_LEN]; 
    float correlation_array_1_q[RANGE_CODE_LEN];	

    int *tx_correlation_array;
    float sum_power_0, sum_power_1, mean_0, mean_1;	
    float sum_power_01, mean_01;	
    float cur_power, adj_power;
    float temp_i, temp_q, temp_power;
    float max_val_0, max_val_1;
    float expect_digital_frq_pwr;
    int32_t    max_id_0, max_id_1, temp_id;	
    int32_t    max_power_ant_id;
    int32_t    time_offset, user_num, user_judge;
    float adjust_gain;
    struct ranging_result_struct *p_result_0, *p_result_1;    

    
	
    u_int32_t i, j, p;

    int symbol_offset;

#ifdef DIGITAL_POWER_ESTIMATION
    struct phy_hook_ranging_power hook_power;
#endif

    user_num = 0;

    FLOG_INFO("IR detection start ...\n");    
    p_result_0 = &(ranging_result->ranging_result_0);

    p_result_1 = &(ranging_result->ranging_result_1);

    p_result_0->ranging_code_id = -1;

    p_result_1->ranging_code_id = -1;	

/*    FLOG_INFO("cali_d is %f, expect_ana is %f, cali_ana is %f \n", 
		rru_parameter->cali_digi_pwr0,
		rru_parameter->expect_ana_pwr,
		rru_parameter->cali_ana_pwr0);
*/
    expect_digital_frq_pwr = rru_parameter->cali_digi_pwr0 
		* exp10f((g_expect_ana_pwr - (int)rru_parameter->cali_ana_pwr0) / 10)
		* phy_ranging_parameter->fft_len / 756 / 8192 / 8192;

    if (phy_ranging_parameter->symbol_offset_ir > SYMBOLS_IN_SLOTSYMBOL)
    {
        printf("E011_ranging: The symbol offset out of range \n");
		
        return ERROR_CODE;
    }

    hook_power.ranging_type = 1; //for IR type
#ifdef DIGITAL_POWER_ESTIMATION

//    if (hook_debug_trace(HOOK_RANGING_POWER_IDX, NULL, 0, 0) > 0)
    {

        sum_power_0 = 0;

        for (i = 0; i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 2; i++)
        {
            sum_power_0 = sum_power_0 + 
		phy_ul_rru_symbol_ranging_0->symbol_i[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_i[i]
		+ phy_ul_rru_symbol_ranging_0->symbol_q[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_q[i];
        }

        mean_0 = sum_power_0 
	    / (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) / 2;

        sum_power_1 = 0;

        for (i = 0; i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 2; i++)
        {
            sum_power_1 = sum_power_1 + 
		phy_ul_rru_symbol_ranging_1->symbol_i[i] 
		* phy_ul_rru_symbol_ranging_1->symbol_i[i]
		+ phy_ul_rru_symbol_ranging_1->symbol_q[i] 
		* phy_ul_rru_symbol_ranging_1->symbol_q[i];
        }

        mean_1 = sum_power_1 
	    / (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) / 2;

        hook_power.frm_num = phy_ul_rru_symbol_ranging_0->frame_index;
        hook_power.mean_power_ant0 = (unsigned int)mean_0;
        hook_power.mean_power_ant1 = (unsigned int)mean_1;
        hook_power.dgain_ant0 = (int)phy_ul_rru_symbol_ranging_0->dgain;
        hook_power.dgain_ant1 = (int)phy_ul_rru_symbol_ranging_1->dgain;
        hook_power.noise_figure = g_noise_figure;

        hook_debug_trace(HOOK_RANGING_POWER_IDX, &hook_power, sizeof(struct phy_hook_ranging_power), 1);

        DO_DUMP(DUMP_RANGING_POWER_ID, 0, &hook_power, sizeof(struct phy_hook_ranging_power));


// To calculate the 3rd OFDM symbol which may contain PR or not
        sum_power_0 = 0;

        for (i = 0; i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len); i++)
        {
	    symbol_offset = (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) 
			* 2 + i;
 	    sum_power_0 = sum_power_0 +
		phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset] 
		* phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]
		+ phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset] 
		* phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset];
        }

        mean_0 = sum_power_0 
	    / (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len);

        sum_power_1 = 0;

        for (i = 0; i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len); i++)
        {

	    symbol_offset = (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) 
			* 2 + i;

            sum_power_1 = sum_power_1 + 
		phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset] 
		* phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]
		+ phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset] 
		* phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset];
        }

        mean_1 = sum_power_1 
	    / (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len);

        hook_power.mean_power_ant0 = (unsigned int)mean_0;
        hook_power.mean_power_ant1 = (unsigned int)mean_1;


        DO_DUMP(DUMP_RANGING_POWER_ID, 3, &hook_power, sizeof(struct phy_hook_ranging_power));

   }
/*
    else
    {
        hook_debug_trace(HOOK_RANGING_POWER_IDX, NULL, -1, 1);
    }
*/

/*
    printf("after adjust is %f, dgain is %f \n", mean_0, 
		phy_ul_rru_symbol_ranging_0->dgain);
*/

#endif

    //To get the second IR symbol for estimation
    symbol_offset = phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len * 2;


	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR_IFFT ));
#ifdef IPP_OPT_TEMP730  

    ippsFFTFwd_CToC_32f(&(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
                         recv_symbol_data_0_i, recv_symbol_data_0_q,
                         phy_ranging_parameter->pFFTSpecFwd,
                         phy_ranging_parameter->BufFwd);

#else
#ifdef VSXOPT
    fft_p( phy_ranging_parameter->fft_len,  &(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
	                 recv_symbol_data_0_i, recv_symbol_data_0_q,   
                         phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                         phy_ranging_parameter->X);
#else
    fft( phy_ranging_parameter->fft_len,  &(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
	                 recv_symbol_data_0_i, recv_symbol_data_0_q,   
                         phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                         phy_ranging_parameter->X);

#endif


#endif
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR_IFFT ));
//    dump_cf_array(recv_symbol_data_0_i, recv_symbol_data_0_q, 1, 20);

    fftshift(recv_symbol_data_0_i, recv_symbol_data_0_q, phy_ranging_parameter->fft_len);


    adjust_gain = exp10f((-1) * 
		((phy_ul_rru_symbol_ranging_0->dgain + rru_parameter->cali_dgain0) 
		/ 20.0))/ sqrt(phy_ranging_parameter->fft_len) / 8192;

    FLOG_INFO("Ant 0: DGAIN = %f, cali_dgain = %f, adjust_gain = %f\n", 
            phy_ul_rru_symbol_ranging_0->dgain, rru_parameter->cali_dgain0, adjust_gain);

//    adjust_gain = 1;
    for (i = 0; i < RANGE_CODE_LEN; i++)
    {
        j = phy_ranging_parameter->ranging_allocation[i + 144] - 1024; //use the second symbol
	
        recv_ranging_data_0_i[i] = recv_symbol_data_0_i[j] * adjust_gain;
        recv_ranging_data_0_q[i] = recv_symbol_data_0_q[j] * adjust_gain;		
    }

//    dump_cf_array(recv_ranging_data_0_i, recv_ranging_data_0_q, 1, 10);

    if (ant_num > 1)
    {
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR_IFFT ));
#ifdef IPP_OPT_TEMP730  
/*
        ippsFFT_CToC_32f(&(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]), 
                         &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
                         recv_symbol_data_1_i, recv_symbol_data_1_q,
                         phy_ranging_parameter->pFFTSpecInv, 
                         phy_ranging_parameter->BufInv);
*/
//by zzb
        ippsFFTFwd_CToC_32f(&(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
                         recv_symbol_data_1_i, recv_symbol_data_1_q,
                         phy_ranging_parameter->pFFTSpecFwd,
                         phy_ranging_parameter->BufFwd);
#else

#ifdef VSXOPT
        fft_p( phy_ranging_parameter->fft_len ,  &(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
	                 recv_symbol_data_1_i, recv_symbol_data_1_q,   
                         phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                         phy_ranging_parameter->X);
#else
        fft( phy_ranging_parameter->fft_len ,  &(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
	                 recv_symbol_data_1_i, recv_symbol_data_1_q,   
                         phy_ranging_parameter->XX, phy_ranging_parameter->x, 
                         phy_ranging_parameter->X);
#endif

#endif
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR_IFFT ));

        fftshift(recv_symbol_data_1_i, recv_symbol_data_1_q, phy_ranging_parameter->fft_len);

        adjust_gain = exp10f((-1) * ((phy_ul_rru_symbol_ranging_1->dgain + rru_parameter->cali_dgain1) / 20))/ sqrt(phy_ranging_parameter->fft_len) / 8192;

        FLOG_INFO("Ant 1: DGAIN = %f, cali_dgain = %f, adjust_gain = %f\n",
                phy_ul_rru_symbol_ranging_1->dgain, rru_parameter->cali_dgain1, adjust_gain);

        for (i = 0; i < RANGE_CODE_LEN; i++)
        {
            j = phy_ranging_parameter->ranging_allocation[i + 144] - 1024;
	
            recv_ranging_data_1_i[i] = recv_symbol_data_1_i[j] * adjust_gain;
            recv_ranging_data_1_q[i] = recv_symbol_data_1_q[j] * adjust_gain;		
        }
    }


    //To do the correlation for antenna 0
    for (i = 0; i < (RANGE_CODE_LEN >> 2); i++)
    {
        for (j = 0; j < 3; j++)
        {
            correlation_array_0_i[i * 3 + j] = recv_ranging_data_0_i[i * 4 + j] 
				  * recv_ranging_data_0_i[i * 4 + j + 1]
				+ recv_ranging_data_0_q[i * 4 + j] 
                                  * recv_ranging_data_0_q[i * 4 + j + 1];
			
            correlation_array_0_q[i * 3 + j] = recv_ranging_data_0_i[i * 4 + j] 
				  * recv_ranging_data_0_q[i * 4 + j + 1]
				- recv_ranging_data_0_q[i * 4 + j] 
                                  * recv_ranging_data_0_i[i * 4 + j + 1];

	}
    }

    sum_power_0 = 0;
    mean_0 = 0;
    
    //To calculate the power level of antenna 0
    for (i = 0; i < ((RANGE_CODE_LEN >> 2) * 3); i ++)
    {
            sum_power_0 = sum_power_0 + correlation_array_0_i[i] * correlation_array_0_i[i]
				+ correlation_array_0_q[i] * correlation_array_0_q[i];
    }


    mean_0 = sum_power_0 / ((RANGE_CODE_LEN >> 2) * 3);

    FLOG_INFO("Ant 0: The sum_power of correlation is %f, mean power is %f \n", 
            sum_power_0, mean_0);
    max_power_ant_id = 0;

    hook_power.mean_power_ant0_adj = mean_0;
    hook_power.mean_power_ant1_adj = 0;

    //To do the correlation for antenna 1
    if (ant_num > 1)
    {
        for (i = 0; i < (RANGE_CODE_LEN >> 2); i++)
        {
            for (j = 0; j < 3; j++)
            {
                correlation_array_1_i[i * 3 + j] = recv_ranging_data_1_i[i * 4 + j] 
				  * recv_ranging_data_1_i[i * 4 + j + 1]
				+ recv_ranging_data_1_q[i * 4 + j] 
                                  * recv_ranging_data_1_q[i * 4 + j + 1];
			
                correlation_array_1_q[i * 3 + j] = recv_ranging_data_1_i[i * 4 + j] 
				  * recv_ranging_data_1_q[i * 4 + j + 1]
				- recv_ranging_data_1_q[i * 4 + j] 
                                  * recv_ranging_data_1_i[i * 4 + j + 1];
	     }
        }


	sum_power_1 = 0;
	 
	//To calculate the power level of antenna 0 and antenna 1
	for (i = 0; i < ((RANGE_CODE_LEN >> 2) * 3); i ++)
	{
            sum_power_1 = sum_power_1 + correlation_array_1_i[i] * correlation_array_1_i[i]
				+ correlation_array_1_q[i] * correlation_array_1_q[i];

            //To add the signal from two antennas into one
	    correlation_array_0_i[i] = correlation_array_0_i[i] + correlation_array_1_i[i];
	    correlation_array_0_q[i] = correlation_array_0_q[i] + correlation_array_1_q[i];		 
	}
	 
        mean_1 = sum_power_1 / ((RANGE_CODE_LEN >> 2) * 3);
	FLOG_INFO("Ant 1: The sum_power of correlation is %f, mean power is %f \n", 
            sum_power_1, mean_1);
 

        hook_power.mean_power_ant1_adj = mean_1;

        if (mean_1 > mean_0)
        {
            max_power_ant_id = 1;
    	    expect_digital_frq_pwr = rru_parameter->cali_digi_pwr1
                * exp10f((g_expect_ana_pwr - rru_parameter->cali_ana_pwr1) / 10)
                * phy_ranging_parameter->fft_len / 756 / 8192 / 8192;
	    //We will use the antenna with higher signal level for the judgement
	    mean_0 = mean_1;
	    sum_power_0 = sum_power_1;
        }
	expect_digital_frq_pwr = expect_digital_frq_pwr * 2;

    }


    // To do the correlation with the ranging code set, and find out the code ID with the top two 
    // correlation power
    max_val_0 = 0;
    max_id_0 = 0;
    max_val_1 = 0;
    max_id_1 = 0;

    temp_i = 0;
    temp_q = 0;
    temp_power = 0;
    sum_power_01 = 0;
    tx_correlation_array = (int *)(phy_ranging_parameter->ranging_corr_codeset);

    for (i = 0; i < 10; i++)
    {	
	hook_power.cor_code_id[i] = 0;
	hook_power.cor_code_result[i] = 0;
    }

    for (i = 0; i < phy_ranging_parameter->ir_ranging_codeset_num; i++)
    {
        p = phy_ranging_parameter->ir_codeset_ID_array[i] 
			* ((RANGE_CODE_LEN >> 2) * 3);
	
        temp_i = 0;
        temp_q = 0;	
        for (j = 0; j < (RANGE_CODE_LEN >> 2) * 3; j++)
        {
            if (tx_correlation_array[p + j] > 0)
            {
                temp_i = correlation_array_0_i[j] + temp_i;
                temp_q = temp_q + correlation_array_0_q[j];				 
	    }
	    else
	    {
                temp_i = temp_i - correlation_array_0_i[j];
                temp_q = temp_q - correlation_array_0_q[j];				 
	    }
	}
        temp_i = temp_i * temp_i;
	temp_q = temp_q * temp_q;
        temp_power = temp_i + temp_q;
	sum_power_01 = sum_power_01 + temp_power;
        FLOG_INFO("Code %d: correlation result is %f\n", 
                phy_ranging_parameter->ir_codeset_ID_array[i], temp_power);
	
        hook_power.cor_code_id[i] = phy_ranging_parameter->ir_codeset_ID_array[i];
	hook_power.cor_code_result[i] = temp_power;

	if ((temp_power > max_val_0) || (temp_power > max_val_1))
	{
            if (max_val_0 > max_val_1)
            {
                max_val_1 = temp_power; //to replace the smaller value one
                max_id_1 = phy_ranging_parameter->ir_codeset_ID_array[i] ;
	    }
	    else
	    {
                max_val_0 = temp_power;
                max_id_0 = phy_ranging_parameter->ir_codeset_ID_array[i];				 
            }
	}
    }
//    dump_cf_array(recv_ranging_data_0_i, recv_ranging_data_0_q, 1, 10);

    if (max_val_1 > max_val_0)
    {
	temp_power = max_val_0;
	max_val_0 = max_val_1;
	max_val_1 = temp_power;

	temp_id = max_id_0;
	max_id_0 = max_id_1;
	max_id_1 = temp_id;
    }

    FLOG_INFO("max_val_0 is %f, max_id_0 is %d", max_val_0, max_id_0);
    FLOG_INFO("max_val_1 is %f, max_id_1 is %d", max_val_1, max_id_1);
 

    FLOG_INFO("mean_0 is %f, mean_0 * threshold is %f\n", mean_0, mean_0 * phy_ranging_parameter->ranging_threshold);


    mean_01 = (sum_power_01 - max_val_0 - max_val_1) / (phy_ranging_parameter->ir_ranging_codeset_num - 2);
    FLOG_INFO("mean_01 is %f, mean_01 * threshold is %f\n", mean_01,
		mean_01 * phy_ranging_parameter->ranging_threshold);

 //   p_result_0->ranging_snr = max_val_0 / mean_01;

    //To check the first ranging code result, and calculate the timeoffset

    hook_power.mean_threshold_jdg = mean_0 * phy_ranging_parameter->ranging_threshold;
    hook_power.snr = max_val_0/mean_01;

    DO_DUMP(DUMP_RANGING_POWER_ID, 1, &hook_power, sizeof(struct phy_hook_ranging_power));

    p_result_0->ranging_snr = max_val_0/mean_01;

    //To check the 1st ranging code result, and calculate the timeoffset
    user_judge = 0;
    if ( ranging_method_flag == 0)
    {    
	if ((max_val_0 > 0.000001) &&
		(max_val_0 > mean_0 * phy_ranging_parameter->ranging_threshold))
	{
	    user_judge = 1;
	}
    }
    else if (ranging_method_flag == 1)
    {
	if (p_result_0->ranging_snr > phy_ranging_parameter->ranging_snr_threshold)
        {
	    user_judge = 1;
	}
    }
    else
    {
	printf("Error ranging method flag \n");
    }

    if (user_judge > 0)
    {
        cur_power = sqrt(max_val_0) / 108;
        adj_power = 10 * log10f(expect_digital_frq_pwr / cur_power);
        p_result_0->adj_power_fix = floor(adj_power * 4 + 0.5); 
        p_result_0->frequency_offset = 0;
		//The value for ranging_respond message
        FLOG_INFO("max_power_ant is %d, cur_power is %f, adj_power is %f, expect_digital_frq_pwr is %f\n", max_power_ant_id, cur_power, adj_power, expect_digital_frq_pwr);
        if (max_power_ant_id == 0)
        {
            p_recv_ranging_data_i = (float *)recv_ranging_data_0_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_0_q;			
	}
	else
	{
            p_recv_ranging_data_i = (float *)recv_ranging_data_1_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_1_q;			
	}


        ranging_timeoffset_estimation(phy_ranging_parameter, 
			p_recv_ranging_data_i, 
			p_recv_ranging_data_q, 
			max_id_0,
			&(time_offset),
			INITIAL_RANGING_FLAG);

	user_num ++;

        ranging_result->user_num = user_num;

        p_result_0->time_offset = time_offset;

	p_result_0->ranging_code_id = max_id_0;
        FLOG_INFO("ranging time_offset for code %d is %d", max_id_0, time_offset);
    }

    p_result_1->ranging_snr = max_val_1/mean_01;

    //To check the 2nd ranging code result, and calculate the timeoffset
    user_judge = 0;
    if ( ranging_method_flag == 0)
    {    
	if ((max_val_1 > 0.000001) &&
		(max_val_1 > mean_0 * phy_ranging_parameter->ranging_threshold))
	{
	    user_judge = 1;
	}
    }
    else if (ranging_method_flag == 1)
    {
	if (p_result_1->ranging_snr > phy_ranging_parameter->ranging_snr_threshold)
        {
	    user_judge = 1;
	}
    }
    else
    {
	printf("Error ranging method flag \n");
    }

    if (user_judge > 0)
    {
        cur_power = sqrt(max_val_1)/ 108;
        adj_power = 10 * log (expect_digital_frq_pwr / cur_power);
        p_result_1->adj_power_fix = floor(adj_power * 4 + 0.5); 
	p_result_1->ranging_snr = max_val_1 / mean_01;
	p_result_1->frequency_offset = 0;
		//The value for ranging_respond message

        if (max_power_ant_id == 0)
        {
            p_recv_ranging_data_i = (float *)recv_ranging_data_0_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_0_q;			
	}
	else
	{
            p_recv_ranging_data_i = (float *)recv_ranging_data_1_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_1_q;			
	}

        ranging_timeoffset_estimation(phy_ranging_parameter, 
			p_recv_ranging_data_i, 
			p_recv_ranging_data_q, 
			max_id_1,
			&(time_offset),
			INITIAL_RANGING_FLAG);

	p_result_1->ranging_code_id = max_id_1;

        p_result_1->time_offset = time_offset;
        FLOG_INFO("ranging time_offset for code %d is %d",  max_id_1, time_offset);
	
	user_num ++;

        ranging_result->user_num = user_num;
    }

//    memcpy(ranging_result->ranging_result_0, p_result_0, sizeof(struct ranging_result_struct));

//    memcpy(ranging_result->ranging_result_1, p_result_1, sizeof(struct ranging_result_struct));

    return 0;
}

/**----------------------------------------------------------------------------

   Function:    phy_ul_pr()

   Description: This is the function for periodic ranging.

   Parameters:

                Input-  
		  	[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_0]
				The pointer refers to the sampled data and related information from RRU on an-tenna 0.
			[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_1]
				The pointer refers to the sampled data and related information from RRU on an-tenna 1.
			[const struct rru_para *rru_parameter]	The RRU parameters to be used in ranging
			[const struct phy_ranging_para *phy_ranging_parameter]	
				The parameters to be used in ranging
			[const int ant_num]	The number of antennas


                Output- 
			[struct phy_pr_result pr_result]	The pointer to the buffer holding the PR result

   Return Value:

                0       Success
                150    Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_ul_pr (const struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_0, 
	                             const struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_1, 
	                             const struct ranging_rru_para *rru_parameter,
	                             const struct phy_ranging_para *phy_ranging_parameter, 
	                             const int ant_num, 
	                             struct phy_ranging_result *ranging_result)
{
#ifdef VSXOPT
    float recv_symbol_data_0_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_0_q[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_1_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float recv_symbol_data_1_q[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));

#else
    float recv_symbol_data_0_i[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_0_q[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_1_i[OFDMA_SYMBOL_SIZE];
    float recv_symbol_data_1_q[OFDMA_SYMBOL_SIZE];
#endif

    float recv_ranging_data_0_i[RANGE_CODE_LEN]; 
    float recv_ranging_data_0_q[RANGE_CODE_LEN]; 
    float recv_ranging_data_1_i[RANGE_CODE_LEN]; 
    float recv_ranging_data_1_q[RANGE_CODE_LEN];	
    float *p_recv_ranging_data_i;
    float *p_recv_ranging_data_q;		
    float correlation_array_0_i[RANGE_CODE_LEN];
    float correlation_array_0_q[RANGE_CODE_LEN];
    float correlation_array_1_i[RANGE_CODE_LEN]; 
    float correlation_array_1_q[RANGE_CODE_LEN];	

    float sum_power_01, mean_01;	
    int *tx_correlation_array;
    float sum_power_0, sum_power_1, mean_0, mean_1;	
    float temp_i, temp_q, temp_power;
    float max_val_0, max_val_1;
    float expect_digital_frq_pwr;
    int32_t    max_id_0, max_id_1, temp_id;	
    int32_t    max_power_ant_id;
    int32_t    time_offset, user_num;
    int32_t    user_judge;
    float adjust_gain, cur_power, adj_power;

	
    u_int32_t i, j, p;

    int32_t symbol_offset;

    struct ranging_result_struct *p_result_0, *p_result_1;

    //To set the ranging_code_id to -1, for initialization purpose

#ifdef DIGITAL_POWER_ESTIMATION
    struct phy_hook_ranging_power hook_power;
#endif


    user_num = 0;

    FLOG_INFO("PR detection start ...\n");

    p_result_0 = &(ranging_result->ranging_result_0);
    p_result_1 = &(ranging_result->ranging_result_1);
    p_result_0->ranging_code_id = -1;
    p_result_1->ranging_code_id = -1;	

   //This calculation is the same with IR, which could be moved to the 
    // phy_ul_ranging in futuer
    expect_digital_frq_pwr = rru_parameter->cali_digi_pwr0 
		* exp10f((g_expect_ana_pwr - rru_parameter->cali_ana_pwr0) / 10.0)	
		* phy_ranging_parameter->fft_len / 756 / 8192 / 8192;

    if (phy_ranging_parameter->subch_offset_pr > SYMBOLS_IN_SLOTSYMBOL)
    {
        printf("E011_ranging: The symbol offset out of range \n");
		
	return ERROR_CODE;
    }

    hook_power.ranging_type = 2;

#ifdef DIGITAL_POWER_ESTIMATION

    sum_power_0 = 0;

    for (i =(phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 2; 
		i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 3; i++)
    {
        sum_power_0 = sum_power_0 + 
		phy_ul_rru_symbol_ranging_0->symbol_i[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_i[i]
		+ phy_ul_rru_symbol_ranging_0->symbol_q[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_q[i];
    }

    mean_0 = sum_power_0 
	/ (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len);

    sum_power_1 = 0;

    for (i =(phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 2; 
		i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 3; i++)
    {
        sum_power_1 = sum_power_1 + 
		phy_ul_rru_symbol_ranging_1->symbol_i[i] 
		* phy_ul_rru_symbol_ranging_1->symbol_i[i]
		+ phy_ul_rru_symbol_ranging_1->symbol_q[i] 
		* phy_ul_rru_symbol_ranging_1->symbol_q[i];
    }

    mean_1 = sum_power_1 
	/ (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len);

//    printf("Mean power of ant 0 is %f, ant 1 is %f, dgain is %f \n", 
//	mean_0, mean_1, phy_ul_rru_symbol_ranging_0->dgain);

    adjust_gain = exp10f((-1) *
                ((phy_ul_rru_symbol_ranging_0->dgain + rru_parameter->cali_dgain0)
                / 20.0));

    sum_power_0 = 0;

    for (i = 0; i < (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) * 2; i++)
    {
	phy_ul_rru_symbol_ranging_0->symbol_i[i] = 
		phy_ul_rru_symbol_ranging_0->symbol_i[i] * adjust_gain;

	phy_ul_rru_symbol_ranging_0->symbol_q[i] = 
		phy_ul_rru_symbol_ranging_0->symbol_q[i] * adjust_gain;


        sum_power_0 = sum_power_0 + 
		phy_ul_rru_symbol_ranging_0->symbol_i[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_i[i]
		+ phy_ul_rru_symbol_ranging_0->symbol_q[i] 
		* phy_ul_rru_symbol_ranging_0->symbol_q[i];
    }

    mean_0 = sum_power_0 
	/ (phy_ranging_parameter->fft_len + phy_ranging_parameter->cp_len) / 2;

#endif


    //To get the 3rd symbol for estimation
    symbol_offset = 2 * phy_ranging_parameter->fft_len 
			+ phy_ranging_parameter->cp_len * 3;



/*    dump_cf_array(
        phy_ul_rru_symbol_ranging_0->symbol_i,
        phy_ul_rru_symbol_ranging_0->symbol_q,phy_ranging_parameter->cp_len,
        phy_ranging_parameter->cp_len+10);

    dump_cf_array(
        phy_ul_rru_symbol_ranging_0->symbol_i,
        phy_ul_rru_symbol_ranging_0->symbol_q,symbol_offset,symbol_offset+10);
*/
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR_IFFT ));
#ifdef IPP_OPT_TEMP730
/*
    ippsFFT_CToC_32f(&(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
                         recv_symbol_data_0_i, recv_symbol_data_0_q,
                         phy_ranging_parameter->pFFTSpecInv,
                         phy_ranging_parameter->BufInv);
*/
// by zzb
    ippsFFTFwd_CToC_32f(&(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
                         recv_symbol_data_0_i, recv_symbol_data_0_q,
                         phy_ranging_parameter->pFFTSpecFwd,
                         phy_ranging_parameter->BufFwd);

#else

#ifdef VSXOPT
    fft_p( phy_ranging_parameter->fft_len ,  &(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                   &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
                         recv_symbol_data_0_i, recv_symbol_data_0_q,
                         phy_ranging_parameter->XX, phy_ranging_parameter->x,
                         phy_ranging_parameter->X);
#else
    fft( phy_ranging_parameter->fft_len ,  &(phy_ul_rru_symbol_ranging_0->symbol_i[symbol_offset]),
                   &(phy_ul_rru_symbol_ranging_0->symbol_q[symbol_offset]),
                         recv_symbol_data_0_i, recv_symbol_data_0_q,
                         phy_ranging_parameter->XX, phy_ranging_parameter->x,
                         phy_ranging_parameter->X);
#endif


#endif
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR_IFFT ));
//    dump_cf_array(recv_symbol_data_0_i, recv_symbol_data_0_q, 160+1, 160+50);

    fftshift(recv_symbol_data_0_i, recv_symbol_data_0_q, phy_ranging_parameter->fft_len);

    adjust_gain = exp10f((-1) 
		* ((phy_ul_rru_symbol_ranging_0->dgain + rru_parameter->cali_dgain0) 
		/ 20.0)) / sqrt(phy_ranging_parameter->fft_len) / 8192;

    FLOG_INFO("Ant 0: DGAIN = %f, cali_dgain = %f, adjust_gain = %f\n",
            phy_ul_rru_symbol_ranging_0->dgain, rru_parameter->cali_dgain0, adjust_gain);


    for (i = 0; i < RANGE_CODE_LEN; i++)
    {
        j = phy_ranging_parameter->ranging_allocation[i + 2 * 144] - 2048;

        recv_ranging_data_0_i[i] = recv_symbol_data_0_i[j] * adjust_gain;
        recv_ranging_data_0_q[i] = recv_symbol_data_0_q[j] * adjust_gain;		
    }

//dump_cf_array(recv_ranging_data_0_i, recv_ranging_data_0_q, 1, 10);
    if (ant_num > 1)
    {
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR_IFFT ));
#ifdef IPP_OPT_TEMP730
        ippsFFTFwd_CToC_32f(&(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                         &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
                         recv_symbol_data_1_i, recv_symbol_data_1_q,
                         phy_ranging_parameter->pFFTSpecFwd,
                         phy_ranging_parameter->BufFwd);

#else
#ifdef VSXOPT
        fft_p(phy_ranging_parameter->fft_len ,  
			&(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                        &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
                        recv_symbol_data_1_i, recv_symbol_data_1_q,
                        phy_ranging_parameter->XX, phy_ranging_parameter->x,
                        phy_ranging_parameter->X);

#else
        fft(phy_ranging_parameter->fft_len ,  
			&(phy_ul_rru_symbol_ranging_1->symbol_i[symbol_offset]),
                        &(phy_ul_rru_symbol_ranging_1->symbol_q[symbol_offset]),
                        recv_symbol_data_1_i, recv_symbol_data_1_q,
                        phy_ranging_parameter->XX, phy_ranging_parameter->x,
                        phy_ranging_parameter->X);


#endif
#endif
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR_IFFT ));

        fftshift(recv_symbol_data_1_i, recv_symbol_data_1_q, phy_ranging_parameter->fft_len);


        adjust_gain = exp10f((-1) * 
		((phy_ul_rru_symbol_ranging_1->dgain + rru_parameter->cali_dgain1) / 20))
                / sqrt(phy_ranging_parameter->fft_len) / 8192;

        FLOG_INFO("Ant 1: DGAIN = %f, cali_dgain = %f, adjust_gain = %f\n",
                phy_ul_rru_symbol_ranging_1->dgain, rru_parameter->cali_dgain1, adjust_gain);
      
        for (i = 0; i < RANGE_CODE_LEN; i++)
        {
            j = phy_ranging_parameter->ranging_allocation[i + 2 *144] - 2048;
	
            recv_ranging_data_1_i[i] = recv_symbol_data_1_i[j] * adjust_gain;
            recv_ranging_data_1_q[i] = recv_symbol_data_1_q[j] * adjust_gain;		
        }
    }

    //To do the correlation for antenna 0
    for (i = 0; i < (RANGE_CODE_LEN >> 2); i++)
    {
        for (j = 0; j < 3; j++)
        {
            correlation_array_0_i[i * 3 + j] = recv_ranging_data_0_i[i * 4 + j] 
			 	  * recv_ranging_data_0_i[i * 4 + j + 1]
				+ recv_ranging_data_0_q[i * 4 + j] 
                                  * recv_ranging_data_0_q[i * 4 + j + 1];
			
            correlation_array_0_q[i * 3 + j] = recv_ranging_data_0_i[i * 4 + j] 
				  * recv_ranging_data_0_q[i * 4 + j + 1]
				- recv_ranging_data_0_q[i * 4 + j] 
                                  * recv_ranging_data_0_i[i * 4 + j + 1];

	 }
    }	
//dump_cf_array(correlation_array_0_i, correlation_array_0_q, 1, 10);

    sum_power_0 = 0;
    mean_0 = 0;

    for (i = 0; i < ((RANGE_CODE_LEN >> 2) * 3); i ++)
    {
            sum_power_0 = sum_power_0 + correlation_array_0_i[i] * correlation_array_0_i[i]
                                + correlation_array_0_q[i] * correlation_array_0_q[i];
    }

    mean_0 = sum_power_0 / ((RANGE_CODE_LEN >> 2) * 3);

    FLOG_INFO("Ant 0: The sum_power of correlation is %f, mean power is %f \n",
            sum_power_0, mean_0);


    max_power_ant_id = 0;

    hook_power.mean_power_ant0_adj = mean_0;
    hook_power.mean_power_ant1_adj = 0;

    //To do the correlation for antenna 1
    if (ant_num > 1)
    {
        for (i = 0; i < (RANGE_CODE_LEN >> 2); i++)
        {
            for (j = 0; j < 3; j++)
            {
                correlation_array_1_i[i * 3 + j] = recv_ranging_data_1_i[i * 4 + j] 
				  * recv_ranging_data_1_i[i * 4 + j + 1]
				+ recv_ranging_data_1_q[i * 4 + j] 
                                  * recv_ranging_data_1_q[i * 4 + j + 1];
			
                correlation_array_1_q[i * 3 + j] = recv_ranging_data_1_i[i * 4 + j] 
				  * recv_ranging_data_1_q[i * 4 + j + 1]
				- recv_ranging_data_1_q[i * 4 + j] 
                                  * recv_ranging_data_1_i[i * 4 + j + 1];
	    }
        }

	sum_power_1 = 0;
	 
	 //To calculate the power level of antenna 0 and antenna 1
	for (i = 0; i < ((RANGE_CODE_LEN >> 2) * 3); i ++)
	{
            sum_power_1 = sum_power_1 + correlation_array_1_i[i] * correlation_array_1_i[i]
				+ correlation_array_1_q[i] * correlation_array_1_q[i];

            //To add the signal from two antennas into one
	    correlation_array_0_i[i] = correlation_array_0_i[i] + correlation_array_1_i[i];
	    correlation_array_0_q[i] = correlation_array_0_q[i] + correlation_array_1_q[i];		 
	}
	 
        mean_1 = sum_power_1 / ((RANGE_CODE_LEN >> 2) * 3);
	
	hook_power.mean_power_ant1_adj = mean_1;

        if (mean_1 > mean_0)
        {
            max_power_ant_id = 1;

    	    expect_digital_frq_pwr = rru_parameter->cali_digi_pwr1
                * exp10f((g_expect_ana_pwr - rru_parameter->cali_ana_pwr1) / 10)
                * phy_ranging_parameter->fft_len / 756 / 8192 / 8192;
	    //We will use the antenna with higher signal level for the judgement
	    mean_0 = mean_1;
	    sum_power_0 = sum_power_1;
 
/*	    for (i = 0; i < ((RANGE_CODE_LEN >> 2) * 3); i ++)
            {
                correlation_array_0_i[i] = correlation_array_1_i[i];
                correlation_array_0_q[i] = correlation_array_1_q[i];         
            }
*/

	}
	expect_digital_frq_pwr = expect_digital_frq_pwr * 2;

    }


    // To do the correlation with the ranging code set, and find out the code ID with the top two 
    // correlation power
    max_val_0 = 0;
    max_id_0 = 0;
    max_val_1 = 0;
    max_id_1 = 0;

    temp_i = 0;
    temp_q = 0;
    temp_power = 0;
    tx_correlation_array = (int *)(phy_ranging_parameter->ranging_corr_codeset);

//    dump_int_array(tx_correlation_array, 1, 10);
    sum_power_01 = 0;

    for (i = 0; i < 10; i++)
    {
        hook_power.cor_code_id[i] = 0;
        hook_power.cor_code_result[i] = 0;
    }


    for (i = 0; i < phy_ranging_parameter->pr_ranging_codeset_num; i++)
    {
        p = phy_ranging_parameter->pr_codeset_ID_array[i] 
			* ((RANGE_CODE_LEN >> 2) * 3);
	
        temp_i = 0;
        temp_q = 0;
		
        for (j = 0; j < (RANGE_CODE_LEN >> 2) * 3; j++)
        {
            if (tx_correlation_array[p + j] > 0)
            {
                 temp_i = correlation_array_0_i[j] + temp_i;
                 temp_q = temp_q + correlation_array_0_q[j];				 
	    }
	    else
	    {
                 temp_i = temp_i - correlation_array_0_i[j];
                 temp_q = temp_q - correlation_array_0_q[j];				 
	    }
	}
	temp_i = temp_i * temp_i;
	temp_q = temp_q * temp_q;
        temp_power = temp_i + temp_q;
	sum_power_01 = sum_power_01 + temp_power;
        FLOG_INFO("Code %d: correlation result is %f\n",
                phy_ranging_parameter->pr_codeset_ID_array[i], temp_power);

        hook_power.cor_code_id[i] = phy_ranging_parameter->pr_codeset_ID_array[i];
        hook_power.cor_code_result[i] = temp_power;




	if ((temp_power > max_val_0) || (temp_power > max_val_1))
	{
            if (max_val_0 > max_val_1)
            {
                 max_val_1 = temp_power; //to replace the smaller value one
                 max_id_1 = phy_ranging_parameter->pr_codeset_ID_array[i] ;
	    }
	    else
	    {
                 max_val_0 = temp_power;
                 max_id_0 = phy_ranging_parameter->pr_codeset_ID_array[i];				 
            }
	}
    }

    FLOG_INFO("max_val_0 is %f, max_id_0 is %d", max_val_0, max_id_0);
    FLOG_INFO("max_val_1 is %f, max_id_1 is %d", max_val_1, max_id_1);

    //To check the first ranging code result, and calculate the timeoffset

    if (max_val_1 > max_val_0)
    {
	temp_power = max_val_0;
	max_val_0 = max_val_1;
	max_val_1 = temp_power;

	temp_id = max_id_0;
	max_id_0 = max_id_1;
	max_id_1 = temp_id;
    }
    FLOG_INFO("mean_0 is %f, mean_0 * threshold is %f\n", mean_0, mean_0 * phy_ranging_parameter->ranging_threshold);
    mean_01 = (sum_power_01 - max_val_0 - max_val_1) / (phy_ranging_parameter->pr_ranging_codeset_num - 2);
    FLOG_INFO("mean_01 is %f, mean_01 * threshold is %f\n", mean_01,
		mean_01 * phy_ranging_parameter->ranging_threshold);

    hook_power.mean_threshold_jdg = mean_0 * phy_ranging_parameter->ranging_threshold;
    hook_power.snr = max_val_0/mean_01;
    DO_DUMP(DUMP_RANGING_POWER_ID, 2, &hook_power, sizeof(struct phy_hook_ranging_power));

    p_result_0->ranging_snr = max_val_0/mean_01;

    //To check the 1st ranging code result, and calculate the timeoffset
    user_judge = 0;
    if ( ranging_method_flag == 0)
    {    
	if ((max_val_0 > 0.000001) &&
		(max_val_0 > mean_0 * phy_ranging_parameter->ranging_threshold))
	{
	    user_judge = 1;
	}
    }
    else if (ranging_method_flag == 1)
    {
	if (p_result_0->ranging_snr > phy_ranging_parameter->ranging_snr_threshold)
        {
	    user_judge = 1;
	}
    }
    else
    {
	printf("Error ranging method flag \n");
    }

    if (user_judge > 0)
    {
        cur_power = sqrt(max_val_0) / 108;
        adj_power = 10 * log10f(expect_digital_frq_pwr / cur_power);
        p_result_0->adj_power_fix = floor(adj_power * 4 + 0.5); 
//	p_result_0->ranging_snr = max_val_0 / mean_01;
	p_result_0->frequency_offset = 0;
		//The value for ranging_respond messagea

        if (max_power_ant_id == 0)
        {
            p_recv_ranging_data_i = (float *)recv_ranging_data_0_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_0_q;			
	}
	else
	{
            p_recv_ranging_data_i = (float *)recv_ranging_data_1_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_1_q;			
	}

        ranging_timeoffset_estimation(phy_ranging_parameter, 
			p_recv_ranging_data_i, 
			p_recv_ranging_data_q, 
			max_id_0,
			&(time_offset),
			PERIODIC_RANGING_FLAG);

	p_result_0->ranging_code_id = max_id_0;

        p_result_0->time_offset = time_offset;
       
        FLOG_INFO("ranging time_offset for code %d is %d", max_id_0, time_offset);

	user_num ++;

        ranging_result->user_num = user_num;
    }


    p_result_1->ranging_snr = max_val_1/mean_01;

    //To check the 1st ranging code result, and calculate the timeoffset
    user_judge = 0;
    if ( ranging_method_flag == 0)
    {    
	if ((max_val_1 > 0.000001) &&
		(max_val_1 > mean_0 * phy_ranging_parameter->ranging_threshold))
	{
	    user_judge = 1;
	}
    }
    else if (ranging_method_flag == 1)
    {
	if (p_result_1->ranging_snr > phy_ranging_parameter->ranging_snr_threshold)
        {
	    user_judge = 1;
	}
    }
    else
    {
	printf("Error ranging method flag \n");
    }

    if (user_judge > 0)
    {
        cur_power = sqrt(max_val_1) / 108;
        adj_power = 10 * log10f(expect_digital_frq_pwr / cur_power);
        p_result_1->adj_power_fix = floor(adj_power * 4 + 0.5); 
//	p_result_1->ranging_snr = max_val_1 / mean_01;
	p_result_1->frequency_offset = 0;
		//The value for ranging_respond message

        if (max_power_ant_id == 0)
        {
            p_recv_ranging_data_i = (float *)recv_ranging_data_0_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_0_q;			
	}
	else
	{
            p_recv_ranging_data_i = (float *)recv_ranging_data_1_i;
            p_recv_ranging_data_q = (float *)recv_ranging_data_1_q;			
	}

        ranging_timeoffset_estimation(phy_ranging_parameter, 
			p_recv_ranging_data_i, 
			p_recv_ranging_data_q, 
			max_id_1,
			&(time_offset),
			PERIODIC_RANGING_FLAG);

	p_result_1->ranging_code_id = max_id_1;

        p_result_1->time_offset = time_offset;
        FLOG_INFO("ranging time_offset for code %d is %d",  max_id_1, time_offset);

	user_num ++;

        ranging_result->user_num = user_num;
    }
/*
    memcpy(ranging_result->ranging_result_0, p_result_0, 
    		sizeof(struct ranging_result_struct));

    memcpy(ranging_result->ranging_result_1, p_result_1, 
		sizeof(struct ranging_result_struct));
*/	
    return 0;
}


/**----------------------------------------------------------------------------

   Function:    phy_ul_ranging()

   Description: This is the function to be called by framework. In this function, 
   it will determine if there is any IR or PR symbols in a frame, and call the 
   phy_ul_ranging_ir() or phy_ul_ranging_pr() accordingly.

   Parameters:

                Input-  
		  	[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_0]
				The pointer refers to the sampled data and related information from RRU on 
				antenna 0.
			[const struct phy_ul_rru_symbol_ranging *phy_ul_rru_symbol_ranging_1]
				The pointer refers to the sampled data and related information from RRU on 
				antenna 1.
			[const struct rru_para *rru_parameter]	The RRU parameters to be used in 
			        ranging
			[const struct phy_ranging_para *phy_ranging_parameter]	
				The parameters to be used in ranging
			[const int ant_num]	The number of antennas


                Output- 
			[struct phy_ir_result ir_result]	The pointer to the buffer holding the IR result
			[struct phy_pr_result pr_result]	The pointer to the buffer holding the PR result

   Return Value:

                0       Success
                150    Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_ul_ranging ( struct phy_ul_rx_syspara *phy_ul_sys_parameter,
	                             const struct ranging_rru_para *rru_parameter,
	                             struct phy_ranging_para *phy_ranging_parameter, 
                                     const int in_que_id_0,
                                     const int in_que_id_1,
                                     const int out_que_id)
//                                     struct queue_msg *p_msg_in_0,
//                                     struct queue_msg *p_msg_in_1,
//                                     struct queue_msg *p_msg_out)
{

    int return_code = 0;
    int ant_num, temp_n, i;
    u_int32_t ava_subch[1];	
    u_int32_t rotation_posindex[31*5];//155
    u_int32_t pilot_allocation[31*24*5];//3720
    u_int32_t data_allocation[31*48*5];//6720
    struct phy_ranging_result *ranging_result;
    struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_0 = NULL;
    struct phy_ul_rru_symbol *phy_ul_rru_symbol_ranging_1 = NULL; 
    struct queue_msg *p_msg_in_0, *p_msg_in_1, *p_msg_out;

#ifdef RANGING_STATISTIC_TEST
    int test_num = 100;
    int err_count = 0;
    int test_count = 0;
    int correct_timeoffset = -26;
    struct ranging_result_struct *p_result_0;
#endif

    struct phy_hook_ranging_result hook_result;

//    struct timeval tv;
//    struct timeval tv_1;
//    struct tm *psysTime = NULL;

//    struct phy_dts_info *rru_phydts;

    FILE *pf_ranging;

//    ranging_method_flag = phy_ranging_parameter->ranging_algorithms;
//    g_expect_ana_pwr = rru_parameter->expect_ana_pwr;


    struct phy_ps_node * p_ps_node = NULL;

    (void) temp_n;
    (void) pf_ranging;

    if (rru_parameter == NULL) 
    {
        printf("E002_ranging: the pointer refer to RRH parameter buffer is null!\n");
        return ERROR_CODE;
    }    

    if (phy_ranging_parameter == NULL) 
    {
        printf("E003_ranging: the pointer refer to RRH parameter buffer is null!\n");
        return ERROR_CODE;
    }    

    ant_num = phy_ul_sys_parameter->rx_div;

    p_msg_in_0  = (struct queue_msg *)my_malloc (sizeof(struct queue_msg));
    p_msg_in_1  = (struct queue_msg *)my_malloc (sizeof(struct queue_msg));
    p_msg_out = (struct queue_msg *)my_malloc (sizeof(struct queue_msg));
    if(p_msg_in_0 == NULL || p_msg_in_1 == NULL || p_msg_out == NULL)
    {
        FLOG_ERROR("malloc failed in phy_ul_ranging\n");
        return -1;
    }

    p_msg_in_0->my_type = in_que_id_0;
    p_msg_in_1->my_type = in_que_id_1;	
    p_msg_out->my_type = out_que_id; 

    //to be removed later
//    phy_ranging_parameter->ranging_snr_threshold = phy_ranging_parameter->ranging_snr_threshold1;

    while (1)
    {
        if (wmrt_dequeue (in_que_id_0, p_msg_in_0, sizeof(struct queue_msg)) == -1)
        {
            FLOG_FATAL ("E021_ranging: In PHY ranging DEQUEUE ERROR\n");
//            return ERROR_CODE;
        }
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING ));
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_ZPERM ));
       
        ranging_method_flag = phy_ranging_parameter->ranging_algorithms;
        g_expect_ana_pwr = (float)rru_parameter->expect_ana_pwr;

        p_ps_node = phy_ps_get_current();

        g_noise_figure = p_ps_node->noise_figure;

        p_ps_node->expected_pwr = g_expect_ana_pwr;

//        gettimeofday(&tv, NULL);
//        psysTime = localtime(&tv.tv_sec);


        phy_ul_rru_symbol_ranging_0 = ( struct phy_ul_rru_symbol *) p_msg_in_0->p_buf;
   
        FLOG_INFO("get slot %d on ant 0\n", phy_ul_rru_symbol_ranging_0->symbol_offset);

	if (phy_ul_rru_symbol_ranging_0 == NULL
            || phy_ul_rru_symbol_ranging_0->symbol_i == NULL
            || phy_ul_rru_symbol_ranging_0->symbol_q == NULL)
        {
            printf("E022_ranging: Error in received frm rru symbol, antenna1!\n");
//            return ERROR_CODE;
        }


        if (ant_num == 2)
        {
            if (wmrt_dequeue (in_que_id_1, p_msg_in_1, sizeof(struct queue_msg) ) == -1)
            {
                printf ("E021_ranging: In PHY ranging DEQUEUE ERROR\n");
                return ERROR_CODE;
            }
       
            phy_ul_rru_symbol_ranging_1 = ( struct phy_ul_rru_symbol *) 
                                         p_msg_in_1->p_buf;
			
            if (phy_ul_rru_symbol_ranging_1 == NULL
                || phy_ul_rru_symbol_ranging_1->symbol_i == NULL
                || phy_ul_rru_symbol_ranging_1->symbol_q == NULL)
            {
                printf("E023_ranging: Error in received frm rru symbol, antenna2!\n");
            }
        }

//        dump_cf_array(phy_ul_rru_symbol_ranging_0->symbol_i,
//		phy_ul_rru_symbol_ranging_0->symbol_q, 1, 20);

        if (phy_ul_rru_symbol_ranging_0->symbol_offset == 0)
	{
/*            get_ranging_symbol_offset_ir(&(phy_ranging_parameter->symbol_offset_ir));

	    get_ranging_symbol_offset_pr(&(phy_ranging_parameter->symbol_offset_pr));	   

	    get_ranging_subch_offset_ir(&(phy_ranging_parameter->subch_offset_ir));

	    get_ranging_subch_offset_pr(&(phy_ranging_parameter->subch_offset_pr));	   
*/
	    return_code = phy_ul_zonepermutation(phy_ul_sys_parameter,
    			phy_ul_sys_parameter->active_band,
	                ava_subch,
	                rotation_posindex,
	                pilot_allocation,
	                data_allocation,
	                &(phy_ranging_parameter->ranging_allocation[0]));

	    for (i = 0; i < RANGE_CODE_LEN; i++)
	    {
		if (phy_ranging_parameter->ranging_allocation[i] < 
			(512 - LEFT_GUARD))
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD;
		}
		else
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD + 1;
		}
	
	    }
	    for (i = RANGE_CODE_LEN; i < RANGE_CODE_LEN * 2; i++)
	    {
		if ((phy_ranging_parameter->ranging_allocation[i] - 1024) < 
			(512 - LEFT_GUARD))
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD;
		}
		else
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD + 1;
		}
	
	    }
	    for (i = RANGE_CODE_LEN * 2; i < RANGE_CODE_LEN * 3; i++)
	    {
		if ((phy_ranging_parameter->ranging_allocation[i] - 2048) < 
			(512 - LEFT_GUARD))
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD;
		}
		else
		{
		    phy_ranging_parameter->ranging_allocation[i] =
			phy_ranging_parameter->ranging_allocation[i] 
			+ LEFT_GUARD + 1;
		}
	
	    }

	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_ZPERM ));

	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR ));


#ifdef RANGING_DEBUG            
            pf_ranging = fopen("ranging_subcar.dat", "r");

            temp_n = fread(phy_ranging_parameter->ranging_allocation, 
			sizeof(u_int32_t), 
			RANGE_CODE_LEN, pf_ranging);

            for (i = 0; i < 144; i++)
	    {
                phy_ranging_parameter->ranging_allocation[i + 144] = 
			phy_ranging_parameter->ranging_allocation[i] + 1024;
	    }

            for (i = 144; i < 144 * 2; i++)
	    {
                phy_ranging_parameter->ranging_allocation[i + 144] = 
			phy_ranging_parameter->ranging_allocation[i] + 1024;
	    }
            FLOG_INFO("in ranging debug\n");
            fclose(pf_ranging);
#endif
/*            dump_int_array(phy_ranging_parameter->ranging_allocation, 1, 144);
            dump_int_array(phy_ranging_parameter->ranging_allocation, 144+1, 144*2);
            dump_int_array(phy_ranging_parameter->ranging_allocation, 144*2+1, 144*3);
*/
 
	    if (return_code < 0)
	    {
	        printf("E005_ranging: phy_ul_zonepermutation error \n");
	    }

	}
	
        if (phy_ul_rru_symbol_ranging_0->symbol_offset ==
			phy_ranging_parameter->symbol_offset_ir)
	{
	    ranging_result = (struct phy_ranging_result *)malloc(
					sizeof (struct phy_ranging_result));
            if(ranging_result == NULL)
            {
                FLOG_ERROR("malloc failed in phy_ul_ranging\n");
                return -1;
            }

 //       FLOG_INFO("Threshold is %f \n", phy_ranging_parameter->ranging_threshold);

 	    if (ranging_result == NULL)
    	    {
                printf("E008_ranging: the memory allocation for result failed!\n");
	        return ERROR_CODE;
    	    }

            ranging_result->ranging_type = 0;

            ranging_result->user_num = 0;

	    return_code = phy_ul_ir(phy_ul_rru_symbol_ranging_0, 
		                         phy_ul_rru_symbol_ranging_1, 
		                         rru_parameter, 
		                         phy_ranging_parameter,
		                         ant_num, 
		                         ranging_result);			                         
            ranging_result->ranging_type = BITMAP_IR;
 
            if (ranging_result->user_num > 0)
            {
                p_msg_out->my_type = out_que_id;

	        p_msg_out->p_buf = (void *)ranging_result;

                hook_result.frm_num = phy_ul_rru_symbol_ranging_0->frame_index;
                hook_result.type = ranging_result->ranging_type;
                hook_result.num = ranging_result->user_num;
                hook_result.code_id0 = ranging_result->ranging_result_0.ranging_code_id;
                hook_result.time0 = ranging_result->ranging_result_0.time_offset;
                hook_result.freq0 = ranging_result->ranging_result_0.frequency_offset;
                hook_result.power0 = ranging_result->ranging_result_0.adj_power_fix;
                hook_result.snr0 = (int) (ranging_result->ranging_result_0.ranging_snr);

                if (ranging_result->user_num == 2)
                {
                    hook_result.code_id1 = ranging_result->ranging_result_1.ranging_code_id;
                    hook_result.time1 = ranging_result->ranging_result_1.time_offset;
                    hook_result.freq1 = ranging_result->ranging_result_1.frequency_offset;
                    hook_result.power1 = ranging_result->ranging_result_1.adj_power_fix;
                    hook_result.snr1 = (int) (ranging_result->ranging_result_1.ranging_snr);
                }

                hook_debug_trace(HOOK_RANGING_RESULT_IDX, &hook_result, sizeof(struct phy_hook_ranging_result), 1);

                DO_DUMP(DUMP_RANGING_RESULT_ID, 0, &hook_result, sizeof(struct phy_hook_ranging_result));

                DO_DUMP(DUMP_RANGING_DEBUG_ID, phy_ul_rru_symbol_ranging_0->frame_index, NULL, 0);

                memset(&hook_result, 0, sizeof(struct phy_hook_ranging_result));

#ifdef RANGING_STATISTIC_TEST
                p_result_0 = &(ranging_result->ranging_result_0);
	    
	        if (p_result_0->time_offset != correct_timeoffset)
	        {
		    err_count = err_count + 1;
	    	    printf("time_offset is %d, correct is %d\n", p_result_0->time_offset, 
		            correct_timeoffset);
		    printf("snr is %f, code_id is %d \n", p_result_0->ranging_snr, 
                            p_result_0->ranging_code_id);
	        }

	        test_count = test_count + 1;
	        if (test_count == test_num)
	        {
		    printf("Tested for %d: err_count is %d, error ratio is %f\n ", 
		            test_num, err_count, (float)err_count/(float)test_num);
	            err_count = 0;
	            test_count = 0;
	        }
#endif



#ifndef __RANGING_UT__
                pthread_mutex_lock(&mutex_tx_phy_en_flag);
                ranging_en_flag ++;
                pthread_mutex_unlock(&mutex_tx_phy_en_flag);
#endif

                if (wmrt_enqueue (out_que_id, p_msg_out,
                            sizeof(struct queue_msg)) == -1)
                {
                    FLOG_FATAL("ENQUEUE FOR IR RESULT ERROR\n");
                }
            }else
            {
                free(ranging_result);
            }
/*
#ifndef __RANGING_UT__
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_0);
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_1);
#endif*/
	}
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_IR ));
	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR ));

        if (phy_ul_rru_symbol_ranging_0->symbol_offset ==
			phy_ranging_parameter->symbol_offset_pr)
	{
 	    ranging_result = (struct phy_ranging_result *)malloc(
					sizeof (struct phy_ranging_result));

 //       FLOG_INFO("Threshold is %f \n", phy_ranging_parameter->ranging_threshold);

  	    if (ranging_result == NULL)
    	    {
                printf("E008_ranging: the memory allocation for result failed!\n");
	        return ERROR_CODE;
    	    }

            ranging_result->ranging_type = 0;

            ranging_result->user_num = 0;

       
	    return_code = phy_ul_pr(phy_ul_rru_symbol_ranging_0, 
				    phy_ul_rru_symbol_ranging_1, 
				    rru_parameter, 
				    phy_ranging_parameter,
		                    ant_num, 
		                    ranging_result);			                         
            
            ranging_result->ranging_type = BITMAP_PR;

            if (ranging_result->user_num > 0)
            {

                p_msg_out->my_type = out_que_id;
 
                p_msg_out->p_buf = (void *)ranging_result;

                hook_result.frm_num = phy_ul_rru_symbol_ranging_0->frame_index;
                hook_result.type = ranging_result->ranging_type;
                hook_result.num = ranging_result->user_num;
                hook_result.code_id0 = ranging_result->ranging_result_0.ranging_code_id;
                hook_result.time0 = ranging_result->ranging_result_0.time_offset;
                hook_result.freq0 = ranging_result->ranging_result_0.frequency_offset;
                hook_result.power0 = ranging_result->ranging_result_0.adj_power_fix;
                hook_result.snr0 = (float)(ranging_result->ranging_result_0.ranging_snr);

                if (ranging_result->user_num == 2)
                {
                    hook_result.code_id1 = ranging_result->ranging_result_1.ranging_code_id;
                    hook_result.time1 = ranging_result->ranging_result_1.time_offset;
                    hook_result.freq1 = ranging_result->ranging_result_1.frequency_offset;
                    hook_result.power1 = ranging_result->ranging_result_1.adj_power_fix;
                    hook_result.snr1 = (float)(ranging_result->ranging_result_1.ranging_snr);
                }

                hook_debug_trace(HOOK_RANGING_RESULT_IDX, &hook_result, sizeof(struct phy_hook_ranging_result), 1);

                DO_DUMP(DUMP_RANGING_RESULT_ID, 0, &hook_result, sizeof(struct phy_hook_ranging_result));

                DO_DUMP(DUMP_RANGING_DEBUG_ID, phy_ul_rru_symbol_ranging_0->frame_index, NULL, 0);

                memset(&hook_result, 0, sizeof(struct phy_hook_ranging_result));

#ifndef __RANGING_UT__
                pthread_mutex_lock(&mutex_tx_phy_en_flag);
                ranging_en_flag ++;
                pthread_mutex_unlock(&mutex_tx_phy_en_flag);
#endif

                if (wmrt_enqueue (out_que_id, p_msg_out,
                            sizeof(struct queue_msg)) == -1)
                {
                    FLOG_FATAL ("ENQUEUE FOR PR RESULT ERROR\n");
                }
/*
#ifndef __RANGING_UT__
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_0);
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_1);
#endif
*/	    }else
            {
                free(ranging_result);
            }
        }
 

        if ((phy_ul_rru_symbol_ranging_0->symbol_offset !=
			phy_ranging_parameter->symbol_offset_pr) 
	    && (phy_ul_rru_symbol_ranging_0->symbol_offset !=
			phy_ranging_parameter->symbol_offset_ir))
	{
/*
            if (ranging_result != NULL)
            {
                free(ranging_result);
                ranging_result = NULL;
            }*/
/*
            p_msg_out->my_type = out_que_id;

            p_msg_out->p_buf = NULL;

            if (wmrt_enqueue (out_que_id, p_msg_out,
                        sizeof(struct queue_msg)) == -1)
            {
                FERROR ("ENQUEUE FOR PR RESULT ERROR\n");
            }
*/          
	    FLOG_INFO("E007_ranging: There is no ranging symbol in this symbol slot \n");
	}

#ifndef __RANGING_UT__
	if (ant_num == 2)
	{
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_0);
	    return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_1);
	}
	else 
	{
            return_code = phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_0);
	}
#endif
        //gettimeofday(&tv_1, NULL);

//        printf("processing time is %d \n", (u_int32_t)(tv_1.tv_usec - tv.tv_usec));

    /* skip free anyway */
/*
        if (phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_0))
        {
            printf("phy_ul_rx: Error in rru symbol deinit!\n");
            return ERROR_CODE;
        }
        if (ant_num == 2)
        {
            if (phy_ul_deinit_rrusymbol(phy_ul_rru_symbol_ranging_1))
            {
                printf("phy_ul_rx: Error in rru symbol deinit!\n");
                return ERROR_CODE;
            }

        }
*/
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING_PR ));
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_RANGING ));
    }

    free(p_msg_in_0);
    free(p_msg_in_1);
    free(p_msg_out);

    return return_code;
}

int32_t phy_dump_ranging_result(struct phy_ranging_result *ranging_result)
{
#ifdef _RANGING_DUMP_
    int i;

    if (ranging_result->user_num == 0)
    {
        return 0;
    }

    if (ranging_result->ranging_type == BITMAP_IR)
    {
        printf("==== It is for initial ranging ====\n");

        printf("There are %d IR user(s) detected \n", ranging_result->user_num);

        printf("\n");

        if (ranging_result->user_num > 0)
	{
            printf("The information of user 0 \n");

            printf("Ranging code id is %d \n", 
			ranging_result->ranging_result_0.ranging_code_id);
      
            printf("Time offset is %d \n", 
			ranging_result->ranging_result_0.time_offset);
     
            printf("Frequency offset is %d \n", 
			ranging_result->ranging_result_0.frequency_offset);

            printf("Adjust power is %d \n", 
			ranging_result->ranging_result_0.adj_power_fix);

            printf("Ranging SNR is %f \n", 
			ranging_result->ranging_result_0.ranging_snr);

            if (ranging_result->user_num > 1)
	    {
	        printf("\n");

                printf("The information of user 0 \n");

                printf("Ranging code id is %d \n", 
			ranging_result->ranging_result_1.ranging_code_id);
      
                printf("Time offset is %d \n", 
			ranging_result->ranging_result_1.time_offset);
     
                printf("Frequency offset is %d \n", 
			ranging_result->ranging_result_1.frequency_offset);

                printf("Adjust power is %d \n", 
			ranging_result->ranging_result_1.adj_power_fix);

                printf("Ranging SNR is %f \n", 
			ranging_result->ranging_result_1.ranging_snr);


	    }
        }

    }

    else if (ranging_result->ranging_type == BITMAP_PR)
    {
        printf("==== It is for periodic ranging ====\n");

        printf("There are %d PR user(s) detected \n", ranging_result->user_num);

        printf("\n");

        if (ranging_result->user_num > 0)
	{
            printf("The information of user 0 \n");

            printf("Ranging code id is %d \n", 
			ranging_result->ranging_result_0.ranging_code_id);
      
            printf("Time offset is %d \n", 
			ranging_result->ranging_result_0.time_offset);
     
            printf("Frequency offset is %d \n", 
			ranging_result->ranging_result_0.frequency_offset);

            printf("Adjust power is %d \n", 
			ranging_result->ranging_result_0.adj_power_fix);

            printf("Ranging SNR is %f \n", 
			ranging_result->ranging_result_0.ranging_snr);

            if (ranging_result->user_num > 1)
	    {
	        printf("\n");

                printf("The information of user 0 \n");

                printf("Ranging code id is %d \n", 
			ranging_result->ranging_result_1.ranging_code_id);
      
                printf("Time offset is %d \n", 
			ranging_result->ranging_result_1.time_offset);
     
                printf("Frequency offset is %d \n", 
			ranging_result->ranging_result_1.frequency_offset);

                printf("Adjust power is %d \n", 
			ranging_result->ranging_result_1.adj_power_fix);

                printf("Ranging SNR is %f \n", 
			ranging_result->ranging_result_1.ranging_snr);


	    }
        }
    }
    else
    {
        printf("==== It's not the ranging opportunity zone ====\n");
    }

    (void) i;
#else
    (void)ranging_result;
#endif
    return 0;
}

int32_t phy_ranging_free_result(struct phy_ranging_result * ranging_result)
{
//    free(ranging_result->ranging_result_0);
//    free(ranging_result->ranging_result_1);
//    free(pr_result->pr_result_0);	
//    free(pr_result->pr_result_1);	

    free(ranging_result);

    return 0;
}

int frm_dump_ranging_power (int flag, FILE * f_p, int len, void * buf)
{
    struct phy_hook_ranging_power * p_tmp = (struct phy_hook_ranging_power *) buf;
    int i;

    if ((buf == NULL) || (len = 0))
    {
        FLOG_ERROR("Wrong Ranging DUMP\n");
        return 1;
    }
    
// flag = 0: start of a new frame; =1: IR, =2: PR
    if (flag == 0)
    {
	fprintf(f_p, "\n");
	fprintf(f_p, "%d,%d,%d,%d,%d,", p_tmp->frm_num,
                                    p_tmp->mean_power_ant0,
                                    p_tmp->mean_power_ant1,
                                    p_tmp->dgain_ant0,
                                    p_tmp->dgain_ant1);
    }
    else if (flag == 3)
    {
	fprintf(f_p, "%d,%d\n", p_tmp->mean_power_ant0, p_tmp->mean_power_ant1);
    }
    else
    {
	fprintf(f_p, "%d,%f,%f\n", p_tmp->ranging_type, 
				    p_tmp->mean_power_ant0_adj,
				    p_tmp->mean_power_ant1_adj);
	for (i = 0; i < 10; i++)
    	{
	    fprintf(f_p, "%d,%f\n", p_tmp->cor_code_id[i], p_tmp->cor_code_result[i]);
    	}    

	fprintf(f_p, "%f,%f\n", p_tmp->mean_threshold_jdg, p_tmp->snr);
    }

    fflush(f_p);

    return 0;
}

int frm_dump_ranging_result (int flag, FILE * f_p, int len, void * buf)
{
    struct phy_hook_ranging_result * p_tmp = (struct phy_hook_ranging_result *) buf;

    if ((buf == NULL) || (len = 0))
    {
        FLOG_ERROR("Wrong Ranging DUMP\n");
        return 1;
    }

    fprintf(f_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                                    p_tmp->frm_num,
                                    p_tmp->type,
                                    p_tmp->num,
                                    p_tmp->code_id0,
                                    p_tmp->time0,
                                    p_tmp->freq0,
                                    p_tmp->power0,
                                    p_tmp->snr0,
                                    p_tmp->code_id1,
                                    p_tmp->time1,
                                    p_tmp->freq1,
                                    p_tmp->power1,
                                    p_tmp->snr1);

    fflush(f_p);

    (void) flag;

    return 0;
}


/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_UL_RX_RANGING

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif
