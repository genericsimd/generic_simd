/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_headermsg_builder.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "mac_headermsg_builder.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "mac_system_info.h"
#include "mac_config.h"
#include "ul_map_align.h"
#include "ulmap.h"
#include "init_maps.h"
#include "memmgmt.h"
#include "mac_hcs.h"
#include "mac_crc.h"
#include "mac.h"
#include "dump_util.h"

#define ULMAP_MSG_TYPE 3

int mem_right_shitf (void * p_mem_dst, void * p_mem_src, unsigned int len,
        unsigned int bits, unsigned int drop_tail)
{
    u_int8_t * p_tmp_src = (u_int8_t *) p_mem_src;
    u_int8_t * p_tmp_dst = (u_int8_t *) p_mem_dst;

    int byte_idx;

    if ( ( p_tmp_src == NULL ) || ( p_tmp_dst == NULL ) || ( len == 0 )
            || ( bits >= 8 ))
    {
        FLOG_WARNING("error input in mem_right_shitf.\n");
        return 1;
    }

    p_tmp_dst[0] = ( p_tmp_dst[0] & ( 0xff << ( 8 - bits ) ) ) | ( p_tmp_src[0]
            >> bits );

//    printf (" /%x %x/ ", p_tmp_dst[0], p_tmp_src[0]);

    for (byte_idx = 1; byte_idx < len - 1; byte_idx++)
    {
        p_tmp_dst[byte_idx] = ( p_tmp_src[byte_idx - 1] << ( 8 - bits ) )
                | ( p_tmp_src[byte_idx] >> bits );

//        printf (" /%x %x/ ", p_tmp_dst[byte_idx], p_tmp_src[byte_idx]);
    }

    if ( ( drop_tail == 0 ) && ( len > 1 ))
    {
        p_tmp_dst[byte_idx] = ( p_tmp_src[byte_idx - 1] << ( 8 - bits ) )
                | ( p_tmp_src[byte_idx] >> bits );

//        printf (" /%x %x/ \n", p_tmp_dst[byte_idx], p_tmp_src[byte_idx]);
    }

    return 0;
}

int build_frame_msg (logical_dl_subframe_map * frame_map,
                     physical_subframe* phy_subframe)
{
    unsigned char * p_tmp_fch_hdr = NULL;
    unsigned char * p_tmp_hdr = NULL;
    unsigned int tmp_fch_len = 0;

    unsigned int tmp_len = 0;

    logical_burst_map * p_burst_hdr = NULL;
/** add by zzb */
//    unsigned char * p_tmp_ulmap = NULL;
/** end by zzb */
    logical_pdu_map * p_pdu_map = NULL;
    //logical_pdu_map * p_pdu = NULL;

    /* Building FCH & DL_MAP */
    p_tmp_fch_hdr = (unsigned char *) malloc (MAX_FCH_LEN);

    if (p_tmp_fch_hdr == NULL)
    {
        FLOG_FATAL("malloc failed\n");
        return 1;
    }

    memset (p_tmp_fch_hdr, sizeof(char), MAX_FCH_LEN);

#ifndef SS_TX
    build_dlframeprefix_foak (frame_map->fch, p_tmp_fch_hdr, 0,(int*) &tmp_fch_len);
#else
    build_dlframeprefix (frame_map->fch, p_tmp_fch_hdr, 0, (int*)&tmp_fch_len);
#endif
    phy_subframe->fch_dl_map_len = tmp_fch_len
        + ( frame_map->fch->dl_map_length ) * BYTES_PER_SLOT;

    p_tmp_hdr = (unsigned char *) phy_subframe->fch_dl_map;

    memcpy (p_tmp_hdr, p_tmp_fch_hdr, tmp_fch_len);

    free (p_tmp_fch_hdr);
    p_tmp_fch_hdr = NULL;
    //printf("In build_frame_msg, start of dlmap: %x, Total bytes available for DLMAP: %d\n", p_tmp_hdr + tmp_fch_len, (frame_map->fch->dl_map_length) * BYTES_PER_SLOT);
    build_dlmap (frame_map->dl_map, p_tmp_hdr + tmp_fch_len, (int*)&tmp_len);

#ifdef DDEBUG
    dl_map_msg *mydlmap = (dl_map_msg*)malloc(sizeof(dl_map_msg));
    int mylen = 0;
    parse_dlmap(p_tmp_hdr + tmp_fch_len, mydlmap, &mylen);
    FLOG_INFO("Printing DLMAP at the o/p of the parser: ");
    print_dlmap(mydlmap);
#endif

    /* Building UL_MAP */
    p_burst_hdr = frame_map->burst_header;
/** add by zzb*/
/*
    p_tmp_ulmap = (unsigned char*) malloc (p_burst_hdr->burst_bytes_num);

    if (p_tmp_ulmap == NULL)
    {
        FLOG_FATAL("malloc failed\n");
        return 1;
    }
*/
/** end by zzb */


	// Walk through all the PDUs in the first (broadcast) burst of the frame
	// map (this contains any broadcast MMM like UCD, DCD and initial ranging
	// MMM like RNG_RSP All except ULMAP). Add a new PDU map at the end, which 
	// has a mac_msg_map containing the ulmap payload
    p_pdu_map = p_burst_hdr->pdu_map_header;

    if (p_pdu_map != NULL)
    {
        while (p_pdu_map->next != NULL)
        {
            p_pdu_map = p_pdu_map->next;
        }

        p_pdu_map->next = (logical_pdu_map *) malloc (sizeof(logical_pdu_map));

        if (p_pdu_map->next == NULL)
        {
            FLOG_WARNING ("malloc failed\n");
            return 1;
        }

        p_pdu_map = p_pdu_map->next;
    }
    else
    {
        p_burst_hdr->pdu_map_header
            = (logical_pdu_map *) malloc (sizeof(logical_pdu_map));

        if (p_burst_hdr->pdu_map_header == NULL)
        {
            FLOG_FATAL ("malloc failed\n");
            return 1;
        }

        p_pdu_map = p_burst_hdr->pdu_map_header;
    }

    init_pdu_map (p_pdu_map);
	p_pdu_map->cid = BROADCAST_CID;

    p_burst_hdr->pdu_num++;

    tmp_len = 0;

    /** add by zzb */
/*
	// tmp_len here does not contain the GMH+CRC length
    build_ul_map_msg (frame_map->ul_map, p_tmp_ulmap, &tmp_len);
*/

    // tmp_len here does not contain the GMH+CRC length
    build_ul_map_msg (frame_map->ul_map, phy_subframe->raw_ul_map, &(phy_subframe->raw_ul_map_len));
/** end by zzb */

#ifdef DDEBUG
    ul_map_msg *myulmap = (ul_map_msg*)malloc(sizeof(ul_map_msg));
    parse_ul_map_msg (p_tmp_ulmap, phy_subframe->raw_ul_map_len, myulmap);
    FLOG_INFO("Printing ULMAP at the output of the parser:\n");
    print_ulmap(myulmap);
#endif

#ifdef _DUMP_MAC_
    ul_map_msg *myulmap = (ul_map_msg*)malloc(sizeof(ul_map_msg));
   // parse_ul_map_msg (p_tmp_ulmap, phy_subframe->raw_ul_map_len, myulmap);
    parse_ul_map_msg ( phy_subframe->raw_ul_map, phy_subframe->raw_ul_map_len, myulmap);
    //dump_ulmap(myulmap);
    DO_DUMP(DUMP_MAC_ULMAP_ID, 0, myulmap, 1);
#endif


    p_pdu_map->mac_msg_map
        = (mgmtmsg_sdu_map *) malloc (sizeof(mgmtmsg_sdu_map));

    if (p_pdu_map->mac_msg_map == NULL)
    {
        FLOG_FATAL ("malloc failed\n");
        return 1;
    }

    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu
        = (logical_packet *) malloc (sizeof(logical_packet));

    if (p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu == NULL)
    {
        FLOG_FATAL ("malloc failed\n");
        return 1;
    }

    init_logical_packet (p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu);

    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->length = 0;

    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head
        = (logical_element *) malloc (sizeof(logical_element));

    if (p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head == NULL)
    {
        FLOG_FATAL ("malloc failed\n");
        return 1;
    }

    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->length = phy_subframe->raw_ul_map_len;
	p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->type = UL_MAP_IE;
	p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->blk_type = NO_FRAGMENTATION;
    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->data
        = (unsigned char*) mac_sdu_malloc (phy_subframe->raw_ul_map_len, ULMAP_MSG_TYPE);

    memcpy (p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->data,
            phy_subframe->raw_ul_map, phy_subframe->raw_ul_map_len);
	p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->next = NULL;

/** add by zzb */
/*
    free (p_tmp_ulmap);
    p_tmp_ulmap = NULL;
*/
/** end by zzb */
    p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->next = NULL;

    //printf("ulmap data address %p\n", p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head->data);
    //printf("element_head %p\n", p_pdu_map->mac_msg_map->mac_mgmt_msg_sdu->element_head);
    return 0;
}

