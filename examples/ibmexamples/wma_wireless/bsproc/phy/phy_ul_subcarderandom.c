/* ----------------------------------------------------------------------------

   (C)Copyright 2009

   International Business Machines Corporation,

   All Rights Reserved.



   This file is IBM Confidential and is not to be distributed.



   File Name: phy_ofdma_ul_subcarderandom.c



   Function: Subcarrier derandomize 

   

   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------




   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                                 */







#include <stdio.h>

#include "phy_ul_rx_common.h"

#include "phy_ul_rx_interface.h"

#include "phy_ul_subcarderandom.h"

#include "flog.h"


/**----------------------------------------------------------------------------

   Function:    phy_ul_subcarderandom()



   Description: This model is used to devide the factor 2*(0.5-Wk) to the constellation-mapped 

                data according to the subcarrier physical index, k.



   Parameters:

   

       Input: const phy_ul_rx_syspara *para;  data structure for system parameters transmission;

              const float *subcar_r [840*3]      The real part of input subcarrier; 

              const float *subcar_i [840*3]      The imag part of input subcarrier;

                       

       Output: float *subcarderandom_r [840*3]     real part of subcarderandom result ;

               float *subcarderandom_i [840*3]     imag part of subcarderandom result ;  


   Return Value:

                0       Success

                150     Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */







int32_t phy_ul_subcarderandom(struct phy_ul_rx_syspara *para, 
                              int32_t *wk,
                              const float *subcar_r, 
                              const float *subcar_i,
                              float *subcarderandom_r, 
                              float *subcarderandom_i)

{

    int nused, nused_dc, sym_offset;
    int nsym, num;	  
    int i,j;
    float temp1;

    nsym = para->symbol_per_slot;//3
    nused = para->ofdma_nused_no_dc;//840
    nused_dc = para->ofdma_nused;//841
    sym_offset = para->symbol_offset;
    num = nused / 2; //420

    if (para == NULL) 
    {
	FLOG_WARNING("E001_subcarderandom: the pointer refer to system parameter is null!\n");
	return ERROR_CODE;
    }

    if (subcar_r == NULL || subcar_i == NULL) 
    {
        FLOG_WARNING("E002_subcarderandom: the pointer refer to input subcar is null!\n");
        return ERROR_CODE;
    }

    if (subcarderandom_r == NULL || subcarderandom_i == NULL) 
    {
        FLOG_WARNING("E003_subcarderandom: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

	
    sym_offset = sym_offset % 32; /* clock 32 */
    for (j=0;j<nsym;j++)
    {
        for (i=0;i<num;i++)
        {
            temp1 = 2 * (0.5 - wk[i+j+sym_offset]);
            subcarderandom_r[j*nused+i] = subcar_r[j*nused+i] * temp1;
            subcarderandom_i[j*nused+i] = subcar_i[j*nused+i] * temp1;
        }
        for (i=num+1;i<nused_dc; i++)
        {
            temp1 = 2 * (0.5 - wk[i+j+sym_offset]);
            subcarderandom_r[j*nused+i-1] = subcar_r[j*nused+i-1] * temp1;
            subcarderandom_i[j*nused+i-1] = subcar_i[j*nused+i-1] * temp1;
        }
    }

    return SUCCESS_CODE;

}







