/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ucd.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _UCD_H_
#define _UCD_H_

#include "mac.h"
#include "mac_config.h"
#include "mac_amc.h"

#define DEFAULT_UIUC 1
#define UCD_BURST_PROFILE_LENGTH 7 // fixed for OFDMA: 1 (for UIUC) + 3 (FEC code type tlv8) + 3 (Ranging data ratio tlv8): see table 357 in 802.16e-2005

typedef struct ulburstprofile ul_burst_profile;

struct ulburstprofile{
    u_int8_t type;
    u_int8_t length;
    u_int8_t rsv;
    u_int8_t coding_type;
    u_int8_t uiuc;
    tlv8 fec_code_modulation;
    tlv8 ranging_data_ratio;
    ul_burst_profile* next;
};

// UL Channel Descriptor message, be transmitted by BS at periodical interval to define the characteristic
// of an UL physical channel
typedef struct ucdmsg ucd_msg;

struct ucdmsg{
    u_int8_t management_message_type; // set to 0
    u_int8_t configuration_change_count; // incremented by one (modulo 256)
    u_int8_t ranging_backoff_start;
    u_int8_t ranging_backoff_end;
    u_int8_t request_backoff_start;
    u_int8_t request_backoff_end;
    tlv8 contention_based_reservation_timeout;
    tlv32 frequency; // UL center frequence
    tlv8 ho_ranging_start;
    tlv8 ho_ranging_end;
    tlv8 available_ul_radio_resources; // indicate the average ratio of non-assigned UL radio to the total usable UL radio resources
    tlv8 initial_ranging_codes; // number of initial ranging CDMA codes, possible val-ues are 0-255
    tlv8 periodic_ranging_codes; // number of periodic ranging CDMA codes
    tlv8 bandwidth_request_codes;
    tlv8 periodic_ranging_backoff_start;
    tlv8 periodic_ranging_backoff_end;
    tlv8 startof_ranging_codes_group; // indicate the starting number
    tlv8 permutation_base;
    tlv72 ul_allocated_subchannels_bitmap;
    tlv104 permutation_ul_allocated_subchannel_bitmap;
    tlv8 band_amc_allocation_threshold;
    tlv8 band_amc_release_threshold;
    tlv8 band_amc_allocation_timer;
    tlv8 band_amc_release_timer;
    tlv8 band_status_reporting_maxperiod;
    tlv8 band_amc_retry_timer;
    tlv8 safety_channel_allocation_threshold;
    tlv8 safety_channel_release_threshold;
    tlv8 safety_channel_allocation_timer;
    tlv8 safety_channel_release_timer;
    tlv8 bin_status_reporting_maxperiod;
    tlv8 safety_channel_retry_timer;
    tlv8 harq_ack_delayfor_dlburst;
    tlv8 cqich_band_amc_transition_delay;
    tlv48 ul_amc_allocated_phyband_bitmap;
    tlv8 maximum_retransmission;
    tlv64 normalized_cn_override;
    tlv8 sizeof_cqich_id_field;
    tlv64 normalzied_cn_override2;
    tlv8 band_amc_entry_average_cinr;
    tlv8 upperbound_aas_preamble;
    tlv8 lowerbound_aas_preamble;
    tlv8 allow_aas_beam_select_message;
    tlv8 use_cqich_indication_flag;
    tlv8 ms_specific_uppower_offset_adjustment_step;
    tlv8 ms_specific_downpower_offset_adjustment_step;
    tlv8 minimum_level_power_offset_adjustment;
    tlv8 maximum_level_power_offset_adjustment;
    tlv8 handover_ranging_codes;
    tlv8 initial_ranging_interval;
    tlv24 tx_power_report;
    tlv8 normalized_cn_for_channel_coding;
    tlv8 initial_ranging_backoff_start;
    tlv8 initial_ranging_backoff_end;
    tlv8 bandwidth_request_backoff_start;
    tlv8 bandwidth_request_backoff_end;
    tlv8 uplink_burst_profile_for_mutlifec;
    tlv8 ul_pusc_subchannel_rotation;
    tlv8 relative_power_offset_for_ulharqburst;
    tlv8 relative_power_offset_for_ulburst;
    tlv40 fast_feedback_region;
    tlv32 harq_ack_region;
    tlv40 ranging_region[4];
    tlv40 sounding_region[2];
    tlv8 ms_transmit_power_limitation_level;
    tlv8 ul_initial_transmit_timing;
    ul_burst_profile* profile_header;
};


extern pthread_t ucd_thd;
extern volatile int ulmap_ccc;

extern void ucd_handler_init();
extern void *ucd_gen(void *parm);
extern void sigusr1_handler();
extern int chan_desc_msleep(unsigned long milisec);
extern int amc_ucd_update(amc_info* amclist);
extern ucd_msg* ucd_msg_query(int query_ucd_ccc);


extern int dummy_ucd_amc();
extern void* dummy_amc_gen(void* param);
ModulCodingType get_mcs_from_uiuc(ucd_msg *ucd, short uiuc);

ucd_msg* stored_ucd[CONFIG_COUNT];



#endif
