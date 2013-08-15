/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ucd_handler.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


/* 
All references to the standard are IEEE Std 802.16e-2005

This thread (ucd_thd) implements the state machine in Table 123 (UCD update) in the standard.
After the channel descriptor change is commanded, an UCD message is sent with the new
ccc after every UCD_INTERVAL. After sending the second UCD message, a UCD_TRANSITION timer
is started (the UCD_TRANSITION timer is emulated using sleep in the program below). When that
timer expires, the ccc used by the ULMAP is updated to the new ccc. Meanwhile, if another
channel descriptor change is commanded, the state machine immediately moves to the next ccc.

In this program, the AMC module calls amc_ucd_update(amc_info* amclist) function to
command a channel descriptor change. This function increments the ucd_ccc, creates a
ucd_msg and stores it in an array ucd_arr[] indexed by ucd_ccc,
and then sends a SIGUSR1 signal to the ucd_thd.
Note that the ucd_thd does not directly used the ucd_ccc to send the UCD
msg to avoid synchronization issue. On getting the signal, it duplicates the ucd_ccc to its
variable t_ucd_ccc, and uses ucd_arr[t_ucd_ccc] as the current ucd msg.
(Note, ucd_thd accesses this entry of the ucd_arr only after amc_ucd_update() has completely
populated it.) On expiration of the UCD_TRANSITION (after the second ucd msg is sent),
the ccc used for ULMAP (ulmap_ccc) is updated.

To avoid problems with asynchronous update of ucd_ccc and t_ucd_ccc on getting a SIGUSR1
signal, the ucd_thd, masks SIGUSR1 at the beginning of the loop, and unmasks the signal at
the end of the loop (before going to sleep). The POSIX standard guarantees that, if a signal
is received one of more times when it is masked, it will be delivered at least once after
the signal is unmasked. (Note that if a signal is received between it is unmasked
by ucd_thd and before ucd_thd goes to sleep, the sleep will not be interrupted by the signal,
and ucd_thd will not be able to send a new ucd message immediately. To circumvent this
problem, on getting a signal, in the signal handler, we set the ucd_sleep_dur to 0.)

  Finally, this implementation using signal makes the amc_update lock-free. A simpler 
implementation can be done using queues where the timer expirations and amc updates are
enqueued in a shared queue, and they are dequeued and processed by the ucd_thd.
But this approach will involve using mutex locks.

  IMP1: scheduler should use ulmap_ccc and dlmap_ccc for the ULMAP and DLMAP msgs.
  IMP2: param_UCD_INTERVAL and param_UCD_TRANSITION should be set properly. (Also for DCD.)
  IMP3: Some minor things are left unimplemented, seach for "TODO" below.
*/



