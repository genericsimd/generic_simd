/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dlmap.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DLMAP_H
#define _DLMAP_H

#include "mac_config.h"

// DL-MAP related params

// Total bits needed for the fixed fields in a DL-MAP
// MM Type (8 bits) + DCD count (8 bits) + BSID (48 bits) + 
// PHY synch field (32 bits for OFDMA) + #OFDMA symbols (8)
// Ref: Table 16 and Table 273 in standard
#define DL_FIXED_FIELD_LENGTH 104

// variable requirement: The number of DLMAP IE's can vary depending on which 
// SS's are being serviced in a DL subframe and some special IEs inserted.

// This is the size expected for a normal (data burst) DLMAP IE
// In the CID list, we include only the basic CID of the SS the burst is meant 
// for, instead of all the transport CIDs whose PDUs are present in the burst 
// We assume only one SS per burst, hence N_CID = 1
// Under these assumptions, each DLMAP IE's length is defined below
// DIUC (4) + Symbol offset (8) + subchannel offset (6) + boosting (3) + 
// Num OFDMA symbols (7) + Num subchannels (6) + Repetition Coding (2) +
// N_CID (8) + CID (16). Ref: Table 275

#define DLMAP_IE_LENGTH 60

// 4 (DIUC) + 4 (Ext DIUC) + 4 (Length) + 32 (Unspecified data field)
#define STC_ZONE_IE_LEN 44

// 4 (DIUC) + 4 (Ext DIUC) + 4 (Length) + 0 (Unspecified data field)
#define CID_SWITCH_IE_LEN 12

// 4 (DIUC) + 4 (Ext DIUC) + 8 (Length)
#define MIMO_DL_BASIC_IE_COMMON_LEN 16

#define NUM_ASSIGN_LEN 4

#define DIUC_FOR_EXTENDED_DLMAP_IE 15

// table 323 extended diuc code assignment for diuc=15
#define CHANNEL_MEASUREMENT_IE 0x00
#define STC_ZONE_IE 0x01
#define AAS_DL_IE 0x02
#define DATA_LOCATION_IN_ANOTHER_BS_IE 0x03
#define CID_SWITCH_IE 0x04
#define HARQ_MAP_POINTER_IE 0x07
#define PHYMOD_DL_IE 0x08
#define BROADCAST_CONTROL_POINTER_IE 0x0a
#define DL_PUSC_BURST_IN_OTHER_SEGMENT_IE 0x0b
#define PUSC_ASCA_ALLOC_IE 0x0c
#define UL_INTERFERENCE_NOISE_LEVEL_IE 0x0f

// table 325, extended-2 diuc code assignment for diuc=14
#define MBS_MAP_IE 0x00
#define HO_ANCHOR_ACTIVE_DL_MAP_IE 0x01
#define HO_ACTIVE_ANCHOR_DL_MAP_IE 0x02
#define HO_CID_TRANSLATION_MAP_IE 0x03
#define MIMO_IN_ANOTHER_BS_IE 0x04
#define MACRO_MIMO_DL_BASIC_IE 0x05
#define SKIP_IE 0x06
#define HARQ_DL_MAP_IE 0x07
#define HARQ_ACK_IE 0x08
#define ENHANCED_DL_MAP_IE 0x09
#define CLOSED_LOOP_MIMO_DL_ENHANCED_IE 0x0a
#define MIMO_DL_BASIC_IE 0x0b
#define MIMO_IE 0x0c
#define AAS_SDMA_DL_IE 0x0e

// These paramaters should be consistent with the PHY values used
#define DL_PERMBASE 3
#define PRBS_ID 2

typedef enum {MATRIX_A=0, MATRIX_B, MATRIX_C} stc_matrix_type;

// table 322 and table 324 DL-MAP extended IE and extended-2 IE
typedef struct{
    u_int8_t extended_diuc;
    u_int8_t length;
    void* unspecified_data;
}extended_diuc_ie;

// table 328 ofdma channel measurement ie, issued by the Bs to request a channel measurement report
typedef struct{
    u_int8_t channel_nr;
    u_int8_t ofdma_symbol_offset;
    u_int16_t cid;
}channel_measurement_ie;

// table 327 ofdma stc dl zone ie 
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t permutation;
    u_int8_t use_all_sc_indicator;
    u_int8_t stc;
    u_int8_t matrix_indicator;
    u_int8_t dl_permbase;
    u_int8_t prbs_id;
    u_int8_t amc_type;
    u_int8_t midamble_presence;
    u_int8_t midamble_boosting;
    u_int8_t num_antenna_select; //0: 2 antennas, 1: 3 antennas for STC
    u_int8_t dedicated_pilots;
}stc_dl_zone_ie;

// table 326 ofdma aas dl ie
typedef struct{
    u_int8_t ofdma_symbol_offset;
    u_int8_t permutation;
    u_int8_t dl_permbase;
    u_int8_t downlink_preamble_config;
    u_int8_t preamble_type;
    u_int8_t prbs_id;
    u_int8_t diversity_map;
    u_int8_t rsv;
}aas_dl_ie;

