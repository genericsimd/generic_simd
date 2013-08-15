
/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_encoding_cctailbiting.c

   Function: This function is to complete the convolutional encoding with
             tail biting.

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

  
   

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "phy_dl_puncture.h" 
#include "phy_dl_encoder_cctailbiting.h"

#define ZERO 0

/* 8-bit parity lookup table */
unsigned char parity_table[] = {
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0
};

/*----------------------------------------------------------------------------
   Function:    phy_dl_encoder_cctailbiting
   Description: Convolutional code encoding in the FEC coding .
   Parameters:
                Input-  [const struct phy_dl_tx_syspara para]  The system parameters that
                        include the slot length and numbers for each FEC block

                []bit]  the pointer refer to the input binary bit in byte data

                Output- [symbol]  The pointer refer to the encoded data  1/2 code rate

   Return Value:
                0       Success
                150     Error
   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                                 */

int phy_dl_encoder_cctailbiting(const struct phy_dl_fec_para *para,
                                unsigned char *bit,
                                unsigned char *symbol)
{
    int32_t i;
    u_int32_t j;
    u_int32_t counter;
    int state_initial;
    unsigned char *encoder_input;
    unsigned char *encoder_output;

    int ceilslotoffset;
    int floorslotoffset;

    encoder_input  = (unsigned char *)(&bit[0]);
    encoder_output = (unsigned char *)(&symbol[0]);

    /* encoding for the blocks with j slots */
    for ( j =0; j<para->blockjslot; j++){
        state_initial = ZERO;
        for(i=5;i>=0;i--){
            state_initial = (state_initial << 1) | encoder_input[(j+1)*para->blocksizejslot-1-i];
        }
        for (counter =0; counter <para->blocksizejslot; counter ++) {
            state_initial = (state_initial << 1) | encoder_input[j * para->blocksizejslot + counter];
            *encoder_output ++ = parity_table[state_initial & CCPOLYA];
            *encoder_output ++ = parity_table[state_initial & CCPOLYB];
        }
    }

    /* Encoding block with ceil(m+j)/2 slots*/
    ceilslotoffset = para->blockjslot*para->blocksizejslot;
    state_initial = ZERO;
    for(i=5;i>=0;i--){
        state_initial = (state_initial << 1) | encoder_input[ceilslotoffset+ para->blocksizeceilslot-1-i];
    }
    for (counter =0; counter <para->blocksizeceilslot; counter ++) {
        state_initial = (state_initial << 1) | encoder_input[ceilslotoffset + counter];
        *encoder_output ++ = parity_table[state_initial & CCPOLYA];
        *encoder_output ++ = parity_table[state_initial & CCPOLYB];
    }

    /* Encoding block with floor(m+j)/2 slots */
    floorslotoffset = para->blockjslot*para->blocksizejslot + para->blocksizeceilslot;
    state_initial = ZERO;
    for(i=5;i>=0;i--){
        state_initial = (state_initial << 1) | encoder_input[floorslotoffset + para->blocksizefloorslot-1-i];
    }
    for (counter =0; counter < para->blocksizefloorslot; counter ++) {
        state_initial = (state_initial << 1) | encoder_input[floorslotoffset + counter];
        *encoder_output ++ = parity_table[state_initial & CCPOLYA];
        *encoder_output ++ = parity_table[state_initial & CCPOLYB];
    }

    return 0;
}


