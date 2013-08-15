/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_cbr_gen.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DL_CBR_GEN_
#define _DL_CBR_GEN_

#include <unistd.h>
#include "mac.h"
#include "memmgmt.h"
#include "log.h"
#include "perf_log.h"
#include "thread_sync.h"
#include "mac_connection.h"

typedef enum{
  G711=0,
  G723=1,
  VANILLA = 3
} cbr_type;

typedef struct cbr_parameter cbr_param;

struct cbr_parameter{  // parameters used for generate cbr packets
  int cid;
  size_t packet_size; // in bytes
  int delay; // in ms - delay between two packet
  int bit_rate; // in bit per second : (packet_size * 8)/(delay/1000)
  int duration; // in ms: number of messages sent = duration/delay (rough estimate)
  int next_tx_time; // in ms: when should the next packet be sent?
       // should be set to initial offset at the beginning (useful for interleaving different cbr traffics)
  cbr_type type;
  int updated;
  FILE *fp; //file pointer to the file where input bytes sequence will be dumped
  cbr_param* next;
};




extern cbr_param* cbr_param_list_head;

extern int cbr_init();

extern int add_cid_cbr(int cid, int intial_offset, int duration, cbr_type type);
extern int add_cid_cbr_vanilla(int cid, int intial_offset, int duration, int packet_size, int delay);

extern int set_G711_param(cbr_param* param);
extern int set_G723_param(cbr_param* param);


extern void* CBR(void* ignore);
int close_files();


#endif
