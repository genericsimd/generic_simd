/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2010, 2011

   All Rights Reserved.

   File Name: ranging_mm.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-July.2010		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _RANGING_MM_H_
#define _RANGING_MM_H_

#include "util.h"
#include "mac.h"
#include "mac_connection.h"
#include "mac_config.h"
#include "mac_amc.h"
#include "app_timer.h"
#include "ulmap.h"
#include "dll_ordered_list.h"
#include "constants.h"
#include "memmgmt.h"

#ifdef SYS_POWER_COMPILE
#define RNG_REVS(n) (((unsigned short)((n) & 0xff)) << 8 | (((n) & 0xff00) >> 8))
#define RNG_REVL(n) ((((unsigned int)((n) & 0xff)) << 24) | ((unsigned int)((n) & 0xff00)) << 8 | ((unsigned int)((n) & 0xff0000)>>  8 | ((unsigned int)((n)&0xff000000)>>24)))
#else
#define RNG_REVS(n) (n)
#define RNG_REVL(n) (n) 
#endif

extern dll_ordered_list* g_timer_list;
extern pthread_mutex_t ranging_mutex;

// Acc to Wimax Spec Rev2/D5, Sec 8.4.7.3, all the ranging codes used will be
// between S and (S+O+M+N+L)mod 256. First N codes are for IR, next M codes 
// for PR, next L codes for BR, next O codes for HO
#if 1
#define RANGING_CODE_S 0
#define RANGING_CODE_N 4
#define RANGING_CODE_M 9
#define RANGING_CODE_L 4
#define RANGING_CODE_O 4
#define NUM_RANGING_CODES 256
#define PR_REGION_START_SYMBOL 2
#else
#define RANGING_CODE_S 0
#define RANGING_CODE_N 4
#define RANGING_CODE_M 5
#define RANGING_CODE_L 4
#define RANGING_CODE_O 4
#define NUM_RANGING_CODES 256
#define PR_REGION_START_SYMBOL 2
#endif

// Ranging backoff window - min & max values. Note this is actual value, not power of two
#define RNG_BACKOFF_WMIN 4
#define RNG_BACKOFF_WMAX 8

// Ref Table 372 (ULMAP-IE format) in Wimax spec Rev2/D5
#define IR_METHOD 0
#define PR_METHOD 2

#if (IR_METHOD==0)
#define NUM_IR_OPPS_PERFRAME ((NUM_IR_SYMBOLS/2)*(NUM_RANGING_SUBCHANNELS/6))
#endif

#if (PR_METHOD==2)
#define NUM_PR_OPPS_PERFRAME (NUM_PR_SYMBOLS*(NUM_RANGING_SUBCHANNELS/6))
#endif

#define POWER_THRESH 12
#define FREQUENCY_THRESH 1
#define TIMING_THRESH 5

#define RNG_CONTINUE 1
#define RNG_ABORT 2
#define RNG_SUCCESS 3

#define NO_RANGING 0
#define INIT_RANGING 1
#define START_PERIODIC_RANGING 2
#define PERIODIC_RANGING 3

#define RANGING_ADJUST 1
#define RNG_REQ_IR_CID 2

// Actions for the MS Tx side
#define DO_NOTHING 0
#define SEND_CDMA_CODE 1
#define INIT_RNG_REQ 2
#define RESCAN 3
#define BASIC_CID_READY 4

// Actions for the MS Rx side
#define START_INIT_RANGING 100
#define ADJUST_FAILED 101

#pragma pack(push)
#pragma pack(1)
extern unsigned char param_MY_MAC[6];
extern short ranging_type;

typedef struct{
cdma_alloc_ie *cdma_ie;
int ucd_count;
}cdma_ranging_ie;

typedef struct{
	u_int8_t management_message_type; 
	u_int8_t rsv; // set to 0

	tlv8 dl_burst_profile;	
	tlv48 ss_mac;
	tlv8 ranging_anomalies;
	tlv8 aas_broadcast_capability;	
	tlv48 serving_bs_id;
	tlv8 rng_purpose_indication;
	tlv8 ho_id;	
	tlv8 power_down_indicator;
	tlv8 dl_rep_coding_level;
	tlv16 cmac_key_count;
}rng_req_msg;

