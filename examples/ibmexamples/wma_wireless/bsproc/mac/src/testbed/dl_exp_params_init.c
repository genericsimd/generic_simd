/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_exp_params_init.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dl_exp_params.h"
#include "mac_connection.h"
#include "mac_serviceflow.h"
#include "arq_ifaces.h"
#include "dl_cbr_gen.h"
#include "memmgmt.h"
#include "mac_arq.h"
#include "debug.h"
#include "util.h"
#include "mac.h"
#include "perf_log.h"
#include "mac_sdu_queue.h"
#include "br_queue.h"
#include "dl_scheduler.h"
#include "mac_hash.h"

#include "ranging_mm.h"
#include "mac_bs_pkm_sim.h"

/** add by zzb for integration, only enabled when integrated */
#ifdef INTEGRATION_TEST
#include "bs_cfg.h"
#include "flog.h"

extern dts_info int_info;
extern int interference_flag; //set by UL to indicate it got Interference info this frame. So this needs to be passed back to phy now.
extern int spectrum_measure_flag;
extern pthread_mutex_t spectrum_measure_flag_lock;
extern pthread_mutex_t int_info_lock;
#endif
/** end by zzb for integration */

#define MAX_VAR_NAME_LENGTH 200
#define MAX_NUM_PARAM 200
#define MAC_ADDR_OFFSET 90000 // for an ss, offset of MAC_ADDR from basic cid

// TODO: G711_RATE should be changed later
// Units of traffic rate are Bytes per sec
#define G711_RATE 10000

char* file_name = "data/test1_param.txt";  // name of the file containing the param
char** var_name;  // array of variable names to be read from a file
int** var_pointer; // array of pointers to the variable that are read from a file
extern int g_sys_procs;


// initialize experiment params

int param_DL_MM_DELAY;    // in ms - delay between two set of mgt_msg
int param_DL_EXP_DURATION;    // total duration of the dl experiment: number of messages sent = duration/delay (very rough estimate)
int param_DL_MM_MAX_SIZE; // maximum size of mgt msg
int param_DL_MM_START_CID;   // lowest CID number for mgt msgs
int param_DL_MM_END_CID;     // highest CID number for mgt msg
int param_DL_MM_NUM_TYPE_PER_CID; // max number of msg msg type sent per CID
int param_DL_MM_MIN_TYPE;  // what is the minimum type number of any mgt msg
int param_DL_ARQ_WINDOW_SIZE; // ARQ Window sixe
int param_DL_ARQ_BLK_SIZE; // ARQ BLK size
int param_DL_ARQ_LOSS_PERCENT; //ARQ PDU loss percentage 
int param_DL_ARQ_CID_PERCENT; // percentage of cids that are arq enabled in both UGS and BE
int param_DL_IS_FRAG_ENABLED; // is FRAGMENTATION enabled (holds for all UGS and BE cids)
int param_DL_IS_PACK_ENABLED; // is PACKING enabled (holds for all UGS and BE cids)
int param_UL_IS_FRAG_ENABLED; // is FRAGMENTATION enabled (holds for all UGS and BE cids)
int param_UL_IS_PACK_ENABLED; // is PACKING enabled (holds for all UGS and BE cids)

int param_NUM_ATTACHED_PROCS; // No. of processors that we want to bind our threads to
int param_BIND_PROC_INDEX; // Starting index of the set of procesors that we want to bind this WMAC instance to. Range: 0 to g_sys_procs-1
int param_DL_MCS;  // Modulation and Coding type for DL and UL
int param_UL_MCS;

int param_FHDC_FLAG;
int param_DL_STC_MATRIX_TYPE;
int param_UL_STC_MATRIX_TYPE;

// FDC chooses the frame duration in millisec as the value corresponding to that index in the frame_duration array
//int FRAME_DURATION_CODE; 
const float frame_duration[9]={0, 2, 2.5, 4, 5, 8, 10, 12.5, 20};

const float bits_per_car[NUM_SUPPORTED_MCS]={1, 1.5, 2, 3, 3, 4, 4.5};

//upper limit of cids that are initialized
int param_MAX_VALID_BASIC_CID;

// valid UGS and BE cids per ss
int param_MAX_VALID_UGS_PER_SS;
int param_MAX_VALID_BE_PER_SS;
int param_TX_UGS_CID_OFFSET;
int param_TX_BE_CID_OFFSET;

int param_DL_MM_NUM_BWR; // Number of Bandwidth Requests generated in every message generator cycle

int param_DL_CBR_PACKET_SIZE;  // default if G711
int param_DL_CBR_PACKET_DELAY;
int param_UL_CBR_PACKET_SIZE;  // default if G711
int param_UL_CBR_PACKET_DELAY;

char param_PERF_ENABLED_BLOCKS[200];
unsigned char param_MY_MAC[6];

int param_RANGING_POWER_THD;

unsigned long long int param_KEY_LIFE_DURATION;
unsigned long long int param_KEY_GRACE_TIME;
unsigned long long int param_KEY_DELTA_ADD;

//#define PHASE_ENABLED
#ifdef PHASE_ENABLED
extern pthread_mutex_t critical_phase_mutex;
extern pthread_cond_t critical_phase_done;
extern int not_critical_phase;
#endif

int free_params()
{
  mac_free(sizeof(char*) * MAX_NUM_PARAM, var_name);
  mac_free(sizeof(int*) * MAX_NUM_PARAM, var_pointer);
  mac_free(sizeof(connection*) * MAX_CIDS, conn_array);

  // Free the SS info list and the service flow structures within each SS info element
  bs_ss_info* temp_ssinfo = NULL;
  serviceflow* temp_sflow = NULL;
  while(ssinfo_list_head!=NULL)
    {
      temp_ssinfo = ssinfo_list_head;
      ssinfo_list_head = ssinfo_list_head->next;
      while (temp_ssinfo->sf_list_head != NULL)
	{
          temp_sflow = temp_ssinfo->sf_list_head;
		  if(temp_sflow->service_class_name != NULL) {free(temp_sflow->service_class_name);}
	  temp_ssinfo->sf_list_head = temp_ssinfo->sf_list_head->next;
	  mac_free(sizeof(serviceflow), temp_sflow);
	}
      mac_free(sizeof(bs_ss_info), temp_ssinfo);
    }

  // Free the connection list and elements contained within
  connection* temp_conn;
  while(connection_list_head!=NULL)
    {
      temp_conn = connection_list_head;
      connection_list_head = connection_list_head->next;
      mac_free(sizeof(arq_state), temp_conn->arq);
      mac_free(sizeof(connection), temp_conn);
    }

  return 0;
}

