/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: ranging_mm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Jul.2009       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "ul_mgt_msg_queue.h"
#include "dl_exp_params.h"
#include "ranging_mm.h"

extern sll_fifo_q *ranging_q;
int ms_ranging_flag;
int num_ranging_retries;
int num_rng_opps_toskip;
int rng_backoff_window;
int8_t power_adjust;
int32_t timing_adjust, frequency_adjust;
extern volatile int ulmap_ccc;
extern volatile int dlmap_ccc;
extern pthread_t ss_ir_thread, ss_pr_thread;


int init_rng_rsp_msg(rng_rsp_msg *rng_rsp, ranging_adjust_ie *p_rng_adjust)
{
	rng_rsp->management_message_type = RNG_RSP; 
	rng_rsp->rsv = 0; // set to 0

	// If length in a TLV = 0, means that TLV is not included 
	// Set the length fields of all TLVs = 0, they will be properly
	// initialized if the TLV is included/decoded
	rng_rsp->timing_adjust.type = 1;
	rng_rsp->timing_adjust.length = 0;

	rng_rsp->power_adjust.type = 2;
	rng_rsp->power_adjust.length = 0;

	rng_rsp->frequency_adjust.type = 3;
	rng_rsp->frequency_adjust.length = 0;

	rng_rsp->ranging_status.type = 4; 
	rng_rsp->ranging_status.length = 1; 
	rng_rsp->ranging_status.value = RNG_CONTINUE; 

	if (p_rng_adjust != NULL)
	{
		if (p_rng_adjust->timing_adjust != 0)
		{
			rng_rsp->timing_adjust.length = 4;
			rng_rsp->timing_adjust.value = p_rng_adjust->timing_adjust;
		}
		if (p_rng_adjust->power_adjust != 0)
		{
			rng_rsp->power_adjust.length = 1;
			rng_rsp->power_adjust.value = p_rng_adjust->power_adjust;
		}
		if (p_rng_adjust->frequency_adjust != 0)
		{
			rng_rsp->frequency_adjust.length = 4;
			rng_rsp->frequency_adjust.value = p_rng_adjust->frequency_adjust;
		}

		rng_rsp->ranging_status.length = 1;

		// Initialize the ranging status to continue, if corrections are
		// below threshold, change it to success (or choose some other logic)
		if ((abs(p_rng_adjust->power_adjust) <= param_RANGING_POWER_THD) && \
		(abs(p_rng_adjust->frequency_adjust) <= FREQUENCY_THRESH) && \
		(abs(p_rng_adjust->timing_adjust) <= TIMING_THRESH))
		{
			rng_rsp->ranging_status.value = RNG_SUCCESS;
		}
	}

	rng_rsp->dl_frequency_override.type = 5;
	rng_rsp->dl_frequency_override.length = 0;

	rng_rsp->ul_channel_id_override.type = 6;
	rng_rsp->ul_channel_id_override.length = 0;

	rng_rsp->dl_burst_profile.type = 7;
	rng_rsp->dl_burst_profile.length = 0;

	rng_rsp->ss_mac.type = 8;
	rng_rsp->ss_mac.length = 0;

	rng_rsp->basic_cid.type = 9;
	rng_rsp->basic_cid.length = 0;

	rng_rsp->primary_cid.type = 10;
	rng_rsp->primary_cid.length = 0;

	rng_rsp->aas_broadcast_perm.type = 11;
	rng_rsp->aas_broadcast_perm.length = 0;

	rng_rsp->frame_number.type = 12;
	rng_rsp->frame_number.length = 0;

	rng_rsp->init_ranging_opp_num.type = 13;
	rng_rsp->init_ranging_opp_num.length = 0;

	rng_rsp->service_level_prediction.type = 17;
	rng_rsp->service_level_prediction.length = 0;

	rng_rsp->resource_retain_flag.type = 20;
	rng_rsp->resource_retain_flag.length = 0;

	rng_rsp->ho_process_optimization.type = 21;
	rng_rsp->ho_process_optimization.length = 0;

	rng_rsp->ho_id.type = 22;
	rng_rsp->ho_id.length = 0;

	rng_rsp->location_update_response.type = 23;
	rng_rsp->location_update_response.length = 0;

	rng_rsp->dl_burst_profile_ofdma.type = 33;
	rng_rsp->dl_burst_profile_ofdma.length = 0;

	rng_rsp->rendezvous_time.type = 36;
	rng_rsp->rendezvous_time.length = 0;

	rng_rsp->cdma_code.type = 37;
	rng_rsp->cdma_code.length = 0;

	rng_rsp->tx_opp_offset.type = 38;
	rng_rsp->tx_opp_offset.length = 0;

#ifdef _OFDM_
	rng_rsp->ranging_subchannel.type = 150;
	rng_rsp->ranging_subchannel.length = 0;
#else
#ifdef _OFDMA_

	rng_rsp->ranging_code_attributes.type = 150;
	if (p_rng_adjust != NULL)
	{
		rng_rsp->ranging_code_attributes.length = 4;
		rng_rsp->ranging_code_attributes.value = (p_rng_adjust->frame_num_index + \
		(p_rng_adjust->ranging_code << 8) + ((p_rng_adjust->ranging_subchannel & 0x3f) << 16) + \
		(p_rng_adjust->ranging_symbol << 22));
	}
	else
	{
		rng_rsp->ranging_code_attributes.length = 0;
	}
#endif
#endif
	return 0;
}

