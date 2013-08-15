/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_common_data.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_COMMON_DATA_H__
#define __MAC_COMMON_DATA_H__

#include <stdlib.h>
#include "mac_message.h"

// const int FRAME_NUMBER_MOD=16777216;
#define FRAME_NUMBER_MOD 16777216
// Number of past ULMAP values that are stored
#define NUM_ULMAP_STORED 5
// Number of past DLMAP values that are stored
#define NUM_DLMAP_STORED 5

int cur_frame_num;

int initialize_common_data();
int release_common_data();
int get_current_frame_num();

typedef struct {
ul_map_msg *ulmap[NUM_ULMAP_STORED];
long int frame_num[NUM_ULMAP_STORED];
int last_stored_index;
} stored_ulmap;

// Global variable storing NUM_ULMAP_STORED recent ULMAP values
stored_ulmap stored_ulmap_list;

int init_stored_ulmap_list();
int free_stored_ulmap_list();

int set_ul_map_msg(ul_map_msg* ulmap, long int frame_num);
int get_ul_map_msg(ul_map_msg** ulmap, long int frame_num);

typedef struct {
dl_map_msg *dlmap[NUM_DLMAP_STORED];
long int frame_num[NUM_DLMAP_STORED];
int last_stored_index;
} stored_dlmap;

// Global variable storing NUM_ULMAP_STORED recent ULMAP values
stored_dlmap stored_dlmap_list;

int init_stored_dlmap_list();
int free_stored_dlmap_list();

int set_dl_map_msg(dl_map_msg* dlmap, long int frame_num);
int get_dl_map_msg(dl_map_msg** dlmap, long int frame_num);


dl_map_msg* dl_map;
ul_map_msg* ul_map;
dcd_msg* dcd;
ucd_msg* ucd;

int set_dcd_msg(dcd_msg* dcdmsg);
int set_ucd_msg(ucd_msg* ucdmsg);

int init_rcvd_ucd_dcd_arr();
int free_rcvd_ucd_dcd_arr();

dcd_msg* get_dcd_msg(int query_dcd_ccc);
ucd_msg* get_ucd_msg(int query_ucd_ccc);

/*

typedef struct systemmapinfo system_map_info;
struct systemmapinfo
{
    int frame_no;
    dl_map_msg* dl_map;
    ul_map_msg* ul_map;
    dcd_msg* dcd;
    ucd_msg* ucd;
    system_map_info* next;
}

typedef struct 
{
    int frame_num;
    system_map_info* maps_header;
    system_map_info* maps_tail;
}system_map_queue;

int initialize_system_mapq(system_map_queue** mapq);

int enqueue_system_map(system_map_queue* mapq, system_map_info* map_info);

int dequeue_system_map(system_map_queue* mapq, int frame_no, system_map_info** map_info);

int release_system_mapq(system_map_queue* mapq);
*/

#endif

