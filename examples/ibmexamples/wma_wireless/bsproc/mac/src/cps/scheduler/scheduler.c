/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: scheduler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <math.h>
#include "scheduler.h"
#include "debug.h"

int ulmap_len_in_bytes;
int num_ul_symbols, num_dl_symbols;
int num_bytes_in_dlslotsymbol;
extern pthread_mutex_t spectrum_measure_flag_lock;
extern int spectrum_measure_flag;
int schedule(sdu_queue* dl_sdu_queue, br_queue **br_q_list, amc_info* amc_info_header, logical_dl_subframe_map* frame_map,int num_dl_subch, int num_ul_subch)
{
  int num_ul_slots = 0, num_dl_slots = 0, dlmap_len_in_slots = 0;
  float dl_capacity = 0, ul_capacity = 0;

  // Calculate theoretical capacity of DL subframe
  calculate_phy_capacity(NUM_SS, frame_map, &dl_capacity, &ul_capacity, &num_dl_slots, &num_ul_slots,num_dl_subch, num_ul_subch);

  // Call the UL scheduler
  int temp_flag=-1;
pthread_mutex_lock(&spectrum_measure_flag_lock);
  temp_flag = spectrum_measure_flag;
pthread_mutex_unlock(&spectrum_measure_flag_lock);
  if (temp_flag == -1) {FLOG_FATAL("Something wrong iwth tempflag assignment");exit(-1);}
  if (temp_flag == 0)
  {
         //Go ahead with usual ulmap allocations. No measurement this frame.

	  ul_scheduler(br_q_list, amc_info_header, num_ul_slots, num_ul_symbols, NUM_SS, frame_map->ul_map);

  	// Use the allocation given by ul_scheduler to construct the ULMAP
  	//ul_map_builder(ul_ss_allocation, num_ul_symbols, NUM_SS, frame_map->ul_map);
  }
  else
  {
	//Nothing here.
	  ul_scheduler(br_q_list, amc_info_header, 0, num_ul_symbols, NUM_SS, frame_map->ul_map);
#ifdef DDEBUG
	if (frame_map->ul_map != NULL) print_ulmap(frame_map->ul_map);
	else FLOG_FATAL("ULMAP is NULL\n");
#endif
  }
   

  dl_scheduler(dl_sdu_queue, amc_info_header, num_dl_slots, frame_map, &dlmap_len_in_slots,num_dl_subch);

  // Construct the dl_subframe_prefix (FCH) - need DL map length for this, 
  // hence called after DL scheduler
  fch_builder(frame_map->fch, dlmap_len_in_slots);
  return 0;
}

int fch_builder(dl_subframe_prefix* fch, int dlmap_len_in_slots)
{
  // Assume all subchannels are used
  fch->used_subchannel_bitmap = 63;
  fch->rsv1 = 0;

  // Default values
  fch->repetition_coding_indication = 0;
  fch->coding_indication = 0;

  // define the length in slots of the burst which contains only 
  // DL-MAP message or compressed DL-MAP messge and compressed UL-MAP.
  fch->dl_map_length = dlmap_len_in_slots; 
  fch->rsv2 = 0;
  
  return 0;
}