int init_rx_rng_req_msg(rng_req_msg *rng_req)
{
	rng_req->management_message_type = RNG_REQ; 
	rng_req->rsv = 0; // set to 0

	// If length in a TLV = 0, means that TLV is not included 
	// Set the length fields of all TLVs = 0, they will be properly
	// initialized if the TLV is included/decoded
	rng_req->dl_burst_profile.type = 1;	
	rng_req->dl_burst_profile.length = 0;	

	rng_req->ss_mac.type = 2;
	rng_req->ss_mac.length = 0;

	rng_req->ranging_anomalies.type = 3;
	rng_req->ranging_anomalies.length = 0;

	rng_req->aas_broadcast_capability.type = 4;	
	rng_req->aas_broadcast_capability.length = 0;	

	rng_req->serving_bs_id.type = 5;
	rng_req->serving_bs_id.length = 0;

	rng_req->rng_purpose_indication.type = 6;
	rng_req->rng_purpose_indication.length = 0;

	rng_req->ho_id.type = 7;	
	rng_req->ho_id.length = 0;	

	rng_req->power_down_indicator.type = 8;
	rng_req->power_down_indicator.length = 0;

	rng_req->dl_rep_coding_level.type = 12;
	rng_req->dl_rep_coding_level.length = 0;

	rng_req->cmac_key_count.type = 13;
	rng_req->cmac_key_count.length = 0;

	return 0;
}

int init_tx_rng_req_msg(rng_req_msg *rng_req)
{
	rng_req->management_message_type = RNG_REQ; 
	rng_req->rsv = 0; // set to 0

	// Here TLV lengths are initialized to values specified in Sec 11.5
	rng_req->dl_burst_profile.type = 1;	
	rng_req->dl_burst_profile.length = 1;	

	rng_req->ss_mac.type = 2;
	rng_req->ss_mac.length = 6;

	rng_req->ranging_anomalies.type = 3;
	rng_req->ranging_anomalies.length = 1;

	rng_req->aas_broadcast_capability.type = 4;	
	rng_req->aas_broadcast_capability.length = 1;	

	rng_req->serving_bs_id.type = 5;
	rng_req->serving_bs_id.length = 6;

	rng_req->rng_purpose_indication.type = 6;
	rng_req->rng_purpose_indication.length = 1;

	rng_req->ho_id.type = 7;	
	rng_req->ho_id.length = 1;	

	rng_req->power_down_indicator.type = 8;
	rng_req->power_down_indicator.length = 1;

	rng_req->dl_rep_coding_level.type = 12;
	rng_req->dl_rep_coding_level.length = 1;

	rng_req->cmac_key_count.type = 13;
	rng_req->cmac_key_count.length = 2;

	return 0;
}

