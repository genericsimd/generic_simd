/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_cps.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


//  Function:  the mac cps framework including both downlink and uplink

#include "mac_cps.h"
#include "phy_params.h"
//This variable holds the number of processors on the machine
//This is actually needed mainly for test bed and in CPS Multi-threading
//To avoid the dependency on testbed.a by cps.a -- this is moved here
// the dependecy for testbed.a on cps.a will resolve the symbol linking
int g_sys_procs;

//br_queue **br_q_list;
extern void init_burst_threads(void);

pthread_mutex_t spectrum_measure_flag_lock; //lock var for locking the flag that indicates that spectrum measure must start.
int spectrum_measure_flag=0; //this flag going to 1 means this frame will have empty ulmap and spectrum measurement will happen
pthread_t dts_info_timer_thread;
pthread_mutex_t int_info_lock; //Lock var to lock int_info and interference_flag

void* mac_cps_handler()
{
#ifdef INTEGRATION_TEST

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }
#endif

    // mac_timer * dl_trigger_timer;
    // int ret;
    crc_init( POLY );
    initialize_common_data();

    initialize_amc_info();

    ul_ugs_cid_info = (ugs_cid_info*)malloc((UL_UGS_CID_MAX_VALUE - UL_UGS_CID_MIN_VALUE +1)*sizeof(ugs_cid_info));
    memset(ul_ugs_cid_info, 0, (UL_UGS_CID_MAX_VALUE - UL_UGS_CID_MIN_VALUE + 1) * sizeof(ugs_cid_info));
	int ii = 0;
	for (ii = 0; ii < UL_UGS_CID_MAX_VALUE - UL_UGS_CID_MIN_VALUE + 1; ii++)
	{
		ul_ugs_cid_info[ii].last_dequeued_frame_number = -param_DL_CBR_PACKET_DELAY/frame_duration[FRAME_DURATION_CODE];
	}
    
    // Initialize the BW request queues
    // We have one BW request queue per class. 
    // br_q_list is the array of all BW request queues
    //br_q_list=br_queue_init();

    // begin the initialization of timer
    // initialize_timerq(&mac_cps_timerq);

    // initialize the downlink timer
    // initialize_timer_schedulertrigger(&dl_trigger_timer);
    // initialize_timer_schedulertrigger_cond();

        // create the mac cps layer timer thread
    //ret = pthread_create(&mac_cps_timer_thread, NULL, check_timer, (void*)mac_cps_timerq);
    //if(ret)
    //{
    //    ERROR_TRACE("Cannot create thread to start timer processing");
    //}

    //initialize the burst threads
    init_burst_threads();
#ifndef SS_TX	
    init_pi_mutex(&(spectrum_measure_flag_lock));
    init_pi_mutex(&int_info_lock);
/*
	pthread_attr_t join_attr;
	pthread_attr_init(&join_attr);
	pthread_attr_setdetachstate(&join_attr, PTHREAD_CREATE_JOINABLE);

    int ret = pthread_create(&dts_info_timer_thread, &join_attr, dts_timer_thread, NULL); 
    if (ret)
    {
	FLOG_FATAL("Cannot create thread to start timer processing\n");
    }
*/
#endif

    // start the downlink cps controller
    dl_cps_controller(bwr_q_list);

	ul_br_queue* ul_brq;
	initialize_br_queue(&ul_brq);
    // start the uplink cps controller
    ul_cps_controller(ul_brq);

//    insert_timer(mac_cps_timerq, dl_trigger_timer);
	FLOG_DEBUG("Returning from mac_cps_handler\n");
    pthread_exit(NULL);
    return 0;    
}

void* dts_timer_thread()
{
#ifdef INTEGRATION_TEST
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }
#endif

	int duration = SPECTRUM_SENSING_GAP;
	//duration is assumed to be in milliseconds
	while(can_sync_continue())
	{
		usleep(duration*1000);
		pthread_mutex_lock(&spectrum_measure_flag_lock);	
		spectrum_measure_flag = 1; //sense spectrum in the next frame	
		pthread_mutex_unlock(&spectrum_measure_flag_lock);	
	}    	 
	return NULL;

}



int release_mac_cps_handler(){
    int rc; 
#ifndef SS_TX	
#ifdef INTEGRATION_TEST
/*
    rc = pthread_cancel(dts_info_timer_thread);
    if(rc) {
        FLOG_FATAL ("Error code from DTS thread join %d\n", rc);
        exit(1);
    }
*/
#endif
/*
	rc = pthread_join(dts_info_timer_thread, NULL);
	if(rc) {
		FLOG_FATAL ("Error code from DTS thread join %d\n", rc);
		exit(1);
	}
*/
#endif
#ifdef INTEGRATION_TEST
    rc = pthread_cancel(dl_processing_thread);
    if(rc) {
        FLOG_FATAL ("Error code from DL processing thread cancel %d\n", rc);
        exit(1);
    }
#endif

    rc = pthread_join(dl_processing_thread, NULL);
	if(rc) {
		FLOG_FATAL ("Error code from DL processing thread join %d\n", rc);
		exit(1);
	}
	release_ul_con_threads();
    rc = pthread_cancel(ul_parse_thread);
	if(rc) {
		FLOG_FATAL ("Error code from UL processing thread join %d\n", rc);
		exit(1);
	}

/*
    for (ii=0;ii<NUM_BR_Q;ii++)
    {
        // Free the memory for the queue for each class
        mac_free(sizeof(br_queue), br_q_list[ii]);
    }
    mac_free(NUM_BR_Q*(sizeof(br_queue*)), br_q_list);
*/

    release_dl_cps_controller();
    release_ul_cps_controller();

#ifdef INTEGRATION_TEST
    rc = pthread_join(ul_parse_thread, NULL);
        if(rc) {
                FLOG_FATAL ("Error code from UL processing thread join %d\n", rc);
                exit(1);
        }
#endif

    release_common_data();
    free(ul_ugs_cid_info);
    free_amc_info();
    // release_timerq(mac_cps_timerq);
    // release_timer_schedulertrigger_cond();
    return 0;
}
