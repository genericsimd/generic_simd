/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_H__
#define __MAC_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h>
#include <openssl/aes.h>
#include "flog.h"

#define __ENCRYPT__
//#define ENCRYPT_TEST
#define IVLEN 8
#define TAGLEN 6 
#define KEYLEN 16
#define AALEN 0
#define PDU_OH_WITH_ENCRYPT 24

#define ARQ_ENABLED
#define PRINT_DATA

/* OFDM or OFDMA - used by CRC, Ranging etc */
#define _OFDMA_

//#define SS_TX
//#define SS_RX
#define RANGING_ENABLED
#define RANGING_TEST
#define DSA_TEST
#define DSD_TEST

//Simple debug statement. does not allow embedded special chars in S
#ifdef NODEBUG
#define DEBUG(S) S; 
#else
#define DEBUG(S) printf("%s\n",S);
#endif

#ifdef RANGING_TEST
//#define RNG_TEST_TRACE(S) printf("%s\n",S);
#define RNG_TEST_TRACE(S) FLOG_INFO(S);
#else
#define RNG_TEST_TRACE
#endif

#define BOOL int
#define TRUE 1
#define FALSE 0

// This is the lock under which any connection parameters can be accessed
extern pthread_rwlock_t conn_info_rw_lock;

// Number of SS: simulation parameter
int NUM_SS;
// CID range related values: simulation parameters
int max_valid_primary_cid;
int max_valid_secondary_cid;
int max_valid_ugs_cid;
int max_valid_ul_ugs_cid;
int max_valid_ertps_cid;
int max_valid_rtps_cid;
int max_valid_nrtps_cid;
int max_valid_be_cid;

#define MAX_NUM_SS 300
#define MAX_TRANSPORT_CONNECTIONS 2000

//minimum value of any basic cid
#define BASIC_CID_MIN_VALUE 11

//maximum value of any basic cid (310)
#define BASIC_CID_MAX_VALUE (BASIC_CID_MIN_VALUE + MAX_NUM_SS - 1)

//minimum value of any primary cid (311)
#define PRIMARY_CID_MIN_VALUE (BASIC_CID_MAX_VALUE + 1)

//maximum value of any primary cid (610)
#define PRIMARY_CID_MAX_VALUE (PRIMARY_CID_MIN_VALUE +  MAX_NUM_SS - 1)

//minimum value of any secondary cid (611)
#define SECONDARY_CID_MIN_VALUE (PRIMARY_CID_MAX_VALUE + 1)

//maximum value of any secondary cid (910)
#define SECONDARY_CID_MAX_VALUE (SECONDARY_CID_MIN_VALUE + MAX_NUM_SS - 1)

//minimum value of any ugs cid (911)
#define UGS_CID_MIN_VALUE (SECONDARY_CID_MAX_VALUE + 1)

//maximum value of any ugs cid (2910)
#define UGS_CID_MAX_VALUE (UGS_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

//minimum value of any ugs cid (2911)
#define UL_UGS_CID_MIN_VALUE (UGS_CID_MAX_VALUE + 1)

//maximum value of any ugs cid (4910)
#define UL_UGS_CID_MAX_VALUE (UL_UGS_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

//minimum value of any ertps cid (4911)
#define ERTPS_CID_MIN_VALUE (UL_UGS_CID_MAX_VALUE + 1)

//maximum value of any ertps cid (6910)
#define ERTPS_CID_MAX_VALUE (ERTPS_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

//minimum value of any rtps cid (6911)
#define RTPS_CID_MIN_VALUE (ERTPS_CID_MAX_VALUE + 1)

//maximum value of any rtps cid (8910)
#define RTPS_CID_MAX_VALUE (RTPS_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

//minimum value of any nrtps cid (8911)
#define NRTPS_CID_MIN_VALUE (RTPS_CID_MAX_VALUE + 1)

//maximum value of any nrtps cid (10910)
#define NRTPS_CID_MAX_VALUE (NRTPS_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

//minimum value of any BE cid (10911)
#define BE_CID_MIN_VALUE (NRTPS_CID_MAX_VALUE + 1)

//maximum value of any BE cid (12910)
#define BE_CID_MAX_VALUE (BE_CID_MIN_VALUE + MAX_TRANSPORT_CONNECTIONS - 1)

// total number of cids in the system - since CID is a 16 bit value
// Upto MAX_CIDS - 2 (1300) are MM & transport connections, 
// MAX_CIDS - 1 (here, 1301) is broadcast
#define MAX_CIDS (BE_CID_MAX_VALUE + 2)

// As per standard, Table 529, broadcast cid is 0xFFFF (MAX_CIDS-1) where MAX_CIDS = 65536
// Since CID is a 16 bit value. currently MAX_CIDS defined to a smaller value
// to reduce memory and overheads
#define BROADCAST_CID  (MAX_CIDS-1)

// For OFDMA, all PDU's have GMH and CRC. not using 
//#define PDU_MIN_OVERHEAD (GENERIC_MAC_HEADER_LEN + MAC_CRC_LEN) to avoid
// repeated calculation in many places
/*If encrypt is enabled, there will be an extra overhead of Initialization vector, to be prepended to encrypted PDU and also an extra taglength, which is needed in AES encryption. This will cause PDU verhead to be (GENERIC_MAC_HEADER_LEN + MAC_CRC_LEN + IVLEN + TAGLEN)*/
#define PDU_MIN_OVERHEAD 10

// In a GMH, 11 bits are reserved for PDU length hence 
// max PDU length is 2^11-1 = 2047
#define MAX_PDU_LEN 2047

typedef enum {
	SERVICE_UGS = 0,
	SERVICE_ertPS = 1,
	SERVICE_rtPS = 2,
	SERVICE_nrtPS = 3,
	SERVICE_BE = 4} SchedulingType;

// This value is tied to the number of elements in the SchedulingType
#define NUM_SERVICE_CLASS 5

//This will ne moved to ARQ related header
#define ARQ_BLK_SIZE 0x040

// MCS related parameters
#define NUM_SUPPORTED_MCS 7
typedef enum { QPSK_12 = 0,
	       QPSK_34 = 1,
	       QAM16_12 = 2,
	       QAM16_34 = 3,
	       QAM64_12 = 4,
	       QAM64_23 = 5,
	       QAM64_34 = 6} ModulCodingType;

typedef enum { UL = 0, DL = 1} SfDirection;

typedef enum {Provisioned = 0, Admitted = 1, Active = 2} SfStatus;

/** add by zzb */
int init_mac_simulation_variables(int argc, char** argv);
int init_mac_core_variables();
int start_mac_threads();
int free_mac_simulation_variables();
int free_mac_core_variables();
/** end by zzb */

#endif