int parse_rng_rsp(u_char *payload, int mm_len, rng_rsp_msg **rng_rsp_mm)
{
	rng_rsp_msg *rng_rsp = (rng_rsp_msg*)malloc(sizeof(rng_rsp_msg));
	init_rng_rsp_msg(rng_rsp, NULL);
	*rng_rsp_mm = rng_rsp;

	if (payload[0] != RNG_RSP)
	{
		FLOG_WARNING("Error in parse_rng_rsp: MM_type in Input is not RNG_RSP");
		return -1;
	}
	// First 2 bytes have mm_type and reserved bits (set to 0)
	int len = 2; 	
	tlv8 *tmp_tlv8 = NULL;
	tlv_s8 *tmp_tlvs8 = NULL;
	tlv16 *tmp_tlv16 = NULL;
	tlv24 *tmp_tlv24 = NULL;
	tlv32 *tmp_tlv32 = NULL;
	tlv_s32 *tmp_tlvs32 = NULL;
	tlv48 *tmp_tlv48 = NULL;

	// Traverse the length of the payload, decoding TLVs one by one
	while(len < mm_len)
	{
		//printf("TLV type is: %d, starting memory is: %x\n", payload[len], payload + len);
		// Switch on type field of the next TLV
		switch(payload[len])
		{
			case 1:
				// Currently assumes 1 byte is enough for length
				// Extended length in TLV not supported
				tmp_tlvs32 = (tlv_s32 *)(payload + len);
				rng_rsp->timing_adjust = *tmp_tlvs32;
                                rng_rsp->timing_adjust.value = RNG_REVL(rng_rsp->timing_adjust.value);
				len = len + rng_rsp->timing_adjust.length + 2;
				break;
			case 2:
				tmp_tlvs8 = (tlv_s8 *)(payload + len);
				rng_rsp->power_adjust = *tmp_tlvs8;
				len = len + rng_rsp->power_adjust.length + 2;
				break;
			case 3:
				tmp_tlvs32 = (tlv_s32 *)(payload + len);
				rng_rsp->frequency_adjust = *tmp_tlvs32;
                                rng_rsp->frequency_adjust.value = RNG_REVL(rng_rsp->frequency_adjust.value);
				len = len + rng_rsp->frequency_adjust.length + 2;
				break;
			case 4:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->ranging_status = *tmp_tlv8;
				len = len + rng_rsp->ranging_status.length + 2;
				break;
			case 5:
				tmp_tlv32 = (tlv32 *)(payload + len);
				rng_rsp->dl_frequency_override = *tmp_tlv32;
                                rng_rsp->dl_frequency_override.value = RNG_REVL(rng_rsp->dl_frequency_override.value);
				len = len + rng_rsp->dl_frequency_override.length + 2;
				break;
			case 6:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->ul_channel_id_override = *tmp_tlv8;
				len = len + rng_rsp->ul_channel_id_override.length + 2;
				break;			
			case 7:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_rsp->dl_burst_profile = *tmp_tlv16;
                                rng_rsp->dl_burst_profile.value = RNG_REVS(rng_rsp->dl_burst_profile.value);
				len = len + rng_rsp->dl_burst_profile.length + 2;
				break;
			case 8:
				tmp_tlv48 = (tlv48 *)(payload + len);
				rng_rsp->ss_mac = *tmp_tlv48;
				len = len + rng_rsp->ss_mac.length + 2;
				break;
			case 9:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_rsp->basic_cid = *tmp_tlv16;
                                rng_rsp->basic_cid.value = RNG_REVS(rng_rsp->basic_cid.value);
				len = len + rng_rsp->basic_cid.length + 2;
				break;
			case 10:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_rsp->primary_cid = *tmp_tlv16;
                                rng_rsp->primary_cid.value = RNG_REVS(rng_rsp->primary_cid.value);
				len = len + rng_rsp->primary_cid.length + 2;
				break;
			case 11:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->aas_broadcast_perm = *tmp_tlv8;
				len = len + rng_rsp->aas_broadcast_perm.length + 2;
				break;
			case 12:
				tmp_tlv24 = (tlv24 *)(payload + len);
				rng_rsp->frame_number = *tmp_tlv24;
				len = len + rng_rsp->frame_number.length + 2;
				break;
			case 13:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->init_ranging_opp_num = *tmp_tlv8;
				len = len + rng_rsp->init_ranging_opp_num.length + 2;
				break;
			case 17:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->service_level_prediction = *tmp_tlv8;
				len = len + rng_rsp->service_level_prediction.length + 2;
				break;
			case 20:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->resource_retain_flag = *tmp_tlv8;
				len = len + rng_rsp->resource_retain_flag.length + 2;
				break;
			case 21:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_rsp->ho_process_optimization = *tmp_tlv16;
                                rng_rsp->ho_process_optimization.value = RNG_REVS(rng_rsp->ho_process_optimization.value);
				len = len + rng_rsp->ho_process_optimization.length + 2;
				break;
		    case 22:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->ho_id = *tmp_tlv8;
				len = len + rng_rsp->ho_id.length + 2;
				break;
	        case 23:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->location_update_response = *tmp_tlv8;
				len = len + rng_rsp->location_update_response.length + 2;
				break;
			case 33:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_rsp->dl_burst_profile_ofdma = *tmp_tlv16;
                                rng_rsp->dl_burst_profile_ofdma.value = RNG_REVS(rng_rsp->dl_burst_profile_ofdma.value);
				len = len + rng_rsp->dl_burst_profile_ofdma.length + 2;
				break;
			case 36:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->rendezvous_time = *tmp_tlv8;
				len = len + rng_rsp->rendezvous_time.length + 2;
				break;
			case 37:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->cdma_code = *tmp_tlv8;
				len = len + rng_rsp->cdma_code.length + 2;
				break;
			case 38:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->tx_opp_offset = *tmp_tlv8;
				len = len + rng_rsp->tx_opp_offset.length + 2;
				break;
			case 150:
#ifdef _OFDM_
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_rsp->ranging_subchannel = *tmp_tlv8;
				len = len + rng_rsp->ranging_subchannel.length + 2;
#endif
#ifdef _OFDMA_
				tmp_tlv32 = (tlv32 *)(payload + len);
				rng_rsp->ranging_code_attributes = *tmp_tlv32;
                               // rng_rsp->ranging_code_attributes.value = RNG_REVL(rng_rsp->ranging_code_attributes.value);
				len = len + rng_rsp->ranging_code_attributes.length + 2;
#endif		
				break;
			default:
				FLOG_WARNING("Error in parse_rng_rsp: Unrecognized TLV type\n");
				return -1;
		}
	}
	return 0;
}