/** the MAC Message is assumed as BIG Endian same as the Internet Order */

int build_ul_map_msg (ul_map_msg const * p_msg_header, u_int8_t * p_payload,
        u_int32_t * p_payload_len)
{
    u_int32_t length = 0;
    u_int8_t * p_payload_tmp = p_payload;
    ul_map_ie * p_ie_tmp = NULL;
    u_int32_t ie_idx;

    u_int8_t ie_buf[MAX_IE_LENGTH];
    u_int8_t * p_ie_buf = ie_buf;
    unsigned int ie_len = 0;

    unsigned int pre_tail_bits = 0;
    unsigned int next_tail_bits = 0;
    unsigned int if_drop_tail = 0;
    unsigned int len_offset = 0;

    struct ul_map_msg_align * p_ul_map_align = NULL;
    struct ul_map_msg_ie_align * p_ul_map_ie_align = NULL;
    struct ul_ext_2_ie_align * p_ul_ext_2_ie_align = NULL;
    struct mubi_align *p_mubi_align = NULL;
    struct ul_map_uiuc_12_align * p_ul_map_uiuc_12_align = NULL;
    struct zone_alloc_ie_align * p_zone_alloc_ie_align = NULL;
    struct cdma_alloc_ie_align * p_cdma_alloc_ie_align = NULL;
    struct ul_ext_ie_align * p_ul_ext_ie_align = NULL;
    struct fast_ack_alloc_ie_align * p_fast_ack_alloc_ie_align = NULL;
    struct unknown_uiuc_ie_align * p_unknown_uiuc_ie_align = NULL;
    struct unknown_uiuc_2_ie_align * p_unknown_uiuc_2_ie_align = NULL;

    if ( ( p_msg_header == NULL ) || ( p_payload == NULL ) || ( p_payload_len
            == NULL ))
    {
        FLOG_WARNING ("in build_ul_map_msg: input pointer is NULL.\n");
        return 1;
    }

    p_ul_map_align = (struct ul_map_msg_align *) p_payload_tmp;
    p_ul_map_align->mgt_msg_t = p_msg_header->manage_msg_type;
    p_ul_map_align->fdd_change_flg = 1;
    p_ul_map_align->rsv = 0;
    p_ul_map_align->ucd_count = p_msg_header->ucd_count;

    /* Use self implementation order convert in future*/
    p_ul_map_align->alloc_start_time = htonl (p_msg_header->alloc_start_time);

    // OFDMA only
    p_ul_map_align->ofdma_symbols_num = p_msg_header->num_ofdma_sym;

    p_payload_tmp += sizeof(struct ul_map_msg_align);
    length += sizeof(struct ul_map_msg_align);

    p_ie_tmp = p_msg_header->ie;

    for (ie_idx = 0; ie_idx < p_msg_header->ulmap_ie_num; ie_idx++)
    {
        if (p_ie_tmp == NULL)
        {
            FLOG_WARNING ("in build_ul_map_msg: ie pointer is NULL.\n");
            return 1;
        }

        p_ul_map_ie_align = (struct ul_map_msg_ie_align *) p_ie_buf;
        p_ul_map_ie_align->cid = htons (p_ie_tmp->cid);
        p_ul_map_ie_align->uiuc = p_ie_tmp->uiuc;

        p_ie_buf += sizeof(struct ul_map_msg_ie_align) - 1;
        ie_len += sizeof(struct ul_map_msg_ie_align) - 1;

        if (p_ie_tmp->uiuc == 11)
        {
            extended_uiuc_ie * p_tmp = p_ie_tmp->uiuc_extend_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: extended_uiuc2_ie"
                    "pointer is NULL.\n");
                return 1;
            }

            p_ul_ext_2_ie_align = (struct ul_ext_2_ie_align *) p_ie_buf;
            p_ul_ext_2_ie_align->ext_2_uiuc = p_tmp->extended_uiuc;
            p_ul_ext_2_ie_align->len = p_tmp->length;
            /* memcpy and padding */

            ie_len += sizeof(struct ul_ext_2_ie_align);
            p_ie_buf += sizeof(struct ul_ext_2_ie_align);

            mimo_ul_basic_ie *mubi = NULL;
            assigned_burst_attri *aba = NULL;
            switch (p_tmp->extended_uiuc)
            {
                case MIMO_UL_BASIC_IE:
                    mubi = p_tmp->unspecified_data;
                    p_mubi_align = (struct mubi_align*) p_ie_buf;
                    p_mubi_align->num_assign = mubi->num_assign;

                    if (mubi->num_assign != 0)
                    {
                        FLOG_ERROR (
                                "Error in mac_headermsg_builder: Only one burst per MIMO UL BASIC IE supported\n");
                        return -1;
                    }

                    aba = mubi->assigned_burst_header;
                    p_mubi_align->collab_sm_ind
                            = aba->collaborative_sm_indication;
                    if (p_mubi_align->collab_sm_ind != 0)
                    {
                        FLOG_ERROR (
                                "Error in mac_headermsg_builder: Invalid value for Collaborative SM Indication\n");
                        return -1;
                    }
                    p_mubi_align->cid_h = ( aba->cid & 0xe000 ) >> 13;
                    p_mubi_align->cid_m = ( aba->cid & 0x1fe0 ) >> 5;
                    p_mubi_align->cid_l = ( aba->cid & 0x001f );

                    p_mubi_align->uiuc_h = ( aba->uiuc & 0x0e ) >> 1;
                    p_mubi_align->uiuc_l = ( aba->uiuc & 0x01 );
                    p_mubi_align->rep_coding_ind
                            = aba->repetition_coding_indication;
                    p_mubi_align->mimo_control = aba->mimo_control;

                    p_mubi_align->duration_h = ( aba->duration & 0x03c0 ) >> 6;
                    p_mubi_align->duration_l = ( aba->duration & 0x3f );
                    p_mubi_align->padding = 0;

                    ie_len += sizeof(struct mubi_align);

                    break;
                default:
                    FLOG_ERROR (
                            "In mac_headermsg_builder: Unsupported Extended-2 UIUC\n");

            }
			// The below piece of code for the if-else condition has to be 
			// included for UIUCs which have byte-aligned data after including
			// the 4-bit UIUC value
            if (pre_tail_bits == 0)
            {
                next_tail_bits = 0;
                len_offset = 0;
            }
            else
            {
                next_tail_bits = pre_tail_bits;
                if_drop_tail = 0;
                ie_len++;
                len_offset = 1;
            }

        }
        else if (p_ie_tmp->uiuc == 12)
        {
            uiuc12_ie * p_tmp = p_ie_tmp->uiuc_12_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: uiuc_12_ie"
                    "pointer is NULL.\n");
                return 1;
            }

            p_ul_map_uiuc_12_align = (struct ul_map_uiuc_12_align *) p_ie_buf;

            p_ul_map_uiuc_12_align->ofdma_offset_l
                    = ( p_tmp->ofdma_symbol_offset ) & 0x0F;
            p_ul_map_uiuc_12_align->ofdma_offset_h
                    = ( ( p_tmp->ofdma_symbol_offset ) & 0xF0 ) >> 4;

            p_ul_map_uiuc_12_align->subchannel_offset_l
                    = ( p_tmp->subchannel_offset ) & 0x07;
            p_ul_map_uiuc_12_align->subchannel_offset_h
                    = ( ( p_tmp->subchannel_offset ) & 0x78 ) >> 3;

            p_ul_map_uiuc_12_align->ofdma_symbols_num_l
                    = ( p_tmp->ofdma_symbol_num ) & 0x03;
            p_ul_map_uiuc_12_align->ofdma_symbols_num_h
                    = ( ( p_tmp->ofdma_symbol_num ) & 0x7C ) >> 2;

            p_ul_map_uiuc_12_align->subchannels_num_l
                    = ( p_tmp->subchannel_num ) & 0x01;
            p_ul_map_uiuc_12_align->subchannels_num_h
                    = ( ( p_tmp->subchannel_num ) & 0x7E ) >> 1;

            p_ul_map_uiuc_12_align->ranging_method = p_tmp->ranging_method;
            p_ul_map_uiuc_12_align->ranging_ind
                    = p_tmp->dedicated_ranging_indicator;
            p_ul_map_uiuc_12_align->padding = 0;

            ie_len += sizeof(struct ul_map_uiuc_12_align);

			// Ref Table 372: OFDMA UL-MAP IE format
			// The below piece of code for the if-else condition has to be
			// included for UIUCs which do not have byte aligned data after
			// including the 4 bit UIUC value in the ULMAP-IE. E.g. UIUC=12
			// has 4(UIUC)+8+7+7+7+2+1=36 bit which isn't byte aligned.
            if (pre_tail_bits == 0)
            {
                next_tail_bits = 4;
                if_drop_tail = 0;
                len_offset = 1;

            }
            else if (pre_tail_bits == 4)
            {
                next_tail_bits = 0;
                if_drop_tail = 0;
                len_offset = 0;
            }
            else
            {
                /* Not support yet!*/
                FLOG_WARNING ("Error in UIUC = 0.\n");
            }
			//printf("after adding UIUC = 12 (ranging IE) ie_len: %d, len_offset: %d\n", ie_len, len_offset);
        }
        else if (p_ie_tmp->uiuc == 13)
        {
            paprreduc_safezone_alloc_ie * p_tmp = p_ie_tmp->uiuc_13_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: paprreduc_safezone_alloc_ie"
                    "pointer is NULL.\n");
                return 1;
            }
            p_zone_alloc_ie_align = (struct zone_alloc_ie_align *) p_ie_buf;

            p_zone_alloc_ie_align->ofdma_offset_l = p_tmp->ofdma_symbol_offset
                    & 0x0F;
            p_zone_alloc_ie_align->ofdma_offset_h
                    = ( p_tmp->ofdma_symbol_offset & 0xF0 >> 4 );

            p_zone_alloc_ie_align->subchannel_offset_l
                    = p_tmp->subchannel_offset & 0x07;
            p_zone_alloc_ie_align->subchannel_offset_h
                    = ( p_tmp->subchannel_offset & 0x78 ) >> 3;

            p_zone_alloc_ie_align->ofdma_symbols_num_l
                    = p_tmp->ofdma_symbol_num & 0x03;
            p_zone_alloc_ie_align->ofdma_symbols_num_h
                    = ( p_tmp->ofdma_symbol_num & 0x7C ) >> 2;

            p_zone_alloc_ie_align->subchannels_shift_num_l
                    = p_tmp->subchannel_num_szshift_value & 0x01;
            p_zone_alloc_ie_align->subchannels_shift_num_h
                    = ( p_tmp->subchannel_num_szshift_value & 0x7E ) >> 1;

            p_zone_alloc_ie_align->papr_zone = p_tmp->paprreduc_safezone;
            p_zone_alloc_ie_align->sounding_zone = p_tmp->sounding_zone;
            p_zone_alloc_ie_align->padding = 0;

            ie_len += sizeof(struct zone_alloc_ie_align);
            if (pre_tail_bits == 0)
            {
                next_tail_bits = 4;
                if_drop_tail = 0;
                len_offset = 1;

            }
            else if (pre_tail_bits == 4)
            {
                next_tail_bits = 0;
                if_drop_tail = 0;
                len_offset = 0;
            }
            else
            {
                /* Not support yet!*/
                FLOG_WARNING ("Error in UIUC = 0.\n");
            }
        }
        else if (p_ie_tmp->uiuc == 14)
        {
            cdma_alloc_ie * p_tmp = p_ie_tmp->uiuc_14_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: cdma_alloc_ie"
                    "pointer is NULL.\n");
                return 1;
            }

            p_cdma_alloc_ie_align = (struct cdma_alloc_ie_align *) p_ie_buf;

            p_cdma_alloc_ie_align->duration_l = p_tmp->duration & 0x03;
            p_cdma_alloc_ie_align->duration_h = ( p_tmp->duration & 0x3C ) >> 2;

            p_cdma_alloc_ie_align->uiuc_tx = p_tmp->uiuc;
            p_cdma_alloc_ie_align->rep_coding_ind
                    = p_tmp->repetition_coding_indication;

            p_cdma_alloc_ie_align->frame_num_idx = p_tmp->frame_num_index;

            p_cdma_alloc_ie_align->ranging_code_l = p_tmp->ranging_code & 0x0F;
            p_cdma_alloc_ie_align->ranging_code_h = ( p_tmp->ranging_code
                    & 0xF0 ) >> 4;

            p_cdma_alloc_ie_align->ranging_symbol_l = p_tmp->ranging_symbol
                    & 0x0F;
            p_cdma_alloc_ie_align->ranging_symbol_h = ( p_tmp->ranging_symbol
                    & 0xF0 ) >> 4;

            p_cdma_alloc_ie_align->ranging_subchannel_l
                    = p_tmp->ranging_subchannel & 0x07;

            p_cdma_alloc_ie_align->ranging_subchannel_h
                    = ( p_tmp->ranging_subchannel & 0x78 ) >> 3;

            p_cdma_alloc_ie_align->bw_req_m = p_tmp->bw_request_mandatory;

            p_cdma_alloc_ie_align->padding = 0;

            ie_len += sizeof(struct cdma_alloc_ie_align);
            if (pre_tail_bits == 0)
            {
                next_tail_bits = 4;
                if_drop_tail = 0;
                len_offset = 1;

            }
            else if (pre_tail_bits == 4)
            {
                next_tail_bits = 0;
                if_drop_tail = 0;
                len_offset = 0;
            }
            else
            {
                /* Not support yet!*/
                FLOG_WARNING ("Error in UIUC = 0.\n");
            }
        }
        else if (p_ie_tmp->uiuc == 15)
        {
            extended_uiuc_ie * p_tmp = p_ie_tmp->uiuc_15_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: extended_uiuc_ie"
                    "pointer is NULL.\n");
                return 1;
            }

            p_ul_ext_ie_align = (struct ul_ext_ie_align *) p_ie_buf;

            p_ul_ext_ie_align->ext_uiuc = p_tmp->extended_uiuc;
            p_ul_ext_ie_align->len = p_tmp->length;
            p_ul_ext_ie_align->padding = 0;
            /* memcpy and padding */

            ie_len += sizeof(struct ul_ext_ie_align);

            if (pre_tail_bits == 0)
            {
                next_tail_bits = 4;
                if_drop_tail = 0;
                len_offset = 1;

            }
            else if (pre_tail_bits == 4)
            {
                next_tail_bits = 0;
                if_drop_tail = 0;
                len_offset = 0;
            }
            else
            {
                /* Not support yet!*/
                FLOG_WARNING ("Error in UIUC = 0.\n");
            }
        }
        else if (p_ie_tmp->uiuc == 0)
        {
            fastfeedback_alloc_ie * p_tmp = p_ie_tmp->uiuc_0_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: FAST-FEEDBACK_Allocation_IE"
                    "pointer is NULL.\n");
                return 1;
            }

            p_fast_ack_alloc_ie_align
                    = (struct fast_ack_alloc_ie_align *) p_ie_buf;

            p_fast_ack_alloc_ie_align->content_l = 0xA0B0C0D0 & 0x0000000F;

            p_fast_ack_alloc_ie_align->content_m = ntohl ( ( 0xA0B0C0D0
                    & 0x0FFFFFF0 ) << 4);

            p_fast_ack_alloc_ie_align->content_h = ( 0xA0B0C0D0 & 0xF0000000 )
                    >> 28;

            p_fast_ack_alloc_ie_align->padding = 0;

            ie_len += sizeof(struct fast_ack_alloc_ie_align);
            if (pre_tail_bits == 0)
            {
                next_tail_bits = 4;
                if_drop_tail = 0;
                len_offset = 1;

            }
            else if (pre_tail_bits == 4)
            {
                next_tail_bits = 0;
                if_drop_tail = 0;
                len_offset = 0;
            }
            else
            {
                /* Not support yet!*/
                FLOG_WARNING ("Error in UIUC = 0.\n");
            }
        }
        else
        {
            other_uiuc_ie * p_tmp = p_ie_tmp->uiuc_other_ie;

            if (p_tmp == NULL)
            {
                FLOG_WARNING ("in build_ul_map_msg: Other UIUC IE"
                    "pointer is NULL.\n");
                return 1;
            }

            p_unknown_uiuc_ie_align = (struct unknown_uiuc_ie_align *) p_ie_buf;

            p_unknown_uiuc_ie_align->duration_l = p_tmp->duration & 0x003F;
            p_unknown_uiuc_ie_align->duration_h = ( p_tmp->duration & 0x03C0 )
                    >> 6;

            p_unknown_uiuc_ie_align->rep_coding_ind
                    = p_tmp->repetition_coding_indication;

            ie_len += sizeof(struct unknown_uiuc_ie_align);

            if (pre_tail_bits == 0)
            {
                next_tail_bits = 0;
                len_offset = 0;
            }
            else
            {
                next_tail_bits = pre_tail_bits;
                if_drop_tail = 0;
                ie_len++;
                len_offset = 1;
            }
			//printf("after adding UIUC = 1-10 ie_len: %d, len_offset: %d\n", ie_len, len_offset);

            /* AAS or AMC UL Zone? */
            if (0)
            {
                p_unknown_uiuc_2_ie_align
                        = (struct unknown_uiuc_2_ie_align *) p_ie_buf;

                p_unknown_uiuc_2_ie_align->slot_offset_l = p_tmp->slot_offset
                        & 0x000F;

                p_unknown_uiuc_2_ie_align->slot_offset_h = ( p_tmp->slot_offset
                        & 0xFFF0 ) >> 4;

                p_unknown_uiuc_2_ie_align->padding = 0;

                p_ie_buf += sizeof(struct unknown_uiuc_2_ie_align);
                ie_len += sizeof(struct unknown_uiuc_2_ie_align);
            }
        }

        p_ie_tmp = p_ie_tmp->next;

        if (pre_tail_bits != 0)
        {
            mem_right_shitf ((void *) p_payload_tmp, (void *) ie_buf, ie_len,
                    pre_tail_bits, if_drop_tail);
        }
        else
        {
            memcpy ((void *) p_payload_tmp, (void *) ie_buf, ie_len);
        }
        p_payload_tmp += ie_len - len_offset;
		//printf("In build_ul_map_msg: length: %d, ie_len: %d, len_offset: %d, pre_tail: %d, next_tail: %d\n", length, ie_len, len_offset, pre_tail_bits, next_tail_bits);
        length += ie_len - len_offset;

        pre_tail_bits = next_tail_bits;
        p_ie_buf = ie_buf;
        ie_len = 0;
    }

    *p_payload_len = length + len_offset;

    return 0;
}