/* 
byte sequence structure of UCD msg sent (size in bits or type in parenthesis):

   - type(8)
   - CCC(8)
   - ranging start (8)
   - ranging end (8)
   - registration start (8)
   - registration end (8)
   - (burst profile1)
        - type (8)
        - length (8)
        - reserved (4)
        - UIUC (4)
        - FEC (tlv8)
        - Ranging data (tlv8)
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

#define MAX_UIUC 12
#define MAX_FEC_CODE_TYPE 25
   // FEC code type beyond 25 are reserved
#define MAX_OLD_UCD_CCC 20
   // ucd_msg query only allowed from t_ucd_ccc to t_ucd_ccc-MAX_OLD_UCD_CCC: this helps avoid synchonization issues and also may help in garbage collecting old ucd_msgs
 

int param_UCD_INTERVAL = 200; // in milli sec: interval between two UCD msgs
int param_UCD_TRANSITION = 300; // in milli sec:  interval after the 2nd UCD msg, when ulmap_ccc is updated


// IMP TODO: memory leak: when overwriting ucd_arr[i], the old memory pointed by the ucd_arr[i] needs to be freed.

volatile int ulmap_ccc; // cc of the latest ulmap sent


// these are only used by the ucd_thd
int ucd_ccc;  // configuation change count of the latest ccc message sent - IMP: should be marked volatile
int t_ucd_ccc;  // chan_dec_thd's copy of ucd_ccc
int ucd_trans;  // transition time left for the latest ucd message
int ucd_sleep_dur;
int ucd_flag;  // to indicate the thread should update ulmap_ccc when it next wakes up

struct ucd_msg_wrapper{
  ucd_msg* umsg;
  int count; // number of times this message has been sent
};

typedef struct ucd_msg_wrapper ucd_wrapper;


ucd_wrapper* ucd_arr[256];  // ucd_msgs indexed by their ccc

// begin version2

u_char ucd_msg_bytes[1000];   // the byte sequence for UCD is first written here and then duplicated to msg_ucd_embedded before sending - this is done to know the exact size of msg

// end version2


void ucd_handler_init(){

  // allocate memory for ucd_arr
  int vv;
  for(vv=0; vv < 256; vv++){

    ucd_arr[vv] = (ucd_wrapper*)mac_malloc(sizeof(ucd_wrapper));
    if(!(ucd_arr[vv])){
      FLOG_FATAL("Fatal error in ucd_handler: cannot init ucd_handler. Exiting... \n");
      exit(-1);
    }

    ucd_arr[vv]->umsg = NULL;
    ucd_arr[vv]->count = 0;
  }


  // start with ucd_arr[0] entry populated - i.e., an initial ucd message

  ulmap_ccc = 0;
  ucd_ccc = 0;
  t_ucd_ccc = 0;
  ucd_trans = 0;
  ucd_sleep_dur = param_UCD_INTERVAL;
  ucd_flag = 0;


  // initial ucd message to be sent
  ucd_msg* ucd_ptr;

  ucd_ptr = (ucd_msg*)mac_malloc(sizeof(ucd_msg));
  if(!ucd_ptr){
    FLOG_FATAL("UCD thread: problem in creating ucd test messages. \n");
    return;
  }

  ucd_ptr->management_message_type = 0;
  ucd_ptr->configuration_change_count = 0;


  //begin version2

  // create burst profile for FEC code type and modulation type for QPSK 1/2 to 64 QAM 3/4

  ul_burst_profile *ulbp1;
  ul_burst_profile *ulbp2;

  int ii;
  for(ii=0; ii < NUM_SUPPORTED_MCS; ii++){ // QPSK 1/2 to 64 QAM 3/4

    ulbp1 = (ul_burst_profile*)mac_malloc(sizeof(ul_burst_profile));
    if(!ulbp1){
      FLOG_FATAL("UCD thread: problem in creating ucd test messages. \n");
      return;
    }

    ulbp1->type = (u_int8_t)1;
    ulbp1->length = (u_int8_t)UCD_BURST_PROFILE_LENGTH;
    ulbp1->uiuc = (u_int8_t)ii + 1; // since uiuc 0 is reserved

    ulbp1->fec_code_modulation.type = (u_int8_t)150;
    ulbp1->fec_code_modulation.length = (u_int8_t)1;
    ulbp1->fec_code_modulation.value = (u_int8_t)ii;  // QPSK 1/2 to 64-QAM 3/4

    if(ii == 0){
      ucd_ptr->profile_header = ulbp1;
    }
    else{
      ulbp2->next = ulbp1;
    }
    ulbp2 = ulbp1;
  }
  ulbp1->next = NULL;

  ucd_arr[ucd_ccc]->umsg = ucd_ptr;

  // end version2

  ucd_arr[ucd_ccc]->count = 0;


  FLOG_INFO("Initialized UCD handler. \n");

}




void *ucd_gen(void *parm)
{
  //pthread_t self = pthread_self();


  sigset_t mask;
  int rc;

  int ucd_byte_length;  // length of the UCD msg sent (in bytes)

  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1); // add SIGUSR1 to the mask


  while(can_sync_continue()){


    // mask SIGUSR1
    rc = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    if (rc==0){
      //printf("[debug] ucd_thd: Blocked SIGUSR1 \n");
    }
    else{
      FLOG_WARNING("WARNING: ucd_thd: Unable to block SIGUSR1 \n");
    }

    
    if (ucd_flag == 1){
      // the thread woke up because UCD_TRANSITION period for the ccc in t_ucd_ccc has expired: update ulmap_ccc
      ulmap_ccc = t_ucd_ccc;
      ucd_sleep_dur = param_UCD_INTERVAL - ucd_trans;  // sleep for the time remaining to send the next UCD message
      ucd_trans = 0;
      ucd_flag = 0;
      //printf("[debug] UCD thead: updated the ulmap_ccc for ccc = %d as the ucd transition period expired \n", t_ucd_ccc);
    }


    else{  // else the thread woke up because UCD_INTERVAL of the cc in t_ucd_ccc has expired: send another UCD message

      ucd_sleep_dur = param_UCD_INTERVAL;  // set next sleep duration


      //start version2

      // write the ucd message from "ucd_arr[t_ucd_ccc]->umsg" to "ucd_msg_bytes" (this step is not needed if the UCD msg has not changed)

      if(ucd_arr[t_ucd_ccc]->count == 0){  // only create the ucd byte sequence if the msg is being send for first time

	u_char *tmp_ptr = ucd_msg_bytes;

	// write UCD type
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->management_message_type;
	tmp_ptr++;
	// write other fields of the msg
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->configuration_change_count;
	tmp_ptr++;
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->ranging_backoff_start;
	tmp_ptr++;
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->ranging_backoff_end;
	tmp_ptr++;
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->request_backoff_start;
	tmp_ptr++;
	*tmp_ptr = (u_char)ucd_arr[t_ucd_ccc]->umsg->request_backoff_end;
	tmp_ptr++;

	// finally copy the burst profiles
	ul_burst_profile *bp = ucd_arr[t_ucd_ccc]->umsg->profile_header;

	while(bp != NULL){
	  //bp type
	  *tmp_ptr = (u_char)(bp->type);
	  tmp_ptr++;

	  //bp length (in bytes)
	  *tmp_ptr = UCD_BURST_PROFILE_LENGTH;
	  tmp_ptr++;

	  //bp uiuc
	  *tmp_ptr = (u_char)(bp->uiuc);  // reserved bytes will be set 0 automatically because uiuc is smaller than 15
	  tmp_ptr++;
	  //printf("\n uiuc = %u", bp->uiuc);

	  //copy FEC tlv8
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.type);
	  tmp_ptr++;
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.length);
	  tmp_ptr++;
	  *tmp_ptr = (u_char)(bp->fec_code_modulation.value);
	  tmp_ptr++;

	  //copy ranging data tlv8 - not initialized
	  *tmp_ptr = 151;
	  tmp_ptr++;
	  *tmp_ptr = 1;
	  tmp_ptr++;
	  *tmp_ptr = 0x00;
	  tmp_ptr++;

	  bp = bp->next;
	}
	// done populating the ucd_msg_bytes

	// number of bytes in ucd msg
	ucd_byte_length = tmp_ptr - ucd_msg_bytes;

      }

      // copy ucd message to a new allocated memory -  the pointer send using enqueue_transport() may be deallocated, and we do not want the message in our ucd_arr to be deallocated

      u_char* msg_ucd_embedded = (u_char*) mac_sdu_malloc(ucd_byte_length, 5);  //pointer to ucd message with burst profiles embedded, with pointers discarded
      if(!msg_ucd_embedded){
	FLOG_FATAL("Memory allocation failed for UCD message creation. \n");
	continue;
      }

      memcpy( msg_ucd_embedded, ucd_msg_bytes, ucd_byte_length); 


/*       //begin TEST */

