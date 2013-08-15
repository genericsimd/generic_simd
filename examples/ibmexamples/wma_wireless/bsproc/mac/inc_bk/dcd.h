/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dcd.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DCD_H_
#define _DCD_H_

#include "mac_config.h"
#include "mac_amc.h"

#define DEFAULT_DIUC 0
#define DEFAULT_DLMAP_CCC 0
#define DCD_BURST_PROFILE_LENGTH 4 // fixed for OFDMA: 1 (for UIUC) + 3 (FEC code type tlv8)

// DL Channel descriptor message, this message shall always be transmitted by BS at a periodical interval
// to define the characteristic of a DL Physical channel, transmitted on a DL burst described by 
// a DL-MAP IE with DIUC=0
typedef struct dlburstprofile dl_burst_profile;

struct dlburstprofile{
    u_int8_t type;
    u_int8_t length;
    u_int8_t rsv;
    u_int8_t coding_type;
    u_int8_t diuc;
    tlv8 fec_code_modulation;
    dl_burst_profile* next;
};

typedef struct dcdmsg dcd_msg;

struct dcdmsg{
    u_int8_t management_message_type; // set to 1
    u_int8_t rsv; // set to zero
    u_int8_t configuration_change_count; // incremented by one (modulo 256)
    tlv16 bs_eirp;
    tlv8 channel_nr;
    tlv16 ttg;
    tlv8 rtg;
    tlv16 eirx_power;
    tlv24 channel_switch_frame_number;
    tlv32 frequency;
    tlv48 bs_id;
    tlv8 harq_ack_delay_for_ulburst;
    tlv8 mac_version;
    tlv8 permutation_type_for_broadcast_in_harq;
    tlv8 maximum_retransmission;
    tlv8 default_rssi_cinr;
    tlv48 dl_mac_allocated_phyband_bitmap;
    tlv8 available_dl_ratio_resources;
    tlv8 ho_type_support;
    tlv8 h_add_threshold;
    tlv8 h_delete_threshold;
    tlv8 asr_slot_length_switching_period;
    tlv8 hysteresis_margin;
    tlv8 time_to_trigger_duration;
    tlv8 mih_capability_support;
    tlv8 nsp_change_count_tlv;
    tlv8 cell_type_tlv;
    tlv8 mbs_zone_identifier_list; // varialbe in length
    tlv16 default_ho_rssi_cinr;
    tlv8 dl_burst_profile_for_multifec;
    tlv8 bs_restart_count;
    dl_burst_profile* profile_header;
}   ;


extern pthread_t dcd_thd;
extern volatile int dlmap_ccc;

extern void dcd_handler_init();
extern void *dcd_gen(void *parm);
//extern void sigusr1_handler();  // only one SIGUSR1 handler for both ucd and dcd thd
//extern int chan_desc_msleep(unsigned long milisec);
extern int amc_dcd_update(amc_info* amclist);
extern dcd_msg* dcd_msg_query(int query_dcd_ccc);

//extern int dummy_dcd_amc();
//extern void* dummy_amc_gen(void* param);


// these variables need to be global for sigusr1_handler in ucd_handler.c
extern int dcd_sleep_dur;
extern int t_dcd_ccc;
extern int dcd_ccc;
extern int dcd_trans;
extern int param_DCD_TRANSITION;
extern int param_DCD_INTERVAL;
extern int dcd_flag;
 
dcd_msg* stored_dcd[CONFIG_COUNT];

#endif