int build_psh (const pack_sub_hdr* const psh, u_char* payload,
               u_int8_t is_extend, int* length)
{
    u_char* curp;
    curp = payload;
    if (is_extend)
    {
        // first char,2 fc, 6 fsn
        ( *curp ) = ( u_int8_t ) ( ( ( psh->fc & 0x03 ) << 6 ) | ( ( psh->fsn
            & 0x07ff ) >> 5 ));
        // second char, 5 fsn, 3 len
        curp++;
        ( *curp ) = ( u_int8_t ) ( ( ( psh->fsn & 0x001f ) << 3 )
            | ( ( psh->length & 0x07ff ) >> 8 ));
        // thrid char, 8 len
        curp++;
        ( *curp ) = ( u_int8_t ) (psh->length & 0x00ff);
        ( *length ) = 3;
    }
    else
    {
        // first char, 2 fc, 3 fsn, 3 len
        ( *curp ) = ( u_int8_t ) ( ( ( psh->fc & 0x03 ) << 6 ) | ( ( psh->fsn
            & 0x07 ) << 3 ) | ( ( psh->length & 0x07ff ) >> 8 ));
        // second char
        curp++;
        ( *curp ) = ( u_int8_t ) (psh->length & 0x00ff);
        ( *length ) = 2;
    }
    return 0;
}

