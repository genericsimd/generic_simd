/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_headermsg_parser.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_headermsg_parser.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "mac_system_info.h"
#include "mac_config.h"
#include "ul_map_align.h"
#include "ulmap.h"
#include "init_maps.h"
#include "ranging_mm.h"
#include "mac_bs_ranging_utm.h"
#include "mac_hcs.h"
#include "mac_crc.h"
#include "mac_amc.h"

extern sll_fifo_q *ranging_q;
extern sll_fifo_q *bs_ranging_test_q;

int mem_left_shitf (void * p_mem_dst, void * p_mem_src, unsigned int len,
        unsigned int bits, unsigned int drop_tail)
{
    u_int8_t * p_tmp_src = (u_int8_t *) p_mem_src;
    u_int8_t * p_tmp_dst = (u_int8_t *) p_mem_dst;

    int byte_idx;

    if ( ( p_tmp_src == NULL ) || ( p_tmp_dst == NULL ) || ( len == 0 )
            || ( bits >= 8 ))
    {
        FLOG_ERROR ("error input in mem_left_shitf.\n");
        return 1;
    }

    for (byte_idx = 0; byte_idx < len - 1; byte_idx++)
    {
        p_tmp_dst[byte_idx] = ( p_tmp_src[byte_idx] << bits )
                | ( p_tmp_src[byte_idx + 1] >> ( 8 - bits ) );

        //        printf(" /%x %x/ ", p_tmp_dst[byte_idx], p_tmp_src[byte_idx]);
    }

    if (drop_tail == 0)
    {
        p_tmp_dst[byte_idx] = ( p_tmp_src[byte_idx] << bits ) & ( 0xff << bits );

        //        printf(" /%x %x/ \n", p_tmp_dst[byte_idx], p_tmp_src[byte_idx]);
    }

    return 0;

}

int parse_ul_map_msg (u_int8_t * const p_payload, u_int32_t const payload_len,
        ul_map_msg * p_msg_header)
{
    int32_t length = (int32_t) payload_len;
    u_int8_t * p_payload_tmp = p_payload;
    ul_map_ie * p_ie_tmp = NULL;
    ul_map_ie * p_previous = NULL;
    u_int8_t uiuc;
    u_int32_t tmp_idx = 0;

    char ie_buf[MAX_IE_LENGTH];
    //char * p_ie_buf = ie_buf;
    unsigned int ie_len = 0;

    unsigned int tail_bits = 0;

    struct ul_map_msg_align * p_ul_map_align = NULL;
    struct ul_map_msg_ie_align * p_ul_map_msg_ie_align = NULL;
    struct ul_ext_2_ie_align * p_ul_ext_2_ie_align = NULL;
    struct ul_map_uiuc_12_align * p_ul_map_uiuc_12_align = NULL;
    struct zone_alloc_ie_align * p_zone_alloc_ie_align = NULL;
    struct cdma_alloc_ie_align * p_cdma_alloc_ie_align = NULL;
    struct ul_ext_ie_align * p_ul_ext_ie_align = NULL;
    struct fast_ack_alloc_ie_align * p_fast_ack_alloc_ie_align = NULL;
    struct unknown_uiuc_ie_align * p_unknown_uiuc_ie_align = NULL;
    struct unknown_uiuc_2_ie_align * p_unknown_uiuc_2_ie_align = NULL;

    if ( ( p_msg_header == NULL ) || ( p_payload == NULL ) || ( payload_len
            < sizeof(struct ul_map_msg_align) ))
    {
        FLOG_ERROR ("in parse_ul_map_msg: input pointer is NULL.\n");
        return 1;
    }

    p_ul_map_align = (struct ul_map_msg_align *) p_payload_tmp;
    p_msg_header->manage_msg_type = p_ul_map_align->mgt_msg_t;
    //    p_ul_map_align->fdd_change_flg;
    //    p_ul_map_align->rsv = 0;
    p_msg_header->ucd_count = p_ul_map_align->ucd_count;

    /* Use self implementation order convert in future*/
    p_msg_header->alloc_start_time = ntohl (p_ul_map_align->alloc_start_time);
    p_msg_header->num_ofdma_sym = p_ul_map_align->ofdma_symbols_num;

    p_payload_tmp += sizeof(struct ul_map_msg_align);
    length -= sizeof(struct ul_map_msg_align);

    if (length == 0)
    {
        p_msg_header->ulmap_ie_num = 0;
        p_msg_header->ie = NULL;

        return 0;
    }

    p_msg_header->ulmap_ie_num = 0;

    p_ie_tmp = malloc (sizeof(ul_map_ie));

    if (p_ie_tmp == NULL)
    {
        FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
    }

    memset(p_ie_tmp, 0, sizeof(ul_map_ie));

    init_ulmap_ie (p_ie_tmp);

    p_msg_header->ie = p_ie_tmp;
    p_msg_header->ulmap_ie_num++;

    while (1)
    {
        p_ie_tmp->ie_index = tmp_idx;
        tmp_idx++;

        if (tail_bits != 0)
        {
            mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                    sizeof(struct ul_map_msg_ie_align), tail_bits, 0);
        }
        else
        {
            memcpy ((void *) ie_buf, (void *) p_payload_tmp,
                    sizeof(struct ul_map_msg_ie_align));
        }

        p_ul_map_msg_ie_align = (struct ul_map_msg_ie_align *) ie_buf;
        p_ie_tmp->cid = ntohs (p_ul_map_msg_ie_align->cid);
        uiuc = p_ul_map_msg_ie_align->uiuc;
        p_ie_tmp->uiuc = uiuc;

        p_payload_tmp += sizeof(struct ul_map_msg_ie_align) - 1;
        length -= sizeof(struct ul_map_msg_ie_align) - 1;

        if (uiuc == 11)
        {
            ie_len = sizeof(struct ul_ext_2_ie_align);

            if (tail_bits != 0)
            {
                ie_len++;
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);
                ie_len--;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);
            }

            extended_uiuc_ie * p_tmp = malloc (sizeof(extended_uiuc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset (p_tmp, 0, sizeof(extended_uiuc_ie));

            p_ul_ext_2_ie_align = (struct ul_ext_2_ie_align *) ie_buf;
            //            p_ie_tmp->uiuc = p_ul_ext_2_ie_align->uiuc;
            p_tmp->extended_uiuc = p_ul_ext_2_ie_align->ext_2_uiuc;
            p_tmp->length = p_ul_ext_2_ie_align->len;
            /* memcpy and padding */

            p_ie_tmp->uiuc_extend_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
#if 1
            ie_len = p_tmp->length;

            if (tail_bits != 0)
            {
                ie_len++;
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);
                ie_len--;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);
            }

            struct mubi_align *p_mubi_align = NULL;
            mimo_ul_basic_ie *p_mubi = NULL;
            assigned_burst_attri *aba = NULL;
            switch (p_tmp->extended_uiuc)
            {
                case MIMO_UL_BASIC_IE:
                    p_mubi_align = (struct mubi_align*) ie_buf;

                    p_mubi = (mimo_ul_basic_ie*) malloc (
                            sizeof(mimo_ul_basic_ie));

                    memset(p_mubi, 0, sizeof(mimo_ul_basic_ie));

                    p_ie_tmp->uiuc_extend_ie->unspecified_data = p_mubi;
                    // Only 1 assigned burst supported
                    p_mubi->num_assign = p_mubi_align->num_assign;

                    aba = (assigned_burst_attri*) malloc (
                            sizeof(assigned_burst_attri));

                    memset(aba, 0, sizeof(assigned_burst_attri));

                    p_mubi->assigned_burst_header = aba;
                    aba->collaborative_sm_indication
                            = p_mubi_align->collab_sm_ind;
                    aba->cid = ( p_mubi_align->cid_h << 13 )
                            + ( p_mubi_align->cid_m << 5 )
                            + p_mubi_align->cid_l;
                    aba->uiuc = ( p_mubi_align->uiuc_h << 1 )
                            + p_mubi_align->uiuc_l;
                    aba->repetition_coding_indication
                            = p_mubi_align->rep_coding_ind;
                    aba->mimo_control = p_mubi_align->mimo_control;

                    aba->duration = ( p_mubi_align->duration_h << 6 )
                            + p_mubi_align->duration_l;
                    aba->next = NULL;

                    p_payload_tmp += ie_len;
                    length -= ie_len;

                    break;
                default:
                    FLOG_WARNING ("Unsupported extended 2 UIUC\n");
                    return -1;

            }
