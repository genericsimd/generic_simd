/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: main.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------                   --------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdlib.h>
#include <time.h>
#include<string.h>
#include <sys/signal.h>
#include <openssl/evp.h>
#include "mac.h"
#include "mac_config.h"
#include "mac_hash.h"
#include "CS.h"
#include "dl_cbr_gen.h"
#include "dl_exp_params.h"
#include "dl_mme_gen.h"
#include "mac_cps.h"
#include "log.h"
#include "ul_cs_consumer.h"
#include "ucd.h"
#include "dcd.h"
#include "thread_sync.h"
#include <errno.h>
#include "mac_common_data.h"
#include "perf_log.h"
#include "ucd_dcd_parser.h"
#include "app_timer.h"
#include "dll_ordered_list.h"
#include "mac_bs_ranging.h"
#include "ul_scheduler.h"
#include "dl_scheduler.h"
#include "mac_bs_ranging_utm.h"
#include "mac_ss_ranging_utm.h"
#include "mac_dsa_list.h"
#include "mac_dsa_utm.h"
#include "mac_sf_sm.h"
#include "mac_dsd_test.h"
#include "mac_auth.h"
#include "mac_amc.h"

pthread_t cs_thread = 0;
pthread_t cps_thread = 0;
pthread_t cbr_thread = 0;
pthread_t mme_gen_thread = 0;
pthread_t ucd_thd = 0;
pthread_t dcd_thd = 0;
pthread_t dummy_amc = 0;  // for TESTING
pthread_mutex_t dts_flag_lock;
pthread_t bs_ranging_thread = 0;

#ifdef DSX_ENABLE
pthread_t dsx_thread = 0;
#endif

#ifdef BR_ENABLE
pthread_t br_thread = 0;
#endif

pthread_t rep_thread = 0;

pthread_t timer_thrd;//timer variable for the app_timer thread
pthread_t pkm_thread = 0;
extern pthread_t ss_ir_thread;
extern pthread_t ss_pr_thread;
pthread_rwlock_t conn_info_rw_lock;

//#define PHASE_ENABLED
#ifdef PHASE_ENABLED
pthread_mutex_t critical_phase_mutex;
pthread_cond_t critical_phase_done;
int not_critical_phase=0;
#endif

/* condition and mutex variables  for
 * cs, cps, and main thread synchronization*/
pthread_mutex_t sync_mutex, count_mutex, ranging_mutex, last_tid_mutex, last_sfid_mutex;
int sync_flag;
int sync_count;
int last_tid, last_sfid;

extern int param_NUM_ATTACHED_PROCS; // No. of processors that we want to bind our threads to
extern int g_sys_procs;
extern sdu_queue* dl_sdu_queue; // the MAC SDU for the downlink,
extern dll_ordered_list* g_timer_list;
extern sll_fifo_q *ranging_q;
extern sll_fifo_q *bs_ranging_q;
sll_fifo_q *ranging_scheduler_q;
extern sll_fifo_q *bs_ranging_test_q;

param_array_wrapper* wr1 = NULL;
sdu_queue* ul_sduq;
int init_mac_simulation_variables(int argc, char** argv)
{
/** add by zzb for integration */
#ifndef INTEGRATION_TEST
    if(argc > 1)
        file_name = argv[1];
    FLOG_INFO("Using file name : %s as configuration file\n",file_name);
#endif
/** end by zzb for integration */

    init_trace_buffer_record(MAX_TRACE_BUFFERS);

    sync_flag = 1;
    dl_exp_init();

#ifdef LOG_METRICS  
    start_log_reader("./logs.out");
#endif

    /* create trace buffer */
    create_init_trace_buffer(16384, "main thread");
    return 0;
}
int init_mac_core_variables()
{
    int rc;
    OpenSSL_add_all_algorithms();
    srand ( time(NULL) );
    

    // initialize the uplink sdu queue
    // currently the uplink sdu queue use the implementation of CRL for ease of integration,
    // will be replaced by the IRL implementation in the future.
    initialize_sduq(&ul_sduq, 0);

    /* initialize conditional variable and mutex */
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);
    pthread_mutex_init(&ranging_mutex, NULL);
    pthread_mutex_init(&last_tid_mutex, NULL);
    pthread_mutex_init(&last_sfid_mutex, NULL);


    pthread_rwlockattr_t rwlock_attr;
    //int pref = PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;
    pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&conn_info_rw_lock, &rwlock_attr);
    sync_count = 2; /* number of threads to wait for */
    last_tid = 0;
    last_sfid = 0;
    
    // Initiate service_flow management related lists 
    initiate_trans_list(&dsa_trans_list);
    initiate_trans_list(&dsd_trans_list);

    /* added by Xianwei.Yi */
    initiate_trans_list(&dsc_trans_list);

    app_timer_init();