int build_fsh (const frag_sub_hdr* const fsh, u_char* payload,
               u_int8_t is_extend, int* length)
{
    u_char* curp;
    curp = payload;
    if (is_extend)
    {
        // first char, 2 fc, 6 fsn
        ( *curp ) = ( u_int8_t ) ( ( ( fsh->fc & 0x03 ) << 6 ) | ( ( fsh->fsn
            & 0x07ff ) >> 5 ));
        // second char, 5 fsn, 3 rsv
        curp++;
        ( *curp ) = ( u_int8_t ) ( ( ( fsh->fsn & 0x001f ) << 3 ) | ( fsh->rsv
            & 0x07 ));
        ( *length ) = 2;
    }
    else
    {
        ( *curp ) = ( u_int8_t ) ( ( ( fsh->fc & 0x03 ) << 6 ) | ( ( fsh->fsn
            & 0x07 ) << 3 ) | ( fsh->rsv & 0x07 ));
        ( *length ) = 1;
    }
    return 0;
}

int build_gmh (const generic_mac_hdr* const gmh, u_char* payload, int* length)
{
    u_char* cur_p;
    // first char
    cur_p = payload;
	if ((gmh->len & 0x7ff) == 0)
	{
		FLOG_ERROR("ERROR: pdu_len = 0 on Tx side\n");
	}

    ( *cur_p ) = ( u_int8_t ) ( ( ( gmh->ht & 0x01 ) << 7 ) | ( ( gmh->ec
        & 0x01 ) << 6 ) | ( gmh->type & 0x3f ));
    // second char
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( ( gmh->esf & 0x01 ) << 7 ) | ( ( gmh->ci
        & 0x01 ) << 6 ) | ( ( gmh->eks & 0x03 ) << 4 ) | ( ( gmh->rsv & 0x01 )
        << 3 ) | ( ( gmh->len & 0x07ff ) >> 8 ));
    // third char
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (gmh->len & 0x00ff);
    // the fourth char
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (gmh->cid >> 8);
    // the fifth char
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (gmh->cid & 0x00ff);
    // the sixth char