// table 329 ofdma data location in another bs ie
typedef struct{
    u_int8_t segment;
    u_int8_t used_subchannels;
    u_int8_t diuc;
    u_int8_t frame_advance;
    u_int8_t rsv;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t boosting;
    u_int8_t preamble_index;
    u_int8_t ofdm_symbols_num;
    u_int8_t subchannels_num;
    u_int8_t repetition_coding_indication;
    u_int16_t cid;
}data_location_in_another_bs_ie;

// table 334 harq and submap pointer ie 
typedef struct{
    u_int8_t diuc;
    u_int8_t slots_num;
    u_int8_t repetition_coding_indication;
    u_int8_t map_version;
    u_int8_t idle_users;
    u_int8_t sleep_users;
    u_int8_t cid_mask_length;
    u_int64_t cid_mask;
}harq_map_pointer_ie;

// table 335 ofdma dl-map physical modifier ie
typedef struct{
    u_int8_t preamble_modifier_type;
    u_int8_t preamble_frequency_shift_index;
    u_int8_t preamble_time_shift_index;
    u_int8_t pilot_pattern_modifier;
    u_int8_t pilot_pattern_index;
}phymod_dl_ie;

// table 358 broadcast control pointer ie
typedef struct{
    u_int8_t dcd_ucd_transmittion_frame;
    u_int8_t skip_broadcast_system_update;
    u_int8_t broadcast_system_update_type;
    u_int8_t broadcast_system_update_transmission_frame;
}broadcast_control_pointer_ie;

//table 336 dl pusc burst allocation in other segment ie
typedef struct{
    u_int16_t cid;
    u_int8_t diuc;
    u_int8_t segment;
    u_int8_t boosting;
    u_int8_t idcell;
    u_int8_t dl_permbase;
    u_int8_t prbs_id;
    u_int8_t repetition_coding_indication;
    u_int8_t used_subchannels;
    u_int8_t ofdma_symbol_offset;
    u_int8_t rsv1;
    u_int8_t ofdma_symbols_num;
    u_int8_t subchannel_offset;
    u_int8_t subchannels_num;
    u_int8_t rsv2;
}dl_pusc_burst_in_other_segment_ie;

// table 360 pusc asca allocation ie
typedef struct{
    u_int8_t diuc;
    u_int16_t short_basic_cid;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t ofdma_symbols_num;
    u_int8_t subchannels_num;
    u_int8_t repetition_coding_info;
    u_int8_t permutation_id;
    u_int8_t rsv;
}pusc_asca_allocation_ie;

// table 342 ul interference and noise level ie
typedef struct{
    u_int8_t bitmap;
    u_int8_t cqi_ack_periodic_ranging_region_ni;
    u_int8_t pusc_region_ni;
    u_int8_t optional_pusc_region_ni;
    u_int8_t amc_region_ni;
    u_int8_t aas_region_ni;
    u_int8_t periodic_ranging_region_ni;
    u_int8_t sounding_region_ni;
    u_int8_t mimo_region_ni;
}ul_interference_noise_level_ie;

// begin the extended 2 diuc ie
// mbs map ie , see 8.4.5.3.12

typedef struct{
    u_int8_t macro_diversity_enhanced;
    u_int8_t next_mbs_map_ie_frame_offset;
}mbs_map_ie;

// table 338 ho active anchor dl map ie
typedef struct{
    u_int8_t active_preamble;
    u_int16_t anchor_cid;
    u_int8_t diuc;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t repetition_coding_indication;
    u_int8_t ofdma_symbols_num;
    u_int8_t subchannels_num;
}ho_active_anchor_ie;

// table 337 ho anchor active dl map ie
typedef struct{
    u_int8_t anchor_preamble;
    u_int16_t anchor_cid;
    u_int8_t diuc;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t repetition_coding_indication;
}ho_anchor_active_ie;


// table 339 ho cid translation map id
typedef struct{
    u_int8_t anchor_preamble;
    u_int16_t anchor_cid;
    u_int16_t active_cid;
}ho_cid_translation_map_ie;


// table 344 rcid ie
typedef struct{
    u_int16_t cid;
    u_int8_t prefix;
    u_int16_t rcid_11;
    u_int8_t rcid_7;
    u_int8_t rcid_3;
}rcid_ie;

// table 340 mimo in another bs ie
// 16: CID, 2: Layer index, 4: DIUC, 2: Rep_coding_ind
#define LAYER_ATTRI_SIZE 24

typedef struct layerattri layer_attri;
struct layerattri{
    u_int16_t cid;
    u_int8_t layer_index;
    u_int8_t diuc;
    u_int8_t repetition_coding_indication;
    layer_attri* next;
};

