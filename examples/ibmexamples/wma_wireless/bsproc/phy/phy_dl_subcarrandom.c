/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_subcarrandom.c

   Function: Subcarrier randomize

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                                 */


#include <stdio.h>
#include "phy_dl_wk_generator.h"
#include "phy_dl_subcarrandom.h"
#include "flog.h"

/**----------------------------------------------------------------------------
   Function:    phy_dl_subcarrandom()

   Description: This model is used to multiply the factor 2*(0.5-Wk) to the constellation-mapped
                data according to the subcarrier physical index, k.

   Parameters:

       Input: struct struct phy_dl_tx_syspara *para;  data structure for system parameters transmission;
              const float *subcar_r [756*3]      The real part of input subcarrier;
              const float *subcar_i [756*3]      The imag part of input subcarrier;

              The parameters used for subcar random include:
                 para.ofdma_nused_no_dc = 756;   used subcarrier for one OFDM symbol;
                 para.ofdma_nused = 757;   used subcarrier for one OFDM symbol with DC;
                 para.symbol_per_slot = 3;   symbol number for one slot under DL_PUSC;

       Output: float *subcarrandom_r [756*3]     real part of subcarrandom result ;
               float *subcarrandom_i [756*3]     imag part of subcarrandom result ;

       Include function: phy_dl_wk_generator();
       This function is to generate the Wk for subcar random/derandom. for detail, please look this function
       in seperated document (phy_dl_wk_generator.c).

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int phy_dl_subcarrandom(struct phy_dl_tx_syspara *para,
                        int *wk,
                        const float *subcar_r,
                        const float *subcar_i,
                        float *subcarrandom_r,
                        float *subcarrandom_i)
{

    int nused, nused_dc;
    int nsym, num, sym_offset;
    int i,j;

    float temp1;

    nsym = para->symbol_per_slot;//3
    nused = para->ofdma_nused_no_dc;//756
    nused_dc = para->ofdma_nused;//757
    num = nused / 2 ;//DC position, for 1024 FFT, DC_index =512
    sym_offset = para->symbol_offset-1;

    if (para == NULL) {
        FLOG_ERROR("E001_subcarrandom: the pointer refer to system parameter is null!\n");
        return ERROR_CODE;
    }

    if (subcar_r == NULL || subcar_i == NULL) {
        FLOG_ERROR("E002_subcarrandom: the pointer refer to input subcar is null!\n");
        return ERROR_CODE;
    }

    if (subcarrandom_r == NULL || subcarrandom_i == NULL) {
        FLOG_ERROR("E003_subcarrandom: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    sym_offset = sym_offset % 32; /* clock 32 */
    for (j=0;j<nsym;j++)
    {
        for (i=0;i<num;i++)
        {
            temp1 = 2 * (0.5 - wk[i+j+sym_offset]);
            subcarrandom_r[j*nused+i] = subcar_r[j*nused+i] * temp1;
            subcarrandom_i[j*nused+i] = subcar_i[j*nused+i] * temp1;
        }
        for (i=num+1;i<nused_dc; i++)
        {
            temp1 = 2 * (0.5 - wk[i+j+sym_offset]);
            subcarrandom_r[j*nused+i-1] = subcar_r[j*nused+i-1] * temp1;
            subcarrandom_i[j*nused+i-1] = subcar_i[j*nused+i-1] * temp1;
        }
    }


    return SUCCESS_CODE;

}



