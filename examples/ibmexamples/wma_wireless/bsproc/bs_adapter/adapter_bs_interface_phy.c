/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_interface_phy.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_bs_interface_phy.h"
#include "mac_shift_bits.h"
#include "mac_hcs.h"
#include "mac_crc.h"
#include <string.h>

#define GENERIC_MAC_HEADER_LEN    6
#define MAC_CRC_LEN               4
#define INC_CID                   1
static struct FCH_subframe    g_fch_subframe;

int set_fch_subframe(const struct FCH_subframe * const p_fch_subframe)
{
    g_fch_subframe.used_subchannel_bitmap = p_fch_subframe->used_subchannel_bitmap;
    g_fch_subframe.rsv1 = p_fch_subframe->rsv1;;
    g_fch_subframe.repetition_coding_indication = p_fch_subframe->repetition_coding_indication;;
    g_fch_subframe.coding_indication = p_fch_subframe->coding_indication;;
    g_fch_subframe.dl_map_length = p_fch_subframe->dl_map_length;
    g_fch_subframe.rsv2 = p_fch_subframe->rsv2;;
    return 0;
}

int get_fch_subframe(struct FCH_subframe * const p_fch_subframe)
{
    p_fch_subframe->used_subchannel_bitmap = g_fch_subframe.used_subchannel_bitmap;
    p_fch_subframe->rsv1 = g_fch_subframe.rsv1;;
    p_fch_subframe->repetition_coding_indication = g_fch_subframe.repetition_coding_indication;;
    p_fch_subframe->coding_indication = g_fch_subframe.coding_indication;;
    p_fch_subframe->dl_map_length = g_fch_subframe.dl_map_length;
    p_fch_subframe->rsv2 = g_fch_subframe.rsv2;
    return 0;
}


int phy_ofdma_dl_fch_parsing(u_int8_t *p_payload,u_int8_t is_128fft,u_int32_t * length,struct FCH_subframe  *const p_fch_subframe)
{
	  
    int offset;
    offset = 0; 
    if (is_128fft)
    {
        p_fch_subframe->used_subchannel_bitmap = p_payload[offset] >> 7;
        p_fch_subframe->rsv1 = (p_payload[offset] & 0x40) >> 6;
        p_fch_subframe->repetition_coding_indication = (p_payload[offset] & 0x30) >> 4;
        p_fch_subframe->coding_indication = (p_payload[offset] & 0x0e) >> 1;
        p_fch_subframe->dl_map_length = ((p_payload[offset] & 0x01) << 4) |(p_payload[offset+1] & 0xf0) >> 4;
        p_fch_subframe->rsv2 = p_payload[offset+1] & 0x0f;
        (*length) = 2;
    }
    else 
    {
        p_fch_subframe->used_subchannel_bitmap = p_payload[offset] >> 2;
        p_fch_subframe->rsv1 = (p_payload[offset] & 0x02) >> 1;
        p_fch_subframe->repetition_coding_indication = ((p_payload[offset] & 0x01) <<1) |(p_payload[offset+1] >> 7);
        offset++;
        p_fch_subframe->coding_indication = ((p_payload[offset] & 0x70) >> 4);
        p_fch_subframe->dl_map_length = ((p_payload[offset] & 0x0f) << 4) | (p_payload[offset+1] >> 4);
        p_fch_subframe->rsv2 = p_payload[offset+1] & 0x0f;
        (*length) = 3;
    }
        
    return 0;
}


int adapter_dl_fch_parsing(u_int8_t *payload,u_int8_t is_128fft,u_int32_t * length,struct FCH_subframe  *const p_fch_subframe)
{
    u_int8_t  p_payload[*length/8 + 1];
    int startoffset = 0,endoffset = 0;
    bits_to_byte(payload,*length,p_payload,&startoffset,&endoffset);
    phy_ofdma_dl_fch_parsing(p_payload,is_128fft,length,p_fch_subframe);
    return 0;
}


