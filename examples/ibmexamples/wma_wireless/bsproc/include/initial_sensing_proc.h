/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: initial_sensing_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __INIT_SENSE_PROC_H_
#define __INIT_SENSE_PROC_H_

#ifndef _SIM_RRH_

/*RRH Config Step*/
enum set_rrh_config_step
{   
   SET_RRH_CONFIG_STEP_1 = 0x00,   
   SET_RRH_CONFIG_STEP_2,      
   SET_RRH_CONFIG_STEP_3,   
   SET_RRH_CONFIG_STEP_4,     
   SET_RRH_CONFIG_STEP_BUF = 0xff    
};


int set_rrh_config(void);
#endif

int init_sensing_process (void);
int init_sensing_release (void);

extern struct spectrum_para g_spectrum;

struct spectrum_para
{
    int loop_step;//loop counter for per frequency point
    int num_freq;//number of frequency point
    unsigned int *frequency; //frequency point
    int  fre_success_flag; //frequency setting success flag 1--success, 0--failed
    int periodic_duration;
    int initial_sensing_enable;
    int initial_sensing_policy;
    float initial_sensing_thd;
//    float dgain[2];
    int cali_ana_pwr[2]; // It will be set when system is initiated
    int cali_digi_pwr[2]; //It will be set when system is initiated.
    int is_nfft;
    char fch_bitmap[6];
    char ul_bitmap[35];
    int carrier_bw;
    int carrier_pwr;
};

struct init_scan_result
{
    float average_power;
    float active_band[21];
    float reserved_bit[3];
};

struct spectrum_config_para
{
    int working_freq;
    char str_active_band[25]; /** Must be a string here! */
    int dl_unused_subch;
    int ul_unused_subch;
};

int cfg_update_ps_duration(int type, int len, void * p_buf);

#endif
