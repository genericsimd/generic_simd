/* ----------------------------------------------------------------------------

   (C)Copyright 2009

   International Business Machines Corporation,

   All Rights Reserved.



   This file is IBM Confidential and is not to be distributed.



   File Name: phy_ul_rx_common.h



   Function: Declare the common functions, define the common variables

             and macros



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------

   07-Mar.2010      Created                                          Wang Qing



   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */





#ifndef __PHY_UL_RX_COMMON_H__

#define __PHY_UL_RX_COMMON_H__

#include <sys/types.h>



/** Macros used by 16e DL modules */

#define ROOT2                    (1.4142135623730951) /* sqrt(2) */

#define CPLEN			 (128)

#define OFDMA_SYMBOL_SIZE        (1024) /* It is also the size for fft */

                                        /* Only used for defining the size of local array */

                                        /* Should equal to g_phy_dl_sys_para.ofdma_nfft */

#define PI     (3.1415926535897932384626433832795)

#define TWO_PI (6.283185307179586476925286766558)



/** Others */

#define ERROR_CODE                (150)

#define MEM_ALLOC_ERROR_CODE      (151)

#define SUCCESS_CODE              (0)



#define LOSE_SYNC                 (50)

//#define IPP_OPT_TEMP730

#ifdef  IPP_OPT_TEMP730
//#include "ipp.h"
#endif

//#define _WIN_TIME_

#define _LINUX_TIME_

//#define _DEBUG_   

//#define _FEC_DUMP_
//#define _PRINTOUT_

#ifdef _PRINTOUT_

#define DBG(str) str

#else

#define DBG(str)

#endif





#define RECORDDATA //for data recoding, added by WQ



#ifndef SQRT2

#define SQRT2 1.414213562373095    //added by wq

#endif



#ifndef ZONE_SYMBOL_OFFSET

#define ZONE_SYMBOL_OFFSET 3072

#endif


#endif //__PHY_UL_COMMON_H__