#endif
        }
        else if (uiuc == 12)
        {
            ie_len = sizeof(struct ul_map_uiuc_12_align);

            if (tail_bits != 0)
            {
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);
                tail_bits = 0;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);
                tail_bits = 4;
                ie_len--;
            }

            uiuc12_ie * p_tmp = malloc (sizeof(uiuc12_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset (p_tmp, 0, sizeof(uiuc12_ie));

            p_ul_map_uiuc_12_align = (struct ul_map_uiuc_12_align *) ie_buf;

            //            p_ie_tmp->uiuc = p_ul_map_uiuc_12_align->uiuc;

            p_tmp->ofdma_symbol_offset
                    = ( p_ul_map_uiuc_12_align->ofdma_offset_h << 4 )
                            | p_ul_map_uiuc_12_align->ofdma_offset_l;

            p_tmp->subchannel_offset
                    = ( p_ul_map_uiuc_12_align->subchannel_offset_h << 3 )
                            | p_ul_map_uiuc_12_align->subchannel_offset_l;

            p_tmp->ofdma_symbol_num
                    = ( p_ul_map_uiuc_12_align->ofdma_symbols_num_h << 2 )
                            | p_ul_map_uiuc_12_align->ofdma_symbols_num_l;

            p_tmp->subchannel_num = ( p_ul_map_uiuc_12_align->subchannels_num_h
                    << 1 ) | p_ul_map_uiuc_12_align->subchannels_num_l;

            p_tmp->ranging_method = p_ul_map_uiuc_12_align->ranging_method;

            p_tmp->dedicated_ranging_indicator
                    = p_ul_map_uiuc_12_align->ranging_ind;

            p_ie_tmp->uiuc_12_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
        }
        else if (uiuc == 13)
        {
            ie_len = sizeof(struct zone_alloc_ie_align);

            if (tail_bits != 0)
            {
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);

                tail_bits = 0;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);
                tail_bits = 4;
                ie_len--;
            }

            paprreduc_safezone_alloc_ie * p_tmp = malloc (
                    sizeof(paprreduc_safezone_alloc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset(p_tmp, 0, sizeof(paprreduc_safezone_alloc_ie));

            p_zone_alloc_ie_align = (struct zone_alloc_ie_align *) ie_buf;

            //            p_ie_tmp->uiuc = p_zone_alloc_ie_align->uiuc;

            p_tmp->ofdma_symbol_offset
                    = ( p_zone_alloc_ie_align->ofdma_offset_h << 4 )
                            | p_zone_alloc_ie_align->ofdma_offset_l;

            p_tmp->subchannel_offset
                    = ( p_zone_alloc_ie_align->subchannel_offset_h << 3 )
                            | p_zone_alloc_ie_align->subchannel_offset_l;

            p_tmp->ofdma_symbol_num
                    = ( p_zone_alloc_ie_align->ofdma_symbols_num_h << 2 )
                            | p_zone_alloc_ie_align->ofdma_symbols_num_l;

            p_tmp->subchannel_num_szshift_value
                    = ( p_zone_alloc_ie_align->subchannels_shift_num_h << 1 )
                            | p_zone_alloc_ie_align->subchannels_shift_num_l;

            p_tmp->paprreduc_safezone = p_zone_alloc_ie_align->papr_zone;
            p_tmp->sounding_zone = p_zone_alloc_ie_align->sounding_zone;

            p_ie_tmp->uiuc_13_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
        }
        else if (uiuc == 14)
        {
            ie_len = sizeof(struct cdma_alloc_ie_align);

            if (tail_bits != 0)
            {
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);
                tail_bits = 0;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);

                tail_bits = 4;
                ie_len--;
            }

            cdma_alloc_ie * p_tmp = malloc (sizeof(cdma_alloc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset(p_tmp, 0, sizeof(cdma_alloc_ie));

            p_cdma_alloc_ie_align = (struct cdma_alloc_ie_align *) ie_buf;

            //            p_ie_tmp->uiuc = p_cdma_alloc_ie_align->uiuc;
            p_tmp->duration = ( p_cdma_alloc_ie_align->duration_h << 2 )
                    | p_cdma_alloc_ie_align->duration_l;

            p_tmp->uiuc = p_cdma_alloc_ie_align->uiuc_tx;

            p_tmp->repetition_coding_indication
                    = p_cdma_alloc_ie_align->rep_coding_ind;

            p_tmp->frame_num_index = p_cdma_alloc_ie_align->frame_num_idx;

            p_tmp->ranging_code = ( p_cdma_alloc_ie_align->ranging_code_h << 4 )
                    | p_cdma_alloc_ie_align->ranging_code_l;

            p_tmp->ranging_symbol = ( p_cdma_alloc_ie_align->ranging_symbol_h
                    << 4 ) | p_cdma_alloc_ie_align->ranging_symbol_l;

            p_tmp->ranging_subchannel
                    = ( p_cdma_alloc_ie_align->ranging_subchannel_h << 3 )
                            | p_cdma_alloc_ie_align->ranging_subchannel_l;

            p_tmp->bw_request_mandatory = p_cdma_alloc_ie_align->bw_req_m;

            // Enqueue the CDMA alloc IE in the ranging queue
            cdma_ranging_ie *cdma_rng_ie = (cdma_ranging_ie*)malloc(sizeof(cdma_ranging_ie));

            memset(cdma_rng_ie, 0, sizeof(cdma_ranging_ie));

            cdma_rng_ie->cdma_ie = malloc(sizeof(cdma_alloc_ie));

            memset(cdma_rng_ie->cdma_ie, 0, sizeof(cdma_alloc_ie));

            memcpy(cdma_rng_ie->cdma_ie, p_tmp, sizeof(cdma_alloc_ie));

            cdma_rng_ie->ucd_count = p_ul_map_align->ucd_count;
			#ifdef SS_RX			
				sll_fifo_q_enqueue(ranging_q, cdma_rng_ie, sizeof(cdma_ranging_ie), CDMA_ALLOC_UIUC);
			#else
				#ifdef RANGING_TEST
					sll_fifo_q_enqueue(bs_ranging_test_q, cdma_rng_ie, sizeof(cdma_ranging_ie), CDMA_ALLOC_UIUC);
				#endif
			#endif
            p_ie_tmp->uiuc_14_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
        }
        else if (uiuc == 15)
        {
            ie_len = sizeof(struct ul_ext_ie_align);

            if (tail_bits != 0)
            {
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);

                tail_bits = 0;
            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);

                tail_bits = 4;
                ie_len--;
            }

            extended_uiuc_ie * p_tmp = malloc (sizeof(extended_uiuc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset(p_tmp, 0, sizeof(extended_uiuc_ie));

            p_ul_ext_ie_align = (struct ul_ext_ie_align *) ie_buf;
            //            p_ie_tmp->uiuc = p_ul_ext_ie_align->uiuc;
            p_tmp->extended_uiuc = p_ul_ext_ie_align->ext_uiuc;
            p_tmp->length = p_ul_ext_ie_align->len;
            /* memcpy and padding */

            p_ie_tmp->uiuc_15_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
        }
        else if (uiuc == 0)
        {
            ie_len = sizeof(struct fast_ack_alloc_ie_align);

            if (tail_bits != 0)
            {
                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);

                tail_bits = 0;

            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);

                tail_bits = 4;
                ie_len--;
            }

            fastfeedback_alloc_ie * p_tmp = malloc (
                    sizeof(fastfeedback_alloc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset (p_tmp, 0, sizeof(fastfeedback_alloc_ie));

            p_fast_ack_alloc_ie_align
                    = (struct fast_ack_alloc_ie_align *) ie_buf;

            //            p_ie_tmp->uiuc = p_fast_ack_alloc_ie_align->uiuc;

            p_ie_tmp->uiuc_0_ie = p_tmp;

            p_payload_tmp += ie_len;
            length -= ie_len;
        }
        else
        {
            ie_len = sizeof(struct unknown_uiuc_ie_align);

            if (tail_bits != 0)
            {
                ie_len++;

                mem_left_shitf ((void *) ie_buf, (void *) p_payload_tmp,
                        ie_len, tail_bits, 0);

                ie_len--;

            }
            else
            {
                memcpy ((void *) ie_buf, (void *) p_payload_tmp, ie_len);
            }

            other_uiuc_ie * p_tmp = malloc (sizeof(other_uiuc_ie));

            if (p_tmp == NULL)
            {
                FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
            }

            memset(p_tmp, 0, sizeof(other_uiuc_ie));

            p_unknown_uiuc_ie_align = (struct unknown_uiuc_ie_align *) ie_buf;

            //            p_ie_tmp->uiuc = p_unknown_uiuc_ie_align->uiuc;
            p_tmp->duration = ( ( u_int16_t ) (
                    p_unknown_uiuc_ie_align->duration_h) << 6 )
                    | ( u_int16_t ) (p_unknown_uiuc_ie_align->duration_l);

            p_tmp->repetition_coding_indication
                    = p_unknown_uiuc_ie_align->rep_coding_ind;

            p_payload_tmp += ie_len;
            length -= ie_len;

            /* AAS or AMC UL Zone? */
            if (0)
            {
                p_unknown_uiuc_2_ie_align
                        = (struct unknown_uiuc_2_ie_align *) p_payload_tmp;

                p_tmp->slot_offset = ( ( u_int16_t ) (
                        p_unknown_uiuc_2_ie_align->slot_offset_h) << 4 )
                        | ( u_int16_t ) (
                                p_unknown_uiuc_2_ie_align->slot_offset_l);

                p_payload_tmp += ie_len;
                length -= ie_len;
            }

            p_ie_tmp->uiuc_other_ie = p_tmp;
        }

        if (length <= 1)
        {
            p_ie_tmp->next = NULL;
            return 0;
        }
        p_previous = p_ie_tmp;

        p_ie_tmp = malloc (sizeof(ul_map_ie));

        if (p_ie_tmp == NULL)
        {
            FLOG_FATAL ("parse_ul_map_msg: malloc ie error");
        }

        memset(p_ie_tmp, 0, sizeof(ul_map_ie));

        init_ulmap_ie (p_ie_tmp);

        p_previous->next = p_ie_tmp;
        p_msg_header->ulmap_ie_num++;
    }

    return 0;
}