void dl_exp_params_init_from_file()
{
    param_DL_MM_DELAY=0;    // in ms - delay between two set of mgt_msg
    param_DL_EXP_DURATION=0;    //  in ms: number of messages sent = duration/delay (very rough estimate)
    param_DL_MM_MAX_SIZE=0; // maximum size of mgt msg
    param_DL_MM_START_CID=0;   // lowest CID number for mgt msgs
    param_DL_MM_END_CID=0;     // highest CID number for mgt msg
    param_DL_MM_NUM_TYPE_PER_CID=0; // max number of msg msg type sent per CID
    param_DL_MM_MIN_TYPE=0;  // what is the minimum type number of any mgt msg
	param_MAX_VALID_BASIC_CID = 0;
	param_MAX_VALID_UGS_PER_SS=0;
	param_MAX_VALID_BE_PER_SS=0;
	param_TX_UGS_CID_OFFSET = 0;
	param_TX_BE_CID_OFFSET = 0;
	param_DL_MM_NUM_BWR=0; //max number of BWR msgs sent in a generator cycle
	param_DL_ARQ_WINDOW_SIZE=1024;
	param_DL_ARQ_BLK_SIZE=128;
	param_DL_ARQ_LOSS_PERCENT=1;
    param_DL_ARQ_CID_PERCENT=0;
	param_DL_IS_FRAG_ENABLED = 0;
	param_DL_IS_PACK_ENABLED = 0;
        param_UL_IS_FRAG_ENABLED = 0;
        param_UL_IS_PACK_ENABLED = 0;

	param_NUM_ATTACHED_PROCS = 4;
	param_DL_MCS = 1;
	param_UL_MCS = 1;
    param_FHDC_FLAG = 0;
    param_DL_STC_MATRIX_TYPE = -1; //-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C
    param_UL_STC_MATRIX_TYPE = -1; //-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C

    param_DL_CBR_PACKET_SIZE = 200;  // Default is G711
    param_DL_CBR_PACKET_DELAY = 20;
    param_UL_CBR_PACKET_SIZE = 200;  // Default is G711
    param_UL_CBR_PACKET_DELAY = 20;

	param_BIND_PROC_INDEX = 0;
	memset(param_PERF_ENABLED_BLOCKS,0,200);
	memset(param_MY_MAC,0,6);


  // initialize the mapping arrays

  var_name = (char**) mac_malloc(sizeof(char*) * MAX_NUM_PARAM);
  var_pointer = (int**) mac_malloc(sizeof(int*) * MAX_NUM_PARAM);

  if((!var_name) || (!var_pointer)){
    //printf("1: Error in allocating memory in init_from_file()");
  }

  int v;
  for(v=0; v < MAX_NUM_PARAM; v++){
    var_name[v] = NULL;
    var_pointer[v] = NULL;
  }


  // var_name contains the variable name string
  // var_pointer contains the pointer to the corresponding variable
  // Some of the array indices are missing here because of the parameters
  // which were removed. Ignore.

  var_name[1] = "DL_MM_DELAY";
  var_pointer[1] = &param_DL_MM_DELAY;
  var_name[2] = "DL_EXP_DURATION";
  var_pointer[2] = &param_DL_EXP_DURATION;
  var_name[3] = "DL_MM_MAX_SIZE";
  var_pointer[3] = &param_DL_MM_MAX_SIZE;
  var_name[4] = "DL_MM_START_CID";
  var_pointer[4] = &param_DL_MM_START_CID;
  var_name[5] = "DL_MM_END_CID";
  var_pointer[5] = &param_DL_MM_END_CID;
  var_name[6] = "DL_MM_NUM_TYPE_PER_CID";
  var_pointer[6] = &param_DL_MM_NUM_TYPE_PER_CID;
  var_name[7] = "DL_MM_MIN_TYPE";
  var_pointer[7] = &param_DL_MM_MIN_TYPE;
  var_name[8] = "MAX_VALID_BASIC_CID";
  var_pointer[8] = &param_MAX_VALID_BASIC_CID;
  var_name[10] = "MAX_VALID_UGS_PER_SS";
  var_pointer[10] = &param_MAX_VALID_UGS_PER_SS;
  var_name[11] = "MAX_VALID_BE_PER_SS";
  var_pointer[11] = &param_MAX_VALID_BE_PER_SS;
  var_name[12] = "DL_MM_NUM_BWR";
  var_pointer[12] = &param_DL_MM_NUM_BWR;
  var_name[13] = "DL_ARQ_WINDOW_SIZE";
  var_pointer[13] = &param_DL_ARQ_WINDOW_SIZE;
  var_name[14] = "DL_ARQ_BLK_SIZE";
  var_pointer[14] = &param_DL_ARQ_BLK_SIZE;
  var_name[15] = "DL_ARQ_LOSS_PERCENT";
  var_pointer[15] = &param_DL_ARQ_LOSS_PERCENT;
  var_name[16] = "DL_ARQ_CID_PERCENT";
  var_pointer[16] = &param_DL_ARQ_CID_PERCENT;
  var_name[17] = "DL_IS_FRAG_ENABLED";
  var_pointer[17] = &param_DL_IS_FRAG_ENABLED;
  var_name[18] = "DL_IS_PACK_ENABLED";
  var_pointer[18] = &param_DL_IS_PACK_ENABLED;
  var_name[19] = "NUM_ATTACHED_PROCS";
  var_pointer[19] = &param_NUM_ATTACHED_PROCS;
  var_name[20] = "DL_MCS";
  var_pointer[20] = &param_DL_MCS;
  var_name[21] = "UL_MCS";
  var_pointer[21] = &param_UL_MCS;
  var_name[22] = "DL_CBR_PACKET_SIZE";
  var_pointer[22] = &param_DL_CBR_PACKET_SIZE;
  var_name[23] = "DL_CBR_PACKET_DELAY";
  var_pointer[23] = &param_DL_CBR_PACKET_DELAY;
  var_name[24] = "PERF_ENABLED_BLOCKS";
  var_name[25] = "BIND_PROC_INDEX";
  var_pointer[25] = &param_BIND_PROC_INDEX;
  var_name[26] = "FHDC_FLAG";
  var_pointer[26] = &param_FHDC_FLAG;
  var_name[27] = "DL_STC_MATRIX_TYPE";
  var_pointer[27] = &param_DL_STC_MATRIX_TYPE;
  var_name[28] = "UL_STC_MATRIX_TYPE";
  var_pointer[28] = &param_UL_STC_MATRIX_TYPE;

  var_name[29] = "TX_UGS_CID_OFFSET";
  var_pointer[29] = &param_TX_UGS_CID_OFFSET;
  var_name[30] = "TX_BE_CID_OFFSET";
  var_pointer[30] = &param_TX_BE_CID_OFFSET;
  var_name[31] = "MY_MAC";
  var_pointer[31] = (int *)&param_MY_MAC;

  var_name[32] = "UL_CBR_PACKET_SIZE";
  var_pointer[32] = &param_UL_CBR_PACKET_SIZE;
  var_name[33] = "UL_CBR_PACKET_DELAY";
  var_pointer[33] = &param_UL_CBR_PACKET_DELAY;

  var_name[34] = "UL_IS_FRAG_ENABLED";
  var_pointer[34] = &param_UL_IS_FRAG_ENABLED;
  var_name[35] = "UL_IS_PACK_ENABLED";
  var_pointer[35] = &param_UL_IS_PACK_ENABLED;

  // initialize the variables from the file

  FILE* fp;

  char* name = (char*) mac_malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);

  FLOG_DEBUG("Start reading params from file");
  fp = fopen(file_name, "r");

  if(!fp){
   FLOG_FATAL("Fatal error: can't read the test param. Exiting ...");
   exit(1); 
  }

  int ret=1;
  int j, ii = 0;
  char *search = ".";
  char *mac_id = (char*)malloc(25*sizeof(char));
  char *token;
  while(ret==1){
    if(fscanf(fp, "%s", name) == EOF){
      ret=0;
      break;
    }
    //printf("-- %s \n", name);

    for(j=0; j < MAX_NUM_PARAM; j++){
      if(var_name[j] == NULL){
        continue;
      }
      if(strcmp(var_name[j],name)==0){
	int ret_val=0;

	if(strcmp(name, "PERF_ENABLED_BLOCKS")==0) {
	  ret_val=fscanf(fp, "%s",param_PERF_ENABLED_BLOCKS);
	}
	else if(strcmp(name, "MY_MAC") == 0)
	{
		ret_val = fscanf(fp, "%s", mac_id);
		if(ret_val == EOF)
		{ 
			ret = 0;
			break;
		}
		token = strtok(mac_id, search);
		param_MY_MAC[0]=atoi(token);
		for(ii=1; ii < 6; ii++)
		{
			token= strtok(NULL, search);
		    param_MY_MAC[ii]=atoi(token);
		}

	}
	else 
	{
	  ret_val=fscanf(fp, "%d", var_pointer[j]);
	  //printf("%s : %d\n", var_name[j], *(var_pointer[j]));
	}
	if(ret_val == EOF){
	  ret=0;        
	  break;
	}
      }
    }
  }

  fclose(fp); 
  mac_free(sizeof(char) * MAX_VAR_NAME_LENGTH, name);
  free(mac_id);

  // update primary cid max from basic cid max (one-to-one mapping)
  max_valid_primary_cid = PRIMARY_CID_MIN_VALUE + (param_MAX_VALID_BASIC_CID - BASIC_CID_MIN_VALUE);

  FLOG_DEBUG("Completed reading params from file");

}

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
void dl_exp_params_init ()
{
    char tmp_string[128];

    param_DL_MM_DELAY = 0; // in ms - delay between two set of mgt_msg
    param_DL_EXP_DURATION = 0; //  in ms: number of messages sent = duration/delay (very rough estimate)
    param_DL_MM_MAX_SIZE = 0; // maximum size of mgt msg
    param_DL_MM_START_CID = 0; // lowest CID number for mgt msgs
    param_DL_MM_END_CID = 0; // highest CID number for mgt msg
    param_DL_MM_NUM_TYPE_PER_CID = 0; // max number of msg msg type sent per CID
    param_DL_MM_MIN_TYPE = 0; // what is the minimum type number of any mgt msg
    param_MAX_VALID_BASIC_CID = 0;
    param_MAX_VALID_UGS_PER_SS = 0;
    param_MAX_VALID_BE_PER_SS = 0;
    param_TX_UGS_CID_OFFSET = 0;
    param_TX_BE_CID_OFFSET = 0;
    param_DL_MM_NUM_BWR = 0; //max number of BWR msgs sent in a generator cycle
    param_DL_ARQ_WINDOW_SIZE = 1024;
    param_DL_ARQ_BLK_SIZE = 128;
    param_DL_ARQ_LOSS_PERCENT = 1;
    param_DL_ARQ_CID_PERCENT = 0;
    param_DL_IS_FRAG_ENABLED = 0;
    param_DL_IS_PACK_ENABLED = 0;
    param_UL_IS_FRAG_ENABLED = 0;
    param_UL_IS_PACK_ENABLED = 0;

    param_NUM_ATTACHED_PROCS = 4;
    param_DL_MCS = 1;
    param_UL_MCS = 1;
    param_FHDC_FLAG = 0;
    param_DL_STC_MATRIX_TYPE = -1; //-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C
    param_UL_STC_MATRIX_TYPE = -1; //-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C

    param_DL_CBR_PACKET_SIZE = 200; // Default is G711
    param_DL_CBR_PACKET_DELAY = 20;
    param_UL_CBR_PACKET_SIZE = 200; // Default is G711
    param_UL_CBR_PACKET_DELAY = 20;

    param_BIND_PROC_INDEX = 0;
//    memset (param_PERF_ENABLED_BLOCKS, 0, 200);
    memset (param_MY_MAC, 0, 6);

    // initialize the mapping arrays

    var_name = (char**) mac_malloc (sizeof(char*) * MAX_NUM_PARAM);
    var_pointer = (int**) mac_malloc (sizeof(int*) * MAX_NUM_PARAM);

    if ( ( !var_name ) || ( !var_pointer ))
    {
        //printf("1: Error in allocating memory in init_from_file()");
    }

    int v;
    for (v = 0; v < MAX_NUM_PARAM; v++)
    {
        var_name[v] = NULL;
        var_pointer[v] = NULL;
    }

    // var_name contains the variable name string
    // var_pointer contains the pointer to the corresponding variable
    // Some of the array indices are missing here because of the parameters
    // which were removed. Ignore.

    var_name[1] = "DL_MM_DELAY";
    var_pointer[1] = &param_DL_MM_DELAY;
    var_name[2] = "DL_EXP_DURATION";
    var_pointer[2] = &param_DL_EXP_DURATION;
    var_name[3] = "DL_MM_MAX_SIZE";
    var_pointer[3] = &param_DL_MM_MAX_SIZE;
//    var_name[4] = "DL_MM_START_CID";
//    var_pointer[4] = &param_DL_MM_START_CID;
//    var_name[5] = "DL_MM_END_CID";
//    var_pointer[5] = &param_DL_MM_END_CID;
    var_name[6] = "DL_MM_NUM_TYPE_PER_CID";
    var_pointer[6] = &param_DL_MM_NUM_TYPE_PER_CID;
    var_name[7] = "DL_MM_MIN_TYPE";
    var_pointer[7] = &param_DL_MM_MIN_TYPE;
    var_name[8] = "MAX_VALID_BASIC_CID";
    var_pointer[8] = &param_MAX_VALID_BASIC_CID;
    var_name[10] = "MAX_VALID_UGS_PER_SS";
    var_pointer[10] = &param_MAX_VALID_UGS_PER_SS;
    var_name[11] = "MAX_VALID_BE_PER_SS";
    var_pointer[11] = &param_MAX_VALID_BE_PER_SS;
    var_name[12] = "DL_MM_NUM_BWR";
    var_pointer[12] = &param_DL_MM_NUM_BWR;
    var_name[13] = "DL_ARQ_WINDOW_SIZE";
    var_pointer[13] = &param_DL_ARQ_WINDOW_SIZE;
    var_name[14] = "DL_ARQ_BLK_SIZE";
    var_pointer[14] = &param_DL_ARQ_BLK_SIZE;
    var_name[15] = "DL_ARQ_LOSS_PERCENT";
    var_pointer[15] = &param_DL_ARQ_LOSS_PERCENT;
    var_name[16] = "DL_ARQ_CID_PERCENT";
    var_pointer[16] = &param_DL_ARQ_CID_PERCENT;
    var_name[17] = "DL_IS_FRAG_ENABLED";
    var_pointer[17] = &param_DL_IS_FRAG_ENABLED;
    var_name[18] = "DL_IS_PACK_ENABLED";
    var_pointer[18] = &param_DL_IS_PACK_ENABLED;
    var_name[19] = "NUM_ATTACHED_PROCS";
    var_pointer[19] = &param_NUM_ATTACHED_PROCS;
    var_name[20] = "DL_MCS";
    var_pointer[20] = &param_DL_MCS;
    var_name[21] = "UL_MCS";
    var_pointer[21] = &param_UL_MCS;
    var_name[22] = "DL_CBR_PACKET_SIZE";
    var_pointer[22] = &param_DL_CBR_PACKET_SIZE;
    var_name[23] = "DL_CBR_PACKET_DELAY";
    var_pointer[23] = &param_DL_CBR_PACKET_DELAY;
    var_name[25] = "BIND_PROC_INDEX";
    var_pointer[25] = &param_BIND_PROC_INDEX;
    var_name[26] = "FHDC_FLAG";
    var_pointer[26] = &param_FHDC_FLAG;
    var_name[27] = "DL_STC_MATRIX_TYPE";
    var_pointer[27] = &param_DL_STC_MATRIX_TYPE;
    var_name[28] = "UL_STC_MATRIX_TYPE";
    var_pointer[28] = &param_UL_STC_MATRIX_TYPE;

    var_name[29] = "TX_UGS_CID_OFFSET";
    var_pointer[29] = &param_TX_UGS_CID_OFFSET;
    var_name[30] = "TX_BE_CID_OFFSET";
    var_pointer[30] = &param_TX_BE_CID_OFFSET;
    var_name[31] = "MY_MAC";
    var_pointer[31] = (int *)&param_MY_MAC;

    var_name[32] = "UL_CBR_PACKET_SIZE";
    var_pointer[32] = &param_UL_CBR_PACKET_SIZE;
    var_name[33] = "UL_CBR_PACKET_DELAY";
    var_pointer[33] = &param_UL_CBR_PACKET_DELAY;

    var_name[34] = "UL_IS_FRAG_ENABLED";
    var_pointer[34] = &param_UL_IS_FRAG_ENABLED;
    var_name[35] = "UL_IS_PACK_ENABLED";
    var_pointer[35] = &param_UL_IS_PACK_ENABLED;

    var_name[36] = "MAX_RANGING_POWER_THD";
    var_pointer[36] = &param_RANGING_POWER_THD;

    char* name = (char*) mac_malloc (sizeof(char) * MAX_VAR_NAME_LENGTH);

    TRACE (10, "Start reading params from file");

    int ret = 1;
    int j, ii = 0;
    char *search = ":";
    char *mac_id = (char*) malloc (25 * sizeof(char));
    char *token;
    int tmp_mac;

    for (j = 0; j < MAX_NUM_PARAM; j++)
    {
        if (var_name[j] == NULL)
        {
            continue;
        }

        if (strcmp (var_name[j], "PERF_ENABLED_BLOCKS") == 0)
        {
            ret = get_global_param (var_name[j], param_PERF_ENABLED_BLOCKS);

            if (ret != 0)
            {
                FLOG_WARNING ("get parameters %s error\n", var_name[j]);
                continue;
            }

//            printf("%s\n", param_PERF_ENABLED_BLOCKS);
        }
        else
        {
            if (strcmp (var_name[j], "MY_MAC") == 0)
            {
                ret = get_global_param (var_name[j], mac_id);

                if (ret != 0)
                {
                    FLOG_WARNING ("get parameters %s error\n", var_name[j]);
                    continue;
                }

                token = strtok (mac_id, search);
//                param_MY_MAC[0] = atoi (token);
                sscanf(token, "%x", &(tmp_mac));
                param_MY_MAC[0] = (unsigned char)tmp_mac;

                for (ii = 1; ii < 6; ii++)
                {
                    token = strtok (NULL, search);
//                    param_MY_MAC[ii] = atoi (token);
                    sscanf(token, "%x", &(tmp_mac));
                    param_MY_MAC[0] = (unsigned char)tmp_mac;

//                    printf("MAC Address: %x\n", param_MY_MAC[ii]);
                }
            }
            else
            {
                ret = get_global_param (var_name[j], var_pointer[j]);

                if (ret != 0)
                {
                    FLOG_WARNING ("get parameters %s error\n", var_name[j]);
                    continue;
                }

//                printf("%s : %d\n", var_name[j], *(var_pointer[j]));
            }
        }
    }

    if (param_RANGING_POWER_THD == 0)
    {
        param_RANGING_POWER_THD = POWER_THRESH;
    }

    mac_free (sizeof(char) * MAX_VAR_NAME_LENGTH, name);
    free (mac_id);

    pthread_mutex_lock (&int_info_lock);

    if (get_global_param ("INTERFERENCE_ACTIVE", tmp_string) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }else
    {
        tmp_string[21] = 0;

        for (ii = 20; ii >= 0; ii--)
        {
            int_info.is_active[ii] = atoi (&tmp_string[ii]);
            tmp_string[ii] = 0;
        }
    }

    if (get_global_param ("NUM_DL_INTERFERENCE",
            & ( int_info.num_dl_interference )) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }

    if (get_global_param ("NUM_UL_INTERFERENCE",
            & ( int_info.num_ul_interference )) != 0)
    {
        FLOG_ERROR ("get init interference info error\n");
    }

    if (get_global_param ("KEY_LIFE_DURATION", tmp_string) != 0)
    {
        FLOG_ERROR ("get KEY_LIFE_DURATION error\n");
    }else
    {
        sscanf(tmp_string, "%lld", &param_KEY_LIFE_DURATION);
    }

    if (get_global_param ("KEY_GRACE_TIME", tmp_string) != 0)
    {
        FLOG_ERROR ("get KEY_GRACE_TIME error\n");
    }else
    {
        sscanf(tmp_string, "%lld", &param_KEY_GRACE_TIME);
    }

    if (get_global_param ("KEY_DELTA_ADD", tmp_string) != 0)
    {
        FLOG_ERROR ("get KEY_DELTA_ADD error\n");
    }else
    {
        sscanf(tmp_string, "%lld", &param_KEY_DELTA_ADD);
    }

    pthread_mutex_unlock (&int_info_lock);

    // update primary cid max from basic cid max (one-to-one mapping)
    max_valid_primary_cid = PRIMARY_CID_MIN_VALUE + ( param_MAX_VALID_BASIC_CID
            - BASIC_CID_MIN_VALUE );

    TRACE (10, "Completed reading params from file");

}
#endif
/** end by zzb for integration */

