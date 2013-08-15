/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2010, 2011

   All Rights Reserved.

   File Name: mac_bs_ranging_utm.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-July.2010		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAB_BS_RANGING_UTM_H
#define _MAB_BS_RANGING_UTM_H

int test_rng_rsp(int32_t timing_adjust, int8_t power_adjust, int32_t frequency_adjust);
int mac_bs_rng_adjust(int32_t timing_adjust, int8_t power_adjust, int32_t frequency_adjust);
int mac_bs_rng_utm();
#endif