typedef struct userattri user_attri;
struct userattri{
    rcid_ie* rcidie;
    u_int8_t encoding_mode;
    u_int8_t cqich_allocation;
    u_int8_t ackch_allocation;
    u_int8_t pilot_pattern_modifier;
    u_int8_t preamble_modifier_index;
    u_int8_t pilot_pattern;
    u_int8_t rsv1;
    u_int8_t diuc;
    u_int8_t repetition_coding_indication;
    u_int8_t ack_ch_index;
    u_int8_t rsv2;
    u_int8_t acid;
    u_int8_t ai_sn;
    u_int8_t nep;
    u_int8_t nsch;
    u_int8_t spid;
    u_int8_t allocation_index;
    u_int8_t period;
    u_int8_t frame_offset;
    u_int8_t duration;
    user_attri* next;
};

typedef struct msattri ms_attri;
struct msattri{
    rcid_ie* rcidie;
    u_int8_t diuc;
    u_int8_t repetition_coding_indication;
    u_int8_t num_stream;
    u_int8_t antenna_selection_index;
    u_int8_t rsv;
    ms_attri* next;
};

// Size of the part excluding the layer attributes
// 8: OFDMA symbol offset, 6: Subchannel offset, 3: Boosting, 7: Num OFDMA symbols, 6: NUm subchannels, 2: Matrix Ind, 2: Num layers, 2: Rsvd
#define REGION_ATTRI_SIZE 36
typedef struct regionattri region_attri;
struct regionattri{
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t boosting;
    u_int8_t ofdma_symbols_num;
    u_int8_t subchannels_num;
    u_int8_t packet_index;
    u_int8_t matrix_indicator;
    u_int8_t num_layer;
    layer_attri* layer_header;
    u_int8_t number_of_users;
    user_attri* user_header;
    // belong to the closed-loop mimo dl enhanced ie
    rcid_ie* rcidie;
    u_int8_t diuc;
    u_int8_t repetition_coding_indication;
    u_int8_t antenna_grouping_index;
    u_int8_t num_stream;
    u_int8_t codebook_precoding_index;
    u_int8_t num_ms;
    ms_attri* ms_header;
    region_attri *next;
};

typedef struct{
    u_int8_t num_region;
    region_attri* region_header;
}mimo_dl_basic_ie;

typedef struct{
    u_int8_t segment;
    u_int8_t used_subchannels;
    u_int8_t idcell;
    u_int8_t num_region;
    region_attri* region_header;
}mimo_in_another_bs_ie;

// table 341 macro mimo dl basic ie
typedef struct{
    u_int8_t segment;
    u_int8_t used_subchannels;
    u_int8_t num_regions;
    region_attri* region_header;
}macro_mimo_dl_basic_ie;


// table 345 skip ie
typedef struct{
    u_int8_t mode;
    u_int8_t rsv;
}skip_ie;

// table 355 harq ack ie
typedef struct{
    u_char* bitmap;
}harq_ack_ie;

// table 356 enhanced dl map ie
typedef struct assignmentattri assignment_attri;
struct assignmentattri{
    u_int8_t n_cid;
    u_int16_t * cid;
    u_int8_t diuc;
    u_int8_t boosting;
    u_int8_t repetition_coding_indication;
    u_int8_t region_id;
    u_int8_t rsv;
    assignment_attri * next;
};

typedef struct{
    u_int8_t num_assignment;
    assignment_attri * assignment_header;
}enhanced_dl_map_ie;

// table 357 closedloop mimo dl enhanced ie 
typedef struct{
    u_int8_t num_region;
    region_attri* region_header;
}closed_loop_mimo_dl_enhanced_ie;

// table 359 aas sdma dl ie 
typedef struct{
    u_int8_t rcid_type;
    u_int8_t num_burst_region;
    u_int8_t rsv;
    region_attri * region_header;
}aas_sdma_dl_ie;


typedef struct{
    u_int8_t n_cid; // number of CIDs assigned for this IE
    u_int16_t * cid;
    u_int8_t ofdma_symbol_offset;
    u_int8_t subchannel_offset;
    u_int8_t boosting;
    u_int8_t ofdma_triple_symbol_num;
    u_int8_t ofdma_Symbols_num;
    u_int8_t subchannels_num;
    u_int8_t repetition_coding_indication;
}normal_diuc_ie;

// table 320
typedef struct dlmapie dl_map_ie;
struct  dlmapie{ 
    int ie_index;
    u_int8_t diuc;
    extended_diuc_ie* extended_ie;
    normal_diuc_ie* normal_ie;
    dl_map_ie* next;
};

typedef struct  {
    u_int8_t  manage_msg_type;
    u_int8_t frame_duration_code;
    u_int32_t frame_number;
    u_int8_t  dcd_count;
    u_int64_t  bs_id;
    u_int8_t  ofdma_symbols_num;
    dl_map_ie*  ie_head;
}dl_map_msg;

#endif
