/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_map_align.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   4-May.2009       Created                                     Zhenbo Zhu 

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __UL_MAP_ALIGN_H
#define __UL_MAP_ALIGN_H

#include "ulmap.h"
#include "mac_config.h"

#define MAX_IE_LENGTH 1024

/** memory align data structure define */

//#define __LITTLE_ENDIAN

#pragma pack(push)
#pragma pack(1)

/* Standard 6.3.2.3.4, Table 42 */
struct ul_map_msg_align
{
        u_int8_t mgt_msg_t;
#ifdef __BIG_ENDIAN__
        u_int8_t fdd_change_flg :1;
        u_int8_t rsv :7;
#else
        u_int8_t rsv :7;
        u_int8_t fdd_change_flg :1;
#endif
        u_int8_t ucd_count;
        u_int32_t alloc_start_time;
        // OFDMA only
        u_int8_t ofdma_symbols_num;
};

struct ul_ext_2_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t ext_2_uiuc :4;
#else
        u_int8_t ext_2_uiuc :4;
        u_int8_t uiuc :4;
#endif
        u_int8_t len;
        /* with unspecified data following and padding*/
};

// Currently this structure is defined assuming that number of assigned
// bursts is 1 and Collaborative SM is not supported.
struct mubi_align 
{
#ifdef __BIG_ENDIAN__
    u_int8_t num_assign :4;
    u_int8_t collab_sm_ind :1;
    u_int8_t cid_h:3;

    u_int8_t cid_m;

    u_int8_t cid_l:5;
    u_int8_t uiuc_h:3;

    u_int8_t uiuc_l:1;
    u_int8_t rep_coding_ind:2;
    u_int8_t mimo_control:1;
    u_int8_t duration_h:4;

    u_int8_t duration_l:6;
    // The padding bits are not specified in Table 398 of P802.16Rev2/D2
    // but they are essential and listed in 802.16e-2005 and 16d corrigendum
    u_int8_t padding:2;
#else
    u_int8_t cid_h:3;
    u_int8_t collab_sm_ind :1;
    u_int8_t num_assign :4;

    u_int8_t cid_m;

    u_int8_t uiuc_h:3;
    u_int8_t cid_l:5;

    u_int8_t duration_h:4;
    u_int8_t mimo_control:1;
    u_int8_t rep_coding_ind:2;
    u_int8_t uiuc_l:1;

    // The padding bits are not specified in Table 398 of P802.16Rev2/D2
    // but they are essential and listed in 802.16e-2005 and 16d corrigendum
    u_int8_t padding:2;
    u_int8_t duration_l:6;
#endif
};

/* Standard 8.4.5.4, Table 372 */
struct ul_map_msg_ie_align
{
        u_int16_t cid;
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t padding :4;
#else
        u_int8_t padding :4;
        u_int8_t uiuc :4;
#endif
};


/* Standard 8.4.5.4.4.2, Table 378, UIUC = 12 */
struct ul_map_uiuc_12_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t ofdma_offset_h :4;

        u_int8_t ofdma_offset_l :4;
        u_int8_t subchannel_offset_h :4;

        u_int8_t subchannel_offset_l :3;
        u_int8_t ofdma_symbols_num_h :5;

        u_int8_t ofdma_symbols_num_l :2;
        u_int8_t subchannels_num_h :6;

        u_int8_t subchannels_num_l :1;
        u_int8_t ranging_method :2;
        u_int8_t ranging_ind :1;
        u_int8_t padding :4;
#else
        u_int8_t ofdma_offset_h :4;
        u_int8_t uiuc :4;

        u_int8_t subchannel_offset_h :4;
        u_int8_t ofdma_offset_l :4;

        u_int8_t ofdma_symbols_num_h :5;
        u_int8_t subchannel_offset_l :3;

        u_int8_t subchannels_num_h :6;
        u_int8_t ofdma_symbols_num_l :2;

        u_int8_t padding :4;
        u_int8_t ranging_ind :1;
        u_int8_t ranging_method :2;
        u_int8_t subchannels_num_l :1;
#endif
};