bs_ss_info* dl_bsssinfo_init(u_int64_t mac_addr, int basic_cid, int primary_cid, int uplink_bandwidth, int downlink_bandwidth, int polling_status, serviceflow* sf_list_head, bs_ss_info *next){

 bs_ss_info* info = (bs_ss_info*) mac_malloc(sizeof(bs_ss_info));
 if(!info){
   FLOG_FATAL("1: Error in memory allocation in dl_bsssinfo_init()");
   return(NULL);
 }

  info->mac_addr = mac_addr;
  info->basic_cid = (u_int16_t)basic_cid;
  info->primary_cid =(u_int16_t)primary_cid ;
  info->polling_status = (u_int8_t)polling_status;
  info->sf_list_head = sf_list_head;
  info->next = next;

  return(info);
}

/*
bs_ss_info* dl_bsssinfo_init_simple(u_int64_t mac_addr, int basic_cid, int primary_cid, serviceflow* sf_list_head, bs_ss_info *next){

  return(dl_bsssinfo_init(mac_addr, basic_cid, primary_cid, 10000000, 10000000, 0, QPSK_12, sf_list_head, next));

}
*/

serviceflow* dl_serviceflow_init(int sfid, int trans_id, int cid, char* svc_name, SfStatus qos_type, SchedulingType schedule_type, int br_trans_plc, int traffic_priority, int max_sustained_traffic_rate, int max_traffic_burst, int min_reserved_traffic_rate, int tolerated_jitter, int max_latency, int ug_interval, int up_interval, int cs_param, int sdu_interval, SfDirection dir, serviceflow *next){

 serviceflow* flow = (serviceflow*) mac_malloc(sizeof(serviceflow));
 if(!flow){
   FLOG_FATAL("1: Error in memory allocation in dl_serviceflow_init()");
   return(NULL);
 }

 flow->sfid = sfid;
 flow->trans_id = trans_id;
 flow->cid = cid;
 
 flow->service_class_name = (char*)malloc(strlen(svc_name) + 1);
 if (flow->service_class_name == NULL) 
 {
    FLOG_FATAL("malloc error in dl_serviceflow_init\n");  
 }
 else
 {
	strcpy(flow->service_class_name, svc_name);
 }

 flow->qos_param_set_type = qos_type; 
 flow->schedule_type = schedule_type;
 flow->br_trans_plc = br_trans_plc;
 flow->traffic_priority = traffic_priority;
 flow->max_sustained_traffic_rate = max_sustained_traffic_rate;
 flow->max_traffic_burst = max_traffic_burst;
 flow->min_reserved_traffic_rate = min_reserved_traffic_rate;
 flow->tolerated_jitter = tolerated_jitter;
 flow->max_latency = max_latency;
 flow->unsolicited_grant_interval = ug_interval;
 flow->unsolicited_polling_interval = up_interval;
 flow->cs_specific_parameter = cs_param;
 flow->sdu_inter_arrival_interval = sdu_interval;
 flow->sf_direction = dir;
 flow->next = next;

 return flow;

}

