/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_wk_generator.c

   Function: This function is used to generate the w_k for subcarrier
             randomization (downlink).

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdio.h>
#include <math.h>
#include <string.h>
#include "phy_dl_wk_generator.h"
#include "flog.h"

/**----------------------------------------------------------------------------
   Function:    phy_dl_wk_generator()

   Description:  802.16's modulation control sequence.
                 This function generates a pseudo-random binary sequence (PRBS) as
                 the WiMAX specification. Direction is downlink. Sequence length shall
                 give the length of the PRBS used in the process.The PRBS used is x^11 + x^9 + 1.
                 The returned values w_k is the sequence element in PRBS series, which is
                 used in the data and pilot modulation.
                 For First Zone, it will use IDCell and Segment Index to generate the PRBS.
                 For non First Zone, it will use DL_PermBase and PRBS_ID to generate.


   Parameters:

       Input: struct struct phy_dl_tx_syspara *para;  data structure for system parameters transmission;

              The parameters used for w_k generation include:
                 para.ofdma_nused = 841;   used subcarrier for one OFDM symbol with DC;
                 para.first_zone ;       first zone set, 1--First Zone; 0--Non First Zone;
                 para.id_cell;           IDCell;
                 para.segement_index;    segment index   (0--2);
                 para.dl_permbase;       DL_PermBase;
                 para.prbs_id;           PRBS_ID;
                 para.symbol_index;      Symbol number index;
                 para.symbol_per_slot;   symbol number in one slot;

       Output: int *w_k [843]     w_k ;

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int phy_dl_wk_generator(struct phy_dl_tx_syspara *para,
                        int *w_k)
{
   
    int seed[11];

    int firstzone;
    int nused;
    int idcell;
    int segment;
    int dl_permbase;
    int prbs_id;
    int nsym;


    int i, j;
    int seq_length, xnew;

    firstzone = para->first_zone;
    nused = para->ofdma_nused -1;//756
    idcell = para->id_cell;//1
    segment = para->segment_index;//1
    dl_permbase = para->dl_permbase;//1
    prbs_id = para->prbs_id;//0
    nsym = para->symbol_per_slot;//3

    if (para == NULL) {
        FLOG_ERROR("E001_wk_generator: the pointer refer to system parameter is null!\n");
        return ERROR_CODE;
    }

    if (w_k == NULL) {
        FLOG_ERROR("E002_wk_generator: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }


    if (firstzone == 1){

        for (i=0; i<5;i++) {
            seed[4-i] = (idcell >> i) & 0x01;  // 5 LSBs of IDcell as indicated by the frame preamble
        }

        for (i=0; i<2; i++) {
            seed[6-i] = ((segment + 1) >> i) & 0x01; //  set to segment number + 1 as indicated by the frame preamble
        }
    }
    else  //non first zone
    {
        for (i=0; i<5;i++) {
            seed[4-i] = (dl_permbase >> i) & 0x01; //b0:b4 (b0 is MSB and b4 is LSB)
        }

        for (i=0; i<2; i++) {
            seed[6-i] = (prbs_id >> i) & 0x01; //b5:b6 (b5 is MSB and b6 is LSB)
        }
    }

    for (i=7;i<11;i++) {
        seed[i] = 1;
    }

    seq_length = nused + 32 + nsym; //subcarrier nused + symbol_offset + symbol  number;

    for (i=0; i<seq_length; i++) {
        w_k[i] = seed [10];
        xnew = seed[10] ^ seed[8];
        for (j=10; j>0; j--) {
            seed[j] = seed[j-1];
        }
        seed[0] = xnew;
    }

    return SUCCESS_CODE;

}
