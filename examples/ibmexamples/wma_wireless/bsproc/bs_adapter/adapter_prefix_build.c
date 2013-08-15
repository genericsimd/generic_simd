/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_prefix_build.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Feb 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_prefix_build.h"
#include "mac_headermsg_builder.h"
#define INC_CID        1
int build_dlmap (const dl_map_msg* const dlmap, u_char* payload, int* length)
{
    // the memory for the dl map is allocated when build the map, since the map usually needs unknown
    // bandwidth, so we could first allocate 500 bytes, if it is not enough, then we could reallocate the memory
    int n = 0;
    int i;
    dl_map_ie* dl_ie;
    generic_mac_hdr *gmh;
    int is_left = 0;
    int hdr_len = 0;
    n++;
    //payload = (u_char*) mm_malloc(n, MAP_ALLOC_LENGTH);

    u_int8_t cid_num;
    normal_diuc_ie* normal_ie;

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
        if (is_left)
        {
            ( *cur_p ) = ( *cur_p ) + ( u_int8_t ) (dl_ie->diuc);
            ( *length )++;
            cur_p++;

            if (dl_ie->diuc == 14)
            {
                // build the extended-2 DIUC ie

            }
            else if (dl_ie->diuc == 15)
            {
                // build the extended DIUC ie

            }
            else
            {
                normal_ie = dl_ie->normal_ie;
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
            }
        }
        else
        {
            cur_p++;
            // diuc, only occupy 4 bits
            ( *cur_p ) = ( u_int8_t ) ( ( dl_ie->diuc ) << 4 & 0xf0);

            if (dl_ie->diuc == 14)
            {
                // build the extended-2 DIUC ie

            }
            else if (dl_ie->diuc == 15)
            {
                // build the extended DIUC ie

            }
            else
            {
                normal_ie = dl_ie->normal_ie;
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
    gmh->cid = 0xffff;
    gmh->len = ( *length ) + MAC_CRC_LEN;
    gmh->type = 0;
    build_gmh (gmh, payload, &hdr_len);
    free (gmh);
    hdr_len--;
    hcs_calculation (payload, ( hdr_len - 1 ), ( payload + hdr_len - 1 ));

    // add the crc checksum
    crc_calculation (payload, ( *length ), ( payload + ( *length ) ));

    ( *length ) = ( *length ) + MAC_CRC_LEN;

    return 0;
}
