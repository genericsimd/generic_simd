/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2010, 2011

   All Rights Reserved.

   File Name: mac_ss_ranging_utm.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-July.2010		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAB_SS_RANGING_UTM_H
#define _MAB_SS_RANGING_UTM_H

int mac_ss_rng_utm();
int rng_rsp_enq(u_int32_t timing_adjust, u_int8_t power_adjust, u_int32_t frequency_adjust, int status);
int enq_rng_rsp_wcid();
int enq_cdma_alloc();
#endif