int parse_rng_req(u_char *payload, int mm_len, rng_req_msg **rng_req_mm)
{
	rng_req_msg *rng_req = (rng_req_msg*)malloc(sizeof(rng_req_msg));
	init_rx_rng_req_msg(rng_req);
	*rng_req_mm = rng_req;

	if (payload[0] != RNG_REQ)
	{
		FLOG_WARNING("Error in parse_rng_req: MM_type in Input is not RNG_REQ");
		return -1;
	}
	// First 2 bytes have mm_type and reserved bits (set to 0)
	int len = 2; 	
	tlv8 *tmp_tlv8 = NULL;
	tlv16 *tmp_tlv16 = NULL;
	tlv48 *tmp_tlv48 = NULL;

	// Traverse the length of the payload, decoding TLVs one by one
	while(len < mm_len)
	{
		// Switch on type field of the next TLV
		switch(payload[len])
		{		
			case 1:
				// Currently assumes 1 byte is enough for length
				// Extended length in TLV not supported
				tmp_tlv8 = (tlv8*)(payload + len);
				rng_req->dl_burst_profile = *tmp_tlv8;
				len = len + rng_req->dl_burst_profile.length + 2;
				break;
			case 2:
				tmp_tlv48 = (tlv48 *)(payload + len);
				rng_req->ss_mac = *tmp_tlv48;
				len = len + rng_req->ss_mac.length + 2;
				break;
			case 3:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->ranging_anomalies = *tmp_tlv8;
				len = len + rng_req->ranging_anomalies.length + 2;
				break;
			case 4:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->aas_broadcast_capability = *tmp_tlv8;
				len = len + rng_req->aas_broadcast_capability.length + 2;
				break;	
			case 5:
				tmp_tlv48 = (tlv48 *)(payload + len);
				rng_req->serving_bs_id = *tmp_tlv48;
				len = len + rng_req->serving_bs_id.length + 2;
				break;
			case 6:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->rng_purpose_indication = *tmp_tlv8;
				len = len + rng_req->rng_purpose_indication.length + 2;
				break;
			case 7:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->ho_id = *tmp_tlv8; 
				len = len + rng_req->ho_id.length + 2;
				break;
			case 8:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->power_down_indicator = *tmp_tlv8;
				len = len + rng_req->power_down_indicator.length + 2;
				break;
			case 12:
				tmp_tlv8 = (tlv8 *)(payload + len);
				rng_req->dl_rep_coding_level = *tmp_tlv8;
				len = len + rng_req->dl_rep_coding_level.length + 2;
				break;
			case 13:
				tmp_tlv16 = (tlv16 *)(payload + len);
				rng_req->cmac_key_count = *tmp_tlv16;
                                rng_req->cmac_key_count.value = RNG_REVS(rng_req->cmac_key_count.value);
				len = len + rng_req->cmac_key_count.length + 2;
				break;
			default:
				FLOG_WARNING("Error in parse_rng_req: Unrecognized TLV type\n");
				return -1;
		}
	}
	return 0;
}