/*       ucd_msg* test_ucd = (ucd_msg*)mac_malloc(sizeof(ucd_msg)); */
/*       if(!test_ucd){ */
/* 	FLOG_FATAL("UCD handler: 10: problem in allocating memory for ucd message. \n"); */
/* 	return -1; */
/*       } */
/*       parse_ucd(msg_ucd_embedded, ucd_byte_length, test_ucd); */
/*       print_ucd_msg(test_ucd); */

/*       //end TEST */

      //enqueue it to the sdu queue
      enqueue_transport_sdu_queue(dl_sdu_queue, BROADCAST_CID, ucd_byte_length, msg_ucd_embedded);


/*       //begin: for TEST */
/*       FLOG_FATAL("[debug] UCD thead: sent UCD message with ccc = %d. \n ucd burst profile FEC codes: ", t_ucd_ccc); */
/*       ul_burst_profile *bp = ucd_arr[t_ucd_ccc]->umsg->profile_header; */

/*       // finally copy the burst profiles */
/*       while(bp != NULL){ */
/* 	FLOG_FATAL("[debug] %u ", bp->fec_code_modulation.value); */
/* 	bp = bp->next; */
/*       } */
/*       printf("[debug] \n"); */

/*       // exit(0); */

/*       //end: for TEST */

      //end version2



      ucd_arr[t_ucd_ccc]->count++;  // increase the number of time this message has been sent

      if((ucd_arr[t_ucd_ccc]->count >= 2) && (ucd_trans > 0)){   // if sent the message for the 2nd time, start keeping track of dlmap transition
	ucd_trans = ucd_trans - param_UCD_INTERVAL;
	if(param_UCD_INTERVAL < ucd_trans){
	  ucd_sleep_dur = param_UCD_INTERVAL;
	}
	else{   // update ulmap_ccc when the thread next wakes up
	  ucd_sleep_dur = ucd_trans;
	  ucd_flag = 1;
	}
      }
    }



    rc = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    if (rc==0){
      //printf("[debug] ucd_thd: Unblocked SIGUSR1 \n");
    }
    else{
      FLOG_WARNING("WARNING: ucd_thd: Unable to unblock SIGUSR1 \n");
    }

    //printf("[debug] ucd_thd: sleeping for %d milli sec \n", ucd_sleep_dur);
    chan_desc_msleep(ucd_sleep_dur);

  }
	return 0;
}



 void sigusr1_handler(){  // common handler for all SIGUSR1 triggers (for both UCD and DCD)

   if(pthread_self() == ucd_thd){
     //printf("[debug] Got UCD AMC trigger. \n");
     ucd_sleep_dur = 0;
     t_ucd_ccc = ucd_ccc;
     ucd_trans = param_UCD_TRANSITION + param_UCD_INTERVAL;
     ucd_flag = 0;
   }

   if(pthread_self() == dcd_thd){
     //printf("[debug] Got DCD AMC trigger. \n");
     dcd_sleep_dur = 0;
     t_dcd_ccc = dcd_ccc;
     dcd_trans = param_DCD_TRANSITION + param_DCD_INTERVAL;
     dcd_flag = 0;
   }

 }


