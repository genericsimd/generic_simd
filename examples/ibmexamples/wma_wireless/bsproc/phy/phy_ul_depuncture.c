/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ofdma_ul_depuncture.c


   Function:  Depuncture in the FEC decoding 

   Change Activity:


   Date             Description of Change                            By

   -----------      ---------------------                            --------



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#include <math.h>

#include <stdlib.h>

#include <stdio.h>

#include "phy_ul_fec_decoding.h"

#include "phy_ul_depuncture.h"

#include "flog.h"



//repetition

int32_t phy_ul_depuncture(struct phy_ul_fec_para *para, 

			  float *inputbits, 

		          float **outputbits)

{

  int i; 

  int encodeslotsymbollen = para->burst_len_encoded; 

  int punctureunitlen;   //puncture unit size 

 

   // rate id (1/2, 2/3, 3/4, 5/6);     

  switch (para->code_id)

  {

	  case CC_QPSK12:

	  case CC_QAM16_12:	  

	  case CC_QAM64_12:	  

		*outputbits = inputbits;  //do nothing we are just set the output pointer to the input pointer

		break; 	   

	  case CC_QAM64_23:	  

			//X1Y1X2Y2 rate is obtained by X1Y1Y2 ,  add  X2 for each 4 bits

//		para->burst_len_punctured = encodeslotsymbollen * 3/4; 

		punctureunitlen = encodeslotsymbollen/4; 

		for(i=0;i<punctureunitlen; i++)

		{

		      (*outputbits)[i*4]  = inputbits[i*3];   //x1

		      (*outputbits)[i*4+1]= inputbits[i*3+1]; //y1

		      (*outputbits)[i*4+2]= 0;                //x2

		      (*outputbits)[i*4+3]= inputbits[i*3+2]; //Y2

		}

		break; 	

	  case CC_QPSK34:

	  case CC_QAM16_34:

	  case CC_QAM64_34:	  

		//X1Y1X2Y2X3Y3 rate is obtained by X1Y1Y2X3 , 
		//remove X2 for each 6 bits, remove Y3 for each 6 bits

	//	para->burst_len_punctured  = encodeslotsymbollen * 4/6; 

		punctureunitlen = encodeslotsymbollen/6; 

		for(i=0;i<punctureunitlen; i++)

		{

                     (*outputbits)[i*6]   = inputbits[i*4];   //x1

		     (*outputbits)[i*6+1] = inputbits[i*4+1]; //y1

		     (*outputbits)[i*6+2] = 0;                //x2			  

		     (*outputbits)[i*6+3] = inputbits[i*4+2]; //y2

  		     (*outputbits)[i*6+4] = inputbits[i*4+3]; //x3	  

		     (*outputbits)[i*6+5]   = 0;                //y3				  

		}		   

		break; 	  		   

	  case CTC_QAM64_56:

		//X1Y1X2Y2X3Y3X4Y4X5Y5 rate is obtained by X1Y1Y2X3y4X5 , 
		//add X2 Y3 X4  Y5 for each 10 bits

	//	para->burst_len_punctured  = encodeslotsymbollen * 6/10; 

		punctureunitlen = encodeslotsymbollen/10; 

		for(i=0;i<punctureunitlen; i++)

		{

                     (*outputbits)[i*10]  = inputbits[i*6];    //x1

		     (*outputbits)[i*10+1]= inputbits[i*6+1];  //y1

		     (*outputbits)[i*10+2]= 0;                 //x2

		     (*outputbits)[i*10+3]= inputbits[i*6+2];  //y2			  

		     (*outputbits)[i*10+4]= inputbits[i*6+3];  //x3	  

		     (*outputbits)[i*10+5]= 0;  				 //y3		  		  

		     (*outputbits)[i*10+6]= 0;                 //x4			  

		     (*outputbits)[i*10+7]= inputbits[i*6+4];  //y4			  

		     (*outputbits)[i*10+8]= inputbits[i*6+5];  //x5			  

		     (*outputbits)[i*10+9]= 0;			     //y5			  

		}

		break; 		   

	  default: 

		FLOG_ERROR("Unsupport code_id%d \n", para->code_id);

		return -1;

  }

  return 0;

}