int build_rng_req(u_char **p_payload, int *mm_len, int num_allotted_bytes)
{
	int ii = 0;
	u_char *payload = (u_char*)malloc(num_allotted_bytes);
	*p_payload = payload;

	// Min size because of mandatory fields: MM type, Rsv, SS MAC TLV, DLBP TLV
	int len = 2 + sizeof(tlv48) + sizeof(tlv8);

	if (num_allotted_bytes < len)
	{
		FLOG_ERROR("ERROR in build_rng_req: Insufficient space allocated\n");
		return -1;
	}

	// Allocate space and initialize the RNG_REQ 
	rng_req_msg* rng_req = (rng_req_msg*)malloc(sizeof(rng_req_msg));
	init_tx_rng_req_msg(rng_req);

	payload[0] = RNG_REQ; //mm_type
	payload[1] = 0; // Rsv bits
	payload += 2;

	tlv8 *tmp_tlv8 = (tlv8*)payload;

	// Ref Table 550. The Requested DLBP TLV in the RNG_REQ mesg contains
	// Bits 0-3: DIUC of reqd DLBP, Bits 4-7: LSB of DCD CCC defining this DIUC
	// Currently since there is no AMC logic, just use dEFAULT_DIUC
	rng_req->dl_burst_profile.value = (u_int8_t)((DEFAULT_DIUC & 0x0f) + ((DEFAULT_DLMAP_CCC & 0x0f) << 4));
	*tmp_tlv8 = rng_req->dl_burst_profile;
	payload += sizeof(tlv8);

	tlv48 *tmp_tlv48 = (tlv48*)payload;
	for (ii = 0; ii < 6; ii++)
	{
		rng_req->ss_mac.value[ii] = param_MY_MAC[ii];
	}
	*tmp_tlv48 = rng_req->ss_mac;
	payload += sizeof(tlv48);

	// Other TLVs are optional, not including them now
	*mm_len = len;
	free(rng_req);
	return 0;
}