int parse_dlmap(u_char* payload, dl_map_msg* dlmap){
    int offset;
    offset = 0; 
    u_int16_t curd_16;
    u_int32_t curd_32;
    u_int64_t curd_64;
    dl_map_ie* pre_ie = NULL;
    dl_map_ie* ie = NULL;
    int is_left = 0;
    int i;
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
        if (is_left)
        {
            ie->diuc = (payload[offset] & 0x0f);
            offset++;
            if (ie->diuc == 14)
            {
                // build the extended-2 DIUC ie

            }else if (ie->diuc == 15){
                // build the extended DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));
                ie->extended_ie->extended_diuc = (payload[offset] & 0xf0) >>4;
                if (ie->extended_ie->extended_diuc == 0x04)
                {
                   ie->extended_ie->length = (payload[offset] & 0x0f); 

                   offset+=ie->extended_ie->length;
                }

                offset++;
                is_left = 0;
            }else {
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

                ie->normal_ie->subchannel_offset = (payload[offset] & 0xfc)>>2;

                ie->normal_ie->boosting = ((payload[offset] & 0x03)<<1) + ((payload[offset+1] & 0x80) >>7);
                offset++;
                ie->normal_ie->ofdma_Symbols_num = payload[offset] & 0x7f ;
                offset++;

                ie->normal_ie->subchannels_num = (payload[offset] & 0xfc) >>2;
                
                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x03);
                offset++;

                is_left = 0;
            }
        }
        else
        {
            ie->diuc = (payload[offset] & 0xf0) >>4;
            if (ie->diuc == 14)
            {
                // build the extended-2 DIUC ie

            }else if (ie->diuc == 15){
                // build the extended DIUC ie
                ie->extended_ie = (extended_diuc_ie*)malloc(sizeof(extended_diuc_ie));
                memset(ie->extended_ie, 0, sizeof(extended_diuc_ie));
                ie->extended_ie->extended_diuc = (payload[offset] & 0x0f) ;
                offset++;
                if (ie->extended_ie->extended_diuc == 0x04)
                {
                   ie->extended_ie->length = (payload[offset] & 0xf0) >>4; 

                   offset+=ie->extended_ie->length;
                }

                is_left = 1;
            }else {
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

                ie->normal_ie->subchannel_offset = ((payload[offset] & 0x0f)<<2) + ((payload[offset+1] & 0xc0) >>6);
                offset++;

                ie->normal_ie->boosting = (payload[offset] & 0x38)>>3;
                ie->normal_ie->ofdma_Symbols_num = ((payload[offset] & 0x07)<<4) +((payload[offset+1] & 0xf0) >>4);
                offset++;

                ie->normal_ie->subchannels_num = ((payload[offset] & 0x0f) <<2) + ((payload[offset+1] &0xc0)>>6);
                offset++;
                
                ie->normal_ie->repetition_coding_indication = (payload[offset]&0x30)>>4;

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

int adapter_init_dlmap(u_int8_t *p_payload, dl_map_msg* dlmap, u_int32_t * length)
{
    u_int8_t  payload[*length/8 + 1];
    int startoffset = 0,endoffset = 0;
    bits_to_byte(p_payload,*length,payload,&startoffset,&endoffset);
    parse_dlmap(payload,dlmap);
#ifdef _FIX_DIUC_BUG_
    dl_map_ie *ie= dlmap->ie_head;
    while(ie != NULL)
    {
        switch(ie->diuc)
        {
            case 0:
                break;
            case 1:
                ie->diuc = 3;
                break;
        }
        ie = ie->next; 
    }
#endif  
    return 0;
}

// The memory for DLMAP IE is dynamically allocated per frame as needed
// needs to be freed after the frame's lifetime
int adapter_deinit_dlmap(dl_map_msg  * dl_map)
{
  dl_map_ie* p_map_ie,*p_prev_map_ie;
  p_map_ie = dl_map->ie_head;
  while(p_map_ie != NULL)
    {
      p_prev_map_ie = p_map_ie;
      p_map_ie = p_map_ie->next;
      // Need to add similar logic for other special DIUC IE's
      if(p_prev_map_ie->normal_ie != NULL)
	{
	  if (p_prev_map_ie->normal_ie->cid != NULL)
	    {
	      free(p_prev_map_ie->normal_ie->cid);
	    }
	  free(p_prev_map_ie->normal_ie);
	}
      if(p_prev_map_ie->extended_ie != NULL)
	{
	  free(p_prev_map_ie->extended_ie);
	}
      free(p_prev_map_ie);
    }
  return 0;
}

int adapter_dl_procblk_valid(u_int32_t n_cid, u_int16_t *cid)
{
    *cid = 0;
    n_cid = 0;
    return 0;
}