int parse_psh(u_char* payload, pack_sub_hdr* psh, u_int8_t is_extend, int* length){
    u_int16_t curd;
    int offset;
    offset = 0;
   
    psh->fc = (payload[offset] & 0xc0) >> 6;

    if (is_extend)
    {
        curd = payload[offset];
        psh->fsn = ((curd & 0x3f) << 5) + ( payload[offset+1] >> 3 );
        offset++;
        curd = payload[offset];
        psh->length = ((curd &0x07) << 8) + payload[offset+1];
        (*length) = 3;
        
    }
    else 
    {
        curd = payload[offset];
        psh->fsn = (u_int8_t) (curd & 0x38) >> 3;
        psh->length = ((curd & 0x07) << 8) + payload[offset+1];
        (*length) = 2;
    }

    return 0;
}

int parse_fsh(u_char* payload, frag_sub_hdr* fsh, u_int8_t is_extend, int* length){
    u_int16_t curd;
    int offset;
    offset = 0;
   
    fsh->fc = (payload[offset] & 0xc0)>>6;

    if (is_extend)
    {
        curd = payload[offset];
        fsh->fsn = ((curd & 0x3f ) << 5) + (payload[offset+1] >> 3);
        fsh->rsv = payload[offset+1] & 0x07;
        (*length) = 2;
    }
    else
    {
        fsh->fsn = (payload[offset] & 0x38)>>3;
        fsh->rsv = payload[offset] & 0x07;
        (*length) = 1;
    }
    return 0;
}

