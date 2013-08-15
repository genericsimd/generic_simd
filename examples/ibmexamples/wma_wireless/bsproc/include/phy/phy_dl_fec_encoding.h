/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name:phy_dl_fec_encoding.h
 

   Function: Declare the functions  for the FEC encoding


      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_DL_FEC_ENCODING_H__
#define __PHY_DL_FEC_ENCODING_H__

#include "adapter_bs_dl_interface_data.h"
#include "phy_dl_tx_interface.h"

struct phy_hook_fch_result
{
    u_int32_t frm_num;
    char fch[48];
}__attribute__((packed));


int phy_dl_fch_fecencoding(struct phy_dl_tx_syspara *para,
                           const struct phy_dl_slot *p_first_slot,
                           const unsigned char num_of_slots,
                           float *fec_encoded_r,
                           float *fec_encoded_i,
                           unsigned int *fec_len);

int phy_dl_fec_encoding(struct phy_dl_tx_syspara *para,
                        const struct phy_dl_slot *p_first_slot,
                        const unsigned char num_of_slots,
                        float *fec_encoded_r,
                        float *fec_encoded_i,
                        unsigned int *fec_len);

#endif
