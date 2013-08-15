/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ulmap.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ULMAP_H
#define _ULMAP_H

#define RANGING_UIUC 12

// table 366 extended uiuc code assignment for uiuc = 15

#define POWER_CONTROL_IE 0x00
#define AAS_UL_IE 0x02
#define CQICH_ALLOCATION_IE 0x03
#define UL_ZONE_IE 0x04
#define UL_MAP_PHYSICAL_MODIFIER_IE 0x05
#define UL_MAP_FAST_TRACKING_IE 0x07
#define UL_PUSC_BURST_ALLOCATION_IE 0x08
#define FAST_RANGING_IE 0x09
#define UL_ALLOCATION_START_IE 0x0a

// table 368 extended-2 uiuc code assignment for uiuc=11

#define CQICH_ENHANCED_ALLOCATION_IE 0x00
#define HO_ANCHOR_ACTIVE_UL_MAP_IE 0x01
#define HO_ACTIVE_ANCHOR_UL_MAP_IE 0x02
#define ANCHOR_BS_SWITCH_IE 0x03
#define UL_SOUNDING_COMMAND_IE 0x04
#define MIMO_UL_ENHANCED_IE 0x06
#define HARQ_UL_MAP_IE 0x07
#define HARQ_ACKCH_REGION_ALLOCATION_IE 0x08
#define MIMO_UL_BASIC_IE 0x09
#define MINI_SUBCHANNEL_ALLOCATION_IE 0x0a
#define AAS_SDMA_UL_IE 0x0e
#define FEEDBACK_POLLING_IE 0x0f
// UL-MAP related params

#define CDMA_ALLOC_UIUC 14

// Duration (in slots) allocated for sending a RNG_REQ on INIT_RNG_CID. Space
// for now should be enough to send a MMtype (1B) + rsv (1B) + ss_mac TLV (8B)
// + Ranging anomalies TLV (3B) + Serving BS ID TLV (8B) + rng purpose TLV (3B)
// + HO ID TLV (3B) + Req DBP TLV (3B) + Req DL rep coding TLV (3B)
#define CDMA_ALLOC_SIZE 8

// From Table 364: CID(16)+UIUC(4)+{6+4+2+4+8+8+7+1 = 40}bits
#define CDMA_ALLOC_IE_LEN_BITS 60

// Total bits needed for the fixed fields in a UL-MAP

// MM Type (8 bits) + Uplink Channel ID (8 bits) UCD count (8 bits) + 
// Allocation Start Time (32 bits)
// Ref: Table 18 in standard
#define UL_FIXED_FIELD_LENGTH 64

// variable requirement: The number of ULMAP IE's can vary depending on which 
// SS's are being serviced in a UL subframe.
// CID (16) + UIUC (4) + Duration (10) + Repetition (2)
// Ref: Table 275
// This can be larger when the slot offset is included for AAS or AMC
#define ULMAP_IE_LENGTH 32

// CID (16) + UIUC (4) + OFDMA Symbol offset(8) + Subchannel offset(7) 
// + No OFDMA Symbols (7) + No. Subchannels (7) + Ranging Method (2) + Dedicated ranging indicator(1)
#define RANGING_ULMAP_IE_LENGTH 52
// For computing length in MIMO UL Basic IE

// 1 (CSM indication) + 16 (CID_A) + 4(UIUC_A) + 2(Rep Coding_A) + 16 (CID_B) + 4(UIUC_B) + 2(Rep Coding_B) + 10 (Duration)
#define BURST_ATTR_CSM_LEN 55

// 1 (CSM indication) + 16 (CID) + 4(UIUC) + 2(Rep Coding) + 1 (MIMO control) + 10(Duration)
#define BURST_ATTR_STC_LEN 34

extern int ulmap_len_in_bytes;  
// Table 367
typedef struct{
    u_int8_t extended_uiuc;
    u_int8_t length;
    void* unspecified_data;
}extended_uiuc_ie;

typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t ofdma_symbol_num;
    u_int8_t subchannel_num;
    u_int8_t ranging_method;
    u_int8_t dedicated_ranging_indicator;
}uiuc12_ie;

// table papr reduction / safety zone/ sounding zone allocation ie
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t ofdma_symbol_num;	
    u_int8_t subchannel_num_szshift_value;
    u_int8_t paprreduc_safezone;
    u_int8_t sounding_zone;
    u_int8_t rsv;
}paprreduc_safezone_alloc_ie;

