/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.


   File Name: phy_dl_tx_common.h

   Function: Declare the common functions, define the common variables
             and macros

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_COMMON_H__
#define __PHY_DL_COMMON_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>


/** Macros for const definition */
#define SQRT2  (1.4142135623730951)
#define OFDMA_SYMBOL_SIZE   (1024) /* It is also the size for fft, should
                                      equal to phy_dl_tx_syspara.ofdma_nfft */
#define MAX_RAW_SYMBOL_LEN  (1440)

/** Macros for return code */
#define ERROR_CODE          (150)
#define SUCCESS_CODE        (0)

/** Macros for debug */
#define _NO_PADDING_ /* Must be enabled if to dump the output of tx without padding data */
#define _ZONE_BOOST_ /* Power adjustment */


#define WRIT_DEBUG_INFO(fmt, args...) \
    do { fprintf(stdout, "[%d]%s: "fmt"\n", getpid(), __FUNCTION__, ##args); fflush(stdout); } while(0)
#define WRIT_ERROR_INFO(fmt, args...) \
    do { fprintf(stderr, "[%d]%s: "fmt": %s\n", getpid(), __FUNCTION__, ##args, strerror(errno)); fflush(stderr); } while(0)

#define DEBUG_LEVL_NONE                    0
#define DEBUG_LEVL_FATAL                   1

#ifndef DEBUG_LEVL
    #define DEBUG_LEVL     DEBUG_LEVL_NONE
#endif

//#define _DEBUG_PHY_ 

#if DEBUG_LEVL > DEBUG_LEVL_NONE
#define WERROR(fmt, args...)    WRIT_ERROR_INFO(fmt, ##args)
#else
#define WERROR(fmt, args...)
#endif

#if DEBUG_LEVL >= DEBUG_LEVL_FATAL
#define WFATAL(fmt, args...)    WRIT_DEBUG_INFO(fmt, ##args)
#else
#define WFATAL(fmt, args...)
#endif

#ifndef PHY_DL_DUGFILE
#define PHY_DL_DUGFILE    "/tmp/witphydl.log"
#endif

#ifdef _PHY_DUMP_
#define IF_NEWFILE(if_new, frame_index, symbol_offset) {\
            (if_new) = if_newfile(frame_index, symbol_offset);}

#define DUMP_DATA(name, file_name, new_file, p_buf, num_to_dump) {\
            FILE *fp;\
            fp = phy_openfile(new_file, file_name);\
            dl_dump_##name(fp, p_buf, num_to_dump);\
            phy_closefile(fp);\
            }
#else
#define IF_NEWFILE(if_new, frame_index, symbol_offset) {}
#define DUMP_DATA(name, file_name, new_file, p_buf, num_to_dump) {}
#endif

#endif //__PHY_DL_COMMON_H__