int calculate_phy_capacity(int num_ss, logical_dl_subframe_map *frame_map, float* dl_capacity, float* ul_capacity, int* num_dl_slots, int* num_ul_slots,int num_dl_subch, int num_ul_subch)
{
  //int num_usable_symbols;
  int num_dlmap_bits, num_ulmap_bits;

  // These settings are assuming OFDM symbol duration of 102.9 microsec, 
  // which depends on B/W, NUM_FFT, oversampling factor, guard ratio etc. This assumption holds
  // for our (Bw = 20 MHz, NUM_FFT = 2048) and (Bw = 10 MHz with NUM_FFT = 1024) test scenarios
  // num_dl_symbols is after subtracting the 1 preamble symbol and TTG/RTG allowance of min 5 microsec
  // Numbers are chosen s.t. num_dl_symbols are multiples of DL_SYMBOLS_PER_SLOT and
  // num_ul_symbols are multiples of UL_SYMBOLS_PER_SLOT, so that full slots can be formed
 
  if (FRAME_DURATION_CODE == 8)
  {
	//only 20ms frames are allowed
	 if (DL_UL_RATIO == 0.667) //10:15 ratio
	 {
		num_dl_symbols = 9;
		num_ul_symbols = 15;
	 }	
	else if (DL_UL_RATIO ==  1.778) //16:9 ratio
	{
		num_dl_symbols = 15;
		num_ul_symbols = 9;
	}
	else if(DL_UL_RATIO ==  1.083) //13:12 ratio
	{
		num_dl_symbols = 12;
		num_ul_symbols = 12;
	}
	else if (DL_UL_RATIO == 0.389)
	{
		num_dl_symbols = 6;
		num_ul_symbols = 18;
	}
	else
	{	
		FLOG_FATAL("This DL_UL_Ratio is not supported\n");
		return -1;
	}
   }
   else
   {
	FLOG_FATAL("Wrong FDC. Only 20ms frames expected\n");
	return -1;
   }

  if ((frame_map != NULL) && (frame_map->dl_map != NULL))
  {
  // The number of OFDMA symbols indicated in the DLMAP should include the preamble symbol, hence +1
  frame_map->dl_map->ofdma_symbols_num = num_dl_symbols + 1;
  }

  FLOG_DEBUG("\nFDC: %d, num_ul_symbol: %d, num_dl_symbols: %d", FRAME_DURATION_CODE, num_ul_symbols, num_dl_symbols);
  // DL capacity calculation. This is approximate assuming worst case overhead
  *num_dl_slots = floor(num_dl_symbols/DL_SYMBOLS_PER_SLOT) * floor(num_dl_subch/DL_SUBCHANNEL_PER_SLOT);//Changed according to interference info
  //printf("Nm DL slots %d\n",*num_dl_slots);
  
  num_bytes_in_dlslotsymbol = floor(num_dl_subch/DL_SUBCHANNEL_PER_SLOT)*BROADCAST_DBPS/8;

  // Calculate worst case number of slots occupied by ULMAP and DLMAP
  num_dlmap_bits = PDU_MIN_OVERHEAD * 8 + DL_FIXED_FIELD_LENGTH + \
    num_ss * DLMAP_IE_LENGTH;
  num_ulmap_bits = PDU_MIN_OVERHEAD * 8 + (UL_FIXED_FIELD_LENGTH + \
					   num_ss * ULMAP_IE_LENGTH) + RANGING_ULMAP_IE_LENGTH;
  ulmap_len_in_bytes = ceil((float)num_ulmap_bits/8);

  // subtract other map-element overheads to get data slots available
  int num_dl_data_slots = *num_dl_slots - NUM_FCH_SLOTS - \
    ceil((float)(num_dlmap_bits + num_ulmap_bits)/DL_UL_MAP_DATA_BITS_PER_SLOT);
  *dl_capacity = num_dl_data_slots * DL_DATA_CAR_PER_SLOT * bits_per_car[param_DL_MCS];
  //printf("In SCH, num_dl_symbols: %d, num_dl_subch: %d, num_dl_data_slots: %d,num_dl_slots: %d, dl_capacity: %f\n ", num_dl_symbols, num_dl_subch, num_dl_data_slots, *num_dl_slots, *dl_capacity);
 
  // UL Capacity calculation
  *num_ul_slots = floor(num_ul_symbols/UL_SYMBOLS_PER_SLOT) * \
    floor((num_ul_subch - NUM_RANGING_SUBCHANNELS)/UL_SUBCHANNEL_PER_SLOT) + \
	floor((num_ul_symbols - NUM_RANGING_SYMBOLS)/UL_SYMBOLS_PER_SLOT) * NUM_RANGING_SUBCHANNELS;
  //printf("Num UL Slots %d\n",*num_ul_slots);
  *ul_capacity = (*num_ul_slots - UL_SLOT_OVERHEAD) * UL_DATA_CAR_PER_SLOT * \
    bits_per_car[param_UL_MCS]; 

  //printf("Approximate capacity (in bits) for a %f ms frame:\nDL: %f\nUL: %f\n", frame_duration[FRAME_DURATION_CODE], *dl_capacity, *ul_capacity);

  return 0;
}
