/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_mme_gen.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DLMME_H_
#define _DLMME_H_

typedef struct{  // parameters used for generating message of a given type on a cid
  int cid;
  int delay; // in ms - delay between two packet (rough estimate)
  int max_size;  // max size of payload
  unsigned char type;  // type of mgt_msg
  int duration; // in ms: number of messages sent = duration/delay (very rough estimate)
} mgt_msg_param;


typedef struct{
  mgt_msg_param** param_array;
  int length;
} param_array_wrapper;

extern void* DL_MM_GEN(void* wr);

param_array_wrapper* dl_mm_params_gen();
int free_mm_params(param_array_wrapper *wr);

void print_param_array(param_array_wrapper *wr);
int dl_mm_msleep(unsigned long milisec);

#endif
