/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_ul_rx_common.h



   Function: Declare the common functions, define the common variables

             and macros



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------



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


#define PI     (3.1415926535897932384626433832795)

#define TWO_PI (6.283185307179586476925286766558)



/** Others */

#define ERROR_CODE                (150)

#define MEM_ALLOC_ERROR_CODE      (151)

#define SUCCESS_CODE              (0)

//#define SSE2OPT

#define LOSE_SYNC                 (50)

//#define IPP_OPT_TEMP730

#ifdef  IPP_OPT_TEMP730
#include "ipp.h"
#endif

//#define _WIN_TIME_

#define _LINUX_TIME_

//#define _PHY_DUMP_   
//#define _DEBUG_ZONEPERM_

//#define _FEC_DUMP_
//#define _PRINTOUT_

#ifdef _PRINTOUT_

#define DBG(str) str

#else

#define DBG(str)

#endif


//#define _DEBUG_WMAPHY_  


#define RECORDDATA 


/*
#ifndef SQRT2

#define SQRT2 1.414213562373095    

#endif
*/


/* parameters for spectrum sensing */
#define PS_NFFT (1024)
#define BITRATE (1.4e6)
#define DELTA_FREQ (1.1e6)
#define THRESHOLD (6e3)
#define NSUBCHAN 21

#define IS_NFFT (1024)

//#define PS_FAKE

#ifndef ZONE_SYMBOL_OFFSET

#define ZONE_SYMBOL_OFFSET 3072

#endif


#endif //__PHY_UL_COMMON_H__