serviceflow* dl_serviceflow_init_simple ( int sfid,
                                          int trans_id,
                                          int cid,
                                          SchedulingType schedule_type,
                                          SfDirection dir,
                                          serviceflow *next )
{
  // check the values and their units (B/kB...)

	// Packet size is in Bytes, Delay is in ms, min_rsvd_rate i sin bytes/sec
    int min_rsvd_rate;
    int packet_delay;

    if (dir == UL)
    {
        min_rsvd_rate = param_UL_CBR_PACKET_SIZE*1000/param_UL_CBR_PACKET_DELAY;
        packet_delay = param_UL_CBR_PACKET_DELAY;
    }
    else
    {
        min_rsvd_rate = param_DL_CBR_PACKET_SIZE*1000/param_DL_CBR_PACKET_DELAY;
        packet_delay = param_DL_CBR_PACKET_DELAY;
    }

    return(dl_serviceflow_init(sfid, trans_id, cid, "default", Active, schedule_type, 0, 1, 100000, 1000, min_rsvd_rate, 100, 10, packet_delay, 10, 10, 10, dir, next));

}

connection* dl_connection_init(int cid, serviceflow* sf, ConnectionType con_type, bs_ss_info* owner, int is_frag_enabled, int is_pack_enabled, int is_arq_enabled, int is_fixed_macsdu_length,
    int is_broadcast_br_enabled,
    int is_multicast_br_enabled,
    int is_piggyback_br_enabled,
    int is_phs_enabled,
    int is_crc_included,
    int is_paging_generated,
    int is_sn_feedback_enabled,
    int is_harq_enabled,
    int is_encrypt_enabled,
    int macsdu_size,
    int macpdu_size,
    int arq_block_size,
    int fsn_size,
    int current_seq_no,
    int modulo,
    connection* next
){
 connection* conn = (connection*) mac_malloc(sizeof(connection));
 if(!conn){
   FLOG_FATAL("1: Error in memory allocation in dl_connection_init()");
   return(NULL);
 }

  conn->cid = cid;
  conn->sf = sf;
  conn->con_type = con_type;
  conn->owner = owner;
  conn->is_frag_enabled = is_frag_enabled;
  conn->is_pack_enabled = is_pack_enabled;
  conn->is_arq_enabled = is_arq_enabled;
  conn->is_fixed_macsdu_length = is_fixed_macsdu_length;
  conn->is_broadcast_br_enabled = is_broadcast_br_enabled;
  conn->is_multicast_br_enabled = is_multicast_br_enabled;
  conn->is_piggyback_br_enabled = is_piggyback_br_enabled;
  conn->is_phs_enabled = is_phs_enabled;
  conn->is_crc_included = is_crc_included;
  conn->is_paging_generated = is_paging_generated;
  conn->is_sn_feedback_enabled = is_sn_feedback_enabled;
  conn->is_harq_enabled = is_harq_enabled;
  conn->is_encrypt_enabled = is_encrypt_enabled;
  conn->macsdu_size = macsdu_size;
  conn->arq_block_size = arq_block_size;
  conn->macpdu_size = macpdu_size;
  conn->fsn_size = fsn_size; // 3 bit or 11 bit;
  conn->current_seq_no = current_seq_no;
  conn->modulo = modulo;
  conn->next = next;

#ifdef BR_ENABLE
  conn->min_reserved_traffic_rate = 0;
#endif
/*
  if (is_encrypt_enabled)
  {
 	FILE *keyfile; int count=0;int retvar=0;int temp;
	keyfile = fopen("keyfile.txt","r");
	
	if (keyfile == NULL) {FLOG_FATAL("Encryption enabled but no keyfile \n");exit(-1);}
	
	

	retvar = 1;int have_read=0;
	while (retvar!=EOF )
	{
		retvar = fscanf(keyfile,"%d",&temp);
		if (temp==cid) {have_read=1;break;}
		else
		{
			char temp1=0;
			while(temp1 !='\n')		
			{
				int retvar2;
				retvar2=fscanf(keyfile,"%c",&temp1); 
				if (retvar2==EOF) {FLOG_FATAL("Encryption key not found for cid %d\n",cid);exit(-1);}
				
			}
		}
	}
	if (have_read==1)
	{
		for (count =0;count < KEYLEN;count++)
		{
			
			retvar = fscanf(keyfile,"%d",&temp);	
			if (retvar==EOF) {FLOG_FATAL("reached end of file while reading the key\n");exit(-1);}
	 		conn->key[count] = (u_char)temp;		
		}
	}
	else
	{
		FLOG_FATAL("Encryption key not found for cid %d\n",cid);exit(-1);
	}
	fclose(keyfile);
	conn->key[KEYLEN]=0;

  }
*/
  // TODO : initialization of arq_state in conn (not all members of the struct are intialized and remaining are hardcoded)

 arq_state* arqs = (arq_state*) mac_malloc(sizeof(arq_state));
 if(!arqs){
   FLOG_FATAL("1: Error in memory allocation in dl_connection_init()");
   return(NULL);
 }
 arqs->arq_window_size = (u_int16_t) param_DL_ARQ_WINDOW_SIZE;
 arqs->is_order_preserved = (u_int8_t) 0;
 arqs->rx_window_start = (u_int32_t) 0;  // TODO: IMPORTANT - should not be used - changes dynamically - should be read using the function given by the arq module

 conn->arq = arqs;

  if(conn->is_arq_enabled){  // CHECK: do arq conn init only for connection for which arq is enabled
    //Call ARQ connection init
    ARQ_downlink_conn_init(cid,
			   param_DL_ARQ_WINDOW_SIZE,
			   conn->arq_block_size,
			   (((float)param_DL_ARQ_LOSS_PERCENT)/100.0),
			   is_frag_enabled ? true : false
			   );

    //TODO: Currently the connection element structures are
    // shared among DL and UL
    //therefore call arq ul init also with the same values as the newly
    // created connection structures
    ARQ_uplink_conn_init(cid, param_DL_ARQ_WINDOW_SIZE, conn->arq_block_size);
  }
 return conn;
}