// Table 364 cdma allocation ie
typedef struct{
    u_int8_t duration;
    u_int8_t uiuc;
    u_int8_t repetition_coding_indication;
    u_int8_t frame_num_index;
    u_int8_t ranging_code;
    u_int8_t ranging_symbol;
    u_int8_t ranging_subchannel;
    u_int8_t bw_request_mandatory;
}cdma_alloc_ie;

//Table 373
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t ofdma_symbol_num;	
    u_int8_t subchannel_num;
    u_int8_t rsv;
}fastfeedback_alloc_ie;

typedef struct{
    u_int16_t duration;
    u_int8_t repetition_coding_indication;
    u_int16_t slot_offset;
}other_uiuc_ie;

typedef struct ulmapie ul_map_ie;

// Ref: Table 275 in standard
struct ulmapie{
    int ie_index;
    u_int16_t cid;
    u_int8_t uiuc; 
    extended_uiuc_ie* uiuc_extend_ie;
    uiuc12_ie* uiuc_12_ie;
    paprreduc_safezone_alloc_ie* uiuc_13_ie;
    cdma_alloc_ie* uiuc_14_ie;
    extended_uiuc_ie* uiuc_15_ie;
    fastfeedback_alloc_ie* uiuc_0_ie;
    other_uiuc_ie* uiuc_other_ie;
    // Link to the next element
    ul_map_ie* next;
};

typedef struct  {
    u_int8_t  manage_msg_type;
    u_int8_t  rsv;
    u_int8_t  ucd_count;
    u_int32_t alloc_start_time;
    u_int8_t num_ofdma_sym;
    u_int8_t  ulmap_ie_num;
    ul_map_ie*  ie;
}ul_map_msg;

//Table 369 ofdma power control ie
typedef struct{
    u_int8_t power_control; // signed integer expressing the change in power level (in 0.25 dB units)
    u_int8_t power_measure_frame;
}power_control_ie;

//Table 370 aas ul ie
typedef struct{
    u_int8_t permutation;
    u_int8_t ul_permbase;
    u_int8_t ofdma_symbol_offset;
    u_int8_t aas_zone_length;
    u_int8_t uplink_preamble_config;
    u_int8_t rsv;
}aas_ul_ie;

//Table 371 ofdma ul zone ie
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t permutation;
    u_int8_t ul_permbase;
    u_int8_t amc_type;
    u_int8_t use_all_sc_indicator;
    u_int8_t disable_subchannel_rotation;
    u_int8_t rsv;
}ul_zone_ie;

// Table 400
typedef struct{
    u_int32_t cqich_id; // variable size, the size is determined by the UCD
    u_int8_t allocation_offset;
    u_int8_t period;
    u_int8_t frame_offset;
    u_int8_t duration;
    u_int8_t report_config;
    u_int8_t feedback_type;
    u_int8_t report_type;
    u_int8_t cinr_preamble_report_type;
    u_int8_t zone_permutation;
    u_int8_t zone_type;
    u_int8_t zone_prbs_id;
    u_int8_t major_group_indication;
    u_int8_t pusc_major_group_bitmap;
    u_int8_t cinr_zone_measure_type;
    u_int8_t average_parameter_included;
    u_int8_t average_parameter;   
    u_int8_t mimo_permutation_feedback_cycle;
}cqich_alloc_ie;

// table 403 ofdma ul map physical modifier ie
typedef struct{
    u_int8_t preamble_modifier_type;
    u_int8_t preamble_frequency_shift_index;
    u_int8_t preamble_time_shift_index;
    u_int8_t pilot_pattern_modifier;
    u_int8_t pilot_pattern_index;
}ul_map_physical_modifier_ie;

// table 404 ul allocation start ie
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t rsv;
}ul_allocation_start_ie;

// table 406 ul pusc burst allocation in other segment ie
typedef struct{
    u_int8_t uiuc;
    u_int8_t segment;
    u_int8_t ul_permbase;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int16_t duration;
    u_int8_t repetition_coding_indication;
    u_int8_t rsv;
}ul_pusc_burst_alloc_ie;

// table 410 ofdma fast ranging ie
typedef struct{
    u_int8_t ho_id_indicator;
    u_int8_t rsv;
    u_int8_t ho_id;
    u_int64_t mac_address;
    u_int8_t uiuc;
    u_int16_t duration;
    u_int8_t repetition_coding_indication;
}fast_ranging_ie;

// table 411 ul map fast tracking ie
typedef struct indicaattri indica_attri;
struct indicaattri{
    u_int8_t power_correction;
    u_int8_t frequency_correction;
    u_int8_t time_correction;
    indica_attri* next;
};

typedef struct{
    u_int8_t map_index;
    u_int8_t rsv;
    indica_attri* indica_header;
}ul_map_fast_tracking_ie;

