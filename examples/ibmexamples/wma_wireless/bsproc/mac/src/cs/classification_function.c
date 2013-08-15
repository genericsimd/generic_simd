/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: classification_function.c

Change Activity:

Date    	Description of Change        	By
----------------------------------------------------------
01-Oct-2008 	     Created		   Malolan Chetlur

----------------------------------------------------------
*/

#include "cs_sdu_header.h"
#include "classifier.h"
#include "flog.h"

int ipv4_addr_match(u_char* sdu_ip_addr, u_char* addr, u_char* addr_mask) {
  return (
	  ((sdu_ip_addr[0] & addr_mask[0]) == addr[0]) &&
	  ((sdu_ip_addr[1] & addr_mask[1]) == addr[1]) &&
	  ((sdu_ip_addr[2] & addr_mask[2]) == addr[2]) &&
	  ((sdu_ip_addr[3] & addr_mask[3]) == addr[3])
	  );
}

int IPV4_address_rule(void* arg, size_t arg_len, cs_sdu_header* sdu, int indx) {
  //First get the src address from IPV4 packet
  u_char* sdu_src_addr= ((u_char*)sdu->upper_layer_sdu)+indx;

  //for all src address, mask in the arg check if they match.
  int num_args= arg_len/8;
  int ii=0;
  for(ii=0; ii< num_args; ii++) {
    u_char* s_addr= (u_char*)arg + (ii*8);
    u_char* s_addr_mask= (u_char*) arg + (ii*8)+4;
    if(ipv4_addr_match(sdu_src_addr, s_addr, s_addr_mask)==1) {
      return 1;
    }
  }
  return 0;
}

int IPV4_source_address_rule(void* arg, size_t arg_len, cs_sdu_header* sdu) {
  int indx=12;
  return IPV4_address_rule(arg, arg_len, sdu, indx);
}

int IPV4_dest_address_rule(void* arg, size_t arg_len, cs_sdu_header* sdu) {
  int indx=16;
  return IPV4_address_rule(arg, arg_len, sdu, indx);
}
