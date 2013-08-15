/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_mme_gen.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "CS.h"
#include "dl_mme_gen.h"
#include "mac_sdu_queue.h"
#include "dl_exp_params.h"
#include "br_queue.h"
#include "memmgmt.h"
#include "log.h"

#define BE_PAYLOAD_SIZE 300

// creates an array of param types

param_array_wrapper* dl_mm_params_gen(){

  mgt_msg_param** param_array;
  mgt_msg_param* tmp_param;

  //printf("\n param array memory = %d\n", (sizeof(mgt_msg_param*) * (max_valid_primary_cid - PRIMARY_CID_MIN_VALUE + 1) * param_DL_MM_NUM_TYPE_PER_CID));
  param_array = (mgt_msg_param**) mac_malloc(sizeof(mgt_msg_param*) * (max_valid_primary_cid - PRIMARY_CID_MIN_VALUE + 1) * param_DL_MM_NUM_TYPE_PER_CID);  // number of primary cid * number of mm types per cid

  if(!param_array){
    FLOG_FATAL("1: Memory allocation failed in dl_mm_params_gen()");
  }
  int i, j, count;
  count = 0;
  for(i=PRIMARY_CID_MIN_VALUE; i <= max_valid_primary_cid; i++){ // cids
    for(j=0; j < param_DL_MM_NUM_TYPE_PER_CID; j++){  // types
      tmp_param = (mgt_msg_param*) mac_malloc(sizeof(mgt_msg_param));
      if(!tmp_param){
	FLOG_FATAL("2: Memory allocation failed in dl_mm_params_gen()");
      }
      tmp_param->cid = i;
      tmp_param->type = param_DL_MM_MIN_TYPE + j;
      tmp_param->max_size = param_DL_MM_MAX_SIZE;
      tmp_param->delay = param_DL_MM_DELAY;
      tmp_param->duration = param_DL_EXP_DURATION;

      param_array[count] = tmp_param;
      count++;
    }
   
  }

  param_array_wrapper *wr = (param_array_wrapper*) mac_malloc(sizeof(param_array_wrapper));
  if(!wr){
    FLOG_FATAL("3: Memory allocation failed in dl_mm_params_gen()");
  }
  wr->param_array = param_array;
  wr->length = count;
  
  return wr;

}

int free_mm_params(param_array_wrapper *wr)
{
	int i = 0, j = 0, count = 0;
	for(i=PRIMARY_CID_MIN_VALUE; i <= max_valid_primary_cid; i++)
	{ // cids
		for(j=0; j < param_DL_MM_NUM_TYPE_PER_CID; j++)
		{  // types
			if(wr->param_array[count] != NULL)
			{
				free(wr->param_array[count]);
			}
			count++;
		}
	}
	free(wr->param_array);
	free(wr);
	return 0;
}

void print_param_array(param_array_wrapper *wr){

  int i;
  mgt_msg_param* tmp;
  for(i=0; i < wr->length; i++){
    tmp=(wr->param_array)[i];
    //printf("cid %d delay %d max_size %d type %d duration %d\n", tmp->cid, tmp->delay, tmp->max_size, tmp->type, tmp->duration);
  }
}


// after every param_DL_MM_DELAY, send a set of mgt_msg - one for each param type

void* DL_MM_GEN(void* wr1) {
#ifdef INTEGRATION_TEST
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        return NULL;
    }
#endif

  FLOG_DEBUG("Starting MAC DL Mgt Msg Generator thread ...");

  //Needed for instrumentation
  create_init_trace_buffer(8192, "dl_MM_GEN");

  
  // Enqueue SDUs to mac sdu queue

  param_array_wrapper* wr = (param_array_wrapper *) wr1;
  int i, j;
  mgt_msg_param** ar = wr->param_array;
#ifndef SS_TX
#ifndef SS_RX
  int total_be_cids=(param_MAX_VALID_BASIC_CID-BASIC_CID_MIN_VALUE+1)*param_MAX_VALID_BE_PER_SS;  // number of ss * number of be per ss
  //printf("\n Total be cids as calculated in MM_GEN() %d \n", total_be_cids);
  int last_served_be_cid=0;
#endif
#endif
  for(i=0; i<((int)(param_DL_EXP_DURATION/param_DL_MM_DELAY)); i++) {
    for(j=0; j< wr->length; j++){

      //create a mgt msg packet
      size_t payload_size =rand()%(ar[j]->max_size) +1; // randomly choose the size of mgt_msg payload
      size_t tmp_mgt_msg_size = payload_size + sizeof(unsigned char);

      // TODO: temporary fix
      // using sdu malloc for mmm because of freeing problem
      // class type set to 5 (see #define above) which is not used by real sdus (ugs is 0 and be is 4)

      void* tmp_mgt_msg = (void*) mac_sdu_malloc(tmp_mgt_msg_size, MMM_CLASS_TYPE);

	                // (void*) mac_malloc(tmp_mgt_msg_size);
      if(!tmp_mgt_msg){
	FLOG_FATAL("1: Memory allocation failed in DL_MM_GEN()");
      }

      // set the first unsigned char sized memory to type
      unsigned char* tmp_type = (unsigned char *)tmp_mgt_msg;
      *tmp_type = ar[j]->type;

      //enqueue it to the sdu queue
      //printf("\nTrying to send MMsg with cid %d and type %d\n", ar[j]->cid, ar[j]->type);
      enqueue_transport_sdu_queue(dl_sdu_queue, ar[j]->cid, tmp_mgt_msg_size, tmp_mgt_msg);

    }

#ifndef SS_TX
    //enqueue BWR messages starting from the next BE CID last served 
    //(in a round robin fashion)
    int kk=0;
    if(total_be_cids!=0) {
      for(kk=0; kk<param_DL_MM_NUM_BWR; kk++) {
	int be_cid=BE_CID_MIN_VALUE +(last_served_be_cid%total_be_cids);
	int num_bytes=rand()% BE_PAYLOAD_SIZE;
	int type=rand()%2; //incremental or aggregate request
	//printf("\nTrying to send BRMsg with be cid %d and type %d\n", be_cid, type);
	enqueue_bw_req_queue(bwr_q_list, be_cid, num_bytes, type);
	last_served_be_cid++;
      }
    }
#endif

    //sleep for some time
    dl_mm_msleep(param_DL_MM_DELAY);

  }
  //  printf("Total bytes enqueued: %d\n", total_bytes);
  //  mac_sdu_queue_finalize();
  return NULL;
}


int dl_mm_msleep(unsigned long milisec)
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
