/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ucd_dcd_parser.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


/* 
   byte sequence structure of UCD msg sent (size in bits or type in parenthesis):
   | type(8) | CCC(8) | ranging start (8) | ranging end (8) | registration start (8) | registration end (8) | (burst profile1) type (8) | length (8) | reserved (4) | UIUC (4) | FEC (tlv8) | Ranging data (tlv8) | cn override (tlv40) | (burst profile 2) ... |
*/


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


int parse_ucd(u_char* payload, int length, ucd_msg* ucd){  // meomry for ucd should be allocated before calling this function

  u_char* tmp_ptr = payload;

  ucd->management_message_type = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  ucd->configuration_change_count = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  ucd->ranging_backoff_start = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  ucd->ranging_backoff_end = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  ucd->request_backoff_start = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  ucd->request_backoff_end = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 

  //read the tlvs (currently only burst profile tlvs are present in ucd)
  int type, len;
  ul_burst_profile *last_bp = NULL;

  while((tmp_ptr - payload) < length){

    type = (u_int8_t)(*tmp_ptr);
    tmp_ptr++;
    len = (u_int8_t)(*tmp_ptr);
    tmp_ptr++;
    if(type == 1){  // burst profile
      //printf("\n parsing: got burst profile : "); //test
      //printf("parsing: type = %u ", type);  //test
      //printf("parsing: length = %u ", len);  //test

      ul_burst_profile *ulbp1 = (ul_burst_profile*)mac_malloc(sizeof(ul_burst_profile));
      if(!ulbp1){
	FLOG_FATAL("UCD parser: problem in creating ucd test messages. \n");
	return -1;
      }

      u_char *bp_value_start = tmp_ptr; // where the value field of the bp (currently from uiuc) 

      ulbp1->type = type;
      ulbp1->length = len;
      ulbp1->uiuc = (u_int8_t)(*tmp_ptr) & 0x0f;
      //printf("parsing: uiuc = %u ", ulbp1->uiuc);  //test
      tmp_ptr++;

      // read the tlvs
      int bp_tlv_type, bp_tlv_length;

      while((tmp_ptr - bp_value_start) < len){

	bp_tlv_type = (u_int8_t)(*tmp_ptr);
	//printf("parsing: bp tlv type = %u ", bp_tlv_type);  //test
	tmp_ptr++;
	bp_tlv_length = (u_int8_t)(*tmp_ptr);
	//printf("parsing: bp tlv length = %u ", bp_tlv_length);  //test
	tmp_ptr++;
	if(bp_tlv_type == 150){  // if FEC
	  ulbp1->fec_code_modulation.type = bp_tlv_type;
	  ulbp1->fec_code_modulation.length = bp_tlv_length;
	  ulbp1->fec_code_modulation.value = (u_int8_t)(*tmp_ptr);
	  //printf("parsing: bp tlv value = %u ", *tmp_ptr);  //test
	  tmp_ptr++;
	}
	else{ // skip
	  tmp_ptr += bp_tlv_length;
	}
	ulbp1->next = NULL;

      }

      if(last_bp == NULL){  // if first burst profile of the msg
	ucd->profile_header = ulbp1;
      }
      else{
	last_bp->next = ulbp1;
      }
      last_bp = ulbp1;
    }
    else{  // if not burst profile then skip
      tmp_ptr += len;
    }
  }
  return(0);

}