#ifdef AMC_ENABLE
	init_shared_tables();
#endif

    // Initialize Ranging queues and threads depending on whether BS or SS
#ifdef SS_RX
    #ifdef SS_TX
        // Initialize the ranging messages queue
        sll_fifo_q_init(&ranging_q);
        // This function will clear earlier data and initialize new. 
        // Works for reset also
        init_ss_nw_entry();

    #else
        FLOG_WARNING("SS_RX & BS_TX: DSA & Network entry will not function\n");
    #endif
#else
    #ifndef SS_TX
        sll_fifo_q_init(&bs_ranging_q);
        sll_fifo_q_init(&ranging_scheduler_q);

        #ifdef RANGING_TEST
             sll_fifo_q_init(&bs_ranging_test_q);
        #endif
    #endif
#endif


#ifdef SS_TX

    // ULMAP list needs to be initialized only for testing now
    // In an actual system, the DL subframe is processed at the SS first 
    // and the ULMAP is recovered from that
    initialize_ulmap_list_for_ss();

    // Rcvd UCD/DCD array is needed at the SS, whether Tx or Rx
    init_rcvd_ucd_dcd_arr();

#else // SS_TX is not defined. This is BS Tx 

    #ifdef SS_RX
        // Rcvd UCD/DCD array is needed at the SS, whether Tx or Rx
        init_rcvd_ucd_dcd_arr();
    #endif

    // Initialize the lists where last few ULMAP and DLMAP values will be stored at the BS
    init_stored_ulmap_list();
    init_stored_dlmap_list();

    // creating a separate thread for UCD/DCD - ideally should be merged with mac_mgt_gen
    // For BS only
    // first create an handler for the SIGUSR1 used by chan_desc_thd

/** add by zzb */
#if 0
    FLOG_INFO("Set up the alarm handler for SIGUSR1 \n");
    struct sigaction        actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = sigusr1_handler;

    rc = sigaction(SIGUSR1,&actions,NULL);
#endif

    // Initialize vars for the ucd handler thread
#ifndef INTEGRATION_TEST
    ucd_handler_init();
#endif

    // Initialize vars for the dcd handler thread
#ifndef INTEGRATION_TEST
//  dcd_handler_init();
#endif

/** end by zzb */

