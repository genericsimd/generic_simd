/* ----------------------------------------------------------------------------

   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_derandomizer.c

   Function:  derandomizer in the FEC decoding 

   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */



#include <stdio.h>

#include <math.h>

#include "phy_ul_rx_common.h"

#include "phy_ul_fec_decoding.h"

#include "phy_ul_derandomizer.h"

#include "flog.h"


//#define _BLER_TEST_



/**----------------------------------------------------------------------------

   Function:    phy_ul_randomizer()

   Description: Randomizer in the FEC coding .

   Parameters:

                Input-  [const phy_ul_sys_para para]  The system parameters that 

                        include the slot length and numbers for each FEC block



		        []p_seed]  the pointer refer to the seed

				

                Output- [p_randomized_data]  The pointer refer to the randomized data



   Return Value:

                0       Success

                150     Error

   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */



int32_t phy_ul_derandomizer( struct  phy_ul_fec_para *para, 

		             u_int8_t *p_seed, 

			     u_int8_t *p_data, 

		             u_int8_t *p_randomized_data)

{

    int n;
    unsigned int i, j;
    unsigned char head, tail, end;
    unsigned char xnew;
    unsigned char seed[15];
    int ceilslotoffset;
    int floorslotoffset;
  
    if (p_seed == NULL || p_data == NULL) {

         FLOG_ERROR("E000_FEC_randomizer: the pointer refer to input buffer is null!\n");
         return ERROR_CODE;

    }
    //randomize for the blocks with j slots

    for (j=0;j<para->blockjslot;j++)
    {

	//initialize for each FEC blcok
	head = 0;   // points to the head of seed
	tail = 13;  // points to the tail of seed
	end  = 14;  // Initialization

	for (n=0;n<15;n++){
            seed[n] = p_seed[n];
	}

	for (i=0;i<para->blocksizejslot;i++) {

	//get the result of XORing X15 and X14
            xnew=seed[tail]^seed[end];
       //compute the randomized data value
            p_randomized_data[j*para->blocksizejslot+i]=xnew^p_data[j*para->blocksizejslot+i];
       //compute the changed seed value
            seed[end]=xnew;
            head=(head+14)%15;
            tail=(tail+14)%15;
            end=(end+14)%15;
        }

#ifdef _BER_TEST_
    i = 0;
    while(i<para->blocksizejslot)
    {
        if(p_randomized_data[j*para->blocksizejslot+i] == 1)
           { i++;}
        else
        {
            (para->blkerr_num)++;
            FLOG_DEBUG("para->blkerr_num = %d\n", para->blkerr_num);
            break;
        }
    }
#endif

    }



	//randomize for the block with ceil(m+j)/2 slots

	ceilslotoffset = para->blockjslot*para->blocksizejslot;   

	//initialize for each FEC blcok

	head = 0;   // points to the head of seed

	tail = 13;  // points to the tail of seed

	end  = 14;  // Initialization

	for (n=0;n<15;n++){

		seed[n] = p_seed[n];

	}

	for (i=0;i<para->blocksizeceilslot;i++) {

		xnew=seed[tail]^seed[end];

		p_randomized_data[ceilslotoffset + i]=xnew^p_data[ceilslotoffset + i];

		seed[end]=xnew;

		head=(head+14)%15;

		tail=(tail+14)%15;

		end=(end+14)%15;

	}
#ifdef _BER_TEST_
    i = 0;
    while(i<para->blocksizeceilslot)
    {
        if( p_randomized_data[ ceilslotoffset+i] == 1)
           { i++;}
        else
           {
            (para->blkerr_num)++;
             FLOG_DEBUG("para->blkerr_num = %d\n", para->blkerr_num);
             break;
           }
    }

#endif




	//randomize for the block with floor(m+j)/2 slots

	floorslotoffset = para->blockjslot*para->blocksizejslot + para->blocksizeceilslot; 		

	//initialize for each FEC blcok

	head = 0;   // points to the head of seed

	tail = 13;  // points to the tail of seed

	end  = 14;  // Initialization

	for (n=0;n<15;n++){

		seed[n] = p_seed[n];

	}



	for (i=0;i<para->blocksizefloorslot;i++) {

		xnew=seed[tail]^seed[end];

		p_randomized_data[floorslotoffset+i]=xnew^p_data[floorslotoffset+i];

		seed[end]=xnew;

		head=(head+14)%15;

		tail=(tail+14)%15;

		end=(end+14)%15;

	}
#ifdef _BER_TEST_
    i = 0;
    while(i<para->blocksizefloorslot)
    {
        if(p_randomized_data[floorslotoffset+i] == 1)
           { i++;}
        else
        {
            (para->blkerr_num)++;
            FLOG_DEBUG("para->blkerr_num = %d\n", para->blkerr_num);
            break;
        }
    }

#endif
	return SUCCESS_CODE;

}