connection* dl_connection_init_simple(int cid, int is_frag, int is_pack, int is_arq, serviceflow* sf, ConnectionType con_type, bs_ss_info* owner, connection* next){
#ifdef __ENCRYPT__
if (cid >PRIMARY_CID_MAX_VALUE && cid != BROADCAST_CID && cid != FRAGMENTABLE_BROADCAST && cid != PADDING_CID && cid != INIT_RNG_CID)
{
  return(dl_connection_init(cid, sf, con_type, owner, is_frag, is_pack, is_arq, 0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    1024,
    2047,
    param_DL_ARQ_BLK_SIZE,
    11,
    0,
    2048,
	next));
}
else
{
#endif
  return(dl_connection_init(cid, sf, con_type, owner, is_frag, is_pack, is_arq, 0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    1024,
    2047,
    param_DL_ARQ_BLK_SIZE,
    11,
    0,
    2048,next));
#ifdef __ENCRYPT__
}
#endif
}


void dl_exp_params_print(){

  //  printf("\n %d \t %d \t %d \t %d \t %d \t %d \t %d \t %d \t %d \t %d \t %d \t %d \n \n", param_DL_MM_DELAY ,  param_DL_EXP_DURATION,  param_DL_MM_MAX_SIZE, param_DL_MM_START_CID, param_DL_MM_END_CID, param_DL_MM_NUM_TYPE_PER_CID, param_DL_MM_MIN_TYPE, param_MAX_VALID_BASIC_CID, param_MAX_VALID_UGS_PER_SS, param_MAX_VALID_BE_PER_SS, param_DL_MM_NUM_BWR);

  //printf("\n");
  int jj;
  for(jj=0; jj <= 18; jj++){
    if((!var_name) && (!var_pointer)){
      //printf("\n %s \t %d \n", var_name, *var_pointer);
    }
  }
}