int parse_gmh(u_char* payload, generic_mac_hdr* gmh, int* length){

    int offset;
    offset = 0;
    u_int16_t curd;
    gmh->ht  = (u_int8_t)(payload[offset] >> 7);
    gmh->ec = (u_int8_t)((payload[offset] & 0x40) >> 6);
    gmh->type = (u_int8_t) (payload[offset] & 0x3f);

    offset++;
    gmh->esf = (u_int8_t)(payload[offset] >> 7);
    gmh->ci =(u_int8_t) (payload[offset] & 0x40) >> 6;
    gmh->eks = (payload[offset] & 0x30) >> 4;
    gmh->rsv = (payload[offset] & 0x08) >> 3;

    curd = payload[offset];
    gmh->len =(u_int16_t) ( (curd & 0x07) << 8) + payload[offset+1];

    offset += 2;
    curd = payload[offset];
    gmh->cid = ((curd)<<8) +payload[offset+1];

    offset += 2;
    gmh->hcs = (u_int8_t)payload[offset];
    (*length) = 6;
    return 0;
}

int parse_dlframeprefix(u_char* payload, dl_subframe_prefix* dlp, u_int8_t is_128fft, int* length){
    int offset;
    offset = 0; 
    if (is_128fft)
    {
        dlp->used_subchannel_bitmap = payload[offset] >> 7;
        dlp->rsv1 = (payload[offset] & 0x40) >> 6;
        dlp->repetition_coding_indication = (payload[offset] & 0x30) >> 4;
        dlp->coding_indication = (payload[offset] & 0x0e) >> 1;
        dlp->dl_map_length = ((payload[offset] & 0x01) << 4) |(payload[offset+1] & 0xf0) >> 4;
        dlp->rsv2 = payload[offset+1] & 0x0f;
        (*length) = 2;
    }
    else 
    {
        dlp->used_subchannel_bitmap = payload[offset] >> 2;
        dlp->rsv1 = (payload[offset] & 0x02) >> 1;
        dlp->repetition_coding_indication = ((payload[offset] & 0x01) <<1) |(payload[offset+1] >> 7);
        offset++;
        dlp->coding_indication = ((payload[offset] & 0x70) >> 4);
        dlp->dl_map_length = ((payload[offset] & 0x0f) << 4) | (payload[offset+1] >> 4);
        dlp->rsv2 = payload[offset+1] & 0x0f;
        (*length) = 3;
    }
        
    return 0;
}

