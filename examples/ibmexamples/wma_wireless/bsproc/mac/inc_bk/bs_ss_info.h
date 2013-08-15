/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: bs_ss_info.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifndef _BSSINFO_H_
#define _BSSINFO_H_

#include <sys/types.h>
#include "mac_serviceflow.h"

typedef struct bsssInfo bs_ss_info;

struct bsssInfo {
  int ss_index;
  u_int64_t mac_addr;
  u_int16_t basic_cid;
  u_int16_t primary_cid;
  u_int8_t polling_status;
  ModulCodingType modulcoding;
  serviceflow* sf_list_head;
  bs_ss_info *next;
};



//Global pointer to list of bssInfo structures
extern bs_ss_info* ssinfo_list_head;
int add_bs_ss_info(bs_ss_info* ss_info);
bs_ss_info* find_bs_ss_info(u_int64_t mac_addr);
u_int16_t get_basic_cid_from_ss_index(int ss_index);
int add_svc_flow_to_bs(serviceflow* sflow, u_int64_t mac_addr);
int delete_sf(serviceflow *sf, bs_ss_info *ssinfo);
extern u_int64_t get_macaddr_from_pcid(int prim_cid);
extern bs_ss_info* get_ssinfo_from_pcid(int prim_cid);

#endif
