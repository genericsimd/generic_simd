/* ----------------------------------------------------------------------------
   (IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_randomizer.c

   Function:  randomizer in the FEC coding

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          ------------
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <math.h>
#include "phy_dl_randomizer.h"
#include "flog.h"


/*
// required parameters
// Ns    is the allocated slots number for data burst 30
// Data allocated      30 * 24 * 4 /2  (DL_PUSC)
// R    is the repetition factor
// FEC block size = (modulation, coding rate, Ns)
*/

/**----------------------------------------------------------------------------
   Function:    phy_dl_randomizer()
   Description: Randomizer in the FEC coding .
   Parameters:
                Input-  [const struct phy_dl_tx_syspara para]  The system parameters that
                        include the slot length and numbers for each FEC block

          []p_seed]  the pointer refer to the seed

                Output- [p_randomized_data]  The pointer refer to the randomized data

   Return Value:
                0       Success
                150     Error
   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

#ifdef USE_OPT_DATA
int phy_dl_randomizer(const  struct phy_dl_fec_para *para,
                      unsigned char *p_seed,
                      unsigned char *p_data,
                      unsigned char *p_randomized_data)
{
    int i,j,n;
    int m, temp;
    unsigned char head, tail, end;
    unsigned char xnew;
    unsigned char seed[15];

    int ceilslotoffset;
    int floorslotoffset;

    int index[15];
    for(i=1; i!=15; i++){
        index[i] = i-1;
    }
    index[0] = 14;

    if (p_seed == NULL || p_data == NULL) {
          FLOG_ERROR("E000_FEC_randomizer: the pointer refer to input buffer is null!\n");
          return ERROR_CODE;
    }

    //randomize for the blocks with j slots
    for (j=0;j<para->blockjslot;j++){
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
            head=index[head];
            tail=index[tail];
            end=index[end];
        }
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
        head=index[head];
        tail=index[tail];
        end=index[end];
    }

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
        head=index[head];
        tail=index[tail];
        end=index[end];
    }

    return SUCCESS_CODE;
}// randomization

#else
int phy_dl_randomizer(const  struct phy_dl_fec_para *para,
                            unsigned char *p_seed,
                            unsigned char *p_data,
                            unsigned char *p_randomized_data)
{
    unsigned int i,j,n;
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
    for (j=0;j<para->blockjslot;j++){
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
    for (i=0;i<para->blocksizeceilslot;i++){
        xnew=seed[tail]^seed[end];
        p_randomized_data[ceilslotoffset + i]=xnew^p_data[ceilslotoffset + i];
        seed[end]=xnew;
        head=(head+14)%15;
        tail=(tail+14)%15;
        end=(end+14)%15;
    }

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

    return SUCCESS_CODE;
}// randomization
#endif

