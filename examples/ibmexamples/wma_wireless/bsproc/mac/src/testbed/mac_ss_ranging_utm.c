/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ss_ranging_utm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Jul.2009       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdio.h>
#include <unistd.h>
#include "mac_ss_ranging.h"
#include "mac_ss_ranging_utm.h"
#include "constants.h"

#ifdef SS_TX
#ifdef SS_RX

extern sll_fifo_q *ranging_q;
extern sdu_queue* dl_sdu_queue;
extern int param_MAX_VALID_BASIC_CID;
static u_int8_t frame_num_index = 14;
int ranging_code, ranging_symbol, ranging_subchannel;
int mac_ss_rng_utm()
{
	// Test 2a: send RNG_RSP with status ABORT. PHY should rescan
	usleep(20000);
	//printf("Started Test 2a\n");
	//rng_rsp_enq(0,0,0,RNG_ABORT);

	// Test 2b:send RNG_RSP with status CONTINUE, should be rxd in ranging_q 
	// and corresponding corrections sent to PHY, checked in p_ranging_info 
	FLOG_INFO("Starting Test 2b. RNG_RSP with Status=Continue should be received\n");
	rng_rsp_enq(5,6,7,RNG_CONTINUE);

	// Test 2c: send RNG_RSP with status SUCCESS, should be rxd in ranging_q 
	// and corresponding corrections sent to PHY, checked in p_ranging_info 
	usleep(10000);
	FLOG_INFO("Starting Test 2c. RNG_RSP with Status=Success should be received\n");
	rng_rsp_enq(0,0,0,RNG_SUCCESS);

	// Test 2d: don't send CDMA alloc IE, T3 should expire and init ranging should restart
	usleep(1000);
	FLOG_INFO("Starting Test 2d. T3 should expire and init ranging should restart\n");
	usleep(T3_RNG + 1000);

	// Test 2e: send CDMA alloc IE within T3_RNG, RNG_REQ should be rcvd in UL
	// with type RNG_REQ_IR_CID and SS mac value checked
	usleep(10000);
	FLOG_INFO("Starting Test 2e. T3_RNG, RNG_REQ should be rcvd in UL with type RNG_REQ_IR_CID and SS mac value checked\n");
	enq_cdma_alloc();

	// Test 2f: send RNG_RSP w/ basic & prim CID, IR should terminate and PR shd start
	usleep(10000);
	FLOG_INFO("Starting Test 2f. Should receive RNG_RSP with matching MAC, IR should terminate and PR shd start\n");
	enq_rng_rsp_wcid();

	// Test 3: After PR started, send RNG_RSP with status SUCCESS, periodic_ranging 
	// retries should reset
	usleep(40000);
	FLOG_INFO("Starting Test 3. PR retries should reset\n");
	rng_rsp_enq(0,0,0,RNG_SUCCESS);
	return 0;
}

int enq_cdma_alloc()
{
	cdma_ranging_ie* cdma_rng_ie = (cdma_ranging_ie*)malloc(sizeof(cdma_ranging_ie));
	cdma_rng_ie->ucd_count = 0;

	cdma_rng_ie->cdma_ie = (cdma_alloc_ie*)malloc(sizeof(cdma_alloc_ie));
    cdma_rng_ie->cdma_ie->duration = CDMA_ALLOC_SIZE;
    cdma_rng_ie->cdma_ie->uiuc = param_UL_MCS + 1;
    cdma_rng_ie->cdma_ie->repetition_coding_indication = 0;
    cdma_rng_ie->cdma_ie->frame_num_index = frame_num_index;
    cdma_rng_ie->cdma_ie->ranging_code = ranging_code;
    cdma_rng_ie->cdma_ie->ranging_symbol = ranging_symbol;
    cdma_rng_ie->cdma_ie->ranging_subchannel = ranging_subchannel;
    cdma_rng_ie->cdma_ie->bw_request_mandatory = 0;

	FLOG_INFO("Enqing CDMA alloc IE in ranging Q\n");
	sll_fifo_q_enqueue(ranging_q, cdma_rng_ie, sizeof(cdma_ranging_ie), CDMA_ALLOC_UIUC);
	return 0;
}
int enq_rng_rsp_wcid()
{
	u_char *payload = NULL;
	int mm_len = 0, ii;
	rng_rsp_msg *rng_rsp = (rng_rsp_msg*)malloc(sizeof(rng_rsp_msg));
	init_rng_rsp_msg(rng_rsp, NULL);

	for (ii = 0; ii < 6; ii++)
		rng_rsp->ss_mac.value[ii] = param_MY_MAC[ii];

	rng_rsp->ss_mac.length = 6;

	rng_rsp->basic_cid.value = param_MAX_VALID_BASIC_CID;
	rng_rsp->basic_cid.length = 2;

	rng_rsp->primary_cid.value = max_valid_primary_cid;
	rng_rsp->primary_cid.length = 2;

	build_rng_rsp(&payload, &mm_len, rng_rsp);
	//enqueue_transport_sdu_queue(dl_sdu_queue, param_MAX_VALID_BASIC_CID, mm_len, payload);
	sll_fifo_q_enqueue(ranging_q, payload, mm_len, RNG_RSP);
	free(rng_rsp);
	return 0;
}

int rng_rsp_enq(int32_t timing_adjust, int8_t power_adjust, int32_t frequency_adjust, int status)
{
	u_char *payload = NULL;
	int mm_len = 0;
	rng_rsp_msg *rng_rsp = (rng_rsp_msg*)malloc(sizeof(rng_rsp_msg));
	ranging_adjust_ie *p_rng_ie = (ranging_adjust_ie*)malloc(sizeof(ranging_adjust_ie));
	
	// Populate the IE with correction values
	p_rng_ie->timing_adjust = timing_adjust;
	p_rng_ie->power_adjust = power_adjust;
	p_rng_ie->frequency_adjust = frequency_adjust;

	p_rng_ie->frame_num_index = frame_num_index;
	p_rng_ie->ranging_code = ranging_code;
	p_rng_ie->ranging_symbol = ranging_symbol;
	p_rng_ie->ranging_subchannel = ranging_subchannel;
	init_rng_rsp_msg(rng_rsp, p_rng_ie);
	rng_rsp->ranging_status.value = status;
	build_rng_rsp(&payload, &mm_len, rng_rsp);
	//enqueue_transport_sdu_queue(dl_sdu_queue, param_MAX_VALID_BASIC_CID, mm_len, payload);
	sll_fifo_q_enqueue(ranging_q, payload, mm_len, RNG_RSP);
	free(rng_rsp);
	free(p_rng_ie);
	//free(p_rng_ie);
	return 0;
}
#endif
#endif
