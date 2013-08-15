/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: CS.c

Change Activity:

Date    	Description of Change        	By
----------------------------------------------------------
01-Oct-2008 	     Created		   Malolan Chetlur

----------------------------------------------------------
*/

#include <assert.h>
#include "mac.h"
#include "CS.h"
#include "cs_sdu_header.h"
#include "classifier.h"


extern int IPV4_source_address_rule(void* arg, size_t arg_len, cs_sdu_header* sdu);
extern int IPV4_dest_address_rule(void* arg, size_t arg_len, cs_sdu_header* sdu);

connection_classifier_info** ipv4_conn_clsfr_array=NULL;
cls_func_ptr* ipv4_f_ptr=NULL;

void cs_init() {
  size_t len=(sizeof(connection_classifier_info*))*MAX_CIDS;
  ipv4_conn_clsfr_array=(connection_classifier_info**) mac_malloc(len);
  assert(ipv4_conn_clsfr_array!=NULL);
  memset(ipv4_conn_clsfr_array,0, len);

  ipv4_f_ptr=(cls_func_ptr*) mac_malloc(sizeof(cls_func_ptr)*NUM_CLSFY_FUNCTIONS);
  //initialize the classification functions
  ipv4_f_ptr[4]=IPV4_source_address_rule;
  ipv4_f_ptr[5]=IPV4_dest_address_rule;
}

void IPV4_CS_DL(cs_sdu_header* sdu) {
  classify(ipv4_conn_clsfr_array, ipv4_f_ptr, sdu);
  performPHS(ipv4_conn_clsfr_array, ipv4_f_ptr, sdu);
  enqueue_cs_dl(sdu);
}

//Currently classification is done sequentially and
// unintelligently;
// multiple threads can do classification among the cid space
void classify(connection_classifier_info** conn_clsfr_array, cls_func_ptr* f_ptr, cs_sdu_header* sdu) {
  int ii=0;
  for(ii=0; ii<MAX_CIDS; ii++) {
    
    //if cid is active check if the sdu is for that cid
    if((conn_clsfr_array[ii]!=NULL)&&(conn_clsfr_array[ii]->cid_status==1)) {
      classifier* cls_rule=conn_clsfr_array[ii]->classification_list;
      while(cls_rule!=NULL) {
	if((*(f_ptr[cls_rule->classifier_indx]))(cls_rule->cls_arg, cls_rule->arg_len, sdu)) {
	  //Satisfied the classification rule
	  //cid is ii
	  sdu->classified=1;
	  sdu->cid=conn_clsfr_array[ii]->cid;
	  return;
	}
	cls_rule=cls_rule->next;
      }
    }
  }
  //Error: Unable to classify the sdu; will be discarded
  sdu->classified=0;
  sdu->cid=-1;
  return;
}

  void performPHS(connection_classifier_info** conn_clsfr_array, cls_func_ptr* f_ptr, cs_sdu_header* sdu) {
  //currently PHS is not supported
  // will be implemented in future phases
}

void enqueue_cs_dl(cs_sdu_header* sdu) {
  if(sdu->classified==1) {
    assert(sdu->cid!=-1);
    if(sdu->hdr_supd==1) {
      assert(sdu->hdr_supd_sdu!=NULL);
      enqueue_transport_sdu_queue(dl_sdu_queue, sdu->cid, sdu->supd_sdu_len, sdu->hdr_supd_sdu);
    }
    else {
      assert(sdu->upper_layer_sdu!=NULL);
      enqueue_transport_sdu_queue(dl_sdu_queue, sdu->cid, sdu->sdu_len, sdu->upper_layer_sdu);
    }
  }
}