int parse_dcd(u_char* payload, int length, dcd_msg* dcd){  // meomry for dcd should be allocated before calling this function


  u_char* tmp_ptr = payload;

  dcd->management_message_type = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 
  dcd->configuration_change_count = (u_int8_t)(*tmp_ptr);
  tmp_ptr++; 

  //read the tlvs (currently only burst profile tlvs are present in dcd)
  int type, len;
  dl_burst_profile *last_bp = NULL;

  while((tmp_ptr - payload) < length){

    type = (u_int8_t)(*tmp_ptr);
    tmp_ptr++;
    len = (u_int8_t)(*tmp_ptr);
    tmp_ptr++;
    if(type == 1){  // burst profile
      //printf("\n parsing: got burst profile : "); //test
      //printf("parsing: type = %u ", type);  //test
      //printf("parsing: length = %u ", len);  //test

      dl_burst_profile *dlbp1 = (dl_burst_profile*)mac_malloc(sizeof(dl_burst_profile));
      if(!dlbp1){
	FLOG_FATAL("DCD parser: problem in creating dcd test messages. \n");
	return -1;
      }

      u_char *bp_value_start = tmp_ptr; // where the value field of the bp (currently from diuc) 

      dlbp1->type = type;
      dlbp1->length = len;
      dlbp1->diuc = (u_int8_t)(*tmp_ptr) & 0x0f;
      //printf("parsing: diuc = %u ", dlbp1->diuc);  //test
      tmp_ptr++;

      // read the tlvs
      int bp_tlv_type, bp_tlv_length;

      while((tmp_ptr - bp_value_start) < len){

	bp_tlv_type = (u_int8_t)(*tmp_ptr);
	//printf("parsing: bp tlv type = %u ", bp_tlv_type);  //test
	tmp_ptr++;
	bp_tlv_length = (u_int8_t)(*tmp_ptr);
	//printf("parsing: bp tlv length = %u ", bp_tlv_length);  //test
	tmp_ptr++;
	if(bp_tlv_type == 150){  // if FEC
	  dlbp1->fec_code_modulation.type = bp_tlv_type;
	  dlbp1->fec_code_modulation.length = bp_tlv_length;
	  dlbp1->fec_code_modulation.value = (u_int8_t)(*tmp_ptr);
	  //printf("parsing: bp tlv value = %u ", *tmp_ptr);  //test
	  tmp_ptr++;
	}
	else{ // skip
	  tmp_ptr += bp_tlv_length;
	}
	dlbp1->next = NULL;

      }

      if(last_bp == NULL){  // if first burst profile of the msg
	dcd->profile_header = dlbp1;
      }
      else{
	last_bp->next = dlbp1;
      }
      last_bp = dlbp1;
    }
    else{  // if not burst profile then skip
      tmp_ptr += len;
    }
  }

  return 0;

}


int print_ucd_msg(ucd_msg *ucd){
  printf("\n \n mgt msg type = %u", ucd->management_message_type);
  printf("\n ccc = %u", ucd->configuration_change_count);
  printf("\n ranging_start = %u", ucd->ranging_backoff_start);
  printf("\n ranging_end = %u", ucd->ranging_backoff_end);
  printf("\n reg_start = %u", ucd->request_backoff_start);
  printf("\n reg_end = %u", ucd->request_backoff_end);
  ul_burst_profile *bp = ucd->profile_header;
  while(bp != NULL){
    printf("\n --- \n burst profile type = %u", bp->type);
    printf("\n burst profile length = %u", bp->length);
    printf("\n burst profile uiuc = %u", bp->uiuc);
    printf("\n fec type = %u ", bp->fec_code_modulation.type);
    printf(" fec length = %u ", bp->fec_code_modulation.length);
    printf(" fec value = %u \n", bp->fec_code_modulation.value);
    bp = bp->next;
  }

  return 0;

}


int print_dcd_msg(dcd_msg *dcd){

  printf("\n \n mgt msg type = %u", dcd->management_message_type);
  printf("\n ccc = %u", dcd->configuration_change_count);
  dl_burst_profile *bp = dcd->profile_header;
  while(bp != NULL){
    printf("\n --- \n burst profile type = %u", bp->type);
    printf("\n burst profile length = %u", bp->length);
    printf("\n burst profile diuc = %u", bp->diuc);
    printf("\n fec type = %u ", bp->fec_code_modulation.type);
    printf(" fec length = %u ", bp->fec_code_modulation.length);
    printf(" fec value = %u \n", bp->fec_code_modulation.value);
    bp = bp->next;
  }

  return 0;

}
