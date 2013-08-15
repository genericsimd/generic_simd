/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_ul_wk_generator.c



   Function: This function is used to generate the w_k for subcarrier 

             randomization (uplink). 

   

   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------




   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                                 */





#include <stdio.h>

#include <math.h>

#include "phy_ul_rx_common.h"

#include "phy_ul_rx_interface.h"

#include "phy_ul_wk_generator.h"

#include "flog.h"

/**----------------------------------------------------------------------------

   Function:    phy_ofdma_ul_wk_generator()



   Description:  802.16's modulation control sequence. 

                 This function generates a pseudo-random binary sequence (PRBS) as

                 the WiMAX specification. Direction is downlink. Sequence length shall 

                 give the length of the PRBS used in the process.The PRBS used is x^11 + x^9 + 1. 

	               The returned values w_k is the sequence element in PRBS series, which is 

	               used in the data and pilot modulation.                

	               For First Zone, it will use IDCell and Segment Index to generate the PRBS.

	               For non First Zone, it will use DL_PermBase and PRBS_ID to generate.

	               



   Parameters:

   

       Input: struct phy_ul_rx_syspara *para;  data structure for system parameters transmission;

       Output: int *w_k [843]     w_k ;

               

   Return Value:

                0       Success

                150     Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */







int32_t phy_ul_wk_generator(struct phy_ul_rx_syspara *para, 
                            int *wk)

{

    unsigned int seed[11];

    unsigned int i, seq_length;

    unsigned int xnew, j;



    unsigned int nused, idcell;

    unsigned int symbol_offset, symbol_number;

    unsigned int frame_number, frame_num_temp;


    nused = para->ofdma_nused;//841

    idcell = para->id_cell;//3  

    symbol_offset = para->symbol_offset; //0

    symbol_number = para->symbol_per_slot;  //3

/*
#ifdef _REAL_FRAME_NUM_
    if (para->frame_index <1)
    {
        frame_number = 0;
    }
    else
    {
        frame_number = frame_number - 1;
    }
#else
    frame_number = para->frame_index; //1
#endif
*/
    frame_number = para->frame_index; //1

    if (para == NULL) {

        FLOG_ERROR("E001_wk_generator: the pointer refer to system parameter is null!\n");

        return ERROR_CODE;

    }

    

    if (wk == NULL) {

        FLOG_ERROR("E002_wk_generator: the pointer refer to output buffer is null!\n");

        return ERROR_CODE;

    }



    for (i=0; i<5;i++){

        seed[4-i] = (idcell >> i) & 0x01;

    }



    seed [5] = 1;

    seed [6] = 1;

    if(para->frame_increased == 1)
    {
        frame_num_temp = frame_number % 16;
    }
    else
    {
        frame_num_temp = 0;
    }
    for (i=0; i<4;i++){

	seed[10-i] = (frame_num_temp >> i) & 0x01;

    }


     seq_length = nused + 32 + symbol_number;


    for (i=0; i<seq_length; i++){

	wk[i] = seed [10];

        xnew = seed[10] ^ seed[8];

        for (j=10; j>0; j--){

            seed[j] = seed[j-1];

	  }

	  seed[0] = xnew;

    }

  

    return SUCCESS_CODE;

}



