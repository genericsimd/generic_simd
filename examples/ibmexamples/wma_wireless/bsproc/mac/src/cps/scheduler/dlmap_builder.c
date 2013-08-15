/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dlmap_builder.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "dlmap_builder.h"
#include "bs_ss_info.h"
#include "debug.h"
#include "mac.h"

#include "sdu_cid_queue.h"

#include "dump_util.h"

extern int dlmap_ccc;

int dl_map_builder(int* dl_ss_allocation, amc_info* amc_info_header, dl_map_msg* dl_map, int* dlmap_length_in_slots,int num_dl_subch)
{
	
	int num_dl_slots_in_freq = num_dl_subch/DL_SUBCHANNEL_PER_SLOT;
	//In the FOAK, the num_dl_subchannels will keep varying. Hence the above change from Macro to variable
	
	int ii = 0, ie_index = 0, num_symbols_temp, cid, stc_zone_start = 0;
	// Ref: Sec 8.4.5.3 in 2007 spec: The symbol after the preamble is indexed 1
	int cur_ofdma_symbol_index = dl_map->ofdma_symbols_num - DL_SYMBOLS_PER_SLOT;
	// Ref Sec 8.4.5.3 in 2007 spec: Subchannels are indexed from 0 to NUM_DL_SLOTS_IN_FREQ - 1
	int cur_slot_index = num_dl_slots_in_freq - 1;
	dl_map_ie* ie = NULL;
	dl_map_ie* prev_ie = NULL;
	dl_map->frame_number = (unsigned int)get_current_frame_number();

	// Maintained by the DCD module
	dl_map->dcd_count = dlmap_ccc;

	// Currently common MCS chosen for all SS's. 
	// later this value will be obtained for each SS from amc_info

    ModulCodingType dl_mcs_type = param_DL_MCS;
	ModulCodingType ul_mcs_type = param_UL_MCS;
	int diuc_code = 0;

    int dlmap_len_in_bits = DL_FIXED_FIELD_LENGTH + (GENERIC_MAC_HEADER_LEN + MAC_CRC_LEN) * 8;

	// Data regions are allocated in reverse order from the end of the frame
	// hence DLMAP-IE list is also built in reverse (IE for the later blocks are built first)
	// but the new IE's are added to the head of the list. 1st IE: broadcast burst
	// (will be built after the for loop), followed by the IEs for all SS's 
	for(ii = 0; ii < NUM_SS; ii++)
	{
		FLOG_DEBUG("\ndl_ss_alloc[%d]: %d", ii, dl_ss_allocation[ii]);

		get_ss_mcs(ii, &dl_mcs_type, &ul_mcs_type);
		diuc_code = dl_mcs_type;

		if(dl_ss_allocation[ii]>0)
		{
			// If this SS has any bandwidth allocated in the current UL subframe
			// construct a DLMAP-IE for it
			cid = get_basic_cid_from_ss_index(ii);

			// TODO: can't provide yet as DLMAP IEs are in reverse order.
			// Clarify need with CRL
			//ie->ie_index = ie_index;

			// When AMC module is implemented, replace by
			// ss_amc_info = get_ss_amc_info(ii);
			// diuc = ss_amc_info->diuc

			// The data region allocation algo is according to this reference: "A Downlink
			// Data Region Allocation Algorithm for IEEE 802.16e OFDMA", by Cicconetti
			// et al, International Conference on Information, Communications & Signal
			// Processing, Dec. 2007
			if ((dl_ss_allocation[ii]%num_dl_slots_in_freq == 0) && \
					(cur_slot_index == num_dl_slots_in_freq - 1))
			{
				// Num of slots needed makes a rectangular data region spanning the full
				// frequency band and possibly spanning multiple OFDMA symbol sets
				// Only one DLMAP-IE needed
				while (dl_ss_allocation[ii]>0)
				{
					init_dlmap_ie(&ie, diuc_code);
					ie_index++;
					ie->next = prev_ie;

					prev_ie = ie;
					
					// Current PHY in FOAK has a restriction that it can't decode
					// if regions described by a DLMAP-IE span multiple slotsymbols
					// Hence build a new IE for each slotsymbol
					#ifdef FOAK_TEMP_FIX
						build_full_band_rect_dlmap_ie(num_dl_slots_in_freq, ie, \
						&cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
						dl_ss_allocation[ii] -= num_dl_slots_in_freq;
					#else
						build_full_band_rect_dlmap_ie(dl_ss_allocation[ii], ie, \
						&cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
						dl_ss_allocation[ii] = 0;
					#endif
				}
				//if (cid == BROADCAST_CID) {ulmap_type_flag =1;start_sc = ie->normal_ie->subchannel_offset;start_sym = ie->normal_ie->ofdma_symbol_offset;}
			}
			else if (dl_ss_allocation[ii] <= cur_slot_index)
			{
				init_dlmap_ie(&ie, diuc_code);
				ie_index++;
				ie->next = prev_ie;

				prev_ie = ie;
				
				// Number of slots needed is < slots available in current
				// symbol set only one DLMAP-IE will be needed
				build_partial_band_dlmap_ie(dl_ss_allocation[ii], ie, &cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_slots_in_freq);
				dl_ss_allocation[ii] = 0;
			}
			else
			{
				init_dlmap_ie(&ie, diuc_code);
				ie_index++;
				ie->next = prev_ie;

				prev_ie = ie;
								
				// At least 2 DLMAP-IEs will be needed
				dl_ss_allocation[ii] = dl_ss_allocation[ii] - cur_slot_index - 1;
				build_partial_band_dlmap_ie(cur_slot_index + 1, ie, &cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_slots_in_freq);

				// When AMC module is implemented, replace by
				// ss_amc_info = get_ss_amc_info(ii);
				// diuc = ss_amc_info->diuc

				if (dl_ss_allocation[ii]%num_dl_slots_in_freq == 0)
				{
					while (dl_ss_allocation[ii]>0)
					{
						init_dlmap_ie(&ie, diuc_code);
						ie_index++;
						ie->next = prev_ie;

						prev_ie = ie;
						
						// Current PHY in FOAK has a restriction that it can't decode
						// if regions described by a DLMAP-IE span multiple slotsymbols
						// Hence build a new IE for each slotsymbol
						#ifdef FOAK_TEMP_FIX
							build_full_band_rect_dlmap_ie(num_dl_slots_in_freq, ie, \
							&cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
							dl_ss_allocation[ii] -= num_dl_slots_in_freq;
						#else
							build_full_band_rect_dlmap_ie(dl_ss_allocation[ii], ie, \
							&cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
							dl_ss_allocation[ii] = 0;
						#endif
					}
				}
				else if(dl_ss_allocation[ii] <= cur_slot_index)
				{
					init_dlmap_ie(&ie, diuc_code);
					ie_index++;
					ie->next = prev_ie;

					prev_ie = ie;
					build_partial_band_dlmap_ie(dl_ss_allocation[ii], ie, &cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_slots_in_freq);
					dl_ss_allocation[ii] = 0;
				}
				else
				{
					while (dl_ss_allocation[ii] >= num_dl_slots_in_freq)
					{
						init_dlmap_ie(&ie, diuc_code);
						ie_index++;
						ie->next = prev_ie;

						prev_ie = ie;
						
						// Current PHY in FOAK has a restriction that it can't decode
						// if regions described by a DLMAP-IE span multiple slotsymbols
						// Hence build a new IE for each slotsymbol
						#ifdef FOAK_TEMP_FIX
							build_full_band_rect_dlmap_ie(num_dl_slots_in_freq, ie, \
							&cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
							dl_ss_allocation[ii] -= num_dl_slots_in_freq;
						#else
							num_symbols_temp = dl_ss_allocation[ii]/num_dl_slots_in_freq;
							dl_ss_allocation[ii] -= (num_symbols_temp * num_dl_slots_in_freq);
							build_full_band_rect_dlmap_ie(num_symbols_temp * num_dl_slots_in_freq, ie, &cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_subch,num_dl_slots_in_freq);
						#endif
					}
					
					if (dl_ss_allocation[ii] == 0) {continue;}
					
					// Build third DLMAP-IE
					init_dlmap_ie(&ie, diuc_code);
					ie_index++;
					ie->next = prev_ie;
					prev_ie = ie;
					// ie->ie_index = ie_index;

					// When AMC module is implemented, replace by
					// ss_amc_info = get_ss_amc_info(ii);
					// diuc = ss_amc_info->diuc

					build_partial_band_dlmap_ie(dl_ss_allocation[ii], ie, &cur_ofdma_symbol_index, &cur_slot_index, cid, diuc_code, &dlmap_len_in_bits,num_dl_slots_in_freq);
					dl_ss_allocation[ii] = 0;
				}
			}
		}
	}
    init_dlmap_ie(&ie, 0);
	ie_index++;
    ie->next = prev_ie;
	prev_ie = ie;
	
	dl_map_ie *broadcast_dlmap_ie = ie;

    // TODO (future): Current setup assumes that STC is either enabled or disabled for the whole frame with same parameters
    // So there is only one STC_IE at the start of DLMAP. IF there are multiple STC zones, this logic will need to be augmented

    if (param_DL_STC_MATRIX_TYPE != -1) //i.e. if STC is enabled
    {
        if (cur_slot_index < num_dl_slots_in_freq - 1)
        {
            // If MIMO mode is enabled, start broadcast IE allocation from a 
			// new symbol since broadcast transmission has to be SISO. The 
			// start of the STC zone will be the symbol offset of the first data IE
            // Assumption: DL zones can change only at symbol boundaries
            stc_zone_start = cur_ofdma_symbol_index;
            cur_ofdma_symbol_index -= DL_SYMBOLS_PER_SLOT;
            cur_slot_index =  num_dl_slots_in_freq - 1;
        }
        else
        {
            stc_zone_start = cur_ofdma_symbol_index + DL_SYMBOLS_PER_SLOT;
		}
	    // Add an extended DLMAP-IE (STC Zone switch IE) to switch from no TX diversity to STC mode
    	init_dlmap_ie(&ie, DIUC_FOR_EXTENDED_DLMAP_IE);
	    ie_index++;
    	ie->next = prev_ie;
	    prev_ie = ie;
	    init_extended_ie(&(ie->extended_ie), STC_ZONE_IE);

        init_stc_zone_ie(ie->extended_ie, (BOOL)param_FHDC_FLAG, param_DL_STC_MATRIX_TYPE, stc_zone_start);
        dlmap_len_in_bits += STC_ZONE_IE_LEN;

    }
	// Add an extended DLMAP-IE (CID switch IE) to toggle INC_CID = 1
	init_dlmap_ie(&ie, DIUC_FOR_EXTENDED_DLMAP_IE);
	ie_index++;
	ie->next = prev_ie;
	prev_ie = ie;
	init_extended_ie(&(ie->extended_ie), CID_SWITCH_IE);
    dlmap_len_in_bits += CID_SWITCH_IE_LEN;

	// DIUC 0 reserved for Broadcast MCS
	cid = BROADCAST_CID;
	// The restriction for FOAK is that the broadcast burst should be described
	// by a single DLMAP-IE. Populate after DLMAP is built and length is known
	dlmap_len_in_bits += DLMAP_IE_LENGTH;

	*dlmap_length_in_slots = ceil((float)(dlmap_len_in_bits)/BROADCAST_DBPS);
	int dlmap_len_in_slotsymbols = ceil((float)(*dlmap_length_in_slots + NUM_FCH_SLOTS)/num_dl_slots_in_freq);
	
	if ((cur_ofdma_symbol_index < DL_SYMBOLS_PER_SLOT * dlmap_len_in_slotsymbols + 1) || \
		((cur_ofdma_symbol_index == DL_SYMBOLS_PER_SLOT * dlmap_len_in_slotsymbols + 1) && \
		 (cur_slot_index + 1 < dl_ss_allocation[NUM_SS])))
	{
		FLOG_ERROR("Not enough space for Broadcast burst. cur_ofdma_symbol_index: %d, dlmap_len_in_slotsymbols: %d, , cur_slot_index: %d, dl_ss_allocation[NUM_SS]: %d\n", cur_ofdma_symbol_index, dlmap_len_in_slotsymbols,cur_slot_index , dl_ss_allocation[NUM_SS]);
		assert(0);
	}
	
    //printf("DLMAP len slots: %d, broadcast_burst:%d ", *dlmap_length_in_slots, dl_ss_allocation[NUM_SS]);
	broadcast_dlmap_ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
	init_normal_diuc_ie(broadcast_dlmap_ie->normal_ie, cid);
	#if (DL_PERMUTATION_TYPE==2)
		broadcast_dlmap_ie->normal_ie->ofdma_triple_symbol_num = 1;
	#ifdef INTEGRATION_TEST
		broadcast_dlmap_ie->normal_ie->ofdma_Symbols_num = DL_SYMBOLS_PER_SLOT;
	#endif
	#endif
	broadcast_dlmap_ie->normal_ie->subchannels_num = dl_ss_allocation[NUM_SS];
	broadcast_dlmap_ie->normal_ie->ofdma_symbol_offset = DL_SYMBOLS_PER_SLOT * dlmap_len_in_slotsymbols + 1;
	broadcast_dlmap_ie->normal_ie->subchannel_offset = 0;
	dl_ss_allocation[NUM_SS] = 0;
		
	// Initialize the list head with the first broadcast DLMAP-IE
	dl_map->ie_head = ie; 
/**************************************
	int dlmap_end_slot_in_freq = (NUM_FCH_SLOTS + *dlmap_length_in_slots)%num_dl_slots_in_freq;
	int dlmap_end_ofdm_symbol = ((int)(NUM_FCH_SLOTS + *dlmap_length_in_slots)/num_dl_slots_in_freq)*DL_SYMBOLS_PER_SLOT + 1;
	int symbols_needed = ceil((float)dl_ss_allocation[NUM_SS]/num_dl_slots_in_freq)*DL_SYMBOLS_PER_SLOT;

	// The if-else condition finds a big enough region for the broadcast burst
	// If the broadcast burst can fit in the remaining slots of this symbol group
	if (dl_ss_allocation[NUM_SS] <= (num_dl_slots_in_freq - dlmap_end_slot_in_freq)*(cur_ofdma_symbol_index - dlmap_end_ofdm_symbol))
	{ // Option 1, as in Scheduler documentation (WimaxScheduler.ppt)
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);
		symbols_needed = ceil((float)dl_ss_allocation[NUM_SS]/(num_dl_slots_in_freq - dlmap_end_slot_in_freq))*DL_SYMBOLS_PER_SLOT;
#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = symbols_needed; 
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = symbols_needed/DL_SYMBOLS_PER_SLOT;
#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = symbols_needed;
#endif
#endif
	    ie->normal_ie->subchannels_num = num_dl_slots_in_freq - dlmap_end_slot_in_freq;
	    ie->normal_ie->ofdma_symbol_offset = dlmap_end_ofdm_symbol;
	    ie->normal_ie->subchannel_offset = dlmap_end_slot_in_freq;
		dl_ss_allocation[NUM_SS] = 0;
	}
	else if(cur_ofdma_symbol_index - dlmap_end_ofdm_symbol > symbols_needed)
	{ // Option 2, as in Scheduler documentation (WimaxScheduler.ppt)
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);
		
#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = symbols_needed; 
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = symbols_needed/DL_SYMBOLS_PER_SLOT;
#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = symbols_needed;
#endif
#endif
	    ie->normal_ie->subchannels_num = num_dl_slots_in_freq;
	    ie->normal_ie->ofdma_symbol_offset = dlmap_end_ofdm_symbol;
	    ie->normal_ie->subchannel_offset = 0;
		dl_ss_allocation[NUM_SS] = 0;
	}
	else if((dlmap_end_slot_in_freq < cur_slot_index) &&
		(dl_ss_allocation[NUM_SS] <= (1+(cur_ofdma_symbol_index - dlmap_end_ofdm_symbol)/DL_SYMBOLS_PER_SLOT)*(cur_slot_index - dlmap_end_slot_in_freq + 1)))
	{ // Option 3, as in Scheduler documentation (WimaxScheduler.ppt)
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);
		symbols_needed = ceil((float)dl_ss_allocation[NUM_SS]/(cur_slot_index-dlmap_end_slot_in_freq + 1))*DL_SYMBOLS_PER_SLOT;
#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = symbols_needed; 
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = symbols_needed/DL_SYMBOLS_PER_SLOT;
#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = symbols_needed;
#endif
#endif
	    ie->normal_ie->subchannels_num = cur_slot_index-dlmap_end_slot_in_freq + 1;
	    ie->normal_ie->ofdma_symbol_offset = dlmap_end_ofdm_symbol;
	    ie->normal_ie->subchannel_offset = dlmap_end_slot_in_freq;
		dl_ss_allocation[NUM_SS] = 0;
	}
	else if(dl_ss_allocation[NUM_SS] <= (cur_slot_index+1)*(cur_ofdma_symbol_index - dlmap_end_ofdm_symbol)/DL_SYMBOLS_PER_SLOT)
	{ // Option 4, as in Scheduler documentation (WimaxScheduler.ppt)
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);
		symbols_needed = ceil((float)dl_ss_allocation[NUM_SS]/(cur_slot_index+1))*DL_SYMBOLS_PER_SLOT;
#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = symbols_needed; 
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = symbols_needed/DL_SYMBOLS_PER_SLOT;
#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = symbols_needed;
#endif
#endif
	    ie->normal_ie->subchannels_num = cur_slot_index+1;
	    ie->normal_ie->ofdma_symbol_offset = dlmap_end_ofdm_symbol + DL_SYMBOLS_PER_SLOT;
	    ie->normal_ie->subchannel_offset = 0;
		dl_ss_allocation[NUM_SS] = 0;
	}
	else
	{
		FLOG_ERROR("Not enough space in frame for Broadcast burst as a single IE");		
	}
**************************************/	
//#ifdef DDEBUG
	//print_dlmap(dl_map);
//#endif
/*
#ifdef _DUMP_MAC_
     dump_dlmap(dl_map);
#endif
*/
    DO_DUMP(DUMP_MAC_DLMAP_ID, 0, dl_map, 1);
    return 0;
}

int print_dlmap(dl_map_msg *dl_map)
{
    stc_dl_zone_ie *stc_ie;
    mimo_dl_basic_ie *mubi;
	dl_map_ie *ie = dl_map->ie_head;
    printf("\nPrinting DLMAP for frame number: %ld", get_current_frame_number());
	while (ie != NULL)
  	{
      printf("\nIE index: %d, DIUC: %d\n", ie->ie_index, ie->diuc);
	  if (ie->normal_ie != NULL)
	    {
    	 	printf("N_CID: %d, CID: %d, OFDMA symbol offset: %d, subchannel offset: %d, num OFDMA symbols: %d, OFDMA triples: %d, num subchannels: %d",\
       	  	ie->normal_ie->n_cid, ie->normal_ie->cid[0], ie->normal_ie->ofdma_symbol_offset, ie->normal_ie->subchannel_offset, ie->normal_ie->ofdma_Symbols_num, ie->normal_ie->ofdma_triple_symbol_num, \
		ie->normal_ie->subchannels_num);
	    }
	  else if (ie->extended_ie != NULL)
	  {
		printf("Extended IE DIUC code: %d, Length: %d\n", ie->extended_ie->extended_diuc, ie->extended_ie->length);
        if(ie->extended_ie->unspecified_data != NULL)
        {
                if(ie->diuc == 14)
                {
            switch(ie->extended_ie->extended_diuc)
            {
                case MIMO_DL_BASIC_IE:
                    mubi = ie->extended_ie->unspecified_data;
                    printf("MIMO DL BASIC IE: Num region: %d\n", mubi->num_region);
                        region_attri *ra = mubi->region_header;
                            while (ra != NULL)
                        {
                                printf("OFDM sym offset: %d, Subch offset: %d, boost: %d, #symbols: %d, #Subchannels: %d, Matrix: %d, #Layers: %d \n",ra->ofdma_symbol_offset, ra->subchannel_offset, ra->boosting, ra->ofdma_symbols_num, ra->subchannels_num, ra->matrix_indicator, ra->num_layer);

                            layer_attri *la = ra->layer_header;
                                while(la != NULL)
                                {
                            printf("CID: %d, Layer Index: %d, DIUC: %d, RCI: %d\n", la->cid, la->layer_index, la->diuc, la->repetition_coding_indication);
                                    la = la->next;
                        }
                                ra = ra->next;
                    }
                    break;
                default:
                            printf("Can't print extended DIUC 2 IE. Unknown type\n");
                    }
                }
                else if (ie->diuc == 15)
                {
                    switch(ie->extended_ie->extended_diuc)
                    {
                        // Need to add more case blocks for other types of extd DIUC IDs
                        case STC_ZONE_IE:
                            stc_ie = ie->extended_ie->unspecified_data;
                            printf("STC Zone IE: OFDMA sym offset: %d, Perm: %d, All SC: %d, STC: %d, Matrix: %d, PermBase: %d, PRBS ID: %d, AMC: %d, Midamble present: %d, Midamble boost: %d, #Antenna: %d, Dedicated Pilots: %d\n", stc_ie->ofdma_symbol_offset, stc_ie->permutation, stc_ie->use_all_sc_indicator, stc_ie->stc, stc_ie->matrix_indicator ,stc_ie->dl_permbase , stc_ie->prbs_id ,stc_ie->amc_type ,stc_ie->midamble_presence , stc_ie->midamble_boosting, stc_ie->num_antenna_select, stc_ie->dedicated_pilots); 
                            break;
                        default:
                    printf("Can't print extended DIUC IE. Unknown type\n");
                    } // end swicth
            }
        }
	  }
	  ie = ie->next;
  	}
    printf("\nFinished printing DLMAP for frame number: %ld\n", get_current_frame_number());
  return 0;
}


int build_full_band_rect_dlmap_ie(int dl_ss_allocation, dl_map_ie* ie, int* cur_ofdma_symbol_index, int* cur_slot_index, int cid, int diuc, int *dlmap_len,int num_dl_subch, int num_dl_slots_in_freq)
{
    int jj = 0;
	// Num of slots needed makes a rectangular data region spanning the full
	// frequency band and possibly spanning multiple OFDMA symbol sets
	// Only one DLMAP-IE needed
	int num_symbols_temp = dl_ss_allocation/num_dl_slots_in_freq;
    int num_ofdma_symbols = DL_SYMBOLS_PER_SLOT * num_symbols_temp;
    
    (*cur_ofdma_symbol_index) -= num_ofdma_symbols;
	*cur_slot_index = num_dl_slots_in_freq - 1;

    if((param_DL_STC_MATRIX_TYPE == -1)||(cid == BROADCAST_CID))
    {
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);

#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = num_ofdma_symbols; 
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = num_symbols_temp;
#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = num_ofdma_symbols;
#endif
#endif
	    ie->normal_ie->subchannels_num = num_dl_subch;
	    ie->normal_ie->ofdma_symbol_offset = *cur_ofdma_symbol_index + DL_SYMBOLS_PER_SLOT;
	    ie->normal_ie->subchannel_offset = 0;
        (*dlmap_len) += DLMAP_IE_LENGTH;
    }
    else
    {
        // Construct a MIMO DL basic IE
        ie->diuc = 14;
        ie->extended_ie = (extended_diuc_ie*)mac_malloc(sizeof(extended_diuc_ie));
        ie->extended_ie->extended_diuc = MIMO_DL_BASIC_IE;
        mimo_dl_basic_ie *mdbi = (mimo_dl_basic_ie*)mac_malloc(sizeof(mimo_dl_basic_ie));
        ie->extended_ie->unspecified_data = mdbi;
        mdbi->num_region = 0;
        mdbi->region_header = (region_attri*)mac_malloc(sizeof(region_attri));
        init_region_attri(mdbi->region_header, param_DL_STC_MATRIX_TYPE); 

        if (param_DL_STC_MATRIX_TYPE == 1)
        {
            // Matrix B has 2 layers
            mdbi->region_header->num_layer = 1;
        }
        layer_attri *la = NULL, *prev_la = NULL;
        for (jj = 0; jj <= mdbi->region_header->num_layer; jj++)
        {
            la = (layer_attri*)mac_malloc(sizeof(layer_attri));
            if (jj == 0)
                mdbi->region_header->layer_header = la;
            else
                prev_la->next = la;
            init_layer_attri(la, cid, jj, diuc, 0);
            prev_la = la;
        }

	    mdbi->region_header->ofdma_symbols_num = num_ofdma_symbols;
    	mdbi->region_header->subchannels_num = num_dl_subch;

	    mdbi->region_header->ofdma_symbol_offset = *cur_ofdma_symbol_index + DL_SYMBOLS_PER_SLOT;
    	mdbi->region_header->subchannel_offset = 0;
        ie->extended_ie->length = ceil((float)(4 + (mdbi->num_region+1)*(REGION_ATTRI_SIZE + (mdbi->region_header->num_layer + 1)*LAYER_ATTRI_SIZE))/8);
        (*dlmap_len) += MIMO_DL_BASIC_IE_COMMON_LEN + 8 * ceil((float)(NUM_ASSIGN_LEN + REGION_ATTRI_SIZE + (mdbi->region_header->num_layer + 1)*LAYER_ATTRI_SIZE)/8); 
    }
	return 0;
}

int build_partial_band_dlmap_ie(int dl_ss_allocation, dl_map_ie* ie, int* cur_ofdma_symbol_index, int* cur_slot_index, int cid, int diuc, int *dlmap_len,int num_dl_slots_in_freq)
{
    int jj = 0;
    int subchannel_offset =  (*cur_slot_index - dl_ss_allocation) * DL_SUBCHANNEL_PER_SLOT + 1;
    if((param_DL_STC_MATRIX_TYPE == -1)||(cid == BROADCAST_CID))
    {
		ie->normal_ie = (normal_diuc_ie*)mac_malloc(sizeof(normal_diuc_ie));
		init_normal_diuc_ie(ie->normal_ie, cid);

#if (DL_PERMUTATION_TYPE==0)
	    ie->normal_ie->ofdma_Symbols_num = DL_SYMBOLS_PER_SLOT;
#endif
#if (DL_PERMUTATION_TYPE==2)
		ie->normal_ie->ofdma_triple_symbol_num = 1;

#ifdef INTEGRATION_TEST
            ie->normal_ie->ofdma_Symbols_num = DL_SYMBOLS_PER_SLOT;
#endif
#endif
    	ie->normal_ie->subchannels_num = dl_ss_allocation * DL_SUBCHANNEL_PER_SLOT;

	    ie->normal_ie->ofdma_symbol_offset = *cur_ofdma_symbol_index;
    	ie->normal_ie->subchannel_offset = subchannel_offset;
        (*dlmap_len) += DLMAP_IE_LENGTH;
    }
    else
    {
        // Construct a MIMO DL basic IE
        ie->diuc = 14;
        ie->extended_ie = (extended_diuc_ie*)mac_malloc(sizeof(extended_diuc_ie));
        ie->extended_ie->extended_diuc = MIMO_DL_BASIC_IE;
        mimo_dl_basic_ie *mdbi = (mimo_dl_basic_ie*)mac_malloc(sizeof(mimo_dl_basic_ie));
        ie->extended_ie->unspecified_data = mdbi;
        mdbi->num_region = 0;
        mdbi->region_header = (region_attri*)mac_malloc(sizeof(region_attri));
        init_region_attri(mdbi->region_header, param_DL_STC_MATRIX_TYPE); 

        if (param_DL_STC_MATRIX_TYPE == 1)
        {
            // Matrix B has 2 layers
            mdbi->region_header->num_layer = 1;
        }
        layer_attri *la = NULL, *prev_la = NULL;
        for (jj = 0; jj <= mdbi->region_header->num_layer; jj++)
        {
            la = (layer_attri*)mac_malloc(sizeof(layer_attri));
            if (jj == 0)
                mdbi->region_header->layer_header = la;
            else
                prev_la->next = la;
            init_layer_attri(la, cid, jj, diuc, 0);
            prev_la = la;
        }

	    mdbi->region_header->ofdma_symbols_num = DL_SYMBOLS_PER_SLOT;
    	mdbi->region_header->subchannels_num = dl_ss_allocation * DL_SUBCHANNEL_PER_SLOT;

	    mdbi->region_header->ofdma_symbol_offset = *cur_ofdma_symbol_index;
    	mdbi->region_header->subchannel_offset = subchannel_offset;
        ie->extended_ie->length = ceil((float)(4 + (mdbi->num_region+1)*(REGION_ATTRI_SIZE + (mdbi->region_header->num_layer + 1)*LAYER_ATTRI_SIZE))/8);
        (*dlmap_len) += MIMO_DL_BASIC_IE_COMMON_LEN + 8 * ceil((float)(NUM_ASSIGN_LEN + REGION_ATTRI_SIZE + (mdbi->region_header->num_layer + 1)*LAYER_ATTRI_SIZE)/8); 
    }
	if (subchannel_offset == 0)
    {
	    // If the current symbol is full
	    *cur_ofdma_symbol_index -= DL_SYMBOLS_PER_SLOT;
	    *cur_slot_index = num_dl_slots_in_freq - 1;
	}
	else
	{
	    *cur_slot_index -= dl_ss_allocation;
	}
	return 0;
}

int init_layer_attri(layer_attri *la, int cid, int layer_index, int diuc, int rci)
{
    la->cid = cid;
    la->layer_index = layer_index;
    la->diuc = diuc;
    la->repetition_coding_indication = rci;
    la->next = NULL;
    return 0;
}

int init_region_attri(region_attri *ra, stc_matrix_type matrix_ind)
{
    ra->ofdma_symbol_offset = 0;
    ra->subchannel_offset = 0;
    ra->boosting = 0;
    ra->ofdma_symbols_num = 0;
    ra->subchannels_num = 0;
    ra->packet_index = 0;
    ra->matrix_indicator = matrix_ind;
    ra->num_layer = 0;
    ra->layer_header = NULL; 
    ra->number_of_users = 0;
    ra->user_header = NULL;
    ra->rcidie = NULL;
    ra->diuc = 0;
    ra->repetition_coding_indication = 0;
    ra->antenna_grouping_index = 0;
    ra->num_stream = 0;
    ra->codebook_precoding_index = 0;
    ra->num_ms = 0;
    ra->ms_header = NULL;
    ra->next = NULL; 
    return 0;
}
int init_dlmap_ie(dl_map_ie **ie_ptr, int diuc_code)
{
	dl_map_ie *ie = (dl_map_ie*)mac_malloc(sizeof(dl_map_ie));
	ie->ie_index = 0;
	ie->diuc = diuc_code;
	ie->normal_ie = NULL;
	ie->extended_ie = NULL;
	ie->next = NULL;
	
	// Return the memory allocated here
	*ie_ptr = ie;
	return 0;
}

int init_normal_diuc_ie(normal_diuc_ie* normal_ie, u_int16_t cid)
{
	// These fields are not used currently. Set to default.
	normal_ie->n_cid = 1;
	normal_ie->cid = (u_int16_t*)mac_malloc(sizeof(u_int16_t) * normal_ie->n_cid); 
	normal_ie->cid[0] = cid;
	normal_ie->repetition_coding_indication = 0;
	normal_ie->boosting = 0;
	normal_ie->ofdma_triple_symbol_num = 0;
	normal_ie->ofdma_Symbols_num = 0; 
	return 0;
}

int init_extended_ie(extended_diuc_ie **ie_ptr, int extd_diuc)
{
	extended_diuc_ie *ie = (extended_diuc_ie*)mac_malloc(sizeof(extended_diuc_ie));
	ie->extended_diuc = extd_diuc;
	ie->length = 0;
	ie->unspecified_data = NULL;
	*ie_ptr = ie;
	return 0;
}

int init_stc_zone_ie(extended_diuc_ie *ie, BOOL fhdc_flag, stc_matrix_type matrix_ind, int stc_zone_start)
{
    ie->extended_diuc = STC_ZONE_IE;
    ie->length = 0x04; //Specified in T327 of spec

    stc_dl_zone_ie *stc_ie = (stc_dl_zone_ie*)mac_malloc(sizeof(stc_dl_zone_ie));
    stc_ie->ofdma_symbol_offset = stc_zone_start;
    stc_ie->permutation = 0; //Currently only PUSC supported
    stc_ie->use_all_sc_indicator = 1; //1: All subchannels used, 0: all not used
    if (fhdc_flag == TRUE)
    {
        stc_ie->stc=0x11;
    }
    else
    {
        stc_ie->stc=0x01; // Presently only 2 antenna STC supported
    }
    stc_ie->matrix_indicator = matrix_ind;
    stc_ie->dl_permbase = DL_PERMBASE;
    stc_ie->prbs_id = PRBS_ID;
    stc_ie->amc_type = 0; // AMC not supported currently
    stc_ie->midamble_presence = 0;
    stc_ie->midamble_boosting = 0;
    stc_ie->num_antenna_select = 0; //0: 2 antennas, 1: 3 antennas for STC
    stc_ie->dedicated_pilots = 0;

    ie->unspecified_data = stc_ie;
    return 0;
}

int dump_dlmap(FILE* fp_dlmap, dl_map_msg *dl_map)
{
    stc_dl_zone_ie *stc_ie;
    mimo_dl_basic_ie *mubi;
    dl_map_ie *ie = dl_map->ie_head;
   // static long loop_num = 0;
   // static FILE *fp_dlmap;
   // if (loop_num ++ == 0)
   // {
   //     fp_dlmap = fopen("dl_map.out", "w+");
   // }

    fprintf(fp_dlmap,"----------Printing DLMAP, FrameNumber:%ld---------------------------------------------\n", get_current_frame_number());
    while (ie != NULL)
    {
        fprintf(fp_dlmap,"IE index: %d, DIUC: %d\n", ie->ie_index, ie->diuc);
        if (ie->normal_ie != NULL)
        {
            fprintf(fp_dlmap,"N_CID: %d, CID:%d, SymbolOffset:%d, SubChanOffset:%d, SymbolNum:%d, SubChanNum:%d, AllocatedSlot:%d\n",\
                    ie->normal_ie->n_cid, ie->normal_ie->cid[0], ie->normal_ie->ofdma_symbol_offset, ie->normal_ie->subchannel_offset,\
                    ie->normal_ie->ofdma_Symbols_num, ie->normal_ie->subchannels_num, ie->normal_ie->ofdma_Symbols_num / 3 * ie->normal_ie->subchannels_num);
        }
        else if (ie->extended_ie != NULL)
        {
            fprintf(fp_dlmap,"Extended IE DIUC code: %d, Length: %d\n", ie->extended_ie->extended_diuc, ie->extended_ie->length);
            if(ie->extended_ie->unspecified_data != NULL)
            {
                if(ie->diuc == 14)
                {
                    switch(ie->extended_ie->extended_diuc)
                    {
                        case MIMO_DL_BASIC_IE:
                            mubi = ie->extended_ie->unspecified_data;
                            fprintf(fp_dlmap,"MIMO DL BASIC IE: Num region: %d\n", mubi->num_region);
                            region_attri *ra = mubi->region_header;
                            while (ra != NULL)
                            {
                                fprintf(fp_dlmap,"OFDM sym offset: %d, Subch offset: %d, boost: %d, #symbols: %d, #Subchannels: %d, Matrix: %d, #Layers: %d \n",\
                                ra->ofdma_symbol_offset, ra->subchannel_offset, ra->boosting, ra->ofdma_symbols_num, ra->subchannels_num, ra->matrix_indicator, ra->num_layer);

                                layer_attri *la = ra->layer_header;
                                while(la != NULL)
                                {
                                    fprintf(fp_dlmap,"CID: %d, Layer Index: %d, DIUC: %d, RCI: %d\n", la->cid, la->layer_index, la->diuc, la->repetition_coding_indication);
                                    la = la->next;
                                }
                                ra = ra->next;
                            }    
                            break;
                        default:
                            fprintf(fp_dlmap, "Can't print extended DIUC 2 IE. Unknown type\n");
                    }
                }
                else if (ie->diuc == 15)
                {
                    switch(ie->extended_ie->extended_diuc)
                    {
                        // Need to add more case blocks for other types of extd DIUC IDs
                        case STC_ZONE_IE:
                            stc_ie = ie->extended_ie->unspecified_data;
                            fprintf(fp_dlmap,"STC Zone IE: OFDMA sym offset: %d, Perm: %d, All SC: %d, STC: %d, Matrix: %d,\
                                              PermBase: %d, PRBS ID: %d, AMC: %d, Midamble present: %d, Midamble boost: %d, #Antenna: %d, Dedicated Pilots: %d\n",\
                                              stc_ie->ofdma_symbol_offset, stc_ie->permutation, stc_ie->use_all_sc_indicator, stc_ie->stc, stc_ie->matrix_indicator,\
                                              stc_ie->dl_permbase , stc_ie->prbs_id ,stc_ie->amc_type ,stc_ie->midamble_presence , stc_ie->midamble_boosting,\
                                              stc_ie->num_antenna_select, stc_ie->dedicated_pilots);
                            break;
                        default:
                        fprintf(fp_dlmap,"Can't print extended DIUC IE. Unknown type\n");
                    } // end swicth
                } 
            } 
        }
        ie = ie->next;
    }
   //fprintf(fp_dlmap,"\nFinished printing DLMAP for frame number: %ld\n", get_current_frame_number());
    return 0;
}

                              
