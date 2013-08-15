/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_derepetition.c

   Function:  derepetion in the FEC decoding 

   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------
   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#include <math.h>

#include <stdlib.h>

#include <stdio.h>

#include "phy_ul_fec_decoding.h"

#include "phy_ul_derepetition.h"

#include "flog.h"

//derepetition

int32_t phy_ul_derepetition(struct phy_ul_fec_para *para, 

	                    float *inputdata, 

		            float **outputdata)

{

  unsigned int i, j; 

  float *pdata = *outputdata;



  switch (para->code_id)

  {

	  case CC_QPSK12:

	  case CC_QPSK34:

//		para->burst_len_punctured = para->burst_len_repeated / para->repetition_code; 

		

		for(i=0;i<para->burst_len_repeated/para->repetition_code;i++)

		{     

                    pdata[i] = 0.0;

		    for(j=0;j<para->repetition_code;j++)

                        {pdata[i] += inputdata[i*para->repetition_code + j];}

                    pdata[i] = pdata[i]/para->repetition_code;

		}

		break; 	 

	  case CC_QAM16_12:

	  case CC_QAM16_34:  

	  case CC_QAM64_12:  

	  case CC_QAM64_23:

	  case CC_QAM64_34:

	        *outputdata = inputdata;   //RepetitionCoding is used only when the burst uses QPSK modulation

	//      para->burst_len_punctured = para->burst_len_repeated;

		break; 	   

	  default: 

		FLOG_ERROR("Unsupport code_id %d \n", para->code_id);

		return -1;

	}	 

  

  return 0;

}





