/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __PHY_UL_IPP_PARA_H__

#define __PHY_UL_IPP_PARA_H__



#include "ipp.h"



#ifdef _BER_DEMO_

#include "frmcfg.h"

extern IppsFFTSpec_C_32f* pFFTSpecFwd_ul[INIS_NUM];

extern Ipp8u* BufFwd_ul[INIS_NUM];

#else

extern IppsFFTSpec_C_32f* pFFTSpecFwd_ul;

extern Ipp8u* BufFwd_ul;

#endif



#endif

