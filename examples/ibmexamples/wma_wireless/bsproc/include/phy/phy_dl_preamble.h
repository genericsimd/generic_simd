/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_dl_preamble.h

   Function: Declare the functions to generate the preamble OFDMA symbols
             used in DL frame.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_DL_PREAMBLE_H__
#define __PHY_DL_PREAMBLE_H__

#include "phy_dl_tx_interface.h"

#define WK_HEX_LENGTH       (63)  //original --71

int phy_dl_preamble_fgen(const struct phy_dl_tx_syspara *para,
                         char *active_band,
                         float *output_fr,
                         float *output_fi);

int phy_dl_preamble_gen(const struct phy_dl_tx_syspara *para,
                        char *active_band,
                        float *output_tr,
                        float *output_ti);

int phy_dl_preamble_gen_cdd(const struct phy_dl_tx_syspara *para,
                            char *active_band,
                            float *output_tr,
                            float *output_ti);

/*  for multiband mode */
int phy_dl_preamble_fgen_multi(const struct phy_dl_tx_syspara *para,
                               char *active_band,
                               u_int32_t *sc_allocation,
                               float *output_fr,
                               float *output_fi);

int phy_dl_preamble_gen_multi(const struct phy_dl_tx_syspara *para,
                              char *active_band,
                              u_int32_t *sc_allocation,
                              float *output_tr,
                              float *output_ti);


int phy_dl_preamble_gen_cdd_multi(const struct phy_dl_tx_syspara *para,
                                  char *active_band,
                                  u_int32_t *sc_allocation,
                                  float *output_tr,
                                  float *output_ti);


int hex_to_binary(const char *data, unsigned int *dest);

int dump_phy_tx_freq_preamble_fgen(int flag, char * name, FILE * fd, int len, void *p_buf);

#endif //__PHY_DL_PREAMBLE_H__
