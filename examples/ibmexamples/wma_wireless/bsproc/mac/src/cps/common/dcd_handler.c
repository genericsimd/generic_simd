/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dcd_handler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


/* see comments for ucd handler */

/*
byte sequence structure of DCD msg sent (size in bits or type in parenthesis):

 - type(8)
 - CCC (8)
 - (burst profile 1)
     - type(8)
     - length(8)
     - reserved(4)
     - DIUC(4)
     - FEC(tlv8)
 - (burst profile 2) ...

*/

/* There are six burst profile with FEC values 0-6 for QPSK-1/2 to 16QAM-3/4 */


#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/signal.h>
#include <string.h>

#include "mac.h"
#include "ucd.h"
#include "dcd.h"
#include "mac_sdu_queue.h"
#include "CS.h"
#include "memmgmt.h"
#include "thread_sync.h"

//#include "mac_config.h"
//#include "mac_amc.h"

#define MAX_DIUC 12
#define MAX_FEC_CODE_TYPE 25  // FEC code type beyond 25 are reserved 

int param_DCD_INTERVAL = 200; // in milli sec
int param_DCD_TRANSITION = 300; // in milli sec


// IMP TODO: memory leak: when overwriting dcd_arr[i], the old memory pointed by the dcd_arr[i] needs to be freed.

// TODO: IMP dlmap_ccc is written by dcd_thd and read by scheduler - should be marked volatile
volatile int dlmap_ccc; // cc of the latest dlmap sent


// these are only used by the dcd_thd
int dcd_ccc;  // configuation change count of the latest ccc message sent - IMP: should be marked volatile
int t_dcd_ccc;  // chan_dec_thd's copy of dcd_ccc
int dcd_trans;  // transition time left for the latest dcd message
int dcd_sleep_dur;
int dcd_flag;  // to indicate the thread should update dlmap_ccc when it next wakes up

// begin version2

u_char dcd_msg_bytes[1000];   // the byte sequence for DCD is first written here and then duplicated to msg_dcd_embedded before sending - this is done to know the exact size of msg

// end version2



struct dcd_msg_wrapper{
  dcd_msg* umsg;
  int count; // number of times this message has been sent
};

typedef struct dcd_msg_wrapper dcd_wrapper;


dcd_wrapper* dcd_arr[256];  // dcd_msgs indexed by their ccc

void dcd_handler_init(){

  // allocate memory for dcd_arr
  int vv;
  for(vv=0; vv < 256; vv++){  // TODO: do I have to allocate memory for it?

    dcd_arr[vv] = (dcd_wrapper*)mac_malloc(sizeof(dcd_wrapper));
    if(!(dcd_arr[vv])){
      FLOG_FATAL("Fatal error in dcd_handler: cannot init dcd_handler. Exiting... \n");
      exit(-1);
    }

    dcd_arr[vv]->umsg = NULL;
    dcd_arr[vv]->count = 0;
  }


  // start with dcd_arr[0] entry populated - i.e., an initial dcd message
  // TODO: check if DIUC=0 entry is populated correctly - it is required for DLMAP message

  dlmap_ccc = 0;
  dcd_ccc = 0;
  t_dcd_ccc = 0;
  dcd_trans = 0;
  dcd_sleep_dur = param_DCD_INTERVAL;
  dcd_flag = 0;


  // initial dcd message to be sent
  dcd_msg* dcd_ptr;

  dcd_ptr = (dcd_msg*)mac_malloc(sizeof(dcd_msg));
  if(!dcd_ptr){
    FLOG_FATAL("dcd thread: problem in creating dcd test messages. \n");
    return;
  }

  dcd_ptr->management_message_type = (u_int8_t)1;
  dcd_ptr->configuration_change_count = (u_int8_t)0;

  //begin version2

  // create burst profile for FEC code type and modulation type for QPSK 1/2 to 64 QAM 3/4

  dl_burst_profile *dlbp1;
  dl_burst_profile *dlbp2;

  int ii;

  /** add by zzb */
/*
  for(ii=0; ii <=6; ii++){ // QPSK 1/2 to 64 QAM 3/4
*/

  for(ii=0; ii <=NUM_SUPPORTED_MCS; ii++){ // QPSK 1/2 to 64 QAM 3/4
  /** add by zzb */

    dlbp1 = (dl_burst_profile*)mac_malloc(sizeof(dl_burst_profile));
    if(!dlbp1){
      FLOG_FATAL("DCD thread: problem in creating dcd test messages. \n");
      return;
    }

    dlbp1->type = (u_int8_t)1;
    // TODO: In future have automatic calculation of Length according to the value field including all embedded TLVs
    dlbp1->length = (u_int8_t)DCD_BURST_PROFILE_LENGTH;
    dlbp1->diuc = (u_int8_t)ii;

    dlbp1->fec_code_modulation.type = (u_int8_t)150;
    dlbp1->fec_code_modulation.length = (u_int8_t)1;
    dlbp1->fec_code_modulation.value = (u_int8_t)ii;  // QPSK 1/2 to 64-QAM 3/4

    if(ii == 0){
      dcd_ptr->profile_header = dlbp1;
    }
    else{
      dlbp2->next = dlbp1;
    }
    dlbp2 = dlbp1;
  }
  dlbp1->next = NULL;

  dcd_arr[dcd_ccc]->umsg = dcd_ptr;

  // end version2


  dcd_arr[dcd_ccc]->count = 0;



  FLOG_INFO("Initialized dcd handler. \n");

}

