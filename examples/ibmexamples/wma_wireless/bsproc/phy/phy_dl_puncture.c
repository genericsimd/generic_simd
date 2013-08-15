/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
   
   File Name: phy_dl_puncture.c

   Function:  puncture in the FEC coding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------



   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "phy_dl_puncture.h"
#include "flog.h"



int phy_dl_puncture(struct phy_dl_fec_para *para,
                          unsigned char *inputbits,
                          unsigned char **outputbits)
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
            //X1Y1X2Y2 rate is obtained by X1Y1Y2 ,  remove X2 for each 4 bits
            para->burst_len_punctured = encodeslotsymbollen * 3/4;
            punctureunitlen = encodeslotsymbollen/4;
            for(i=0;i<punctureunitlen; i++){
                (*outputbits)[i*3]  = inputbits[i*4];   //x1
                (*outputbits)[i*3+1]= inputbits[i*4+1]; //y1
                (*outputbits)[i*3+2]= inputbits[i*4+3]; //y2
            }
            break;
        case CC_QPSK34:
        case CC_QAM16_34:
        case CC_QAM64_34:
            //X1Y1X2Y2X3Y3 rate is obtained by X1Y1Y2X3 ,
            //remove X2 for each 6 bits, remove Y3 for each 6 bits
            para->burst_len_punctured  = encodeslotsymbollen * 4/6;
            punctureunitlen = encodeslotsymbollen/6;
            for(i=0;i<punctureunitlen; i++)
            {
                (*outputbits)[i*4]  = inputbits[i*6];   //x1
                (*outputbits)[i*4+1]= inputbits[i*6+1]; //y1
                (*outputbits)[i*4+2]= inputbits[i*6+3]; //y2
                (*outputbits)[i*4+3]= inputbits[i*6+4]; //x3
            }
            break;
        case CTC_QAM16_56:
            //X1Y1X2Y2X3Y3X4Y4X5Y5 rate is obtained by X1Y1Y2X3Y4X5 ,
            //remove X2 Y3 X4  Y5 for each 10 bits
            para->burst_len_punctured  = encodeslotsymbollen * 6/10;
            punctureunitlen = encodeslotsymbollen/10;
            for(i=0;i<punctureunitlen; i++){
                (*outputbits)[i*6]  = inputbits[i*10];    //x1
                (*outputbits)[i*6+1]= inputbits[i*10+1];  //y1
                (*outputbits)[i*6+2]= inputbits[i*10+3];  //y2
                (*outputbits)[i*6+3]= inputbits[i*10+4];  //x3
                (*outputbits)[i*6+4]= inputbits[i*10+7];  //Y4
                (*outputbits)[i*6+5]= inputbits[i*10+8];  //x5
            }
            break;
        default:
            FLOG_ERROR("Unsupport code id %d \n", para->code_id);
            return -1;
    }
    return 0;
}

