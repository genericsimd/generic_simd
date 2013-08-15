/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_amc.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_AMC_H__
#define __MAC_AMC_H__
#include <sys/types.h>
#include "mac_config.h"

typedef struct ssamcinfo ss_amc_info;
struct ssamcinfo{
u_int64_t ss_mac;
int subchannel_num;
int ul_fec_code_modulation_type;
int dl_fec_code_modulation_type;
ss_amc_info* next;
};

typedef struct{
    int ss_num;
    ss_amc_info* ss_amc_head;
}amc_info;

amc_info* amc_list;

extern int initialize_amc_info();

extern int get_amc_info(amc_info** amc_info_header);

extern int free_amc_info();

#endif