void *dcd_gen(void *parm)
{
  //pthread_t self = pthread_self();


  sigset_t mask;
  int rc;

  int dcd_byte_length;  // length of the DCD msg sent (in bytes)


  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1); // add SIGUSR1 to the mask


  while(can_sync_continue()){


    // mask SIGUSR1
    rc = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    if (rc==0){
      //FLOB_DEBUG("dcd_thd: Blocked SIGUSR1 \n");
    }
    else{
      FLOG_WARNING("dcd_thd: Unable to block SIGUSR1 \n");
    }

    
    if (dcd_flag == 1){
      // the thread woke up because dcd_TRANSITION period for the ccc in t_dcd_ccc has expired: update dlmap_ccc
      dlmap_ccc = t_dcd_ccc;
      dcd_sleep_dur = param_DCD_INTERVAL - dcd_trans;  // sleep for the time remaining to send the next DCD message
      dcd_trans = 0;
      dcd_flag = 0;
      //printf("[debug] DCD thead: updated the dlmap_ccc for ccc = %d as the dcd transition period expired \n", t_dcd_ccc);
    }


    else{  // else the thread woke up because DCD_INTERVAL of the cc in t_dcd_ccc has expired: send another DCD message

      dcd_sleep_dur = param_DCD_INTERVAL;  // set next sleep duration

      // write the dcd message from "dcd_arr[t_dcd_ccc]->umsg" to "dcd_msg_bytes" (this step is not needed if the DCD msg has not changed)

      if(dcd_arr[t_dcd_ccc]->count == 0){  // only create the dcd byte sequence if the msg is being send for first time

	u_char *tmp_ptr = dcd_msg_bytes;

	// write DCD type
	*tmp_ptr = (u_char)dcd_arr[t_dcd_ccc]->umsg->management_message_type;
	tmp_ptr++;
	// write CCC
	*tmp_ptr = (u_char)dcd_arr[t_dcd_ccc]->umsg->configuration_change_count;
	tmp_ptr++;

	// finally copy the burst profiles
	dl_burst_profile *bp = dcd_arr[t_dcd_ccc]->umsg->profile_header;

	while(bp != NULL){
	  //bp type
	  *tmp_ptr = (u_char)(bp->type);
	  tmp_ptr++;

	  //bp length (in bytes)
	  *tmp_ptr = DCD_BURST_PROFILE_LENGTH;
	  tmp_ptr++;

	  //bp diuc
	  *tmp_ptr = (u_char)(bp->diuc);  // reserved bytes will be set 0 automatically because diuc is smaller than 15
	  tmp_ptr++;
	  //printf("\n diuc = %u", bp->diuc);

	  //copy FEC tlv8
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.type);
	  tmp_ptr++;
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.length);
	  tmp_ptr++;
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.value);
	  tmp_ptr++;

	  bp = bp->next;
	}
	// done populating the dcd_msg_bytes

	// number of bytes in dcd msg
	dcd_byte_length = tmp_ptr - dcd_msg_bytes;

      }

      // copy dcd message to a new allocated memory -  the pointer send using enqueue_transport() may be deallocated, and we do not want the message in our dcd_arr to be deallocated

      u_char* msg_dcd_embedded = (u_char*) mac_sdu_malloc(dcd_byte_length, 5);  //pointer to dcd message with burst profiles embedded, with pointers discarded
      if(!msg_dcd_embedded){
	FLOG_FATAL("Memory allocation failed for DCD message creation. \n");
	continue;
      }

      memcpy( msg_dcd_embedded, dcd_msg_bytes, dcd_byte_length); 

/*       //begin TEST */