int chan_desc_msleep(unsigned long milisec)
{
  struct timespec req={0};
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milisec*1000000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
} 



// this function is called by the amc module to create a new ucd message from its amc_list and signal the ucd_thd
// TODO: the function populates only a small number of fields in the ucd message; needs to expanded later

int amc_ucd_update(amc_info* amclist){

   ucd_msg* ucd_ptr = (ucd_msg*)mac_malloc(sizeof(ucd_msg));
   if(!ucd_ptr){
     FLOG_FATAL("UCD thread: 1: problem in allocating memory for ucd message in amc_ucd_update. \n");
     return 0;
   }

   // increase the ucd ccc

   ucd_ccc = (ucd_ccc + 1) % 256;


   // populate the ucd_msg and its burst profiles using the amclist

   ucd_ptr->management_message_type = (u_int8_t)0;
   ucd_ptr->configuration_change_count = (u_int8_t)ucd_ccc;

   ss_amc_info* tmpinfo = amclist->ss_amc_head;
   ul_burst_profile* ulbp0;
   ul_burst_profile* ulbp1;
   int tmp_uiuc = 1;
   //tlv8 *fec_ptr;

   int is_fec_already_seen[MAX_FEC_CODE_TYPE+1];  // fec code type beyond 25 are reserved
   int jj;
   for(jj=0; jj <= MAX_FEC_CODE_TYPE; jj++){
     is_fec_already_seen[jj] = 0;
   }

   while((tmpinfo != NULL) &&(tmp_uiuc < MAX_UIUC)){   // uiuc value can be at most MAX_UIUC


     // two burst profiles should not have the same fec
     if ((tmpinfo->ul_fec_code_modulation_type > MAX_FEC_CODE_TYPE) || (is_fec_already_seen[(tmpinfo->ul_fec_code_modulation_type)] == 1)){
       tmpinfo = tmpinfo->next;
       continue;
     }
     else{
       is_fec_already_seen[(tmpinfo->ul_fec_code_modulation_type)] = 1;
     }



     ulbp1 = (ul_burst_profile*)mac_malloc(sizeof(ul_burst_profile));
     if(!ulbp1){
       FLOG_FATAL("UCD thread: 2: problem in allocating memory for ucd message in amc_ucd_update. \n");
       return 0;
     }

     ulbp1->type = (u_int8_t)1;
     ulbp1->length = (u_int8_t)UCD_BURST_PROFILE_LENGTH;
     ulbp1->uiuc = (u_int8_t)tmp_uiuc;

/*      fec_ptr = (tlv8*)mac_malloc(sizeof(tlv8)); */
/*      if(!fec_ptr){ */
/*        printf("UCD thread: 3: problem in allocating memory for ucd message in amc_ucd_update. \n"); */
/*        return; */
/*      } */
/*      fec_ptr->type = (u_int8_t)150; */
/*      fec_ptr->length = (u_int8_t)1; */
/*      fec_ptr->value = (u_int8_t)(tmpinfo->ul_fec_code_modulation_type); */

/*      ulbp1->fec_code_modulation = *fec_ptr; */

     ulbp1->fec_code_modulation.type = (u_int8_t)150;
     ulbp1->fec_code_modulation.length = (u_int8_t)1;
     ulbp1->fec_code_modulation.value = (u_int8_t)(tmpinfo->ul_fec_code_modulation_type);
     ulbp1->next = NULL;

     if(tmp_uiuc == 1){
       ucd_ptr->profile_header = ulbp1;
     }
     else{
       ulbp0->next = ulbp1;
     }

     tmp_uiuc++;
     ulbp0 = ulbp1;
     tmpinfo = tmpinfo->next;
   }



   // populate the ucd_msg array with the message created above
   
   ucd_arr[ucd_ccc]->umsg = ucd_ptr;
   ucd_arr[ucd_ccc]->count = 0;


   // TODO: free current ucd_arr[ucd_ccc]->umsg (don't touch the content at lower index which might be in use)


   // send the signal to the ucd_thd
   int rc1;
   rc1 = pthread_kill(ucd_thd, SIGUSR1);
   if(rc1!=0){
     FLOG_WARNING("Warning: AMC : Error in raising SIGUSR1. \n");
   }
   return 0;
}