//	printf("pointer: %x, CID MSB: %d, CID LSB: %d, length: %d (gmh->len: %d)\n", payload, payload[3], payload[4], (((payload[1]&0x07) << 8)+payload[2]), gmh->len);
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (gmh->hcs);
    ( *length ) = GENERIC_MAC_HEADER_LEN;
    return 0;
}


int build_dlframeprefix_foak (const dl_subframe_prefix* const dlp, u_char* payload,
        int is_128fft, int* length)
{
    u_int8_t hcs_result;
    u_int32_t ret;
    

    u_char* curp;
    // first char
    curp = payload;
    

    if (is_128fft)
    {
        return 1;
    }
    else
    {
        // first char, 6 used subchannel bitmap, 1 reserved, 1 repetition coding indication
        ( *curp ) = ( u_int8_t ) ( ( ( dlp->used_subchannel_bitmap & 0x3f )
                << 2 ) | ( ( dlp->repetition_coding_indication & 0x03 ) >> 1 ));
        // second char, 1 repetition coding indication, 3 coding indication, 4 dl map length
        curp++;

        ( *curp ) = ( u_int8_t ) (
                ( ( dlp->repetition_coding_indication & 0x01 ) << 7 )
                        | ( ( dlp->coding_indication & 0x07 ) << 4 )
                        | ( dlp->dl_map_length >> 4 ));

        curp++;

        ( *curp ) = ( u_int8_t ) ( ( ( dlp->dl_map_length & 0x0f ) << 4 )
                | ( ( dlp->p_dts_info->is_active[0] & 0x01 ) << 3 )
                | ( ( dlp->p_dts_info->is_active[1] & 0x01 ) << 2 )
                | ( ( dlp->p_dts_info->is_active[2] & 0x01 ) << 1 )
                | ( dlp->p_dts_info->is_active[3] & 0x01 ));
        curp++;

        ( *curp ) = ( (( dlp->p_dts_info->is_active[4] & 0x01 ) << 7)
                | (( dlp->p_dts_info->is_active[5] & 0x01 ) << 6)
                | (( dlp->p_dts_info->is_active[6] & 0x01 ) << 5)
                | (( dlp->p_dts_info->is_active[7] & 0x01 ) << 4)
                | (( dlp->p_dts_info->is_active[8] & 0x01 ) << 3)
                | (( dlp->p_dts_info->is_active[9] & 0x01 ) << 2)
                | (( dlp->p_dts_info->is_active[10] & 0x01 ) << 1)
                | ( dlp->p_dts_info->is_active[11] & 0x01 ));

        curp++;

        ( *curp ) = ( (( dlp->p_dts_info->is_active[12] & 0x01 ) << 7)
                | (( dlp->p_dts_info->is_active[13] & 0x01 ) << 6)
                | (( dlp->p_dts_info->is_active[14] & 0x01 ) << 5)
                | (( dlp->p_dts_info->is_active[15] & 0x01 ) << 4)
                | (( dlp->p_dts_info->is_active[16] & 0x01 ) << 3)
                | (( dlp->p_dts_info->is_active[17] & 0x01 ) << 2)
                | (( dlp->p_dts_info->is_active[18] & 0x01 ) << 1)
                | ( dlp->p_dts_info->is_active[19] & 0x01 ));

        curp++;

        ( *curp ) = (( dlp->p_dts_info->is_active[20] & 0x01 ) << 7);

        ret = hcs_calculation (payload, 6, &hcs_result);
        payload[0] = payload[0] | ( (hcs_result & 0x80) >> 6);
        payload[5] = payload[5] | (hcs_result & 0x7f);

        curp++;

        memcpy(curp, payload, 6);
        curp += 6;

        memcpy(curp, payload, 6);
        curp += 6;

        memcpy(curp, payload, 6);
        curp += 6;

        ( *length ) = 24;

    }

    return 0;
}




int build_dlframeprefix (const dl_subframe_prefix* const dlp, u_char* payload,
                         int is_128fft, int* length)
{
    u_char* curp;
    // first char
    curp = payload;

    if (is_128fft)
    {
        // first char, 1 used subchannel bitmap, 1 reserved, 2 repetition coding indication, 3 bit coding indication, 1 dlmap
        ( *curp ) = ( u_int8_t ) ( ( ( dlp->used_subchannel_bitmap & 0x01 )
            << 7 ) | ( ( dlp->rsv1 & 0x01 ) << 6 )
            | ( ( dlp->repetition_coding_indication & 0x3 ) << 4 )
            | ( ( dlp->coding_indication & 0x7 ) << 1 )
            | ( ( dlp->dl_map_length & 0x1f ) >> 4 ));
        // second char, 4 dl map length, 4 reserved
        curp++;
        ( *curp ) = ( u_int8_t ) ( ( ( dlp->dl_map_length & 0x1f ) << 4 )
            | ( ( dlp->used_subchannel_bitmap & 0x01 ) << 3 ) | ( ( dlp->rsv1
            & 0x01 ) << 2 ) | ( dlp->repetition_coding_indication & 0x3 ));
        curp++;

        ( *curp ) = ( u_int8_t ) ( ( ( dlp->coding_indication & 0x7 ) << 5 )
            | ( dlp->dl_map_length & 0x1f ));
        curp++;
        // duplicate to become 48 bit
        memcpy (curp, payload, 3);
        ( *length ) = 6;
    }
    else
    {
        // first char, 6 used subchannel bitmap, 1 reserved, 1 repetition coding indication
        ( *curp ) = ( u_int8_t ) ( ( ( dlp->used_subchannel_bitmap & 0x3f )
            << 2 ) | ( ( dlp->rsv1 & 0x01 ) << 1 )
            | ( ( dlp->repetition_coding_indication & 0x03 ) >> 1 ));
        // second char, 1 repetition coding indication, 3 coding indication, 4 dl map length
        curp++;
        ( *curp )
            = ( u_int8_t ) ( ( ( dlp->repetition_coding_indication & 0x01 )
                << 7 ) | ( ( dlp->coding_indication & 0x07 ) << 4 )
                | ( dlp->dl_map_length >> 4 ));
        // third char, 4 dl map length, 4 reserved
        curp++;
        ( *curp ) = ( u_int8_t ) ( ( ( dlp->dl_map_length & 0xf ) << 4 )
            | ( dlp->rsv2 & 0x0f ));
        curp++;

        // duplicate to become 48 bit
        memcpy (curp, payload, 3);
        curp += 3;

        ( *length ) = 24;

    }

    return 0;
}