/*       dcd_msg* test_dcd = (dcd_msg*)mac_malloc(sizeof(dcd_msg)); */
/*       if(!test_dcd){ */
/* 	printf("DCD handler: 10: problem in allocating memory for dcd message. \n"); */
/* 	return -1; */
/*       } */
/*       parse_dcd(msg_dcd_embedded, dcd_byte_length, test_dcd); */
/*       print_dcd_msg(test_dcd); */

/*       //end TEST */


      //enqueue it to the sdu queue
      enqueue_transport_sdu_queue(dl_sdu_queue, BROADCAST_CID, dcd_byte_length, msg_dcd_embedded);


/*       //begin: for TEST */
/*       printf("[debug] DCD thead: sent DCD message with ccc = %d. \n dcd burst profile FEC codes: ", t_dcd_ccc); */
/*       dl_burst_profile *bp = dcd_arr[t_dcd_ccc]->umsg->profile_header; */

/*       // finally copy the burst profiles */
/*       while(bp != NULL){ */
/* 	printf("[debug] %u ", bp->fec_code_modulation.value); */
/* 	bp = bp->next; */
/*       } */
/*       printf("[debug] \n"); */

/*       //exit(0); */

/*       //end: for TEST */






      dcd_arr[t_dcd_ccc]->count++;  // increase the number of time this message has been sent

      if((dcd_arr[t_dcd_ccc]->count >= 2) && (dcd_trans > 0)){   // if sent the message for the 2nd time, start keeping track of dlmap transition
	dcd_trans = dcd_trans - param_DCD_INTERVAL;
	if(param_DCD_INTERVAL < dcd_trans){
	  dcd_sleep_dur = param_DCD_INTERVAL;
	}
	else{   // update dlmap_ccc when the thread next wakes up
	  dcd_sleep_dur = dcd_trans;
	  dcd_flag = 1;
	}
      }
    }



    rc = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    if (rc==0){
      //printf("[debug] dcd_thd: Unblocked SIGUSR1 \n");
    }
    else{
      FLOG_WARNING("WARNING: dcd_thd: Unable to unblock SIGUSR1 \n");
    }

    //printf("[debug] DCD thread: sleeping for %d milli sec \n", dcd_sleep_dur);
    chan_desc_msleep(dcd_sleep_dur);

  }
  return 0;
}


// there is only one sigusr1 handler for both ucd and dcd in ucd_handler.c

/*  void sigusr1_handler(){  // common handler for all SIGUSR1 triggers (for both DCD and DCD) */

/*    if(pthread_self() == dcd_thd){ */
/*      printf("Got DCD AMC trigger. \n"); */
/*      dcd_sleep_dur = 0; */
/*      t_dcd_ccc = dcd_ccc; */
/*      dcd_trans = param_DCD_TRANSITION + param_DCD_INTERVAL; */
/*      dcd_flag = 0; */
/*    } */

/*  } */



// this function is called by the amc module to create a new dcd message from its amc_list and signal the dcd_thd
// TODO: the function populates only a small number of fields in the dcd message; needs to expanded later