// query interface for the ucd - there will be synchronization issues if the corresponding ucd_arr entry is being populated by the amc_update
// but if the querying entity (PHY, scheduler) is behaving properly, such a query should never happen - a UCD query for a ccc is only done
// by these modules only after a UCD message is sent with that ccc, and ucd_arr[ccc] entry is populated before sending a UCD msg with ccc.

ucd_msg* ucd_msg_query(int query_ucd_ccc){

  if((query_ucd_ccc >= 0) && (query_ucd_ccc <= 255)){
    return(ucd_arr[query_ucd_ccc]->umsg);
  }
  else{
    FLOG_WARNING("WARNING: illegal UCD query. \n");
    return(NULL);
  }

}

ModulCodingType get_mcs_from_uiuc(ucd_msg *ucd, short uiuc)
{
  ul_burst_profile *ulbp = ucd->profile_header;
  while (ulbp != NULL)
  {
    if (ulbp->uiuc == uiuc)
    {
      //DEBUG("Found an MCS for this UIUC\n");
      return ulbp->fec_code_modulation.value;
    }
  }
  FLOG_WARNING("Didn't find an MCS for this UIUC\n");
  return -1;
}





// ********************************* everything below this line is for TESTING ****************************************



void* dummy_amc_gen(void* param){ // for testing - structures created are not freed

  int dummy_amc_sleep = 12000;
  amc_info* amclist1;
  int count = 2;

  while(1){

    amclist1 = (amc_info*)mac_malloc(sizeof(amc_info));
    if(!amclist1){
      FLOG_FATAL("dummy_amc1: 1: error in allocating memory. \n");
      exit(1);
    }
    amclist1->ss_num = 10; // not used

    int cc;
    ss_amc_info* info1;
    ss_amc_info* info2;

    for(cc=0; cc < count; cc++){

      info1 = (ss_amc_info*)mac_malloc(sizeof(ss_amc_info));
      if(!info1){
	FLOG_FATAL("dummy_amc1: 2: error in allocating memory. \n");
	exit(1);
      }

      info1->ul_fec_code_modulation_type = cc;
      info1->dl_fec_code_modulation_type = (cc/2)+5;
      info1->next = NULL;

      if(cc==0){
	amclist1->ss_amc_head = info1;
      }
      else{
	info2->next = info1;
      }
      info2 = info1;

    }

    amc_ucd_update(amclist1);
    amc_dcd_update(amclist1);

    count = (count+1)%26;
    if (count == 0) count = 2;
    chan_desc_msleep(dummy_amc_sleep);
  }

}







 int dummy_ucd_amc(int num_bp2){  // for TESTING


   // FOR TEST (need to be removed in the final version): create a new UCD structure with pointer ucd_ptr
   ucd_msg* ucd_ptr;

   ucd_ptr = (ucd_msg*)mac_malloc(sizeof(ucd_msg));
   if(!ucd_ptr){
     FLOG_FATAL("UCD thread: problem in creating ucd test messages. \n");
     return 0;
   }

   int cc;
   ul_burst_profile* ulbp0;
   //ul_burst_profile* ulbp1;

   for(cc=0; cc < num_bp2; cc++){
     ul_burst_profile* ulbp1 = (ul_burst_profile*)mac_malloc(sizeof(ul_burst_profile));
     if(!ulbp1){
       FLOG_FATAL("UCD thread: problem in creating ucd test messages. \n");
       return 0;
     }
     ulbp1->next = NULL;

     if(cc==0){
       ucd_ptr->profile_header = ulbp1;
     }
     else{
       ulbp0->next = ulbp1;
     }
     ulbp0 = ulbp1;
   }

   ucd_ccc = (ucd_ccc + 1) % 256;
   
   ucd_arr[ucd_ccc]->umsg = ucd_ptr;
   ucd_arr[ucd_ccc]->count = 0;

   //END FOR TEST


   // TODO: free current ucd_arr[ucd_ccc]->umsg (don't touch the content at lower index which might be in use)


   // send the signal
   int rc1;
   rc1 = pthread_kill(ucd_thd, SIGUSR1);
   if(rc1!=0){
     FLOG_WARNING("Warning: AMC : Error in raising SIGUSR1. \n");
   }
   return 0;
 }


