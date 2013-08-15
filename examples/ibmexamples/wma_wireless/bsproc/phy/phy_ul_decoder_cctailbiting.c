/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_decoder_cctailbiting.c

   Function: Convolutional code decoding in the FEC coding

   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------


      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#include <stdio.h>

#include <stdlib.h>

#include <math.h>

#include <string.h>

#include <time.h>



#include "phy_ul_rx_common.h"

#include "phy_ul_fec_decoding.h"

#include "phy_ul_decoder_cctailbiting.h"

#include "viterbicore.h"

#include "flog.h"



/*----------------------------------------------------------------------------

   Function:    phy_ofdma_ul_decoder_cctailbiting

   Description: Convolutional code decoding in the FEC coding .

   Parameters:

                Input-  [const phy_ul_sys_para para]  The system parameters that 

                        include the slot length and numbers for each FEC block

	                [symbols]  the pointer refer to the input encoded 


                Output- [data]  The pointer refer to the decoded data



   Return Value:

                0       Success

                150     Error

   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                                 */



#ifndef ZERO   

#define ZERO 0

#endif



int32_t phy_ul_decoder_cctailbiting(struct phy_ul_fec_para *para,

				    u_int8_t *symbols,

			            u_int8_t *data)

{

	int j,k;

	unsigned int endstate=0,startstate=0;

	u_int8_t *tmpdata;

	int Ncbps0, Ncbps1, Ncbps2;

	int tb_len;

	unsigned char tmpsymbols[3*288*2]__attribute__ ((aligned (128)));

	void * vp = para->vp;
        

	float code_rate = (float)para->burst_len_encoded/(float)para->bits_slotsymbol;

        Ncbps0 = 0;
        Ncbps1 = 0;
        Ncbps2 = 0;
      

       ////////////////////////////////////////////////////////////////////////////////////////////////////

       ///////////////////////////////////multi TB blocks (TB0 TB1 TB2 TB3 )///////////////////////////////

       ///////////////////////////////////algorighm: multi TB blocks //////////////////////////////////////

       ////////////////////////////////////TB0 TB1 TB2 TB3 TB0 TB1 ////////////////////////////////////////

       //                                        <---------------- TB1 is for training

       //                                                    |

       //                                                output the data 

       // TB1 for training, and TB0 output the 0 data 

       ////////////////////////////////////////////////////////////////////////////////////////////////////

	Ncbps0 = (int)(para->blocksizejslot * code_rate);  
      //  printf("Ncbps0 in viterbi = %d\n", Ncbps0);
	tb_len = para->blocksizejslot/4;  //use 1/4 of the input length as the padding block size

  	k = para->blockjslot;

	

        for ( j =0; j<k; j++){

		 /////////generate the buffer for decoding /////////////////

		 memcpy(&tmpsymbols[0],      &symbols[j*Ncbps0], Ncbps0);

		 memcpy(&tmpsymbols[Ncbps0], &symbols[j*Ncbps0], 2*tb_len*code_rate); //add TB0 and TB1



		 tmpdata=&data[j*para->blocksizejslot]; 

	         /////////training ////////////

		 startstate = ZERO;


               /*
               FLOG_INFO("Ncbps0 =%d tb_len=%d blocksizejslot =%d, blockjslot =%d\n", Ncbps0, tb_len, 
                             para->blocksizejslot, para->blockjslot);
               */
	

                 init_viterbi(vp,startstate);
                 update_viterbi_blk(vp,tmpsymbols,(para->blocksizejslot+tb_len*2)); 

		 //chainback

		 chainback_viterbi_notail(vp,tmpdata,para->blocksizejslot, tb_len, &endstate);

	 }

	

	////////////////////////////////////////////////

	//////////decode block with ceil slots//////////

	////////////////////////////////////////////////

        if(para->blocksizeceilslot!=0)

        {

         Ncbps1 = (int)(para->blocksizeceilslot * code_rate);

	 tb_len = para->blocksizeceilslot/4;  //use 1/4 of the input length as the padding block size

	 

	 startstate = ZERO;

	 init_viterbi(vp,startstate);



	 memcpy(&tmpsymbols[0],      &symbols[k*Ncbps0], Ncbps1);

	 memcpy(&tmpsymbols[Ncbps1], &symbols[k*Ncbps0], 2*tb_len*code_rate); //add TB0 and TB1



	 tmpdata=&data[k*para->blocksizejslot]; 



        update_viterbi_blk(vp,tmpsymbols,para->blocksizeceilslot + tb_len*2);

         //chainback 
	 chainback_viterbi_notail(vp,tmpdata,para->blocksizeceilslot, tb_len, &endstate);

        }



	////////////////////////////////////////////////

	//////////decode block with floor slots/////////

	////////////////////////////////////////////////

        if(para->blocksizefloorslot!=0)

        {

	 Ncbps2 = (int)(para->blocksizefloorslot * code_rate); 

	 tb_len = para->blocksizefloorslot/4;  //use 1/4 of the input length as the padding block size

	 

	 startstate = ZERO;

	 init_viterbi(vp,startstate);



	 memcpy(&tmpsymbols[0],      &symbols[k*Ncbps0 + Ncbps1], Ncbps2);

	 memcpy(&tmpsymbols[Ncbps2], &symbols[k*Ncbps0 + Ncbps1], 2*tb_len*code_rate); //add TB0 and TB1



	 tmpdata=&data[k*para->blocksizejslot+para->blocksizeceilslot]; 

         update_viterbi_blk(vp,tmpsymbols,para->blocksizefloorslot + tb_len*2);


         //chainback 

	 chainback_viterbi_notail(vp,tmpdata,para->blocksizefloorslot, tb_len, &endstate);

        }



    return 0; 

}