/* Standard 8.4.5.4.2, Table 374, UIUC = 13 */
struct zone_alloc_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t ofdma_offset_h :4;

        u_int8_t ofdma_offset_l :4;
        u_int8_t subchannel_offset_h :4;

        u_int8_t subchannel_offset_l :3;
        u_int8_t ofdma_symbols_num_h :5;

        u_int8_t ofdma_symbols_num_l :2;
        u_int8_t subchannels_shift_num_h :6;

        u_int8_t subchannels_shift_num_l :1;
        u_int8_t papr_zone :1;
        u_int8_t sounding_zone :1;
        u_int8_t padding :5;
#else
        u_int8_t ofdma_offset_h :4;
        u_int8_t uiuc :4;

        u_int8_t subchannel_offset_h :4;
        u_int8_t ofdma_offset_l :4;

        u_int8_t ofdma_symbols_num_h :5;
        u_int8_t subchannel_offset_l :3;

        u_int8_t subchannels_shift_num_h :6;
        u_int8_t ofdma_symbols_num_l :2;

        u_int8_t padding :5;
        u_int8_t sounding_zone :1;
        u_int8_t papr_zone :1;
        u_int8_t subchannels_shift_num_l :1;
#endif
};

/* Standard 8.4.5.4.3, Table 375, UIUC = 14 */
struct cdma_alloc_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t duration_h :4;

        u_int8_t duration_l :2;
        u_int8_t uiuc_tx :4;
        u_int8_t rep_coding_ind :2;

        u_int8_t frame_num_idx :4;
        u_int8_t ranging_code_h : 4;

        u_int8_t ranging_code_l : 4;
        u_int8_t ranging_symbol_h : 4;

        u_int8_t ranging_symbol_l : 4;
        u_int8_t ranging_subchannel_h :4;

        u_int8_t ranging_subchannel_l :3;
        u_int8_t bw_req_m :1;
        u_int8_t padding : 4;
#else
        u_int8_t duration_h :4;
        u_int8_t uiuc :4;

        u_int8_t rep_coding_ind :2;
        u_int8_t uiuc_tx :4;
        u_int8_t duration_l :2;

        u_int8_t frame_num_idx :4;
        u_int8_t ranging_code_h :4;

        u_int8_t ranging_symbol_h :4;
        u_int8_t ranging_code_l :4;

        u_int8_t ranging_subchannel_h :4;
        u_int8_t ranging_symbol_l :4;

        u_int8_t padding :4;
        u_int8_t bw_req_m :1;
        u_int8_t ranging_subchannel_l :3;
#endif
};

/* Standard 8.4.5.4.4.1, Table 376, UIUC = 15 */
struct ul_ext_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t ext_uiuc :4;
        u_int8_t len :4;
        u_int8_t padding :4;
#else
        u_int8_t ext_uiuc :4;
        u_int8_t uiuc :4;

        u_int8_t padding :4;
        u_int8_t len :4;
#endif
        /* with unspecified data following and padding*/
};

/* UIUC = 0 FAST-FEEDBACK_Allocation_IE() Unknown format */
struct fast_ack_alloc_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t content_h :4;

        u_int32_t content_m : 24;

        u_int32_t content_l : 4;
        u_int8_t padding :4;
#else
        u_int8_t content_h :4;
        u_int8_t uiuc :4;

        u_int32_t content_m : 24;

        u_int8_t padding :4;
        u_int8_t content_l : 4;
#endif
};

/* Unknown UIUC */
struct unknown_uiuc_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t uiuc :4;
        u_int8_t duration_h :4;

        u_int8_t duration_l :6;
        u_int8_t rep_coding_ind :2;
#else
        u_int8_t duration_h :4;
        u_int8_t uiuc :4;

        u_int8_t rep_coding_ind :2;
        u_int8_t duration_l :6;
#endif
};

struct unknown_uiuc_2_ie_align
{
#ifdef __BIG_ENDIAN__
        u_int8_t slot_offset_h;

        u_int8_t slot_offset_l :4;
        u_int8_t padding :4;

#else
        u_int8_t slot_offset_h;

        u_int8_t padding :4;
        u_int8_t slot_offset_l :4;
#endif
};

#pragma pack(pop)

/*
int build_ul_map_msg (ul_map_msg const * p_msg_header, u_int8_t * p_payload,
                      u_int32_t * p_payload_len);

int parse_ul_map_msg (u_int8_t * const p_payload, u_int32_t const payload_len,
                      ul_map_msg * p_msg_header);
*/

#endif

