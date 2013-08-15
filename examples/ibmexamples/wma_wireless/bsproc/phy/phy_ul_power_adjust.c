/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_power_adjust.c



   Function:

   Change Activity:


   Date             Description of Change                          By

   -------------    ---------------------                          ------------


   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

  #include <stdio.h>
  #include <math.h>
  #include "phy_ul_power_adjust.h"
  #include "phy_ul_rx_common.h"
  #include "flog.h"



int32_t phy_ul_power_adjust_single(struct phy_ul_rx_syspara *para,
                                   const float * input_r,
                                   const float * input_i,
                                   const float *dgain,
                                   float * output_r,
                                   float * output_i)
{
    u_int32_t i;
    float temp;

    if (input_r == NULL || input_i == NULL) {
        FLOG_ERROR("E001_poweradjust: the pointer refer to input buffer is null!\n");
    }

    if (output_r == NULL || output_i == NULL) {
        FLOG_ERROR("E001_poweradjust: the pointer refer to output buffer is null!\n");
    }

    /* processing for antenna 0 */
    temp = exp10f(-dgain[0]/20.0F);
    for( i=0; i< para->sample_per_slotsymbol; i++ )
    {
        output_r[i] = input_r[i]*temp;
        output_i[i] = input_i[i]*temp;
    }


    return SUCCESS_CODE;
}






int32_t phy_ul_power_adjust(struct phy_ul_rx_syspara *para,
                            const float * input_ant0_r,
                            const float * input_ant0_i,
                            const float * input_ant1_r,
                            const float * input_ant1_i,
                            const float *dgain,
                            float * output_ant0_r,
                            float * output_ant0_i,
                            float * output_ant1_r,
                            float * output_ant1_i) 
{ 
    u_int32_t i;
    float temp;
	
    if (input_ant0_r == NULL || input_ant0_i == NULL ||input_ant1_r == NULL || input_ant1_i == NULL) {
        FLOG_ERROR("E001_poweradjust: the pointer refer to input buffer is null!\n");
    }

    if (output_ant0_r == NULL || output_ant0_i == NULL||output_ant1_r == NULL || output_ant1_i == NULL) {
        FLOG_ERROR("E001_poweradjust: the pointer refer to output buffer is null!\n");
    }
 
    /* processing for antenna 0 */
    temp = exp10f(-dgain[0]/20.0F);
    for( i=0; i< para->sample_per_slotsymbol; i++ )
    {
        output_ant0_r[i] = input_ant0_r[i]*temp;
        output_ant0_i[i] = input_ant0_i[i]*temp;
    }

    /* processing for antenna 1 */
    temp = exp10f(-dgain[1]/20.0F) * sqrt(para->CaliDigiPwr0/para->CaliDigiPwr1);
    for( i=0; i< para->sample_per_slotsymbol; i++ )
    {
        output_ant1_r[i] = input_ant1_r[i]*temp;
        output_ant1_i[i] = input_ant1_i[i]*temp;
    }


    return SUCCESS_CODE;
}