void change_processor_affinity(int num_procs)
{
 	cpu_set_t cpuset;

 	assert (num_procs + param_BIND_PROC_INDEX < CPU_SETSIZE);
 	CPU_ZERO(&cpuset);

 	// We now don't care about "which" processors to 
 	// attach -- simply choose the first n 
	// 	for (int idx = 0; idx < max(1, num_procs-1); idx++) { 
	for (int idx = 0; idx < max(1, num_procs); idx++) { 
 		CPU_SET(idx + param_BIND_PROC_INDEX, &cpuset);
 	}

 	sched_setaffinity(0, sizeof(cpuset), &cpuset);
	FLOG_INFO("WMAC attached to processors %d to %d\n",  param_BIND_PROC_INDEX,  param_BIND_PROC_INDEX + num_procs - 1);
 } 

int get_num_system_procs()
{
 	cpu_set_t cpuset;

 	CPU_ZERO(&cpuset);

 	for (int idx = 0; idx < 8; idx++) { 
 		CPU_SET(idx, &cpuset);
 	}

 	sched_setaffinity(0, sizeof(cpuset), &cpuset);

	int sys_procs = 0;
 	CPU_ZERO(&cpuset);
	sched_getaffinity(0, sizeof(cpuset), &cpuset);
	for (int cpu = 0; cpu < 8; cpu++) {
		if (CPU_ISSET(cpu, &cpuset)) {
		  FLOG_INFO( "WMAC has affinity to processor %d\n", cpu);		
			sys_procs++;
		}
	}
	return sys_procs;
}

// initializations for an experiment
void dl_exp_init(){
  perf_log_init();

#ifdef PHASE_ENABLED
  not_critical_phase=0;
  pthread_mutex_init(&(critical_phase_mutex), NULL);
  pthread_cond_init(&(critical_phase_done), NULL);
#endif
  //cs_init();
  mac_sdu_queue_init();
  ht_init();
#ifndef INTEGRATION_TEST
  dl_exp_params_init_from_file();
#else
  dl_exp_params_init();
#endif
//  dl_exp_params_print();
 
/** add by zzb */
#ifndef INTEGRATION_TEST
  g_sys_procs = get_num_system_procs();
  // Use only as many processors as specified in NUM_PROCS_ATTACHED
  // Do this before creating any thread
  change_processor_affinity(param_NUM_ATTACHED_PROCS);
#endif
/** end by zzb */

  // initialize connection array
  dl_conn_array_init();

  //initialize scheduler and simulation parameters
  //TODO:  need to connect once scheduler init is available
  NUM_SS = param_MAX_VALID_BASIC_CID - BASIC_CID_MIN_VALUE + 1;
  max_valid_secondary_cid = 100;
  max_valid_ugs_cid = UGS_CID_MIN_VALUE + NUM_SS*param_MAX_VALID_UGS_PER_SS - 1;

  // temporarily used
  max_valid_ul_ugs_cid = UL_UGS_CID_MIN_VALUE + NUM_SS*param_MAX_VALID_UGS_PER_SS - 1;

  max_valid_be_cid = BE_CID_MIN_VALUE + NUM_SS*param_MAX_VALID_BE_PER_SS - 1;
  max_valid_ertps_cid = 500;
  max_valid_rtps_cid = 600;
  max_valid_nrtps_cid = 700;

  set_current_frame_number(0);
  alloc_dl_scheduler_var();

// initialize BR queue if either BS is transmitting or receiving
#ifndef SS_TX
  br_queue_init();
#else
#ifndef SS_RX
  br_queue_init();
#endif
#endif

#ifdef ARQ_ENABLED
  //initialize ARQ module
  ARQ_init(); 
#endif

  //initialize cbr gen
  cbr_init();

  // initialize the bsinfos, connections, and sflows
  int bcid, ssnum, tcid;
  u_int64_t mac_addr;
  connection* conn;
  //bs_ss_info* ss_info;
  //serviceflow* sflow;

  int total_ugs_cids = param_MAX_VALID_UGS_PER_SS * (param_MAX_VALID_BASIC_CID - BASIC_CID_MIN_VALUE + 1);
  int total_be_cids = param_MAX_VALID_BE_PER_SS * (param_MAX_VALID_BASIC_CID - BASIC_CID_MIN_VALUE + 1);

  // NOTE: lowest (arq_ugs_cids_count) of the UGS cids and lowest (arq_be_cids_count) of the BE cids are arq enabled: arq enabled cids are NOT equal divided across SSes
  int arq_ugs_cids_count = (param_DL_ARQ_CID_PERCENT * total_ugs_cids)/100;
  int arq_be_cids_count = (param_DL_ARQ_CID_PERCENT * total_be_cids)/100;
  BOOL arq_enabled;

    // initialize broadcast connection element
    conn=dl_connection_init_simple(BROADCAST_CID, 0, 0, 0, NULL, CONN_BROADCAST, NULL, NULL);
    add_conection(conn);

    // initialize INIT_RNG_CID connection element
    conn=dl_connection_init_simple(INIT_RNG_CID, 0, 0, 0, NULL, CONN_INIT_RANGING, NULL, NULL);
    add_conection(conn);

  for(bcid=BASIC_CID_MIN_VALUE; bcid<= param_MAX_VALID_BASIC_CID; bcid++) {
    // bs_ss_info* ss_info=(bs_ss_info*) mac_malloc(sizeof(bs_ss_info));
    ssnum = bcid  - BASIC_CID_MIN_VALUE;
    mac_addr = (u_int64_t)(bcid+MAC_ADDR_OFFSET);

    add_basic_con(bcid, ssnum, mac_addr);
	add_primary_con(ssnum + PRIMARY_CID_MIN_VALUE, ssnum, mac_addr);

    // initialize UGS sflows and connection elements
    int min_ugs = (UGS_CID_MIN_VALUE) + (param_MAX_VALID_UGS_PER_SS*ssnum) + param_TX_UGS_CID_OFFSET;
    int max_ugs = (UGS_CID_MIN_VALUE)+ (param_MAX_VALID_UGS_PER_SS*(ssnum+1)) - 1 + param_TX_UGS_CID_OFFSET;

    for(tcid=min_ugs; tcid <= max_ugs; tcid++){
	  arq_enabled = ((arq_ugs_cids_count>0) ? 1 : 0);
	  add_transport_con(tcid, mac_addr, arq_enabled, NULL, SERVICE_UGS, DL);
      arq_ugs_cids_count--;

    }

    // initialize UGS sflows and connection elements
    int min_ul_ugs = (UL_UGS_CID_MIN_VALUE) + (param_MAX_VALID_UGS_PER_SS*ssnum) + param_TX_UGS_CID_OFFSET;
    int max_ul_ugs = (UL_UGS_CID_MIN_VALUE)+ (param_MAX_VALID_UGS_PER_SS*(ssnum+1)) - 1 + param_TX_UGS_CID_OFFSET;

    for(tcid=min_ul_ugs; tcid <= max_ul_ugs; tcid++){
	  arq_enabled = ((arq_ugs_cids_count>0) ? 1 : 0);
	  add_transport_con(tcid, mac_addr, arq_enabled, NULL, SERVICE_UGS, UL);
      arq_ugs_cids_count--;
    }

    // initialize BE sflows and connections elements
    int min_be = (BE_CID_MIN_VALUE) + (param_MAX_VALID_BE_PER_SS*ssnum) + param_TX_BE_CID_OFFSET;
    int max_be = (BE_CID_MIN_VALUE)+ (param_MAX_VALID_BE_PER_SS*(ssnum+1)) - 1 + param_TX_BE_CID_OFFSET;
    //max_valid_be_cid=max_be;
    
    for(tcid=min_be; tcid <= max_be; tcid++){

	  arq_enabled = ((arq_be_cids_count>0) ? 1 : 0);
	  add_transport_con(tcid, mac_addr, arq_enabled, NULL, SERVICE_BE, DL);
      arq_be_cids_count--;

    }
  }

  dl_exp_print();

}