int build_rng_rsp(u_char **p_payload, int *mm_len, rng_rsp_msg *rng_rsp)
{
	u_char *payload = (u_char*)malloc(MAX_RNG_RSP_LEN);
	*p_payload = payload;

	payload[0] = RNG_RSP; //mm_type
	payload[1] = 0; // Rsv bits
	payload += 2;
	int len = 2;

	tlv32 *tmp_tlv32 = NULL; 
	tlv_s32 *tmp_tlvs32 = NULL; 
	tlv8 *tmp_tlv8 = NULL;
	tlv_s8 *tmp_tlvs8 = NULL;

	if (rng_rsp->timing_adjust.length != 0)
	{
		tmp_tlvs32 = (tlv_s32*)payload;
		*tmp_tlvs32 = rng_rsp->timing_adjust;
		tmp_tlvs32->value = RNG_REVL(tmp_tlvs32->value);
		payload += sizeof(tlv_s32);
		len += sizeof(tlv_s32);
	}
	if (rng_rsp->power_adjust.length != 0)
	{
		tmp_tlvs8 = (tlv_s8*)payload;
		*tmp_tlvs8 = rng_rsp->power_adjust;
		payload += sizeof(tlv_s8);
		len += sizeof(tlv_s8);
	}
	if (rng_rsp->frequency_adjust.length != 0)
	{
		tmp_tlvs32 = (tlv_s32*)payload;
		*tmp_tlvs32 = rng_rsp->frequency_adjust;
		tmp_tlvs32->value = RNG_REVL(tmp_tlvs32->value);
		payload += sizeof(tlv_s32);
		len += sizeof(tlv_s32);
	}

	// Add the Ranging Status TLV 
	tmp_tlv8 = (tlv8*)payload;
	*tmp_tlv8 = rng_rsp->ranging_status;
	payload += sizeof(tlv8);
	len += sizeof(tlv8);

	tlv48 *tmp_tlv48 = NULL;
	tlv16 *tmp_tlv16 = NULL; 

	if (rng_rsp->ss_mac.length != 0)
	{
		tmp_tlv48 = (tlv48*)payload;
		*tmp_tlv48 = rng_rsp->ss_mac;
		payload += sizeof(tlv48);
		len += sizeof(tlv48);
	}
	if (rng_rsp->basic_cid.length != 0)
	{
		tmp_tlv16 = (tlv16*)payload;
		*tmp_tlv16 = rng_rsp->basic_cid;
		tmp_tlv16->value = RNG_REVS(tmp_tlv16->value);
		payload += sizeof(tlv16);
		len += sizeof(tlv16);
	}
	if (rng_rsp->primary_cid.length != 0)
	{
		tmp_tlv16 = (tlv16*)payload;
		*tmp_tlv16 = rng_rsp->primary_cid;
		tmp_tlv16->value = RNG_REVS(tmp_tlv16->value);
		payload += sizeof(tlv16);
		len += sizeof(tlv16);
	}
	if (rng_rsp->ranging_code_attributes.length != 0)
	{
		tmp_tlv32 = (tlv32*)payload;
		*tmp_tlv32 = rng_rsp->ranging_code_attributes;
		tmp_tlv32->value = RNG_REVL(tmp_tlv32->value);
		payload += sizeof(tlv32);
		len += sizeof(tlv32);
	}

	*mm_len = len;

	return 0;
}


int free_ranging_info(ranging_info *p_ranging_info)
{
	ranging_adjust_ie *rng_ie = NULL;
	if (p_ranging_info->p_rng_tx_info != NULL)
	{
		free(p_ranging_info->p_rng_tx_info);
	}
	while(p_ranging_info->p_ranging_ie_list != NULL)
	{
		rng_ie = p_ranging_info->p_ranging_ie_list;
		p_ranging_info->p_ranging_ie_list = rng_ie->p_next;
		free(rng_ie);
	}
	free(p_ranging_info);
	return 0;
}

int release_ss_ranging_sm()
{
	if (ranging_type == PERIODIC_RANGING)
	{
		FLOG_INFO("Canceling PR thread\n");
		pthread_cancel(ss_pr_thread);
	}
	else if (ranging_type == INIT_RANGING)
	{
		FLOG_INFO("Canceling IR thread\n");
		pthread_cancel(ss_ir_thread);
	}
	return 0;
}