int build_dlmap (const dl_map_msg* const dlmap, u_char* payload, int* length)
{
    // the memory for the dl map is allocated when build the map, since the map usually needs unknown
    // bandwidth, so we could first allocate 500 bytes, if it is not enough, then we could reallocate the memory
    int n = 0;
    int i = 0;
    dl_map_ie* dl_ie;
    generic_mac_hdr *gmh;
    int is_left = 0, nibble_left = 0;
    int hdr_len = 0;
    //int crc_len = 0;
    n++;
    //payload = (u_char*) mm_malloc(n, MAP_ALLOC_LENGTH);

    u_int8_t cid_num;
    normal_diuc_ie* normal_ie;
    extended_diuc_ie* extended_ie;
    u_char* cur_p;
    // management message type
    cur_p = payload + GENERIC_MAC_HEADER_LEN;
    ( *length ) = GENERIC_MAC_HEADER_LEN;
    ( *cur_p ) = ( u_int8_t ) (dlmap->manage_msg_type);
    ( *length )++;

    // frame duration code
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->frame_duration_code);
    ( *length )++;

    // frame number
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->frame_number >> 16 ) & 0x000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->frame_number >> 8 ) & 0x000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->frame_number & 0x000000ff);
    ( *length )++;

    // dcd count
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->dcd_count);
    ( *length )++;

    // base station id
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->bs_id >> 40);
    ( *length )++;

    // bs_id
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->bs_id >> 32 ) & 0x0000000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->bs_id >> 24 ) & 0x0000000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->bs_id >> 16 ) & 0x0000000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) ( ( dlmap->bs_id >> 8 ) & 0x0000000000ff);
    ( *length )++;
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->bs_id & 0x0000000000ff);
    ( *length )++;

    // No. OFDMA symbols
    cur_p++;
    ( *cur_p ) = ( u_int8_t ) (dlmap->ofdma_symbols_num);
    ( *length )++;

    dl_ie = dlmap->ie_head;

    while (dl_ie)
    {
    //printf("Now is_left: %d, length: %d\n", is_left, (*length));
        if (is_left)
        {
            ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) (dl_ie->diuc);
            ( *length )++;
            cur_p++;

            if (dl_ie->diuc == 14)
            {
                // build the extended-2 DIUC ie
                extended_ie = dl_ie->extended_ie;
                (*cur_p) = (u_int8_t)((extended_ie->extended_diuc << 4) & 0xf0);
                (*cur_p) = (*cur_p) + (u_int8_t)((extended_ie->length & 0xf0) >> 4);
                (*length)++;
                cur_p++;

                // Length is 8 bits for DIUC = 14 (Ext 2 IEs)
                (*cur_p) = (u_int8_t)((extended_ie->length & 0x0f) << 4);
                mimo_dl_basic_ie *mdbi = NULL;
                region_attri *ra = NULL;

                switch(extended_ie->extended_diuc)
                {
                    case MIMO_DL_BASIC_IE:
                        mdbi = extended_ie->unspecified_data;
                        (*cur_p) = (*cur_p) + (u_int8_t)(mdbi->num_region & 0x0f);
                        (*length)++;
                        cur_p++;
                        ra = mdbi->region_header;
                        nibble_left = 0;
                        for(i = 0; i <= mdbi->num_region; i++)
                        {
                            build_mdbi(ra, &cur_p, length, nibble_left);
                            if (nibble_left == 0)
                            {
                                nibble_left = 1;
                            }
                            else
                            {
                                nibble_left = 0;
                            }
                            ra = ra->next;
                        }
                        if (ra != NULL)
                        {
                            FLOG_ERROR("Error: Number of region attribute sections exceeds num_region \n");
                        }
                        if (nibble_left)
                        {
                            //Pad to the nearest byte boundary
                            (*length)++;
                        }
                        is_left = 0;
                        break;
                    default:
                        FLOG_ERROR("Unknown DIUC in Ext2 DIUC IE\n");
                }

            }
            else if (dl_ie->diuc == 15)
            {
                // build the extended DIUC ie
                extended_ie = dl_ie->extended_ie;
                    ( *cur_p ) = ( u_int8_t ) ( ( extended_ie->extended_diuc  << 4) & 0xf0);
                    ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) (extended_ie->length);
                    ( *length )++;

                is_left = 0;
                stc_dl_zone_ie *stc_ie = NULL;

                switch(extended_ie->extended_diuc)
                {
                    case CID_SWITCH_IE:
                        // nothing more to do for CID switch ie as data part is NULL
                    break;
                    case STC_ZONE_IE:
                        stc_ie = extended_ie->unspecified_data;
                        if(stc_ie == NULL)
                        {
                            FLOG_ERROR("Error: STC DL IE is empty\n");
                            return -1;
                        }

                        cur_p++;
                        (*cur_p) = (u_int8_t)(stc_ie->ofdma_symbol_offset & 0xff);
                        cur_p++;
                        (*length)++;
                      
                        (*cur_p) = (u_int8_t)(((stc_ie->permutation & 0x03) << 6) + \
                                   ((stc_ie->use_all_sc_indicator & 0x01) << 5) + \
                                   ((stc_ie->stc & 0x03) << 3) + \
                                   ((stc_ie->matrix_indicator & 0x03) << 1) +\
                                   ((stc_ie->dl_permbase & 0x10) >> 4));
                        cur_p++;
                        (*length)++;

                        (*cur_p) = (u_int8_t)(((stc_ie->dl_permbase & 0x0f) << 4) + \
                                   ((stc_ie->prbs_id & 0x03) << 2) + \
                                   (stc_ie->amc_type & 0x03));
                        cur_p++;
                        (*length)++;

                        // Includes last 4 reserved bits set to 0
                        (*cur_p) = (u_int8_t)(((stc_ie->midamble_presence & 0x01) << 7) + \
                                   ((stc_ie->midamble_boosting & 0x01) << 6) +\
                                   ((stc_ie->num_antenna_select & 0x01) << 5) +\
                                   ((stc_ie->dedicated_pilots & 0x01) << 4));
                        (*length)++;
                    is_left = 0;
                    break;
                    default:
                        FLOG_ERROR("\nUnknown Ext DIUC in mac_headermsg_builder.c\n");
                    break;
                }

            }
            else
            {
                normal_ie = dl_ie->normal_ie;
                if (normal_ie == NULL)
                {
                    FLOG_ERROR("Error in mac_headermsg_builder: NULL DLMAP-IE\n");
                    return -1;
                }
                if (INC_CID == 1)
                {
                    // now begin another 4 bits
                    cid_num = normal_ie->n_cid;
                    ( *cur_p ) = cid_num;
                    ( *length )++;
                    cur_p++;
                    //(*cur_p) = (u_int8_t) ((cid_num) <<4);

                    // now begin to insert the cids
                    for (i = 0; i < cid_num; i++)
                    {
                        // build the cid
                        ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->cid[i] ) >> 8);
                        ( *length )++;
                        cur_p++;
                        ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->cid[i] )
                            & 0x00ff);
                        ( *length )++;
                        cur_p++;
                        //(*cur_p) = (u_int8_t) (((normal_ie->cid[i]) & 0x000f) <<4);
                    }
                }

                // ofdma symbol offset
                ( *cur_p ) = ( u_int8_t ) (normal_ie->ofdma_symbol_offset);
                ( *length )++;
                cur_p++;