int amc_dcd_update(amc_info* amclist){

   dcd_msg* dcd_ptr = (dcd_msg*)mac_malloc(sizeof(dcd_msg));
   if(!dcd_ptr){
     FLOG_FATAL("DCD thread: 1: problem in allocating memory for dcd message in amc_dcd_update. \n");
     return 0;
   }

   // increase the dcd ccc

   dcd_ccc = (dcd_ccc + 1) % 256;


   // populate the dcd_msg and its burst profiles using the amclist

   dcd_ptr->management_message_type = (u_int8_t)1;
   dcd_ptr->configuration_change_count = (u_int8_t)dcd_ccc;

   ss_amc_info* tmpinfo = amclist->ss_amc_head;
   dl_burst_profile* dlbp0;
   dl_burst_profile* dlbp1;
   int tmp_diuc = 0;
   //tlv8 *fec_ptr;

   int is_fec_already_seen[MAX_FEC_CODE_TYPE+1];  // fec code type beyond 25 are reserved
   int jj;
   for(jj=0; jj <= MAX_FEC_CODE_TYPE; jj++){
     is_fec_already_seen[jj] = 0;
   }

   while((tmpinfo != NULL) &&(tmp_diuc < MAX_DIUC)){   // diuc value can be at most MAX_DIUC


     // two burst profiles should not have the same fec
     if ((tmpinfo->dl_fec_code_modulation_type > MAX_FEC_CODE_TYPE) || (is_fec_already_seen[(tmpinfo->dl_fec_code_modulation_type)] == 1)){
       tmpinfo = tmpinfo->next;
       continue;
     }
     else{
       is_fec_already_seen[(tmpinfo->dl_fec_code_modulation_type)] = 1;
     }



     dlbp1 = (dl_burst_profile*)mac_malloc(sizeof(dl_burst_profile));
     if(!dlbp1){
       FLOG_FATAL("DCD thread: 2: problem in allocating memory for dcd message in amc_dcd_update. \n");
       return 0;
     }

     dlbp1->type = (u_int8_t)1;
     dlbp1->diuc = (u_int8_t)tmp_diuc;
     dlbp1->length = (u_int8_t)DCD_BURST_PROFILE_LENGTH;

/*      fec_ptr = (tlv8*)mac_malloc(sizeof(tlv8)); */
/*      if(!fec_ptr){ */
/*        printf("DCD thread: 3: problem in allocating memory for dcd message in amc_dcd_update. \n"); */
/*        return; */
/*      } */
/*      fec_ptr->type = (u_int8_t)150; */
/*      fec_ptr->length = (u_int8_t)1; */
/*      fec_ptr->value = (u_int8_t)(tmpinfo->dl_fec_code_modulation_type); */

/*      dlbp1->fec_code_modulation = *fec_ptr; */

     dlbp1->fec_code_modulation.type = (u_int8_t)150;
     dlbp1->fec_code_modulation.length = (u_int8_t)1;
     dlbp1->fec_code_modulation.value = (u_int8_t)(tmpinfo->dl_fec_code_modulation_type);

     dlbp1->next = NULL;  // initialize to NULL

     if(tmp_diuc == 0){
       dcd_ptr->profile_header = dlbp1;
     }
     else{
       dlbp0->next = dlbp1;
     }

     tmp_diuc++;
     dlbp0 = dlbp1;
     tmpinfo = tmpinfo->next;
   }



   // populate the dcd_msg array with the message created above
   
   dcd_arr[dcd_ccc]->umsg = dcd_ptr;
   dcd_arr[dcd_ccc]->count = 0;


   // TODO: free current dcd_arr[dcd_ccc]->umsg (don't touch the content at lower index which might be in use)


   // send the signal to the dcd_thd
   int rc1;
   rc1 = pthread_kill(dcd_thd, SIGUSR1);
   if(rc1!=0){
     FLOG_WARNING("Warning: AMC : Error in raising SIGUSR1. \n");
   }
   return 0;
}



// query interface for the dcd - there will be synchronization issues if the corresponding dcd_arr entry is being populated by the amc_update
// but if the querying entity (PHY, scheduler) is behaving properly, such a query should never happen - a DCD query for a ccc is only done
// by these modules only after a DCD message is sent with that ccc, and dcd_arr[ccc] entry is populated before sending a DCD msg with ccc.

dcd_msg* dcd_msg_query(int query_dcd_ccc){

  if((query_dcd_ccc >= 0) && (query_dcd_ccc <= 255)){
    return(dcd_arr[query_dcd_ccc]->umsg);
  }
  else{
    FLOG_WARNING("WARNING: illegal DCD query. \n");
    return(NULL);
  }

}




// ********************************* everything below this line is for TESTING ****************************************


/* void* dummy_amc_gen(void* param){ // for testing - structures created are not freed */

/*   int dummy_amc_sleep = 12000; */
/*   amc_info* amclist1; */
/*   int count = 2; */

/*   while(1){ */

/*     amclist1 = (amc_info*)mac_malloc(sizeof(amc_info)); */
/*     if(!amclist1){ */
/*       printf("dummy_amc1: 1: error in allocating memory. \n"); */
/*       exit(1); */
/*     } */
/*     amclist1->ss_num = 10; // not used */

/*     int cc; */
/*     ss_amc_info* info1; */
/*     ss_amc_info* info2; */

/*     for(cc=0; cc < count; cc++){ */

/*       info1 = (ss_amc_info*)mac_malloc(sizeof(ss_amc_info)); */
/*       if(!info1){ */
/* 	printf("dummy_amc1: 2: error in allocating memory. \n"); */
/* 	exit(1); */
/*       } */

/*       info1->dl_fec_code_modulation_type = cc; */

/*       if(cc==0){ */
/* 	amclist1->ss_amc_head = info1; */
/*       } */
/*       else{ */
/* 	info2->next = info1; */
/*       } */
/*       info2 = info1; */

/*     } */

/*     amc_dcd_update(amclist1); */

/*     count = (count+1)%26; */
/*     chan_desc_msleep(dummy_amc_sleep); */
/*   } */

/* } */