int add_basic_con(int bcid, int ssnum, u_int64_t mac_addr)
{
    //initiailize bsinfos
    bs_ss_info *ss_info = dl_bsssinfo_init(mac_addr, bcid, ssnum + PRIMARY_CID_MIN_VALUE, 10000000, 10000000, 0, NULL, NULL);
	if (ss_info != NULL)
	{
	    add_bs_ss_info(ss_info);

	    // initialize basic connection element
	    connection *conn=dl_connection_init_simple(bcid, 0, 0, 0, NULL, CONN_BASIC, NULL, NULL);
	    add_conection(conn);
	    associate_conn_to_bs(conn, mac_addr);
		return 0;
	}
}

int add_primary_con(int cid, int ssnum, u_int64_t mac_addr)
{
    // initialize primary management connection element
    //TODO: primary management messages are assumed to be not packed or fragmented
    //TODO: IMPORTANT - getting an error if is_arq param is set of 0
    connection *conn=dl_connection_init_simple(cid, 1, 1, 0, NULL, CONN_PRIMARY, NULL, NULL);
    add_conection(conn);

	// TODO: inefficient, unnecessarily traversing the bs_ss_info list. can do conn->owner = ss_info;
	associate_conn_to_bs(conn, mac_addr);

	return 0;
}

#include "mac_sf_api.h"
extern int insert_addr_con(void *p_table,char *p_addr,u_int32_t  con_id);
extern struct hash_table * gp_hash_table;
void add_conn_for_service_flow
(
	u_int64_t				peer_mac,
	struct service_flow 	*flow
)
{
	connection	*conn;
	char		cid_string[32];
	
	assert(flow != NULL);
	if (flow != NULL)
	{
		if (flow->sf_direction == UL)
		{
#ifdef ARQ_ENABLED
			conn = dl_connection_init_simple(flow->sfid, param_UL_IS_FRAG_ENABLED, param_UL_IS_PACK_ENABLED, 1,  NULL, CONN_DATA, NULL, NULL);
#else
			conn = dl_connection_init_simple(flow->sfid, param_UL_IS_FRAG_ENABLED, param_UL_IS_PACK_ENABLED, 0,  NULL, CONN_DATA, NULL, NULL);
#endif
		}
		else
		{
#ifdef ARQ_ENABLED
			conn = dl_connection_init_simple(flow->sfid, param_DL_IS_FRAG_ENABLED, param_DL_IS_PACK_ENABLED, 1,  NULL, CONN_DATA, NULL, NULL); 
#else
			conn = dl_connection_init_simple(flow->sfid, param_DL_IS_FRAG_ENABLED, param_DL_IS_PACK_ENABLED, 0,  NULL, CONN_DATA, NULL, NULL); 
#endif
		}

		if (conn != NULL)
		{
			conn->macsdu_size = flow->min_reserved_traffic_rate;
			
			pthread_rwlock_wrlock(&conn_info_rw_lock);
			associate_conn_to_bs(conn, peer_mac);
			associate_connection_with_svc_flow(conn, flow->sfid); 
			add_conection(conn);
			pthread_rwlock_unlock(&conn_info_rw_lock);

			if (flow->sf_direction == DL)
			{
				add_cid_cbr_vanilla( flow->sfid, 0, param_DL_EXP_DURATION, flow->min_reserved_traffic_rate, flow->sdu_inter_arrival_interval);
			}
		}

#ifdef SS_RX
		if (flow->sf_direction == UL)
		{
			sprintf(cid_string, "%d", param_MAX_VALID_BASIC_CID);
			insert_addr_con(gp_hash_table, cid_string, flow->sfid);
			if (flow->sfid > max_valid_ugs_cid)
			{
				max_valid_ugs_cid = flow->sfid;
			}
		}
		else
		{
			if (flow->sfid > max_valid_ul_ugs_cid)
			{
				max_valid_ul_ugs_cid = flow->sfid;
			}
		}
#endif
	}
}

void add_transport_con_callback(sf_result *result)
{
	assert(result != NULL);

	if (result->cfm_code != CC_SUCCESS)
	{
		FLOG_INFO("%s: peer reject service flow %d addition\n", __FUNCTION__, result->sf_id);
	}
	else
	{
		FLOG_INFO("%s: peer accept service flow %d addition\n", __FUNCTION__, result->sf_id);
	}
}