#if(DL_PERMUTATION_TYPE == 0)
                // now begin subchannel offset
                ( *cur_p ) = ( u_int8_t ) (normal_ie->subchannel_offset << 2);

                // now begin boosting
                ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) (normal_ie->boosting
                    >> 1);
                ( *length )++;
                cur_p++;

                ( *cur_p ) = ( u_int8_t ) (normal_ie->boosting << 7);
                // now begin no of ofdma symbols
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->ofdma_Symbols_num);
                ( *length )++;
                cur_p++;

                // no of subchannels
                ( *cur_p ) = ( u_int8_t ) (normal_ie->subchannels_num << 2);
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->repetition_coding_indication);
                is_left = 0;
                ( *length )++;
#endif
#if(DL_PERMUTATION_TYPE==2)
                // now begin subchannel offset
                ( *cur_p ) = ( u_int8_t ) (normal_ie->subchannel_offset);
                ( *length )++;
                cur_p++;

                // now begin boosting
                ( *cur_p ) = (( u_int8_t ) (normal_ie->boosting << 5)) & 0xF8;

                // now begin no of ofdma symbols
                ( *cur_p ) = ( *cur_p )
                    + ((( u_int8_t ) (normal_ie->ofdma_triple_symbol_num)) & 0x1F);
                ( *length )++;
                cur_p++;

                // no of subchannels
                ( *cur_p ) = (( u_int8_t ) (normal_ie->subchannels_num << 2)) & 0xFC;
                ( *cur_p ) = ( *cur_p )
                    + ((( u_int8_t ) (normal_ie->repetition_coding_indication)) & 0x03);
                is_left = 0;
                ( *length )++;
#endif
            }
        }
        else //is_left
        {
            cur_p++;
            // diuc, only occupy 4 bits
            ( *cur_p ) = ( u_int8_t ) ( ( dl_ie->diuc  << 4) & 0xf0);

            if (dl_ie->diuc == 14)
            {
                // build the extended-2 DIUC ie
                // Has 4 bits of Ext2 DIUC and 8 bits of length
                // build the extended-2 DIUC ie
                extended_ie = dl_ie->extended_ie;
                (*cur_p) = (u_int8_t)((*cur_p) + (extended_ie->extended_diuc & 0x0f));
                (*length)++;
                cur_p++;

                // Length is 8 bits for DIUC = 14 (Ext 2 IEs)
                (*cur_p) = (u_int8_t)(extended_ie->length & 0xff);
                (*length)++;
                cur_p++;

                mimo_dl_basic_ie *mdbi = NULL;
                region_attri *ra = NULL;

                switch(extended_ie->extended_diuc)
                {
                    case MIMO_DL_BASIC_IE:
                        mdbi = extended_ie->unspecified_data;
                        (*cur_p) = (u_int8_t)((mdbi->num_region & 0x0f) << 4);
                        ra = mdbi->region_header;
                        nibble_left = 1;
                        for(i = 0; i <= mdbi->num_region; i++)
                        {
                            build_mdbi(ra, &cur_p, length, nibble_left);
                            if (nibble_left == 0)
                            {
                                nibble_left = 1;
                            }
                            else
                            {
                                nibble_left = 0;
                            }
                            ra = ra->next;
                        }
                        if (ra != NULL)
                        {
                            FLOG_ERROR("Error: Number of region attribute sections exceeds num_region \n");
                        }
                        if (nibble_left)
                        {
                            //Pad to the nearest byte boundary
                            (*length)++;
                        }
                        is_left = 0;
                        break;
                    default:
                        FLOG_ERROR("Unknown DIUC in Ext2 DIUC IE\n");
                }
            }
            else if (dl_ie->diuc == 15)
            {
                // build the extended DIUC ie - 4 bits of Ext DIUC and 4 bits 
                // of length are a part of all extended DIUCs (Table 322)
                extended_ie = dl_ie->extended_ie;
                ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) (extended_ie->extended_diuc);
                ( *length )++;
                cur_p++;   

                ( *cur_p ) = ( u_int8_t ) ( ( extended_ie->length  << 4) & 0xf0); 
                is_left = 1;
                stc_dl_zone_ie *stc_ie;

                switch(extended_ie->extended_diuc)
                {
                    case CID_SWITCH_IE:
                        // nothing more to do for CID switch ie as data part is NULL
                    break;
                    case STC_ZONE_IE:
                        stc_ie = extended_ie->unspecified_data;
                        // is_left = 1, hence 4 bits left from prev field
                        (*cur_p) = (u_int8_t)((*cur_p) + ((stc_ie->ofdma_symbol_offset & 0xf0) >> 4));
                        cur_p++;
                        (*length)++;
 
                        (*cur_p) = (u_int8_t)(((stc_ie->ofdma_symbol_offset & 0x0f) << 4) + \
                                   ((stc_ie->permutation & 0x03) << 2) + \
                                   ((stc_ie->use_all_sc_indicator & 0x01) << 1) + \
                                   ((stc_ie->stc & 0x02) >> 1));

                        cur_p++;
                        (*length)++;

                        (*cur_p) = (u_int8_t)(((stc_ie->stc & 0x01) << 7) + \
                                   ((stc_ie->matrix_indicator & 0x03) << 5) +\
                                   (stc_ie->dl_permbase & 0x1f));
                        cur_p++;
                        (*length)++;

                        (*cur_p) = (u_int8_t)(((stc_ie->prbs_id & 0x03) << 6) + \
                                   ((stc_ie->amc_type & 0x03) << 4) + \
                                   ((stc_ie->midamble_presence & 0x01) << 3) + \
                                   ((stc_ie->midamble_boosting & 0x01) << 2) + \
                                   ((stc_ie->num_antenna_select & 0x01) << 1) + \
                                   (stc_ie->dedicated_pilots & 0x01));
                        cur_p++;
                        (*length)++;

                        (*cur_p) = 0; // reserved 4 bits;
                        is_left = 1;
                    break;
                    default:
                        FLOG_ERROR("\nUnknown Ext DIUC in mac_headermsg_builder.c\n");
                    break;
                } 

            }
            else
            {
                normal_ie = dl_ie->normal_ie;
                if (normal_ie == NULL)
                {
                    FLOG_ERROR("Error in mac_headermsg_builder: NULL DLMAP-IE\n");
                    return -1;
                }
                if (INC_CID == 1)
                {
                    // now begin another 4 bits
                    cid_num = normal_ie->n_cid;
                    ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) ( ( cid_num ) >> 4);
                    ( *length )++;
                    cur_p++;
                    ( *cur_p ) = ( u_int8_t ) ( ( cid_num ) << 4);

                    // now begin to insert the cids
                    for (i = 0; i < cid_num; i++)
                    {
                        // build the cid
                        ( *cur_p ) = ( *cur_p )
                            + ( u_int8_t ) ( ( normal_ie->cid[i] ) >> 12);
                        ( *length )++;
                        cur_p++;
                        ( *cur_p ) = ( u_int8_t ) ( ( ( normal_ie->cid[i] )
                            >> 4 ) & 0x00ff);
                        ( *length )++;
                        cur_p++;
                        ( *cur_p ) = ( u_int8_t ) ( ( ( normal_ie->cid[i] )
                            & 0x000f ) << 4);
                    }
                }

                // ofdma symbol offset
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) ( ( normal_ie->ofdma_symbol_offset ) >> 4);
                ( *length )++;
                cur_p++;
                ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->ofdma_symbol_offset )
                    << 4);