// In bytes - calculated from the sum of TLV sizes below + 2 bytes 
// (1 each for type and length) for each TLV = 1+1+(2+4)+(2+1)+(2+4)
// +(2+1)+(2+4)+(2+1)+(2+2)+(2+3)+(2+2)+(2+2)+(2+1)+(2+3)+(2+1)+(2+1)
// +(2+1)+(2+2)+(2+1)+(2+1)+(2+2)+(2+1)+(2+1)+(2+1)+(2+4) = 92
#define MAX_RNG_RSP_LEN 92

// TLV values are from Section 11.6, Table 552
typedef struct{
	u_int8_t management_message_type; 
	u_int8_t rsv; // set to 0
	
	tlv_s32 timing_adjust;
	tlv_s8 power_adjust;
	tlv_s32 frequency_adjust;
	tlv8 ranging_status; // 1: continue 2: abort 3: success
	tlv32 dl_frequency_override;
	tlv8 ul_channel_id_override;
	tlv16 dl_burst_profile;
	tlv48 ss_mac;
	tlv16 basic_cid;
	tlv16 primary_cid;
	tlv8 aas_broadcast_perm;
	tlv24 frame_number;
	tlv8 init_ranging_opp_num;
	tlv8 service_level_prediction;
	tlv8 resource_retain_flag;
	tlv16 ho_process_optimization;
	tlv8 ho_id;
	tlv8 location_update_response;
	tlv16 dl_burst_profile_ofdma;
	tlv8 rendezvous_time;
	tlv8 cdma_code;
	tlv8 tx_opp_offset;

#ifdef _OFDM_
	tlv8 ranging_subchannel;
#endif	
#ifdef _OFDMA_
	tlv32 ranging_code_attributes;
#endif

}rng_rsp_msg;

typedef struct rangingie ranging_adjust_ie;
struct rangingie{
	int32_t timing_adjust; 
	int8_t power_adjust;
	int32_t frequency_adjust;

	// TODO: this info will suffice for CDMA based contention ranging and 
	// periodic ranging on BASIC_CID. No corrections sent for invited ranging
	// on INIT_RNG_CID. 
	u_int8_t frame_num_index;
   	u_int8_t ranging_code;
   	u_int8_t ranging_symbol;
   	u_int8_t ranging_subchannel;

	//u_int16_t basic_cid;
	ranging_adjust_ie *p_next;
};

typedef struct{
	int8_t power_adjust;
   	u_int8_t ranging_code;
   	u_int8_t ranging_symbol;
   	u_int8_t ranging_subchannel;
}ranging_tx_info;

typedef struct{
	// set by PHY/adapter when DL and UL synch at MS Rx is established and 
	// MAC state machine needs to be started (or restarted): TRUE or FALSE
	int mac_ranging_start_flag; 	

	// At the BS Rx, PHY/adapter will give a list of power, freq, timing 
	// adjustments for different SS (identified by ranging code attributes)
	// At the SS Tx, MAC will pass any adjustment values received in RNG_RSP
	// messages to PHY. In this case, num_ranging_adjust_ie = 1
	ranging_adjust_ie *p_ranging_ie_list; 	
	
	// Number of elements in the above ranging_ie_list
	int num_ranging_adjust_ie; 		
	
	// Sent by MAC for MS Tx. 0: do nothing, 1: send CDMA code with attributes
	// specified in p_rng_tx_info, 2: Send RNG_REQ on INIT_RNG_CID 3:rescan
	int ms_ranging_flag;		
	ranging_tx_info *p_rng_tx_info;
	u_int16_t basic_cid;
}ranging_info;

int init_rng_rsp_msg(rng_rsp_msg *rng_rsp, ranging_adjust_ie *p_rng_adjust);
int init_tx_rng_req_msg(rng_req_msg *rng_req);
int init_rx_rng_req_msg(rng_req_msg *rng_req);
int build_rng_rsp(u_char **p_payload, int *mm_len, rng_rsp_msg *rng_rsp);
int build_rng_req(u_char **p_payload, int *mm_len, int num_allotted_bytes);
int parse_rng_req(u_char *payload, int mm_len, rng_req_msg **rng_req_mm);
int parse_rng_rsp(u_char *payload, int mm_len, rng_rsp_msg **rng_rsp_mm);
int free_ranging_info(ranging_info *p_ranging_info);
int release_ss_ranging_sm();

#pragma pack(pop)
#endif