#endif
    return 0;
}
int start_mac_threads()
{
    int rc=0;
    pthread_attr_t tattr;
    cpu_set_t cpuset;

    FLOG_INFO("Creating Timer Thread\n");
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    // TODO: should the timer thread be bound to a diff core?
    if(pthread_create(&timer_thrd, &tattr, timer_handler, (void*)g_timer_list)) {
        FLOG_FATAL("Unable to create Timer thread\n");
        exit(-1);
    }
    pthread_attr_destroy(&tattr);

#ifndef SS_RX
    #ifndef SS_TX
        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
        rc = pthread_create(&bs_ranging_thread, &tattr, bs_init_ranging, NULL);
        if(rc) 
        {
            FLOG_FATAL("Error returned while creating bs_ranging thread %d\n", rc);
            exit(-1);
        }
        else
        {
            FLOG_INFO("Started BS Ranging thread\n");
        }
        pthread_attr_destroy(&tattr);
    #else
        FLOG_WARNING(10,"SS_TX & BS_RX: DSA & Network entry will not function\n");
    #endif
#endif

#ifndef SS_TX
/*  //Creating UCD Handler thread
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc=pthread_create(&ucd_thd, &tattr, ucd_gen, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating ucd_thd thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
    
    // create the dcd handler thread
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc=pthread_create(&dcd_thd, &tattr, dcd_gen, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating ucd_thd thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
    
    #ifdef AMC_TEST
    // create a thread to emulate amc module - FOR TESTING
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc=pthread_create(&dummy_amc, &tattr, dummy_amc_gen, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating chan_desc_thd thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
    #endif
    */
#endif
    
    /* cbr thread */
#ifdef SIM_TRAFFIC
    pthread_attr_t cbr_join_attr;
    pthread_attr_init(&cbr_join_attr);
    pthread_attr_setdetachstate(&cbr_join_attr, PTHREAD_CREATE_JOINABLE);
    //CPU_ZERO(&cpuset);
    //bind_to_proc = min(g_sys_procs-1, param_NUM_ATTACHED_PROCS);
    //CPU_SET(bind_to_proc, &cpuset);
    //pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
    rc=pthread_create(&cbr_thread, &cbr_join_attr, CBR, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating CBR thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&cbr_join_attr);
#endif

    // create a separate thread for CpS layer
    // Separate threads for UL and DL CPS processing are created inside this handler

    pthread_attr_t join_attr;
    pthread_attr_init(&join_attr);

    pthread_attr_setdetachstate(&join_attr, PTHREAD_CREATE_JOINABLE);
//  CPU_ZERO(&cpuset);
//  CPU_SET(param_NUM_ATTACHED_PROCS-1, &cpuset);

    // mac_cps_handler initializes UL and DL queues, spawns off the UL and 
    // DL threads and returns
    rc=pthread_create(&cps_thread, &join_attr, mac_cps_handler, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating CPS thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&join_attr);
    rc = pthread_join(cps_thread, NULL);
    if(rc) {
        FLOG_FATAL ("Error code from cps join %d\n", rc);
        exit(1);
    }
/*
    // create a thread for mac_msg_gen
    wr1 = dl_mm_params_gen();
    print_param_array(wr1);
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    rc=pthread_create(&mme_gen_thread, &tattr, DL_MM_GEN, ((void*)wr1));
    if(rc) {
        FLOG_FATAL("Error returned while creating mme_gen_thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
*/
    // create uplink CS layer sdu consumer thread

/*
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    rc=pthread_create(&cs_thread, &tattr, ul_cs, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating uplink cs sdu consumer thead %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
*/
#ifdef DSX_ENABLE
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc = pthread_create(&dsx_thread, &tattr, sf_state_machine, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating dsx_thd thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);

    extern void dsx_utm(void);
    dsx_utm();
#endif

#ifdef BR_ENABLE
    extern void *mac_br_thread(void *args);
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc = pthread_create(&br_thread, &tattr, mac_br_thread, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating br thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
#endif

#ifdef AMC_ENABLE 
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	rc = pthread_create(&rep_thread, &tattr, rep_state_machine, NULL);
	if(rc)
	{
		FLOG_FATAL("Error returned while creating rep_thd thread %d\n", rc);
		exit(-1);
	}
	pthread_attr_destroy(&tattr);
#endif

#ifdef __ENCRYPT__  
    pthread_attr_init(&tattr);
    pthread_mutex_init(&security_list_lock,NULL);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    rc = pthread_create(&pkm_thread, &tattr, pkm_msg_handler, NULL);
    if(rc) {
        FLOG_FATAL("Error returned while creating pkm_thd thread %d\n", rc);
        exit(-1);
    }
    pthread_attr_destroy(&tattr);
#endif  

    return 0;
}
int init_ranging_and_dsa_test()
{

#ifdef RANGING_TEST
    #ifdef SS_TX
        #ifdef SS_RX
            mac_ss_rng_utm();
        #endif
    #else
        #ifndef SS_RX
            mac_bs_rng_utm();
        #endif
    #endif
#endif
    sleep(1);
#ifdef DSA_TEST
    #ifdef SS_TX
        #ifdef SS_RX
            mac_ss_dsa_utm();
        #endif
    #else
        #ifndef SS_RX
            mac_bs_dsa_utm();
        #endif
    #endif
#endif
    return 0;
}
int dsd_test()
{
#ifdef DSD_TEST
    #ifdef SS_TX
        #ifdef SS_RX
            dsd_test2();
        #endif
    #endif

    #ifndef SS_TX
        #ifndef SS_RX
            dsd_test1();
        #endif
    #endif

#endif
return 0;
}

int free_mac_simulation_variables()
{
    perf_log_cleanup();

    if (wr1 != NULL)
        {
        free_mm_params(wr1);
        }

    free_params();
    free_trace_buffer_record();
    release_log_reader();

#ifdef PRINT_DATA  
    close_files();
#endif  
    return 0;
}
int free_mac_core_variables()
{
#ifdef INTEGRATION_TEST
    if (cbr_thread != 0)
    {
        pthread_cancel(cbr_thread);
        pthread_join(cbr_thread, NULL);
    }

    if (mme_gen_thread != 0)
    {
        pthread_cancel(mme_gen_thread);
        pthread_join(mme_gen_thread, NULL);
    }
#endif

#ifndef SS_RX
    #ifndef SS_TX
        pthread_cancel(bs_ranging_thread);
    #endif
#else
    #ifdef SS_TX
        release_ss_ranging_sm();
    #endif
#endif

#ifdef DSX_ENABLE
    FLOG_INFO("cancel dsx thread");
    pthread_cancel(dsx_thread);
#endif

#ifdef BR_ENABLE
    FLOG_INFO("cancel br thread");
    pthread_cancel(br_thread);
#endif

#ifdef AMC_ENABLE
	FLOG_INFO("cancel amc thread");
    pthread_cancel(rep_thread);
#endif

    release_mac_cps_handler();

    // Cleanup the SDU queues
    clear_ss_security_list();

#ifdef INTEGRATION_TEST
    int rc = pthread_cancel(timer_thrd);
    if(rc) {
        FLOG_FATAL ("Error code from cbr join %d\n", rc);
        exit(1);
    }

    rc = pthread_join(timer_thrd, NULL);
#else
    int rc = pthread_join(timer_thrd, NULL);
#endif
    if(rc) {
        FLOG_FATAL ("Error code from cbr join %d\n", rc);
        exit(1);
    }
    int cid=0;

    // Cancel the UL CS consumer thread, then cleanup SDU queues
    pthread_rwlock_wrlock(&conn_info_rw_lock);

    for (cid = 0; cid < MAX_CIDS; cid++)
    {
        del_sdu_cid_queue(dl_sdu_queue, cid);
    }

    pthread_rwlock_unlock(&conn_info_rw_lock);

#ifdef ARQ_ENABLED
    ARQ_shutdown();
#endif
    mac_sdu_queue_finalize();
    free_dl_scheduler_var();

#ifndef SS_RX
    #ifndef SS_TX
        // BW request queues will not be needed for the MAC instance at the SS (both Tx and Rx chains)
        // They are needed (initialized and released) for all other cases - MAC instance at BS and BS-SS/SS-BS loopback tests 
        release_bwreq_queue();
        release_sll_fifo_q(bs_ranging_q);
        release_sll_fifo_q(ranging_scheduler_q);
        #ifdef RANGING_TEST
            release_sll_fifo_q(bs_ranging_test_q);
        #endif
    #endif
#else
    #ifdef SS_TX
    //  release_sll_fifo_q(ranging_q);
//      release_ss_ranging_sm();
    #endif
#endif

    free_stored_dlmap_list();
    free_stored_ulmap_list();

    // Cancel the DSX thread, then clean up the lists 
    free_trans_list(&dsa_trans_list);
    free_trans_list(&dsc_trans_list);
    free_trans_list(&dsd_trans_list);

#ifdef SS_TX
    free_rcvd_ucd_dcd_arr();
#else
    #ifdef SS_RX
        free_rcvd_ucd_dcd_arr();
    #endif
#endif

    /* uninitialize the synchronization variables */
    pthread_mutex_destroy(&sync_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&ranging_mutex);
    pthread_mutex_destroy(&last_tid_mutex);
    pthread_mutex_destroy(&last_sfid_mutex);
    pthread_rwlock_destroy(&conn_info_rw_lock);
    return 0;
}
int init_amc_test()
{
#ifdef AMC_ENABLE
#ifdef AMC_TEST
	pthread_t amc_test_thread;
	pthread_attr_t amc_attr;
	pthread_attr_init(&amc_attr);
	pthread_attr_setdetachstate(&amc_attr,PTHREAD_CREATE_JOINABLE);
	#ifdef SS_TX
		pthread_create(&(amc_test_thread), &amc_attr,ss_amc_test, NULL);
	#else
		pthread_create(&(amc_test_thread), &amc_attr,bs_amc_test, NULL);
	#endif
#endif
#endif
	return 0;
}
#ifndef INTEGRATION_TEST
int main(int argc, char**argv) {
	int rc;
	LOG_INIT_CONSOLE_ONLY("flog");
	app_timer_init();
	init_shared_tables();

	init_mac_simulation_variables(argc,argv);
	init_mac_core_variables();
	start_mac_threads();
	init_ranging_and_dsa_test();
	init_amc_test();
	/* wait for the CBR thread to join and signal */
	//void *status;

	//simulate_pkm_req();
	//test_pkm_rsp_funcs_auth_reply();

	rc = pthread_join(cbr_thread, NULL);
	if(rc) {
		FLOG_FATAL ("Error code from cbr join %d\n", rc);
		exit(1);
	}
	rc = pthread_join(mme_gen_thread, NULL);
	if(rc) {
		FLOG_FATAL ("Error code from mme_gen join %d\n", rc);
		exit(1);
	}

    /* wait for queues to clean up */
    sleep(1);
    dsd_test();
    sleep(2);

    FLOG_INFO("releasing sync \n");
    pthread_mutex_lock(&sync_mutex);
    sync_flag = 0;
    pthread_mutex_unlock(&sync_mutex);

    /* wait for processing of current packets to finish before freeing memory */
/*
    sleep(1);
    rc = pthread_join(cs_thread, NULL);
    if(rc) {
        FLOG_FATAL ("Error code from cs_thread join %d\n", rc);
        exit(1);
    }
*/  
    //Freeing up allocated variables and mutex variables.
    free_mac_core_variables();
    free_mac_simulation_variables();
    exit(0);

}
#endif