#if(DL_PERMUTATION_TYPE==0)
                // now begin subchannel offset
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->subchannel_offset >> 2);
                ( *length )++;
                cur_p++;
                ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->subchannel_offset )
                    << 6);

                // now begin boosting
                ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) ( ( normal_ie->boosting
                    & 0x7 ) << 3);

                // now begin no of ofdma symbols
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->ofdma_Symbols_num >> 4);
                ( *length )++;
                cur_p++;
                ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->ofdma_Symbols_num )
                    << 4);

                // no of subchannels
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->subchannels_num >> 2);
                ( *length )++;
                cur_p++;
                ( *cur_p )
                    = ( u_int8_t ) ( ( normal_ie->subchannels_num ) << 6);
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->repetition_coding_indication
                        << 4);
                // still 4 bits left
                is_left = 1;
                //(*length)++;
#endif
#if(DL_PERMUTATION_TYPE==2)
                // now begin subchannel offset
                ( *cur_p ) = ( *cur_p )
                    + ((( u_int8_t ) (normal_ie->subchannel_offset >> 4)) & 0x0f);
                ( *length )++;
                cur_p++;
                ( *cur_p ) = (( u_int8_t ) ( ( normal_ie->subchannel_offset )
                    << 4)) & 0xf0;

                // now begin boosting
                ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) ( ( normal_ie->boosting
                    & 0x07 ) << 1);

                // now begin no of ofdma symbols
                ( *cur_p ) = ( *cur_p )
                    + ((( u_int8_t ) (normal_ie->ofdma_triple_symbol_num >> 4)) & 0x01);
                ( *length )++;
                cur_p++;
                ( *cur_p ) = ( u_int8_t ) ( ( normal_ie->ofdma_triple_symbol_num )
                    << 4);

                // no of subchannels
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->subchannels_num >> 2);
                ( *length )++;
                cur_p++;
                ( *cur_p )
                    = ( u_int8_t ) ( ( normal_ie->subchannels_num ) << 6);
                ( *cur_p ) = ( *cur_p )
                    + ( u_int8_t ) (normal_ie->repetition_coding_indication
                        << 4);
                // still 4 bits left
                is_left = 1;
                //(*length)++;

#endif

            }
        }

        dl_ie = dl_ie->next;
    }
    if (is_left)
        ( *length )++;

    // add the generic mac header
    gmh = (generic_mac_hdr*) malloc (sizeof(generic_mac_hdr));
    memset (gmh, 0, sizeof(generic_mac_hdr));
    gmh->ht = 0;
    gmh->ec = 0;
    gmh->ci = 1;
    gmh->eks = 0;
    gmh->esf = 0;
    gmh->rsv = 0;
    gmh->cid = BROADCAST_CID;
    gmh->len = ( *length ) + MAC_CRC_LEN;
    gmh->type = 0;
    build_gmh (gmh, payload, &hdr_len);
    free (gmh);
    hdr_len--;
    hcs_calculation (payload, ( hdr_len - 1 ), ( payload + hdr_len - 1 ));

    // add the crc checksum
    crc_calculation (payload, ( *length ), ( payload + ( *length ) ));
    //printf("In mac_headermsg_builder, build_dlmap: Original payload ptr: %x, is_left: %d, total length: %d", payload, is_left, (*length) + MAC_CRC_LEN);

    ( *length ) = ( *length ) + MAC_CRC_LEN;

    return 0;
}
/*
 int build_ulmap(const ul_map_msg* const ulmap, u_char* payload, int* length){

 return 0;
 }

 */

int build_mdbi(region_attri *ra, u_char** cur_ptr, int *length, int nibble_left)
{
    int jj = 0;
    u_char *cur_p = *cur_ptr;
    if (ra == NULL)
    {
        FLOG_ERROR("Error: Number of region attribute sections is less than num_region\n");
        return -1;
    }

    if (nibble_left)
    {
        (*cur_p) = (u_int8_t)((ra->ofdma_symbol_offset & 0xf0) >> 4);
        (*length)++;
        cur_p++;

        // Currently only PUSC (permutation = 0) is supported
        (*cur_p) = (u_int8_t)(((ra->ofdma_symbol_offset & 0x0f) << 4) + ((ra->subchannel_offset & 0x3c) >> 2));
        (*length)++;
        cur_p++;

        (*cur_p) = (u_int8_t)(((ra->subchannel_offset & 0x03) << 6) + ((ra->boosting & 0x07) << 3) + ((ra->ofdma_symbols_num & 0x70) >> 4));
        (*length)++;
        cur_p++;

        (*cur_p) = (u_int8_t)(((ra->ofdma_symbols_num & 0x0f) << 4) + ((ra->subchannels_num & 0x3c) >> 2)); 
        (*length)++;
        cur_p++;

        // Includes 2 reserved bits set to 0
        (*cur_p) = (u_int8_t)(((ra->subchannels_num & 0x03) << 6) + ((ra->matrix_indicator & 0x03) << 4) + ((ra->num_layer & 0x03) << 2));
        (*length)++;
        layer_attri *la = ra->layer_header;

        for (jj = 0; jj <= ra->num_layer; jj++)
        {
            cur_p++;
            if (la == NULL)
            {
                FLOG_ERROR("Error: layer attribute is NULL\n");
                return -1;
            }
            if (INC_CID == 1)
            {
                (*cur_p) = (u_int8_t)((la->cid & 0xff00) >> 8);
                (*length)++;
                cur_p++;
                        
                (*cur_p) = (u_int8_t)(la->cid & 0x00ff);
                (*length)++;
                cur_p++;

            }
            (*cur_p) = (u_int8_t)(((la->layer_index & 0x03) << 6) + ((la->diuc & 0x0f) << 2) + (la->repetition_coding_indication & 0x03));
            (*length)++;
            la = la->next; 
        }
        ra = ra->next;
    }
    else
    {
        (*cur_p) = (u_int8_t)ra->ofdma_symbol_offset;
        (*length)++;
        cur_p++;

        // Currently only PUSC (permutation = 0) is supported
        (*cur_p) = (u_int8_t)(((ra->subchannel_offset & 0x3f) << 2) + ((ra->boosting & 0x06) >> 1));
        (*length)++;
        cur_p++;

        (*cur_p) = (u_int8_t)(((ra->boosting & 0x01) << 7) + (ra->ofdma_symbols_num & 0x7f));
        (*length)++;
        cur_p++;

        (*cur_p) = (u_int8_t)(((ra->subchannels_num & 0x3f) << 2) + (ra->matrix_indicator & 0x03));
        (*length)++;
        cur_p++;

        // Includes 2 reserved bits set to 0
        (*cur_p) = (u_int8_t)((ra->num_layer & 0x03) << 6);
        layer_attri *la = ra->layer_header;

        for (jj = 0; jj <= ra->num_layer; jj++)
        {
            if (la == NULL)
            {
                FLOG_ERROR("Error: layer attribute is NULL\n");
                return -1;
            }
            if (INC_CID == 1)
            {
                (*cur_p) = (u_int8_t)((*cur_p) + ((la->cid & 0xf000) >> 12));
                (*length)++;
                cur_p++;
                        
                (*cur_p) = (u_int8_t)((la->cid & 0x0ff0) >> 4);
                (*length)++;
                cur_p++;

                (*cur_p) = (u_int8_t)((la->cid & 0x000f) << 4);
            }
            (*cur_p) = (u_int8_t)((*cur_p) + ((la->layer_index & 0x03) << 2) + ((la->diuc & 0x0c) >> 2));
            (*length)++;
            cur_p++;
    
            (*cur_p) = (u_int8_t)(((la->diuc & 0x03) << 6) + ((la->repetition_coding_indication & 0x03) << 4));
            la = la->next;
        }
        ra = ra->next;
    }
    *cur_ptr = cur_p;
    return 0;
}
