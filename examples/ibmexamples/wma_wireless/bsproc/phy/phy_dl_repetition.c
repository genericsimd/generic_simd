/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_repetition.c

   Function:  repetition in the FEC coding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "phy_dl_repetition.h"
#include "flog.h"

int phy_dl_repetition(struct phy_dl_fec_para *para,
                      unsigned char *inputbits,
                      unsigned char **outputbits)
{
   u_int32_t i,j;
   unsigned char *pdata = *outputbits;

    switch (para->code_id)
    {
        case CC_QPSK12:
        case CC_QPSK34:
            para->burst_len_repeated = para->burst_len_punctured*para->repetition_code;
            for(i=0; i<para->repetition_code; i++)
            {
		    for(j=0; j<para->burst_len_punctured; j++)
                     {
			pdata[i * para->burst_len_punctured + j] =  inputbits[j];
                     }
            }
            
            break;
        case CC_QAM16_12:
        case CC_QAM16_34:
        case CC_QAM64_12:
        case CC_QAM64_23:
        case CC_QAM64_34:
            *outputbits = inputbits;   //RepetitionCoding is used only when the burst uses QPSK modulation
            para->burst_len_repeated = para->burst_len_punctured;
            break;
        default:
            FLOG_ERROR("Unsupport code id %d \n", para->code_id);
            return -1;
    }
    return 0;
}


