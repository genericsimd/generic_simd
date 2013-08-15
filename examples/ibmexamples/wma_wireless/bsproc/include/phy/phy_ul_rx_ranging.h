/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_rx_ranging.h

   Change Activity:

   Date             	Description of Change                            By
   -----------      ---------------------                 --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_UL_RANGING_H__

#define __PHY_UL_RANGING_H__

#include "phy_ul_rx_interface.h"
#include "queue_util.h"

#define LEFT_GUARD (134)

//#define RANGING_DEBUG (1)

#define DIGITAL_POWER_ESTIMATION (1) 

//#define RANGING_STATISTIC_TEST (1)

#define INITIAL_RANGING_FLAG (0)

#define PERIODIC_RANGING_FLAG (1)

#define DEFAULT_IR_SYMBOL_OFFSET (0)

#define DEFAULT_PR_SYMBOL_OFFSET (0)

#define DEFAULT_IR_SUBCH_OFFSET (0)

#define DEFAULT_PR_SUBCH_OFFSET (0)

#define BITMAP_IR    0x0001
#define BITMAP_PR   0x0002


#define MAX_FFT_SIZE 2048

#define RANGE_CODE_LEN (144)

#define IR_CODE_SET_MAX_NUM (35)

#define PR_CODE_SET_MAX_NUM (35)

#define TOTAL_CODE_SET_MAX_NUM (255)

#define SYMBOLS_IN_SLOTSYMBOL (3)


#ifdef __RANGING_UT__

#define RANGING_SNR_THRESHOLD (2000)   // This should be reconfiguration by management system
#define IR_CODE_SET_NUM (3)
#define PR_CODE_SET_NUM (3)
#define IR_CODE_SET {1, 2, 3}
#define PR_CODE_SET {4, 5, 6}
#define DIGTAL_GAIN0 (1) //will get from RRH
#define DIGTAL_GAIN1 (4) //will get from RRH
#define CALIBRATE_ANALOG_POWER0 (-60) //dBm The real value should be aquired from RRU
#define CALIBRATE_DIGITAL_POWER0 (10000) //The real value should be aquired from RRU
#define EXPECT_ANALOG_POWER (-80) //dBm, the expected receive power level
#define CALIBRATE_DIGTAL_GAIN0 (0) //not to be used in current system
#define CALIBRATE_DIGTAL_GAIN1 (0) //not to be used in current system

#endif


#if 0
struct phy_ul_rru_symbol_ranging
{
   float   dgain0;        //got from RRH
   float   dgain1;     //got from RRH

   u_int32_t    frame_index;
   u_int32_t    ranging_bitmap; /*To mark if there is IR symbol or 
   						PR symbol in this frame. Framework will 
   						check the DL IE, and mark the bitmap. 
   						Will we support more than one PR opportunity 
   						in one frame ? */
   u_int32_t    symbol_offset_ir; /* Symbol offset of IR, to get from UL_MAP */
   u_int32_t    symbol_offset_pr; /* Symbol offset of PR, to get from UL_MAP */
   u_int32_t    subch_offset_ir; /* Sub-channel offset of IR, to get from UL_MAP */
   u_int32_t    subch_offset_pr; /* Sub-channel offset of PR, to get from UL_MAP */
   u_int32_t    symbol_num; /* Number of symbol */
   u_int32_t    symbol_len; /* Length in float for each symbol */
   float  *symbol_i;        /* Real part of the entire frame*/
   float  *symbol_q;        /* Imaginary of the entire frame */      
};
#endif



struct phy_ranging_para{
    u_int32_t    ir_ranging_codeset_num;
    u_int32_t    pr_ranging_codeset_num;
    u_int32_t    ranging_allocation[RANGE_CODE_LEN * 3]; //the ranging subcarrier IDs
    u_int32_t    ir_codeset_ID_array[IR_CODE_SET_MAX_NUM];
    u_int32_t    pr_codeset_ID_array[PR_CODE_SET_MAX_NUM];

    u_int32_t    symbol_offset_ir; /* Symbol offset of IR, to get from UL_MAP */
    u_int32_t    symbol_offset_pr; /* Symbol offset of PR, to get from UL_MAP */
    u_int32_t    subch_offset_ir; /* Sub-channel offset of IR, to get from UL_MAP */
    u_int32_t    subch_offset_pr; /* Sub-channel offset of PR, to get from UL_MAP */
    u_int32_t    cp_len;
    u_int32_t    fft_len;

