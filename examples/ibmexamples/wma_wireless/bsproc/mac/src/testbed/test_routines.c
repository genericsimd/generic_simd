/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: test_routines.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "test_routines.h"

int compare_ul_map (ul_map_msg * p_1, ul_map_msg * p_2)
{
    int ie_idx = 0;
    ul_map_ie * p_ie_1 = NULL;
    ul_map_ie * p_ie_2 = NULL;

    /** Basic compare */
    if (p_1->manage_msg_type != p_2->manage_msg_type)
    {
        ERR_PRINT("manage_msg_type", p_1->manage_msg_type, p_2->manage_msg_type);
        return 1;
    }

    if (p_1->ucd_count != p_2->ucd_count)
    {
        ERR_PRINT("ucd_count", p_1->ucd_count, p_2->ucd_count);
        return 1;
    }

    if (p_1->alloc_start_time != p_2->alloc_start_time)
    {
        ERR_PRINT("alloc_start_time", p_1->alloc_start_time, p_2->alloc_start_time);
        return 1;
    }

    if (p_1->ulmap_ie_num != p_2->ulmap_ie_num)
    {
        ERR_PRINT("ulmap_ie_num", p_1->ulmap_ie_num, p_2->ulmap_ie_num);
        return 1;
    }

    p_ie_1 = p_1->ie;
    p_ie_2 = p_2->ie;

    for (ie_idx = 0; ie_idx < p_1->ulmap_ie_num; ie_idx++)
    {
        if ( ( p_ie_1 == NULL ) || ( p_ie_2 == NULL ))
        {
            ERR_POINTER(ie_idx, "ie", p_ie_1);
            ERR_POINTER(ie_idx, "ie", p_ie_2);
            return 1;
        }

        if (p_ie_1->ie_index != p_ie_2->ie_index)
        {
            ERR_PRINT("ie_index", p_ie_1->ie_index, p_ie_2->ie_index);
            return 1;
        }

        if (p_ie_1->cid != p_ie_2->cid)
        {
            ERR_PRINT("cid", p_ie_1->cid, p_ie_2->cid);
            return 1;
        }

        if (p_ie_1->uiuc != p_ie_2->uiuc)
        {
            ERR_PRINT("uiuc", p_ie_1->uiuc, p_ie_2->uiuc);
            return 1;
        }

        if (p_ie_1->uiuc == 11)
        {
            if ( ( p_ie_1->uiuc_extend_ie == NULL ) || ( p_ie_2->uiuc_extend_ie
                == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_extend_ie", p_ie_1->uiuc_extend_ie);
                ERR_POINTER(ie_idx, "uiuc_extend_ie", p_ie_2->uiuc_extend_ie);
                return 1;
            }

            if (p_ie_1->uiuc_extend_ie->extended_uiuc
                != p_ie_2->uiuc_extend_ie->extended_uiuc)
            {
                ERR_PRINT("uiuc_extend_ie->extended_uiuc",
                    p_ie_1->uiuc_extend_ie->extended_uiuc,
                    p_ie_2->uiuc_extend_ie->extended_uiuc);
                return 1;
            }

            if (p_ie_1->uiuc_extend_ie->length
                != p_ie_2->uiuc_extend_ie->length)
            {
                ERR_PRINT("uiuc_extend_ie->length",
                    p_ie_1->uiuc_extend_ie->length,
                    p_ie_2->uiuc_extend_ie->length);
                return 1;
            }
        }
        else if (p_ie_1->uiuc == 12)
        {
            if ( ( p_ie_1->uiuc_12_ie == NULL )
                || ( p_ie_2->uiuc_12_ie == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_12_ie", p_ie_1->uiuc_12_ie);
                ERR_POINTER(ie_idx, "uiuc_12_ie", p_ie_2->uiuc_12_ie);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->ofdma_symbol_offset
                != p_ie_2->uiuc_12_ie->ofdma_symbol_offset)
            {
                ERR_PRINT("uiuc_12_ie->ofdma_symbol_offset",
                    p_ie_1->uiuc_12_ie->ofdma_symbol_offset,
                    p_ie_2->uiuc_12_ie->ofdma_symbol_offset);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->subchannel_offset
                != p_ie_2->uiuc_12_ie->subchannel_offset)
            {
                ERR_PRINT("uiuc_12_ie->subchannel_offset",
                    p_ie_1->uiuc_12_ie->subchannel_offset,
                    p_ie_2->uiuc_12_ie->subchannel_offset);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->ofdma_symbol_num
                != p_ie_2->uiuc_12_ie->ofdma_symbol_num)
            {
                ERR_PRINT("uiuc_12_ie->ofdma_symbol_num",
                    p_ie_1->uiuc_12_ie->ofdma_symbol_num,
                    p_ie_2->uiuc_12_ie->ofdma_symbol_num);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->subchannel_num
                != p_ie_2->uiuc_12_ie->subchannel_num)
            {
                ERR_PRINT("uiuc_12_ie->subchannel_num",
                    p_ie_1->uiuc_12_ie->subchannel_num,
                    p_ie_2->uiuc_12_ie->subchannel_num);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->ranging_method
                != p_ie_2->uiuc_12_ie->ranging_method)
            {
                ERR_PRINT("uiuc_12_ie->ranging_method",
                    p_ie_1->uiuc_12_ie->ranging_method,
                    p_ie_2->uiuc_12_ie->ranging_method);
                return 1;
            }

            if (p_ie_1->uiuc_12_ie->dedicated_ranging_indicator
                != p_ie_2->uiuc_12_ie->dedicated_ranging_indicator)
            {
                ERR_PRINT("uiuc_12_ie->dedicated_ranging_indicator",
                    p_ie_1->uiuc_12_ie->dedicated_ranging_indicator,
                    p_ie_2->uiuc_12_ie->dedicated_ranging_indicator);
                return 1;
            }
        }
        else if (p_ie_1->uiuc == 13)
        {
            if ( ( p_ie_1->uiuc_13_ie == NULL )
                || ( p_ie_2->uiuc_13_ie == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_13_ie", p_ie_1->uiuc_13_ie);
                ERR_POINTER(ie_idx, "uiuc_13_ie", p_ie_2->uiuc_13_ie);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->ofdma_symbol_offset
                != p_ie_2->uiuc_13_ie->ofdma_symbol_offset)
            {
                ERR_PRINT("uiuc_13_ie->ofdma_symbol_offset",
                    p_ie_1->uiuc_13_ie->ofdma_symbol_offset,
                    p_ie_2->uiuc_13_ie->ofdma_symbol_offset);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->subchannel_offset
                != p_ie_2->uiuc_13_ie->subchannel_offset)
            {
                ERR_PRINT("uiuc_13_ie->subchannel_offset",
                    p_ie_1->uiuc_13_ie->subchannel_offset,
                    p_ie_2->uiuc_13_ie->subchannel_offset);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->ofdma_symbol_num
                != p_ie_2->uiuc_13_ie->ofdma_symbol_num)
            {
                ERR_PRINT("uiuc_13_ie->ofdma_symbol_num",
                    p_ie_1->uiuc_13_ie->ofdma_symbol_num,
                    p_ie_2->uiuc_13_ie->ofdma_symbol_num);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->subchannel_num_szshift_value
                != p_ie_2->uiuc_13_ie->subchannel_num_szshift_value)
            {
                ERR_PRINT("uiuc_13_ie->subchannel_num_szshift_value",
                    p_ie_1->uiuc_13_ie->subchannel_num_szshift_value,
                    p_ie_2->uiuc_13_ie->subchannel_num_szshift_value);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->paprreduc_safezone
                != p_ie_2->uiuc_13_ie->paprreduc_safezone)
            {
                ERR_PRINT("uiuc_13_ie->paprreduc_safezone",
                    p_ie_1->uiuc_13_ie->paprreduc_safezone,
                    p_ie_2->uiuc_13_ie->paprreduc_safezone);
                return 1;
            }

            if (p_ie_1->uiuc_13_ie->sounding_zone
                != p_ie_2->uiuc_13_ie->sounding_zone)
            {
                ERR_PRINT("uiuc_13_ie->sounding_zone",
                    p_ie_1->uiuc_13_ie->sounding_zone,
                    p_ie_2->uiuc_13_ie->sounding_zone);
                return 1;
            }
        }
        else if (p_ie_1->uiuc == 14)
        {
            if ( ( p_ie_1->uiuc_14_ie == NULL )
                || ( p_ie_2->uiuc_14_ie == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_14_ie", p_ie_1->uiuc_14_ie);
                ERR_POINTER(ie_idx, "uiuc_14_ie", p_ie_2->uiuc_14_ie);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->duration != p_ie_2->uiuc_14_ie->duration)
            {
                ERR_PRINT("uiuc_14_ie->duration",
                    p_ie_1->uiuc_14_ie->duration,
                    p_ie_2->uiuc_14_ie->duration);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->uiuc != p_ie_2->uiuc_14_ie->uiuc)
            {
                ERR_PRINT("uiuc_14_ie->uiuc",
                    p_ie_1->uiuc_14_ie->uiuc,
                    p_ie_2->uiuc_14_ie->uiuc);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->repetition_coding_indication
                != p_ie_2->uiuc_14_ie->repetition_coding_indication)
            {
                ERR_PRINT("uiuc_14_ie->repetition_coding_indication",
                    p_ie_1->uiuc_14_ie->repetition_coding_indication,
                    p_ie_2->uiuc_14_ie->repetition_coding_indication);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->frame_num_index
                != p_ie_2->uiuc_14_ie->frame_num_index)
            {
                ERR_PRINT("uiuc_14_ie->frame_num_index",
                    p_ie_1->uiuc_14_ie->frame_num_index,
                    p_ie_2->uiuc_14_ie->frame_num_index);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->ranging_code
                != p_ie_2->uiuc_14_ie->ranging_code)
            {
                ERR_PRINT("uiuc_14_ie->ranging_code",
                    p_ie_1->uiuc_14_ie->ranging_code,
                    p_ie_2->uiuc_14_ie->ranging_code);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->ranging_symbol
                != p_ie_2->uiuc_14_ie->ranging_symbol)
            {
                ERR_PRINT("uiuc_14_ie->ranging_symbol",
                    p_ie_1->uiuc_14_ie->ranging_symbol,
                    p_ie_2->uiuc_14_ie->ranging_symbol);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->ranging_subchannel
                != p_ie_2->uiuc_14_ie->ranging_subchannel)
            {
                ERR_PRINT("uiuc_14_ie->ranging_subchannel",
                    p_ie_1->uiuc_14_ie->ranging_subchannel,
                    p_ie_2->uiuc_14_ie->ranging_subchannel);
                return 1;
            }

            if (p_ie_1->uiuc_14_ie->bw_request_mandatory
                != p_ie_2->uiuc_14_ie->bw_request_mandatory)
            {
                ERR_PRINT("uiuc_14_ie->bw_request_mandatory",
                    p_ie_1->uiuc_14_ie->bw_request_mandatory,
                    p_ie_2->uiuc_14_ie->bw_request_mandatory);
                return 1;
            }
        }
        else if (p_ie_1->uiuc == 15)
        {
            if ( ( p_ie_1->uiuc_15_ie == NULL )
                || ( p_ie_2->uiuc_15_ie == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_15_ie", p_ie_1->uiuc_15_ie);
                ERR_POINTER(ie_idx, "uiuc_15_ie", p_ie_2->uiuc_15_ie);
                return 1;
            }

            if (p_ie_1->uiuc_15_ie->extended_uiuc
                != p_ie_2->uiuc_15_ie->extended_uiuc)
            {
                ERR_PRINT("uiuc_15_ie->extended_uiuc",
                    p_ie_1->uiuc_15_ie->extended_uiuc,
                    p_ie_2->uiuc_15_ie->extended_uiuc);
                return 1;
            }

            if (p_ie_1->uiuc_15_ie->length != p_ie_2->uiuc_15_ie->length)
            {
                ERR_PRINT("uiuc_15_ie->length",
                    p_ie_1->uiuc_15_ie->length,
                    p_ie_2->uiuc_15_ie->length);
                return 1;
            }
        }
        else if (p_ie_1->uiuc == 0)
        {
            if ( ( p_ie_1->uiuc_0_ie == NULL ) || ( p_ie_2->uiuc_0_ie == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_0_ie", p_ie_1->uiuc_0_ie);
                ERR_POINTER(ie_idx, "uiuc_0_ie", p_ie_2->uiuc_0_ie);
                return 1;
            }

            if (p_ie_1->uiuc_0_ie->ofdma_symbol_offset
                != p_ie_2->uiuc_0_ie->ofdma_symbol_offset)
            {
                ERR_PRINT("uiuc_0_ie->ofdma_symbol_offset",
                    p_ie_1->uiuc_0_ie->ofdma_symbol_offset,
                    p_ie_2->uiuc_0_ie->ofdma_symbol_offset);
                return 1;
            }

            if (p_ie_1->uiuc_0_ie->subchannel_offset
                != p_ie_2->uiuc_0_ie->subchannel_offset)
            {
                ERR_PRINT("uiuc_0_ie->subchannel_offset",
                    p_ie_1->uiuc_0_ie->subchannel_offset,
                    p_ie_2->uiuc_0_ie->subchannel_offset);
                return 1;
            }

            if (p_ie_1->uiuc_0_ie->ofdma_symbol_num
                != p_ie_2->uiuc_0_ie->ofdma_symbol_num)
            {
                ERR_PRINT("uiuc_0_ie->ofdma_symbol_num",
                    p_ie_1->uiuc_0_ie->ofdma_symbol_num,
                    p_ie_2->uiuc_0_ie->ofdma_symbol_num);
                return 1;
            }

            if (p_ie_1->uiuc_0_ie->subchannel_num
                != p_ie_2->uiuc_0_ie->subchannel_num)
            {
                ERR_PRINT("uiuc_0_ie->subchannel_num",
                    p_ie_1->uiuc_0_ie->subchannel_num,
                    p_ie_2->uiuc_0_ie->subchannel_num);
                return 1;
            }
        }
        else
        {
            if ( ( p_ie_1->uiuc_other_ie == NULL ) || ( p_ie_2->uiuc_other_ie
                == NULL ))
            {
                ERR_POINTER(ie_idx, "uiuc_other_ie", p_ie_1->uiuc_other_ie);
                ERR_POINTER(ie_idx, "uiuc_other_ie", p_ie_2->uiuc_other_ie);
                return 1;
            }

            if (p_ie_1->uiuc_other_ie->duration
                != p_ie_2->uiuc_other_ie->duration)
            {
                ERR_PRINT("uiuc_other_ie->duration",
                    p_ie_1->uiuc_other_ie->duration,
                    p_ie_2->uiuc_other_ie->duration);
                return 1;
            }

            if (p_ie_1->uiuc_other_ie->repetition_coding_indication
                != p_ie_2->uiuc_other_ie->repetition_coding_indication)
            {
                ERR_PRINT("uiuc_other_ie->repetition_coding_indication",
                    p_ie_1->uiuc_other_ie->repetition_coding_indication,
                    p_ie_2->uiuc_other_ie->repetition_coding_indication);
                return 1;
            }

/*            if (p_ie_1->uiuc_other_ie->slot_offset
                != p_ie_2->uiuc_other_ie->slot_offset)
            {
                ERR_PRINT("uiuc_other_ie->slot_offset",
                    p_ie_1->uiuc_other_ie->slot_offset,
                    p_ie_2->uiuc_other_ie->slot_offset);
                return 1;
            }*/
        }
        p_ie_1 = p_ie_1->next;
        p_ie_2 = p_ie_2->next;

    }

    FLOG_INFO ("ULMAP match successful\n");

    return 0;
}

int compare_ucd (ucd_msg * ucd1, ucd_msg * ucd2)
{
    if(ucd1->management_message_type != ucd2->management_message_type)
        return -1;
    if(ucd1->configuration_change_count != ucd2->configuration_change_count)
        return -2;
    if(ucd1->ranging_backoff_start != ucd2->ranging_backoff_start)
        return -1;
    if(ucd1->ranging_backoff_end != ucd2->ranging_backoff_end)
        return -1;
    if(ucd1->request_backoff_start != ucd2->request_backoff_start)
        return -1;
    if(ucd1->request_backoff_end != ucd2->request_backoff_end)
        return -1;

	ul_burst_profile* ulbp1 = ucd1->profile_header;
	ul_burst_profile* ulbp2 = ucd2->profile_header;
	while ((ulbp1 != NULL) || (ulbp2 != NULL))
    {
		// If only one of them is NULL, it indicates an unequal number of burst profiles in the UCD. Error
        if((ulbp1 == NULL && ulbp2 != NULL) | (ulbp1 != NULL && ulbp2 == NULL))
            return -3;
        if(ulbp1->type != ulbp2->type)
            return -4;
        if(ulbp1->length != ulbp2->length)
            return -5;
        if(ulbp1->uiuc!= ulbp2->uiuc)
            return -6;
        if(ulbp1->fec_code_modulation.type != ulbp2->fec_code_modulation.type)
            return -7;
        if(ulbp1->fec_code_modulation.length != ulbp2->fec_code_modulation.length)
            return -8;
        if(ulbp1->fec_code_modulation.value != ulbp2->fec_code_modulation.value)
            return -9;

        ulbp1 = ulbp1->next;
        ulbp2 = ulbp2->next;

    }
    FLOG_INFO ("UCD match successful\n");
	return 0;
}

int compare_dcd (dcd_msg * dcd1, dcd_msg * dcd2)
{
    if(dcd1->management_message_type != dcd2->management_message_type)
        return -1;
    if(dcd1->configuration_change_count != dcd2->configuration_change_count)
        return -2;

	dl_burst_profile* dlbp1 = dcd1->profile_header;
	dl_burst_profile* dlbp2 = dcd2->profile_header;
	while (dlbp1 != NULL || dlbp2 != NULL)
    {
		// If only one of them is NULL, it indicates an unequal number of burst profiles in the UCD. Error
        if((dlbp1 == NULL && dlbp2 != NULL) | (dlbp1 != NULL && dlbp2 == NULL))
            return -3;
        if(dlbp1->type != dlbp2->type)
            return -4;
        if(dlbp1->length != dlbp2->length)
            return -5;
        if(dlbp1->diuc!= dlbp2->diuc)
            return -6;
        if(dlbp1->fec_code_modulation.type != dlbp2->fec_code_modulation.type)
            return -7;
        if(dlbp1->fec_code_modulation.length != dlbp2->fec_code_modulation.length)
            return -8;
        if(dlbp1->fec_code_modulation.value != dlbp2->fec_code_modulation.value)
            return -9;

        dlbp1 = dlbp1->next;
        dlbp2 = dlbp2->next;

    }
    FLOG_INFO ("DCD match successful\n");
	return 0;
}