#ifdef DSX_ENABLE
int add_transport_con(int cid, u_int64_t mac_addr, BOOL arq_enabled, serviceflow *sflow, SchedulingType type, SfDirection dir)
{
	int ret;
	int max_sustained_traffic_rate;
	int min_reserved_traffic_rate;
	int sdu_inter_arrival_interval;

	if (sflow != NULL)
	{
		add_svc_flow_to_bs(sflow, mac_addr);
		add_conn_for_service_flow(mac_addr, sflow);
		ret = 0;
	}
	else
	{
		if (dir == UL)
		{
			max_sustained_traffic_rate = (param_UL_CBR_PACKET_SIZE*1000)/(param_UL_CBR_PACKET_DELAY);
	        min_reserved_traffic_rate = (param_UL_CBR_PACKET_SIZE*1000)/(param_UL_CBR_PACKET_DELAY);
			sdu_inter_arrival_interval = param_UL_CBR_PACKET_DELAY;
		}
		else
		{
			max_sustained_traffic_rate = (param_DL_CBR_PACKET_SIZE*1000)/(param_DL_CBR_PACKET_DELAY);
			min_reserved_traffic_rate = (param_DL_CBR_PACKET_SIZE*1000)/(param_DL_CBR_PACKET_DELAY);
			sdu_inter_arrival_interval = param_DL_CBR_PACKET_DELAY;
		}

		ret = sf_add(mac_addr, cid, "default", type,\
						0, 1, max_sustained_traffic_rate, 1000,\
						min_reserved_traffic_rate, 100, 10, sdu_inter_arrival_interval, \
						10, 10, sdu_inter_arrival_interval, dir, add_transport_con_callback);
		if (ret != 0)
		{
			FLOG_ERROR("%s: error in sf_add\n", __FUNCTION__);
		}
	}
    
    return ret;
}
#else
int add_transport_con(int cid, u_int64_t mac_addr, BOOL arq_enabled, serviceflow *sflow, SchedulingType type, SfDirection dir)
{
    // create sflow
    // sfid = cid
    connection *conn;

    if(sflow == NULL)
    {
        sflow=dl_serviceflow_init_simple(cid, 0, cid,  type, dir, NULL);

        // TODO: G711_RATE should be changed later
        // Units of traffic rate are Bytes per sec

        //TODO: hope there is no problem due to rounding
        //rate in Bytes per sec size in B and delay in millisec

        if (dir == UL)
        {
            sflow->max_sustained_traffic_rate = (param_UL_CBR_PACKET_SIZE*1000)/(param_UL_CBR_PACKET_DELAY);
            sflow->min_reserved_traffic_rate = (param_UL_CBR_PACKET_SIZE*1000)/(param_UL_CBR_PACKET_DELAY);
            sflow->sdu_inter_arrival_interval = param_UL_CBR_PACKET_DELAY;
        }else
        {
            sflow->max_sustained_traffic_rate = (param_DL_CBR_PACKET_SIZE*1000)/(param_DL_CBR_PACKET_DELAY);
            sflow->min_reserved_traffic_rate = (param_DL_CBR_PACKET_SIZE*1000)/(param_DL_CBR_PACKET_DELAY);
            sflow->sdu_inter_arrival_interval = param_DL_CBR_PACKET_DELAY;
        }
    }

    add_svc_flow_to_bs(sflow, mac_addr);
    //create_connection element

    if (dir == UL)
    {
        conn = dl_connection_init_simple(cid, param_UL_IS_FRAG_ENABLED, param_UL_IS_PACK_ENABLED, arq_enabled,  NULL, CONN_DATA, NULL, NULL);
        conn->macsdu_size = param_UL_CBR_PACKET_SIZE;
    }else
    {
        conn = dl_connection_init_simple(cid, param_DL_IS_FRAG_ENABLED, param_DL_IS_PACK_ENABLED, arq_enabled,  NULL, CONN_DATA, NULL, NULL);
        conn->macsdu_size = param_DL_CBR_PACKET_SIZE;
    }

    add_conection(conn);
//        printf("Adding connection with mac: %lld\n", mac_addr);
    associate_conn_to_bs(conn, mac_addr);
    associate_connection_with_svc_flow(conn, cid); // recall: sfid = cid
	//add a cbr to this ugs connection
	//add_cid_cbr(cid, 2000, param_DL_EXP_DURATION, G711);
	if (dir == DL)
	{
		add_cid_cbr_vanilla(cid, 0, param_DL_EXP_DURATION, param_DL_CBR_PACKET_SIZE, param_DL_CBR_PACKET_DELAY);
	}
	
	return 0;
}
#endif

void dl_exp_print(){

  
  //print the bs_ss_info
  //printf(" \n\n BS SS Info ...\n");
  bs_ss_info* bs_ss=ssinfo_list_head;
  while(bs_ss!=NULL) {
    /*printf("BS/SS:%x sf_list_head:%x next:%x\n",
	   bs_ss,
	   bs_ss->sf_list_head,
	   bs_ss->next);*/
    //printf("mac_addr: %d \n", (int)bs_ss->mac_addr);
    //printf("basic_cid: %d primary_cid: %d\n", (int)bs_ss->basic_cid, (int)bs_ss->primary_cid);
    //printf("Service flow for this BS/SS....\n");
    serviceflow* sflow=bs_ss->sf_list_head;
    while(sflow!=NULL) {
      /*printf("\tSF:%x sfid:%d cid:%d schedule_type:%d next:%x\n",
	     sflow,
	     sflow->sfid,
	     sflow->cid,
	     sflow->schedule_type,
	     sflow->next);*/
      sflow=sflow->next; 
	     
    }
    bs_ss=bs_ss->next;
  }

//printf("\n ----------- \n");

  //print connection
  connection* conn=connection_list_head;
  while(conn!=NULL) {
    /*printf("Connection:%x cid:%d serviceflow:%x owner:%x next:%x arq: %d frag: %d pack: %d \n",
	   conn,
	   conn->cid,
	   conn->sf,
	   conn->owner,
	   conn->next, conn->is_arq_enabled, conn->is_frag_enabled, conn->is_pack_enabled);
    */
    int cid1 = conn->cid;
    serviceflow* sflow1=NULL;
    SchedulingType sched_type1=5;
    bs_ss_info* ss_info1=NULL;
    int b_cid1, p_cid1;
	if ((cid1 == BROADCAST_CID)||(cid1 == INIT_RNG_CID))
	{
		//printf("Broadcast CID\n");
	}
	else
	{
    get_service_flow(cid1, &sflow1);
    //printf(" For cid:%d sflow:%x\n", cid1, sflow1);
    get_scheduling_type(cid1, &sched_type1);
    //printf(" For cid:%d sched_type:%d\n", cid1, sched_type1);
    get_bs_ss_info(cid1, &ss_info1);
    //printf(" For cid:%d ss_info:%x\n", cid1, ss_info1);
    get_basic_cid(cid1, &b_cid1);
    //printf(" For cid:%d basic_cid:%d\n", cid1, b_cid1);
    get_primary_cid(cid1, &p_cid1);
    //printf(" For cid:%d primary_cid:%d\n", cid1, p_cid1);
    //printf("\n\n");
	}

    conn=conn->next;
  }


}


int dl_conn_array_init(){
  conn_array = (connection**) mac_malloc(sizeof(connection*) * MAX_CIDS);
  if(!conn_array){
    FLOG_FATAL("1: error allocating memory in dl_conn_array_init()");
    return(-1);
  }
  int i;
  for(i=0; i < MAX_CIDS; i++){
     conn_array[i] = NULL;
  }
  return(0);
}