    int32_t   ranging_codeset[TOTAL_CODE_SET_MAX_NUM * RANGE_CODE_LEN];   
    int32_t   ranging_corr_codeset[TOTAL_CODE_SET_MAX_NUM * (RANGE_CODE_LEN >> 2) * 3];   
    float   ranging_threshold; //the threshold which could be adjusted externally
    float   ranging_snr_threshold; //the threshold being used the SNR method

    int ranging_algorithms;

#ifdef IPP_OPT_TEMP730
   IppsFFTSpec_C_32f* pFFTSpecFwd;
   IppsFFTSpec_C_32f* pFFTSpecInv;
   Ipp8u* BufFwd;
   Ipp8u* BufInv;
#endif

   float **XX;
   float **x;
   float **X;

};


struct ranging_rru_para{
    float    dgain0;		//got from RRH
    float    dgain1;		//got from RRH
    float    cali_dgain0; //It is not used in this RRU system.
    float    cali_dgain1; //It is not used in this RRU system.
    int    expect_ana_pwr; //It will be set when system is initiated
    int     cali_ana_pwr0; // It will be set when system is initiated
    int      cali_digi_pwr0; //It will be set when system is initiated.
    int     cali_ana_pwr1; // It will be set when system is initiated
    int      cali_digi_pwr1; //It will be set when system is initiated.
};


struct ranging_result_struct {
    int32_t	ranging_code_id;
    int32_t 	time_offset;
    int32_t	frequency_offset;
    int	adj_power_fix; //the required power adjustment for subscriber
    float	ranging_snr;	//To evaluate the ranging performance
};


struct phy_ranging_result{
    u_int32_t  ranging_type;
    u_int32_t  user_num; // The number of detected IR users. 
                                       //  In our system, this number will be <=2.
    struct ranging_result_struct ranging_result_0;
    struct ranging_result_struct ranging_result_1;
};

struct phy_hook_ranging_power
{
    u_int32_t frm_num;
    u_int32_t mean_power_ant0;
    u_int32_t mean_power_ant1;
    int dgain_ant0;
    int dgain_ant1;
    float noise_figure;
    int ranging_type;
    float mean_power_ant0_adj;
    float mean_power_ant1_adj;
    u_int32_t cor_code_id[10];
    float cor_code_result[10];
    float mean_threshold_jdg;
    float snr;
}__attribute__((packed));

struct phy_hook_ranging_result
{
    u_int32_t frm_num;
    int type;
    int num;
    int code_id0;
    int time0;
    int freq0;
    int power0;
    int snr0;
    int code_id1;
    int time1;
    int freq1;
    int power1;
    int snr1;
}__attribute__((packed));


/*
struct pr_result_struct{
    int32_t	ranging_code_id;
    int32_t 	time_offset;
    float	frequency_offset;
    int		adj_power_fix; //the required power adjustment for subscriber
    float	ranging_snr;	//To evaluate the ranging performance
};


struct phy_pr_result{
    u_int32_t  pr_user_num; // The number of detected PR users. In our system, this number will be <=2.
    struct pr_result_struct *pr_result_0;
    struct pr_result_struct *pr_result_1;
};
*/

int32_t phy_dump_ranging_result(struct phy_ranging_result *ranging_result);

int32_t phy_ul_ranging ( struct phy_ul_rx_syspara *phy_ul_sys_parameter,
	                             const struct ranging_rru_para *rru_parameter,
	                             struct phy_ranging_para *phy_ranging_parameter, 
//	                             struct phy_ranging_result *ranging_result, 
//	                             struct phy_pr_result *pr_result,
                                     const int in_que_id_0,
                                     const int in_que_id_1,
                                     const int out_que_id);
//                                     struct queue_msg *p_msg_in_0,
//                                     struct queue_msg *p_msg_in_1,
//                                     struct queue_msg *p_msg_out);


int32_t	phy_ul_ranging_init(const struct phy_ul_rx_syspara *phy_ul_sys_parameter,
                            struct phy_ranging_para *ranging_parameter, 
                            struct ranging_rru_para *rru_parameter);

int32_t phy_ul_ranging_deinit (struct phy_ranging_para *phy_ranging_parameter, 
	                             struct ranging_rru_para *rru_parameter);
//	                             struct phy_ranging_result *ranging_result);
//	                             struct phy_pr_result *pr_result);


int32_t phy_ranging_free_result(struct phy_ranging_result * ranging_result);

int frm_dump_ranging_power (int flag, FILE * f_p, int len, void * buf);

int frm_dump_ranging_result (int flag, FILE * f_p, int len, void * buf);


#endif //__PHY_UL_RANGING_H__