// extended-2 ie
//Table 372 mini-subchannel allocation ie
typedef struct minisubchanneldesc mini_subchannel_desc;
struct mini_subchannel_desc{
    u_int16_t cid;
    u_int8_t uiuc;
    u_int8_t repetition;
    mini_subchannel_desc* next;
};

typedef struct{
    u_int8_t ctype; // define the number of minisubchannel
    u_int8_t duration; // in ODDMA slots
    mini_subchannel_desc* sch_header;
}mini_subchannel_allocation_ie;

typedef struct assignedburstattri assigned_burst_attri;
struct assignedburstattri{
    u_int8_t collaborative_sm_indication;
    u_int16_t duration;
    u_int16_t cid;
    u_int8_t uiuc;
    u_int8_t repetition_coding_indication;
    u_int8_t mimo_control;
    u_int16_t cid_a;
    u_int8_t uiuc_a;
    u_int8_t repetition_coding_indication_a;
    u_int16_t cid_b;
    u_int8_t uiuc_b;
    u_int8_t repetition_coding_indication_b;
    assigned_burst_attri * next;
};

//Table 398 mimo ul basic ue
typedef struct{
    u_int8_t num_assign;
    assigned_burst_attri* assigned_burst_header;
}mimo_ul_basic_ie;

typedef struct cqichattri cqich_attri;
struct cqichattri{
    u_int8_t feedback_type;
    u_int8_t allocation_index;
    u_int8_t cqich_type;
    u_int8_t sttd_indication;
    cqich_attri* next;
};

typedef struct{
    u_int32_t cqich_id;
    u_int8_t period;
    u_int8_t frame_offset;
    u_int8_t duration;
    u_int8_t cqich_num;
    cqich_attri* cqich_header;
    u_int8_t band_amc_precoding_mode;
    u_int8_t nr_precoders_feedback;
}cqich_enhanced_alloc_ie;

// table 407 ho anchor active ul map ie
typedef struct hoanchoractiveulmapie ho_anchor_active_ul_map_ie;
struct hoanchoractiveulmapie{
    u_int16_t anchor_preamble;
    u_int16_t anchor_cid;
    u_int16_t start_subchannel_offset;
    u_int8_t uiuc;
    u_int16_t duration;
    u_int8_t repetition_coding_indication;
    ho_anchor_active_ul_map_ie* next;
};

// table 409 mimo ul enhanced ie
typedef struct cidattri cid_attri;
struct cidattri{
    u_int16_t cid;
    u_int8_t uiuc;
    u_int8_t repetition_coding_indication;
    u_int8_t matrix_indicator;
    u_int8_t pilot_pattern_indicator;
    cid_attri* next;
};

typedef struct burstattri burst_attri;
struct burstattri{
    u_int8_t num_cid;
    cid_attri* cid_header;
    u_int16_t duration;
    burst_attri* next;
};

typedef struct{
    u_int8_t num_assign;
    burst_attri* bust_header;
}mimo_ul_enhanced_ie;

// table 412 anchor bs switch ie
typedef struct anchorattri anchor_attri;
struct anchor_attri{
    u_int16_t reduced_cid;
    u_int8_t action_code;
    u_int8_t action_time;
    u_int8_t temp_bs_id;
    u_int8_t rsv;
    u_int8_t ack_change_indicator;
    u_int8_t cqich_allocation_indicator;
    u_int32_t cqich_id;
    u_int8_t feedback_channel_offset;
    u_int8_t period;
    u_int8_t frame_offset;
    u_int8_t duration;
    u_int8_t mimo_permutation_feedback_cycle;
    anchor_attri* next;
};
typedef struct{
    u_int8_t n_anchor_bs_switch;
    anchor_attri* anchor_header;
    u_int8_t rsv;
}anchor_bs_switch_ie;

// table 434 harq ackch region allocation ie

typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t ofdma_symbols_num;
    u_int8_t subchannels_num;
}harq_ackch_region_alloc_ie;

typedef struct{

}ul_sounding_command_ie;

typedef struct{

}aas_sdma_ul_ie;

// table 425 feedback polling ie
typedef struct channelattri channel_attri;
struct channelattri{
    u_int16_t basic_cid;
    u_int8_t allocation_duration;
    u_int8_t feedback_type;
    u_int8_t frame_offset;
    u_int8_t period;
    u_int8_t uiuc;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t duration;
    u_int8_t repetition_coding_indication;
    channel_attri* next;
};

typedef struct{
    u_int8_t num_allocations;
    u_int8_t dedicated_ul_allocation_included;
    u_int8_t rsv;
    channel_attri* channel_header;
}feedback_polling_ie;

#endif