int parse_dlmap(u_char* payload, dl_map_msg* dlmap, int* length){
    int offset, nibble_left = 0; 
    offset = 0; 
    u_int16_t curd_16;
    u_int32_t curd_32;
    u_int64_t curd_64;
    dl_map_ie* pre_ie = NULL;
    dl_map_ie* ie = NULL;
    int is_left = 0;
    int i, jj = 0;
    int pdu_len;
    int temp_pdu_len;


    // first check the hcs checksum
    if (hcs_verification(payload, (GENERIC_MAC_HEADER_LEN-1), (payload + GENERIC_MAC_HEADER_LEN-1)))
    {
        // header check incorrect
        return 1;
    }

    curd_16 = payload[1];
    pdu_len = ((curd_16 & 0x07 ) <<8) +payload[2];

    temp_pdu_len = pdu_len-MAC_CRC_LEN;

    if (crc_verification(payload, temp_pdu_len, payload+temp_pdu_len))
    {
         // crc checksum is wrong
         return 1;
    }

    offset = GENERIC_MAC_HEADER_LEN;

    dlmap->manage_msg_type = payload[offset];
    offset++;

    dlmap->frame_duration_code = payload[offset];
    offset++;

    curd_32 = payload[offset];
    dlmap->frame_number = (curd_32<<16);
    offset++;

    curd_32 = payload[offset];
    dlmap->frame_number += (curd_32<<8);
    offset++;

    curd_32 = payload[offset];
    dlmap->frame_number += curd_32;
    offset++;

    dlmap->dcd_count = payload[offset];
    offset++;

    curd_64 = payload[offset];
    dlmap->bs_id= (curd_64<<40);
    offset++;

    curd_64 = payload[offset];
    dlmap->bs_id += (curd_64<<32);
    offset++;
   
    curd_64 = payload[offset];
    dlmap->bs_id += (curd_64<<24);
    offset++;
 
    curd_64 = payload[offset];
    dlmap->bs_id += (curd_64<<16);
    offset++;

    curd_64 = payload[offset];
    dlmap->bs_id += (curd_64<<8);
    offset++;

    dlmap->bs_id += payload[offset];
    offset++;

    dlmap->ofdma_symbols_num = payload[offset];
    offset++;

    while (offset < (temp_pdu_len -1))
    {
        ie = (dl_map_ie *) malloc(sizeof(dl_map_ie));
        memset(ie, 0, sizeof(dl_map_ie));
        //printf("In parser, New DL IE. is_left: %d, DIUC: %d or %d, addr: %x\n", is_left, payload[offset] & 0x0f, (payload[offset] & 0xf0) >> 4, payload+offset);
        if (is_left)
        {
            ie->diuc = (payload[offset] & 0x0f);
            offset++;
            if (ie->diuc == 14)
            {
                // build the extended-2 DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));

                ie->extended_ie->extended_diuc = (payload[offset] & 0xf0) >> 4;
                ie->extended_ie->length = (payload[offset] & 0x0f) << 4;
                offset++;
                ie->extended_ie->length += ((payload[offset] & 0xf0) >> 4);
                mimo_dl_basic_ie *mdbi = NULL;
                region_attri *ra = NULL;

                switch(ie->extended_ie->extended_diuc)
                {
                    case MIMO_DL_BASIC_IE:
                        mdbi = (mimo_dl_basic_ie*)malloc(sizeof(mimo_dl_basic_ie));

                        memset(mdbi, 0, sizeof(mimo_dl_basic_ie));

                        ie->extended_ie->unspecified_data = mdbi;
                        mdbi->num_region = (payload[offset] & 0x0f);
                        offset++;
                        nibble_left = 0;
                        ra = (region_attri*)malloc(sizeof(region_attri));
                        memset(ra, 0, sizeof(region_attri));

                        mdbi->region_header = ra;

                        for (jj = 0; jj <= mdbi->num_region; jj++)
                        {
                            parse_mdbi(ra, payload, &offset, nibble_left);
                            // Toggle nibble_left
                            if (nibble_left == 0)
                            {
                                nibble_left = 1;
                            }
                            else
                            {
                                nibble_left = 0;
                            }

                            if (jj < mdbi->num_region)
                            {
                                ra->next = (region_attri*)malloc(sizeof(region_attri));

                                memset(ra->next, 0, sizeof(region_attri));
                                ra = ra->next;
                            }
                            else
                            {
                                ra->next = NULL;
                            }
                        }
                        if (nibble_left)
                        {
                            // Pad to the next byte boundary
                            offset++;
                        }
                        is_left = 0;
                        break;
                    default:
                        FLOG_WARNING("Unknown Ext2 DIU\n");
                }

            }else if (ie->diuc == 15){
                // build the extended DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));
                ie->extended_ie->extended_diuc = (payload[offset] & 0xf0) >>4;
                   ie->extended_ie->length = (payload[offset] & 0x0f); 

                stc_dl_zone_ie *stc_ie = NULL;

                switch(ie->extended_ie->extended_diuc)
                {
                    case CID_SWITCH_IE:
                        break;
                    case STC_ZONE_IE:
                        stc_ie = (stc_dl_zone_ie*)malloc(sizeof(stc_dl_zone_ie));
                        if (stc_ie == NULL)
                        {
                            FLOG_FATAL("Failed to allocate memory for stc_ie\n");
                            return -1;
                        }

                        memset(stc_ie, 0, sizeof(stc_dl_zone_ie));

                        ie->extended_ie->unspecified_data = stc_ie;
                        offset++;
                        stc_ie->ofdma_symbol_offset = payload[offset];
                        offset++;
                        stc_ie->permutation = (payload[offset] & 0xc0) >> 6;
                        stc_ie->use_all_sc_indicator = (payload[offset] & 0x20) >> 5;
                        stc_ie->stc = (payload[offset] & 0x18) >> 3;
                        stc_ie->matrix_indicator = (payload[offset] & 0x06) >> 1;
                        stc_ie->dl_permbase = (payload[offset] & 0x01) << 4;
                        offset++;
                        stc_ie->dl_permbase += ((payload[offset] & 0xf0) >> 4);
                        stc_ie->prbs_id = (payload[offset] & 0x0c) >> 2;
                        stc_ie->amc_type = (payload[offset] & 0x03);
                        offset++;

                        stc_ie->midamble_presence = (payload[offset] & 0x80) >> 7;
                        stc_ie->midamble_boosting = (payload[offset] & 0x40) >> 6;
                        stc_ie->num_antenna_select = (payload[offset] & 0x20) >> 5;
                        stc_ie->dedicated_pilots = (payload[offset] & 0x10) >> 4;
                        break;
                }

                offset++;
                is_left = 0;
            }else {
#ifdef SS_TX
		if (prev_dl_amc != ie->diuc)
		{
			prev_dl_amc = ie->diuc;
			pthread_mutex_lock(&crc_table_lock);
			crc_error_count_table[0][0] = 0;
			packet_count[0] = 0;	
			pthread_mutex_unlock(&crc_table_lock);
		}
#endif
                ie->normal_ie = (normal_diuc_ie*) malloc(sizeof(normal_diuc_ie));
                memset(ie->normal_ie, 0, sizeof(normal_diuc_ie));
                if (INC_CID)
                {
                    ie->normal_ie->n_cid = payload[offset] ;
                    offset++;

                    ie->normal_ie->cid = (u_int16_t*) malloc(ie->normal_ie->n_cid*sizeof(u_int16_t));
                    memset(ie->normal_ie->cid, 0, ie->normal_ie->n_cid*sizeof(u_int16_t));

                    for (i=0; i<ie->normal_ie->n_cid; i++)
                    {
                        curd_16 = payload[offset] <<8;
                        ie->normal_ie->cid[i] = curd_16;
                        offset++;

                        curd_16 = payload[offset];
                        ie->normal_ie->cid[i]+= curd_16;
                        offset++;
                        
                    }
                }

                ie->normal_ie->ofdma_symbol_offset = payload[offset] ;
                offset++;

#if (DL_PERMUTATION_TYPE==0) // PUSC

                ie->normal_ie->subchannel_offset = (payload[offset] & 0xfc)>>2;

                ie->normal_ie->boosting = ((payload[offset] & 0x03)<<1) + ((payload[offset+1] & 0x80) >>7);
                offset++;
                ie->normal_ie->ofdma_Symbols_num = payload[offset] & 0x7f ;
                offset++;

                ie->normal_ie->subchannels_num = (payload[offset] & 0xfc) >>2;

                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x03);
                offset++;

#endif
#if(DL_PERMUTATION_TYPE==2) // AMC

                ie->normal_ie->subchannel_offset = payload[offset];
                offset++;

                ie->normal_ie->boosting = ((payload[offset] & 0xE0) >> 5);

                ie->normal_ie->ofdma_triple_symbol_num = payload[offset] & 0x1f ;
                offset++;

                ie->normal_ie->subchannels_num = (payload[offset] & 0xfc) >>2;

                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x03);
                offset++;


#endif
                is_left = 0;
            }
        }
        else
        {
            ie->diuc = (payload[offset] & 0xf0) >> 4;
            if (ie->diuc == 14)
            {
                // build the extended-2 DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));

                ie->extended_ie->extended_diuc = (payload[offset] & 0x0f);
                offset++;
                ie->extended_ie->length = (payload[offset] & 0xff);
                offset++;
                mimo_dl_basic_ie *mdbi = NULL;
                region_attri *ra = NULL;

                switch(ie->extended_ie->extended_diuc)
                {
                    case MIMO_DL_BASIC_IE:
                        mdbi = (mimo_dl_basic_ie*)malloc(sizeof(mimo_dl_basic_ie));
                        memset(mdbi, 0, sizeof(mimo_dl_basic_ie));

                        ie->extended_ie->unspecified_data = mdbi;
                        mdbi->num_region = (payload[offset] & 0xf0) >> 4;
                        nibble_left = 1;
                        ra = (region_attri*)malloc(sizeof(region_attri));
                        memset(ra, 0, sizeof(region_attri));

                        mdbi->region_header = ra;

                        for (jj = 0; jj <= mdbi->num_region; jj++)
                        {
                            parse_mdbi(ra, payload, &offset, nibble_left);
                            // Toggle nibble_left
                            if (nibble_left == 0)
                            {
                                nibble_left = 1;
                            }
                            else
                            {
                                nibble_left = 0;
                            }

                            if (jj < mdbi->num_region)
                            {
                                ra->next = (region_attri*)malloc(sizeof(region_attri));
                                memset(ra->next, 0, sizeof(region_attri));
                                ra = ra->next;
                            }
                            else
                            {
                                ra->next = NULL;
                            }
                        }
                        if (nibble_left)
                        {
                            // Pad to the next byte boundary
                            offset++;
                        }
                        is_left = 0;
                        break;
                    default:
                        FLOG_WARNING("Unknown Ext2 DIU\n");
                }
            }
            else if (ie->diuc == 15){
                // build the extended DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));
                ie->extended_ie->extended_diuc = (payload[offset] & 0x0f) ;
                offset++;
                   ie->extended_ie->length = (payload[offset] & 0xf0) >>4; 

                stc_dl_zone_ie *stc_ie = NULL;

                switch(ie->extended_ie->extended_diuc)
                {
                    case CID_SWITCH_IE:
                        break;
                    case STC_ZONE_IE:
                        stc_ie = (stc_dl_zone_ie*)malloc(sizeof(stc_dl_zone_ie));
                        if (stc_ie == NULL)
                        {
                            FLOG_FATAL("Failed to allocate memory for stc_ie\n");
                            return -1;
                        }
                        memset (stc_ie, 0, sizeof(stc_dl_zone_ie));
                        ie->extended_ie->unspecified_data = stc_ie;
                        stc_ie->ofdma_symbol_offset = (payload[offset] & 0x0f) << 4;
                        offset++;
                        stc_ie->ofdma_symbol_offset += ((payload[offset] & 0xf0) >> 4);

                        stc_ie->permutation = (payload[offset] & 0x0c) >> 2;
                        stc_ie->use_all_sc_indicator = (payload[offset] & 0x02) >> 1;
                        
                        stc_ie->stc = (payload[offset] & 0x01) << 1;
                        offset++;
                        stc_ie->stc += ((payload[offset] & 0x80) >> 7);

                        stc_ie->matrix_indicator = (payload[offset] & 0x60) >> 5;
                        stc_ie->dl_permbase = (payload[offset] & 0x1f);
                        offset++;

                        stc_ie->prbs_id = (payload[offset] & 0xc0) >> 6;
                        stc_ie->amc_type = (payload[offset] & 0x30) >> 4;
                        stc_ie->midamble_presence = (payload[offset] & 0x08) >> 3;
                        stc_ie->midamble_boosting = (payload[offset] & 0x04) >> 2;
                        stc_ie->num_antenna_select = (payload[offset] & 0x02) >> 1;
                        stc_ie->dedicated_pilots = (payload[offset] & 0x01);
                        offset++;

                        break;
                }

                is_left = 1;
            }else {
#ifdef SS_TX
		if (prev_dl_amc != ie->diuc)
		{
			prev_dl_amc = ie->diuc;
			pthread_mutex_lock(&crc_table_lock);
			crc_error_count_table[0][0] = 0;
			packet_count[0] = 0;	
			pthread_mutex_unlock(&crc_table_lock);
		}
#endif
                ie->normal_ie = (normal_diuc_ie*) malloc(sizeof(normal_diuc_ie));
                memset(ie->normal_ie, 0, sizeof(normal_diuc_ie));
                if (INC_CID)
                {
                    ie->normal_ie->n_cid = ((payload[offset] & 0x0f) <<4) + (payload[offset+1]>>4);
                    offset++;

                    ie->normal_ie->cid = (u_int16_t*) malloc(ie->normal_ie->n_cid*sizeof(u_int16_t));
                    memset(ie->normal_ie->cid, 0, ie->normal_ie->n_cid*sizeof(u_int16_t));

                    for (i=0; i<ie->normal_ie->n_cid; i++)
                    {
                        curd_16 = (payload[offset] & 0x0f) << 12;
                        ie->normal_ie->cid[i] = curd_16;
                        offset++;

                        curd_16 = payload[offset] <<4;
                        ie->normal_ie->cid[i]+= curd_16;
                        offset++;
                        
                        ie->normal_ie->cid[i]+= ((payload[offset] &0xf0)>>4);
                        
                    }
                }

                ie->normal_ie->ofdma_symbol_offset = ((payload[offset] & 0x0f) <<4) + ((payload[offset+1] & 0xf0)>>4);
                offset++;

#if(DL_PERMUTATION_TYPE==0) // PUSC 

                ie->normal_ie->subchannel_offset = ((payload[offset] & 0x0f)<<2) + ((payload[offset+1] & 0xc0) >>6);
                offset++;

                ie->normal_ie->boosting = (payload[offset] & 0x38)>>3;
                ie->normal_ie->ofdma_Symbols_num = ((payload[offset] & 0x07)<<4) +((payload[offset+1] & 0xf0) >>4);
                offset++;

                ie->normal_ie->subchannels_num = ((payload[offset] & 0x0f) <<2) + ((payload[offset+1] &0xc0)>>6);
                offset++;

                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x30)>>4;

#endif
#if(DL_PERMUTATION_TYPE==2) // AMC
                ie->normal_ie->subchannel_offset = ((payload[offset] & 0x0f)<<4) + ((payload[offset+1] & 0xf0) >>4);
                offset++;

                ie->normal_ie->boosting = (payload[offset] & 0x0E) >> 1;
                ie->normal_ie->ofdma_triple_symbol_num = ((payload[offset] & 0x01)<<4) +((payload[offset+1] & 0xf0) >>4);
                offset++;

                ie->normal_ie->subchannels_num = ((payload[offset] & 0x0f) <<2) + ((payload[offset+1] &0xc0)>>6);
                offset++;

                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x30)>>4;
#endif

                is_left = 1;

            }
        }
        
        if (pre_ie == NULL)
        {
            dlmap->ie_head = ie;
            pre_ie = ie;
        }
        else
        {
            pre_ie->next = ie;
            pre_ie = ie;
        }
    }

    return 0;
}

int parse_mdbi(region_attri *ra, u_char* payload, int *p_offset, int nibble_left)
{
    int offset = *p_offset;
    int jj = 0;
    layer_attri *la = NULL, *prev_la = NULL;

    if (nibble_left == 0)
    {
        ra->ofdma_symbol_offset = payload[offset] & 0xff;
        offset++;

        // Only PUSC permutation supported yet
        ra->subchannel_offset = (payload[offset] & 0xfc) >> 2;
        ra->boosting =  (payload[offset] & 0x03) << 1;
        offset++;

        ra->boosting +=  ((payload[offset] & 0x80) >> 7);
        ra->ofdma_symbols_num = (payload[offset] & 0x7f);
        offset++;

        ra->subchannels_num = (payload[offset] & 0xfc) >> 2;
        ra->matrix_indicator = (payload[offset] & 0x03);
        offset++;

        ra->num_layer = (payload[offset] & 0xc0) >> 6;
        for (jj = 0; jj <= ra->num_layer; jj++)
        {
            la = (layer_attri*)malloc(sizeof(layer_attri));
            memset(la, 0, sizeof(layer_attri));

            if (INC_CID == 1)
            {
                la->cid = (payload[offset] & 0x0f) << 12;
                offset++;
                la->cid += ((payload[offset] & 0xff) << 4);
                offset++;
                la->cid += ((payload[offset] & 0xf0) >> 4);
            }

            la->layer_index = ((payload[offset] & 0x0c) >> 2);
            la->diuc = ((payload[offset] & 0x03) << 2);
            offset++;
            la->diuc += ((payload[offset] & 0xc0) >> 6);
            la->repetition_coding_indication = ((payload[offset] & 0x30) >> 4);

            if (jj == 0)
            {
                ra->layer_header = la;
            }
            else
            {
                prev_la->next = la;
            }
            prev_la = la;
        }
    }
    else
    {
        ra->ofdma_symbol_offset = (payload[offset] & 0x0f) << 4;
        offset++;
        ra->ofdma_symbol_offset += ((payload[offset] & 0xf0) >> 4);

        // Only PUSC permutation supported yet
        ra->subchannel_offset = ((payload[offset] & 0x0f) << 2);
        offset++;

        ra->subchannel_offset += ((payload[offset] & 0xc0) >> 6);
        ra->boosting =  (payload[offset] & 0x31) >> 3;
        ra->ofdma_symbols_num = (payload[offset] & 0x07) << 4;
        offset++;

        ra->ofdma_symbols_num += ((payload[offset] & 0xf0) >> 4);
        ra->subchannels_num = (payload[offset] & 0x0f) << 2;
        offset++;

        ra->subchannels_num += ((payload[offset] & 0xc0) >> 6);
        ra->matrix_indicator = (payload[offset] & 0x30) >> 4;
        ra->num_layer = (payload[offset] & 0x0c) >> 2;
        offset++;

        for (jj = 0; jj <= ra->num_layer; jj++)
        {
            la = (layer_attri*)malloc(sizeof(layer_attri));
            memset(la, 0, sizeof(layer_attri));
            if (INC_CID == 1)
            {
                la->cid = (payload[offset] & 0xff) << 8;
                offset++;
                la->cid += (payload[offset] & 0xff);
                offset++;
            }
            la->layer_index = ((payload[offset] & 0xc0) >> 6);
            la->diuc = ((payload[offset] & 0x3c) >> 2);
            la->repetition_coding_indication = (payload[offset] & 0x03);
            offset++;

            if (jj == 0)
            {
                ra->layer_header = la;
            }
            else
            {
                prev_la->next = la;
            }
            prev_la = la;
        }
    }

    // Update the value of offset back in input pointer
    *p_offset = offset;

    return 0;
}
/*
int parse_ulmap(u_char* payload, ul_map_msg* ulmap, int* length, ){
    
    return 0;
}

*/
