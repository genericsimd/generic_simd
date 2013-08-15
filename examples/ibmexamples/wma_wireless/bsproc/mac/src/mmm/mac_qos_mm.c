/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_qos_mm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Mar.2011       Created                                 Parul Gupta
   12-April.2012    Fix bugs and implement more				Xianwei.Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

//#include "ul_mgt_msg_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mac_qos_mm.h"
#include "debug.h"

int init_dsd_req(u_int32_t sfid, u_int16_t trans_id, dsd_req_msg* dsd_req) //in, out
{
	
	dsd_req->mgm_msg_type = DSD_REQ; //dsd_req is of type 17
	dsd_req->trans_id = trans_id;
	dsd_req->sfid = sfid;
	
	return 0;	
} 

int build_dsd_req (dsd_req_msg* dsd_req, unsigned char* payload, int* length)//in,out,out
{

    unsigned char* cur_p;


    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsd_req->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsd_req->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsd_req->trans_id  & 0x00ff);
    ( *length )++;
 

    //SFID

    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_req->sfid >> 24) & 0x00ff);
   (*length)++;
   

    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_req->sfid >> 16) & 0x00ff);
   (*length)++;


    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_req->sfid >> 8) & 0x00ff);
   (*length)++;


    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_req->sfid) & 0x00ff);
   (*length)++;


    return 0;
}

int parse_dsd_req (unsigned char *payload, int mm_len, dsd_req_msg *dsd_req)//in,in,out
{
    int cur_p = 0;

	if (payload[cur_p] != DSD_REQ)
	{
		FLOG_ERROR("Error in parse_dsd_req: MM_type in Input is not DSD_REQ");
		return -1;
	}
	dsd_req->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsd_req->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      (((u_int16_t)payload[cur_p+1]));
    cur_p += 2;



    dsd_req->sfid  = (((u_int32_t)payload[cur_p])<<24)+(((u_int32_t)payload[cur_p+1])<<16)+(((u_int32_t)payload[cur_p+2]) << 8) +(((u_int32_t)payload[cur_p+3])) ; 
   
    cur_p+=4;

    if(mm_len != cur_p)
    {
        FLOG_ERROR("parse nums error!");
        return -1;
    }
    return 0;
}



int init_dsd_rsp(dsd_req_msg* dsd_req, dsd_rsp_msg* dsd_rsp)//in,out
{
    dsd_rsp->mgm_msg_type = DSD_RSP;//dsd response is 18
    dsd_rsp->trans_id = dsd_req->trans_id;

    //cfm_code needs to be added after init
    dsd_rsp->cfm_code = CC_SUCCESS;//initiate 18
	dsd_rsp->sfid = dsd_req->sfid;

    return 0;
}

int build_dsd_rsp (dsd_rsp_msg* dsd_rsp, unsigned char* payload, int* length)//in,out,out
{

    unsigned char* cur_p;


    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsd_rsp->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsd_rsp->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsd_rsp->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsd_rsp->cfm_code);
    ( *length )++;

	//SFID
    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_rsp->sfid >> 24) & 0x00ff);
   (*length)++;
   

    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_rsp->sfid >> 16) & 0x00ff);
   (*length)++;


    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_rsp->sfid >> 8) & 0x00ff);
   (*length)++;


    cur_p++;
   (*cur_p) = (unsigned char) ((dsd_rsp->sfid) & 0x00ff);
   (*length)++;


    return 0;
}


int parse_dsd_rsp (unsigned char *payload, int mm_len, dsd_rsp_msg *dsd_rsp)//in,in,out
{
    int cur_p = 0;

	if (payload[cur_p] != DSD_RSP)
	{
		FLOG_ERROR("Error in parse_dsd_rsp: MM_type in Input is not DSD_RSP");
		return -1;
	}
	dsd_rsp->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsd_rsp->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      (((u_int16_t)payload[cur_p+1]));
    cur_p += 2;

    dsd_rsp->cfm_code = payload[cur_p];
   
    cur_p++;	

    dsd_rsp->sfid  = (((u_int32_t)payload[cur_p])<<24)+(((u_int32_t)payload[cur_p+1])<<16)+(((u_int32_t)payload[cur_p+2]) << 8) +(((u_int32_t)payload[cur_p+3])) ; 
   
    cur_p+=4;

    if(mm_len != cur_p)
    {
        FLOG_ERROR("parse nums error!");
        return -1;
    }
    return 0;
}

int dsa_req_to_sf(dsa_req_msg* dsa_req, serviceflow *sf) //in, out
{
	memset(sf, 0, sizeof(serviceflow));
	assert(sf != NULL);
	if (dsa_req->mgm_msg_type != DSA_REQ)
	{
		FLOG_ERROR("Error in dsa_req_to_sf: MM type is not 11\n");
		return -1;
	}
	sf->trans_id = dsa_req->trans_id;
	
	if(dsa_req->tlv_sf == NULL)
	{
		FLOG_ERROR("Error in dsa_req_to_sf: No SF TLV's found in dsa_req\n");
		return -1;
	}
	if (dsa_req->tlv_sf->type == 145)
		sf->sf_direction = UL;
	else if (dsa_req->tlv_sf->type == 146)
		sf->sf_direction = DL;
	else
	{
		FLOG_ERROR("Error in dsa_req_to_sf: Invalid SF direction\n");
		return -1;
	}
	struct tlv_info *curp = dsa_req->tlv_sf->encapTLV;
	while (curp != NULL)
	{
		// TODO: add error checks for the length fields later
		switch(curp->type)
		{
			case 1: //sfid
				sf->sfid = *((int*)(curp->value));
				break;
			case 2: //cid
				sf->cid = *((u_int16_t*)(curp->value));
				break;
			case 3: // Service class name
				sf->service_class_name = (char*)malloc(curp->length);
				memcpy(sf->service_class_name, (u_char*)curp->value, curp->length);
				break;
			case 4:
				// Not supported yet.
				break;
			case 5: 
				sf->qos_param_set_type = *((u_char*)(curp->value));
				break;
			case 6:
				sf->traffic_priority = *((u_char*)(curp->value));
				break;
			case 7:
				sf->max_sustained_traffic_rate= *((int*)(curp->value));
				break;			
			case 8:
				sf->max_traffic_burst = *((int*)(curp->value));
				break;
			case 9:
				sf->min_reserved_traffic_rate = *((int*)(curp->value));
				break;	
			case 11:
				sf->schedule_type= *((u_char*)(curp->value));
				break;			
			case 12:
				sf->br_trans_plc = *((u_char*)(curp->value));
				break;
			case 13:
				sf->tolerated_jitter = *((int*)(curp->value));
				break;				
			case 14:
				sf->max_latency = *((int*)(curp->value));
				break;	
			case 20:
				sf->unsolicited_grant_interval = *((u_int16_t*)(curp->value));
				break;
			case 21:
				sf->unsolicited_polling_interval = *((u_int16_t*)(curp->value));
				break;
			case 26:
				sf->sdu_inter_arrival_interval = *((u_int16_t*)(curp->value));
				break;
			default:
				FLOG_ERROR("In dsa_req_to_sf: Unsupported TLV\n");
		}
		curp = curp->next;
	}
	return 0;
}

int dsa_rsp_to_sf(dsa_rsp_msg* dsa_rsp, serviceflow *sf) //in, out
{
	assert(dsa_rsp != NULL);

	if (sf == NULL)
	{
		return -1;
	}
		
	if (dsa_rsp->mgm_msg_type != DSA_RSP)
	{
		FLOG_ERROR("Error in dsa_rsp_to_sf: MM type is not 11\n");
		return -1;
	}
	sf->trans_id = dsa_rsp->trans_id;
	
	if(dsa_rsp->tlv_sf == NULL)
	{
		FLOG_ERROR("Error in dsa_rsp_to_sf: No SF TLV's found in dsa_rsp\n");
		return -1;
	}
	if (dsa_rsp->tlv_sf->type == 145)
		sf->sf_direction = UL;
	else if (dsa_rsp->tlv_sf->type == 146)
		sf->sf_direction = DL;
	else
	{
		FLOG_ERROR("Error in dsa_rsp_to_sf: Invalid SF direction\n");
		return -1;
	}
	struct tlv_info *curp = dsa_rsp->tlv_sf->encapTLV;
	while (curp != NULL)
	{
		// TODO: add error checks for the length fields later
		switch(curp->type)
		{
			case 1: //sfid
				sf->sfid = *((int*)(curp->value));
				break;
			case 2: //cid
				sf->cid = *((u_int16_t*)(curp->value));
				break;
			case 3: // Service class name
                                if (sf->service_class_name != NULL)
                                {
                                        free(sf->service_class_name);
                                }
				sf->service_class_name = (char*)malloc(curp->length);
				memcpy(sf->service_class_name, (u_char*)curp->value, curp->length);
				break;
			case 4:
				// Not supported yet.
				break;
			case 5: 
				sf->qos_param_set_type = *((u_char*)(curp->value));
				break;
			case 6:
				sf->traffic_priority = *((u_char*)(curp->value));
				break;
			case 7:
				sf->max_sustained_traffic_rate= *((int*)(curp->value));
				break;			
			case 8:
				sf->max_traffic_burst = *((int*)(curp->value));
				break;
			case 9:
				sf->min_reserved_traffic_rate = *((int*)(curp->value));
				break;	
			case 11:
				sf->schedule_type= *((u_char*)(curp->value));
				break;			
			case 12:
				sf->br_trans_plc = *((u_char*)(curp->value));
				break;
			case 13:
				sf->tolerated_jitter = *((int*)(curp->value));
				break;				
			case 14:
				sf->max_latency = *((int*)(curp->value));
				break;	
			case 20:
				sf->unsolicited_grant_interval = *((u_int16_t*)(curp->value));
				break;
			case 21:
				sf->unsolicited_polling_interval = *((u_int16_t*)(curp->value));
				break;
			case 26:
				sf->sdu_inter_arrival_interval = *((u_int16_t*)(curp->value));
				break;
			default:
				FLOG_ERROR("In dsa_rsp_to_sf: Unsupported TLV\n");
		}
		curp = curp->next;
	}
	return 0;
}

int dsc_req_to_sf(dsc_req_msg* dsc_req, serviceflow *sf) //in, out
{
	memset(sf, 0, sizeof(serviceflow));
	assert(sf != NULL);
	if (dsc_req->mgm_msg_type != DSC_REQ)
	{
		FLOG_ERROR("Error in dsc_req_to_sf: MM type is not 11\n");
		return -1;
	}
	sf->trans_id = dsc_req->trans_id;
	
	if(dsc_req->tlv_sf == NULL)
	{
		FLOG_ERROR("Error in dsc_req_to_sf: No SF TLV's found in dsc_req\n");
		return -1;
	}
	if (dsc_req->tlv_sf->type == 145)
		sf->sf_direction = UL;
	else if (dsc_req->tlv_sf->type == 146)
		sf->sf_direction = DL;
	else
	{
		FLOG_ERROR("Error in dsc_req_to_sf: Invalid SF direction\n");
		return -1;
	}
	struct tlv_info *curp = dsc_req->tlv_sf->encapTLV;
	while (curp != NULL)
	{
		// TODO: add error checks for the length fields later
		switch(curp->type)
		{
			case 1: //sfid
				sf->sfid = *((int*)(curp->value));
				break;
			case 2: //cid
				sf->cid = *((u_int16_t*)(curp->value));
				break;
			case 3: // Service class name
				sf->service_class_name = (char*)malloc(curp->length);
				memcpy(sf->service_class_name, (u_char*)curp->value, curp->length);
				break;
			case 4:
				// Not supported yet.
				break;
			case 5: 
				sf->qos_param_set_type = *((u_char*)(curp->value));
				break;
			case 6:
				sf->traffic_priority = *((u_char*)(curp->value));
				break;
			case 7:
				sf->max_sustained_traffic_rate= *((int*)(curp->value));
				break;			
			case 8:
				sf->max_traffic_burst = *((int*)(curp->value));
				break;
			case 9:
				sf->min_reserved_traffic_rate = *((int*)(curp->value));
				break;	
			case 11:
				sf->schedule_type= *((u_char*)(curp->value));
				break;			
			case 12:
				sf->br_trans_plc = *((u_char*)(curp->value));
				break;
			case 13:
				sf->tolerated_jitter = *((int*)(curp->value));
				break;				
			case 14:
				sf->max_latency = *((int*)(curp->value));
				break;
			case 20:
				sf->unsolicited_grant_interval = *((u_int16_t*)(curp->value));
				break;
			case 21:
				sf->unsolicited_polling_interval = *((u_int16_t*)(curp->value));
				break;
			case 26:
				sf->sdu_inter_arrival_interval = *((u_int16_t*)(curp->value));
				break;
			default:
				FLOG_ERROR("In dsc_req_to_sf: Unsupported TLV\n");
		}
		curp = curp->next;
	}
	return 0;
}

int dsc_rsp_to_sf(dsc_rsp_msg* dsc_rsp, serviceflow *sf) //in, out
{
	assert(dsc_rsp != NULL);

	if (sf == NULL)
	{
		return -1;
	}
		
	if (dsc_rsp->mgm_msg_type != DSC_RSP)
	{
		FLOG_ERROR("Error in dsc_rsp_to_sf: MM type is not 15\n");
		return -1;
	}
	sf->trans_id = dsc_rsp->trans_id;
	
	if(dsc_rsp->tlv_sf == NULL)
	{
		FLOG_ERROR("Error in dsc_rsp_to_sf: No SF TLV's found in dsc_rsp\n");
		return -1;
	}
	if (dsc_rsp->tlv_sf->type == 145)
		sf->sf_direction = UL;
	else if (dsc_rsp->tlv_sf->type == 146)
		sf->sf_direction = DL;
	else
	{
		FLOG_ERROR("Error in dsc_rsp_to_sf: Invalid SF direction\n");
		return -1;
	}
	struct tlv_info *curp = dsc_rsp->tlv_sf->encapTLV;
	while (curp != NULL)
	{
		// TODO: add error checks for the length fields later
		switch(curp->type)
		{
			case 1: //sfid
				sf->sfid = *((int*)(curp->value));
				break;
			case 2: //cid
				sf->cid = *((u_int16_t*)(curp->value));
				break;
			case 3: // Service class name
				if (sf->service_class_name != NULL)
				{
					free(sf->service_class_name);
				}
				sf->service_class_name = (char*)malloc(curp->length);
				memcpy(sf->service_class_name, (u_char*)curp->value, curp->length);
				break;
			case 4:
				// Not supported yet.
				break;
			case 5: 
				sf->qos_param_set_type = *((u_char*)(curp->value));
				break;
			case 6:
				sf->traffic_priority = *((u_char*)(curp->value));
				break;
			case 7:
				sf->max_sustained_traffic_rate= *((int*)(curp->value));
				break;			
			case 8:
				sf->max_traffic_burst = *((int*)(curp->value));
				break;
			case 9:
				sf->min_reserved_traffic_rate = *((int*)(curp->value));
				break;	
			case 11:
				sf->schedule_type= *((u_char*)(curp->value));
				break;			
			case 12:
				sf->br_trans_plc = *((u_char*)(curp->value));
				break;
			case 13:
				sf->tolerated_jitter = *((int*)(curp->value));
				break;				
			case 14:
				sf->max_latency = *((int*)(curp->value));
				break;	
			case 20:
				sf->unsolicited_grant_interval = *((u_int16_t*)(curp->value));
				break;
			case 21:
				sf->unsolicited_polling_interval = *((u_int16_t*)(curp->value));
				break;
			case 26:
				sf->sdu_inter_arrival_interval = *((u_int16_t*)(curp->value));
				break;
			default:
				FLOG_ERROR("In dsc_rsp_to_sf: Unsupported TLV\n");
		}
		curp = curp->next;
	}
	return 0;
}


// Will be used at the sender side which initiates DSA-REQ
int init_dsa_req(serviceflow *sf, dsa_req_msg* dsa_req)//in,out
{
    int length;

    dsa_req->mgm_msg_type = DSA_REQ;//create msg dsaREQ
    dsa_req->trans_id = sf->trans_id;

    dsa_req->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsa_req->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));


	if (sf->sf_direction == UL)
	{
		dsa_req->tlv_sf->type = 145;
	}
	else
	{
		dsa_req->tlv_sf->type = 146;
	}

    length = 0;

    struct tlv_info *curp;
    //sfid
    curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));//redefine malloc containing detect
//    if(!flow){
//       printf("1: Error in memory allocation in dl_serviceflow_init()");
//       return(-1);
//    }
 //   dsa_req->tlv_sf->encapTLV = curp;

    dsa_req->tlv_sf->encapTLV = curp;

    //only if bs-init, SFID and CID fields exist.
	#ifndef SS_TX
    curp->type = 1;//sfid
    curp->length = 4;

    int *tmp1 = (int*)malloc(curp->length);
    *tmp1 = sf->sfid;
    curp->value  = tmp1;
	//printf("In init_dsa_req, sf->sfid: %d, *tmp1: %d, curp->value: %d\n",sf->sfid, *tmp1, *((int*)curp->value) );


    length = length + curp->length+ 2;

    //cid
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 2;//CID
    curp->length = 2;


    u_int16_t *tmp2 = (u_int16_t*)malloc(curp->length);
    *tmp2 = sf->cid;
    curp->value = tmp2;

    length = length + curp->length+ 2;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
	#endif

    //service class name
    curp->type = 3;//service class name

    if(strlen(sf->service_class_name)+1 <=127)
    {
      curp->length = strlen(sf->service_class_name)+1;

      length = length + curp->length+ 2;
    }
    else if(strlen(sf->service_class_name)+1 == 128)
    {
      curp->length = 128;//note when building
      length = length + curp->length+ 3;
    }
    else
        FLOG_ERROR("service class name must be less than 128!");


    curp->value = (unsigned char*)malloc(strlen(sf->service_class_name)+1);
    memcpy(curp->value, sf->service_class_name, curp->length);
//    temp = curp->length;
//    while(temp>0)
//    {
//        * curp->value++ = * sf->service_class_name++;
//        temp--;
//    }



    //QoS parameters set type
    //pls look table 600 on page 1347
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 5;//provisioned, admitted or active set


    curp->length = 1;

    unsigned char *tmp3 = (unsigned char*)malloc(curp->length);
    *tmp3 = sf->qos_param_set_type;
    curp->value = tmp3;


    length = length + curp->length+ 2;

   //6,7,8,9, 11,12,13,14are QoS parameters
   //traffic priority parameters

    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 6;//traffic priority parameters

    curp->length = 1;
    unsigned char * tmp4 = (unsigned char*)malloc(curp->length);
    *tmp4 = sf->traffic_priority;
    curp->value = tmp4;


    length = length + curp->length+ 2;

    //maximum sustained rate
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 7;//maximum sustained rate


    curp->length = 4;
    int * tmp5 = (int*)malloc(curp->length);
    *tmp5 = sf->max_sustained_traffic_rate;
    curp->value = tmp5;


    length = length + curp->length+ 2;

    //maximum burst size

    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 8;//maximum burst size


    curp->length = 4;

    int * tmp6 = (int*)malloc(curp->length);
    *tmp6 = sf->max_traffic_burst;
    curp->value = tmp6;


    length = length + curp->length+ 2;

    //minimum reserved rate
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 9;//minimum reserved rate


    curp->length = 4;

    int* tmp7 = (int*)malloc(curp->length);
    *tmp7 = sf->min_reserved_traffic_rate;
    curp->value = tmp7;



    length = length + curp->length+ 2;

    //UL grant scheduling type
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 11;//UL grant scheduling type


    curp->length = 1;
    unsigned char* tmp8 = (unsigned char*)malloc(curp->length);
    *tmp8 = sf->schedule_type;
    curp->value = tmp8;



    length = length + curp->length+ 2;

    //request/transmission policy
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 12;//request/transmission policy


    curp->length = 1;
    unsigned char* tmp9 = (unsigned char*)malloc(curp->length);
    *tmp9 = sf->br_trans_plc;
    curp->value = tmp9;



    length = length + curp->length+ 2;

    //tolerated jitter
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 13;//tolerated jitter


    curp->length = 4;
    int* tmp10 = (int*)malloc(curp->length);
    *tmp10 = sf->tolerated_jitter;
    curp->value = tmp10;

    length = length + curp->length+ 2;

    //maximum latency
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 14;//maximum latency

    curp->length = 4;
    int* tmp11 = (int*)malloc(curp->length);
    *tmp11 = sf->max_latency;
    curp->value = tmp11;
	length = length + curp->length+ 2;

	//Unsolicited Grant Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 20;

    curp->length = 2;
    short* tmp12 = (short *)malloc(curp->length);
    *tmp12 = sf->unsolicited_grant_interval;
    curp->value = tmp12;
	length = length + curp->length + 2;
	
	//Unsolicited Polling Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 21;

    curp->length = 2;
    short* tmp13 = (short *)malloc(curp->length);
    *tmp13 = sf->unsolicited_polling_interval;
    curp->value = tmp13;
	length = length + curp->length + 2;
	
	//SDU Inter-Arrival Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 26;

    curp->length = 2;
    short* tmp14 = (short *)malloc(curp->length);
    *tmp14 = sf->sdu_inter_arrival_interval;
    curp->value = tmp14;
	length = length + curp->length + 2;

    curp->next =NULL;
    dsa_req->tlv_sf->length = length;

    return 0;
}

// Will be used at the sender side which initiates DSC-REQ
int init_dsc_req(serviceflow *sf, dsc_req_msg* dsc_req)//in,out
{
    int length;

    dsc_req->mgm_msg_type = DSC_REQ;//create msg dsaREQ
    dsc_req->trans_id = sf->trans_id;
	dsc_req->sf_id = sf->sfid;

    dsc_req->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsc_req->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));


	if (sf->sf_direction == UL)
	{
		dsc_req->tlv_sf->type = 145;
	}
	else
	{
		dsc_req->tlv_sf->type = 146;
	}

    length = 0;

    struct tlv_info *curp;
    //sfid
    curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));//redefine malloc containing detect

    dsc_req->tlv_sf->encapTLV = curp;

    curp->type = 1;//sfid
    curp->length = 4;

    int *tmp1 = (int*)malloc(curp->length);
    *tmp1 = sf->sfid;
    curp->value  = tmp1;
	//printf("In init_dsc_req, sf->sfid: %d, *tmp1: %d, curp->value: %d\n",sf->sfid, *tmp1, *((int*)curp->value) );

    length = length + curp->length+ 2;

    //cid
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 2;//CID
    curp->length = 2;


    u_int16_t *tmp2 = (u_int16_t*)malloc(curp->length);
    *tmp2 = sf->cid;
    curp->value = tmp2;

    length = length + curp->length+ 2;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;

    //service class name
    curp->type = 3;//service class name

    if(strlen(sf->service_class_name)+1 <=127)
    {
      curp->length = strlen(sf->service_class_name)+1;

      length = length + curp->length+ 2;
    }
    else if(strlen(sf->service_class_name)+1 == 128)
    {
      curp->length = 128;//note when building
      length = length + curp->length+ 3;
    }
    else
        FLOG_ERROR("service class name must be less than 128!");


    curp->value = (unsigned char*)malloc(strlen(sf->service_class_name)+1);
    memcpy(curp->value, sf->service_class_name, curp->length);

    //QoS parameters set type
    //pls look table 600 on page 1347
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 5;//provisioned, admitted or active set


    curp->length = 1;

    unsigned char *tmp3 = (unsigned char*)malloc(curp->length);
    *tmp3 = sf->qos_param_set_type;
    curp->value = tmp3;


    length = length + curp->length+ 2;

   //6,7,8,9, 11,12,13,14are QoS parameters
   //traffic priority parameters

    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 6;//traffic priority parameters

    curp->length = 1;
    unsigned char * tmp4 = (unsigned char*)malloc(curp->length);
    *tmp4 = sf->traffic_priority;
    curp->value = tmp4;


    length = length + curp->length+ 2;

    //maximum sustained rate
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 7;//maximum sustained rate


    curp->length = 4;
    int * tmp5 = (int*)malloc(curp->length);
    *tmp5 = sf->max_sustained_traffic_rate;
    curp->value = tmp5;


    length = length + curp->length+ 2;

    //maximum burst size

    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 8;//maximum burst size


    curp->length = 4;

    int * tmp6 = (int*)malloc(curp->length);
    *tmp6 = sf->max_traffic_burst;
    curp->value = tmp6;


    length = length + curp->length+ 2;

    //minimum reserved rate
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 9;//minimum reserved rate


    curp->length = 4;

    int* tmp7 = (int*)malloc(curp->length);
    *tmp7 = sf->min_reserved_traffic_rate;
    curp->value = tmp7;



    length = length + curp->length+ 2;

    //UL grant scheduling type
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 11;//UL grant scheduling type


    curp->length = 1;
    unsigned char* tmp8 = (unsigned char*)malloc(curp->length);
    *tmp8 = sf->schedule_type;
    curp->value = tmp8;



    length = length + curp->length+ 2;

    //request/transmission policy
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 12;//request/transmission policy

    curp->length = 1;
    unsigned char* tmp9 = (unsigned char*)malloc(curp->length);
    *tmp9 = sf->br_trans_plc;
    curp->value = tmp9;

    length = length + curp->length+ 2;

    //tolerated jitter
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 13;//tolerated jitter


    curp->length = 4;
    int* tmp10 = (int*)malloc(curp->length);
    *tmp10 = sf->tolerated_jitter;
    curp->value = tmp10;

    length = length + curp->length+ 2;

    //maximum latency
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 14;//maximum latency

    curp->length = 4;
    int* tmp11 = (int*)malloc(curp->length);
    *tmp11 = sf->max_latency;
    curp->value = tmp11;
	
    length = length + curp->length+ 2;

	//Unsolicited Grant Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 20;

    curp->length = 2;
    short* tmp12 = (short *)malloc(curp->length);
    *tmp12 = sf->unsolicited_grant_interval;
    curp->value = tmp12;
	length = length + curp->length + 2;
	
	//Unsolicited Polling Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 21;

    curp->length = 2;
    short* tmp13 = (short *)malloc(curp->length);
    *tmp13 = sf->unsolicited_polling_interval;
    curp->value = tmp13;
	length = length + curp->length + 2;
	
	//SDU Inter-Arrival Interval parameter
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 26;

    curp->length = 2;
    short* tmp14 = (short *)malloc(curp->length);
    *tmp14 = sf->sdu_inter_arrival_interval;
    curp->value = tmp14;
	length = length + curp->length + 2;

	curp->next =NULL;
    dsc_req->tlv_sf->length = length;

    return 0;
}

int build_dsa_req (dsa_req_msg* dsa_req, unsigned char* payload, int* length)//in,out,out
{
    unsigned char* cur_p;
    struct tlv_info *curp;

    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsa_req->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsa_req->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsa_req->trans_id  & 0x00ff);
    ( *length )++;

    // TLV field
    cur_p++; //type
    ( *cur_p ) = ( unsigned char ) (dsa_req->tlv_sf->type);
    ( *length )++;

    cur_p++; //length
    if(dsa_req->tlv_sf->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(dsa_req->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_req->tlv_sf->length <= 0xff)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length);
        (*length)++;

    }
    else if(dsa_req->tlv_sf->length <= 0xffff)
    {
        (*cur_p) = 0x82;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_req->tlv_sf->length <= 0xffffff)
    {
        (*cur_p) = 0x83;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length);
        (*length)++;
    }
    else
    {
        (*cur_p) = 0x84;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 24);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_req->tlv_sf->length);
        (*length)++;
    }

    curp = dsa_req->tlv_sf->encapTLV;

// TODO: Need to change this parser. Temp fix now
// SFID and CID TLVs are included only in BS-initiated DSA-REQ. Hence, in 
// build_dsa_req, we'll have SFID and CID TLV's only at the BS_TX
#ifndef SS_TX 
    //sfid
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value));
    ( *length )++;

    curp = curp->next;
    //CID
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value));
    ( *length )++;

    curp = curp->next;
#endif

    //service class name
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    if(curp->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(curp->length);
        (*length)++;
    }
    else if(curp->length == 0x80)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(curp->length);
        (*length)++;

    }
    else
        FLOG_ERROR("service class name must be less than 128!");

    cur_p++;
    memcpy(cur_p, curp->value, curp->length);
    *length = *length + curp->length;

    cur_p = cur_p + curp->length - 1;

    curp=curp->next;

    //QoS set parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;

    //traffic priority parameters
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum sustained rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum burst size
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //minimum reserved rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //UL grant scheduling type
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //request/transmission policy
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //tolerated jitter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum latency
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
    //Unsolicited Grant Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
	//Unsolicited Polling Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
	//SDU Inter-Arrival Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    //build dsa_req end
    curp = curp->next;
    if(curp!=NULL)
    {
        FLOG_WARNING("In build_dsa_req, discarding extra TLVs\n");
        return -1;
    }

    return 0;
}

int build_dsc_req (dsc_req_msg* dsc_req, unsigned char* payload, int* length)//in,out,out
{
    unsigned char* cur_p;
    struct tlv_info *curp;


    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsc_req->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsc_req->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsc_req->trans_id  & 0x00ff);
    ( *length )++;

    // TLV field
    cur_p++; //type
    ( *cur_p ) = ( unsigned char ) (dsc_req->tlv_sf->type);
    ( *length )++;

    cur_p++; //length
    if(dsc_req->tlv_sf->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(dsc_req->tlv_sf->length);
        (*length)++;
    }
    else if(dsc_req->tlv_sf->length <= 0xff)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length);
        (*length)++;

    }
    else if(dsc_req->tlv_sf->length <= 0xffff)
    {
        (*cur_p) = 0x82;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length);
        (*length)++;
    }
    else if(dsc_req->tlv_sf->length <= 0xffffff)
    {
        (*cur_p) = 0x83;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length);
        (*length)++;
    }
    else
    {
        (*cur_p) = 0x84;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 24);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_req->tlv_sf->length);
        (*length)++;
    }

    curp = dsc_req->tlv_sf->encapTLV;

    //sfid
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value));
    ( *length )++;

    curp = curp->next;
    //CID
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value));
    ( *length )++;

    curp = curp->next;

    //service class name
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    if(curp->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(curp->length);
        (*length)++;
    }
    else if(curp->length == 0x80)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(curp->length);
        (*length)++;

    }
    else
        FLOG_ERROR("service class name must be less than 128!");

    cur_p++;
    memcpy(cur_p, curp->value, curp->length);
    *length = *length + curp->length;

    cur_p = cur_p + curp->length - 1;

    curp=curp->next;

    //QoS set parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;

    //traffic priority parameters
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum sustained rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum burst size
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //minimum reserved rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;


    curp = curp->next;
    //UL grant scheduling type
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //request/transmission policy
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //tolerated jitter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum latency
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
    //Unsolicited Grant Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
	//Unsolicited Polling Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

	curp = curp->next;
	//SDU Inter-Arrival Interval parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    //build dsc_req end
    curp = curp->next;
    if(curp!=NULL)
    {
        FLOG_WARNING("In build_dsc_req, discarding extra TLVs\n");
        return -1;
    }

    return 0;
}

int parse_dsa_req (unsigned char *payload, int mm_len, dsa_req_msg *dsa_req)//in,in,out
{
    int tlv_length_nums;
    int cur_p = 0;
    int count;
    struct tlv_info *curp;
    unsigned char * tmp1;
    u_int16_t* tmp2;
    int *tmp4;



	if (payload[cur_p] != DSA_REQ)
	{
		FLOG_ERROR("Error in parse_dsa_req: MM_type in Input is not DSA_REQ");
		return -1;
	}
//				for (ii = 0; ii < mm_len; ii++)
//				{
//					printf("%d ", payload[ii]);
//				}
//				printf("\n");
	dsa_req->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsa_req->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;


	//tlv field
	dsa_req->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsa_req->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));
	dsa_req->tlv_sf->type = payload[cur_p++];
    if(payload[cur_p]>>7)//cur_p = 4
    {
        tlv_length_nums = payload[cur_p] & 0x7f;
        switch(tlv_length_nums)
        {
            case 1:
                dsa_req->tlv_sf->length = (int)payload[++cur_p];
                break;
            case 2:
                cur_p++;
                dsa_req->tlv_sf->length = (((int)payload[cur_p])<<8) +
                                           (int)payload[cur_p+1];
                cur_p++;
                break;
            case 3:
                cur_p++;
                dsa_req->tlv_sf->length = (((int)payload[cur_p])<<16) +
                                           (((int)payload[cur_p+1])<<8) +
                                           (int)payload[cur_p+2];
                cur_p+=2;
                break;
            case 4:
                cur_p++;
                dsa_req->tlv_sf->length = (((int)payload[cur_p])<<24) +
                                            (((int)payload[cur_p+1])<<16) +
                                             (((int)payload[cur_p+2])<<8) +
                                              (int)payload[cur_p+3];
                cur_p+=3;
                break;
            default:
                FLOG_ERROR("dsa_req->tlv_sf->length is wrong!");
                return -1;
        }

    }
    else
    {
        dsa_req->tlv_sf->length = (int)payload[cur_p];
    }
    cur_p++;

    if(mm_len != dsa_req->tlv_sf->length + cur_p)
    {
        FLOG_ERROR("parse nums error!");
        return -1;
    }
    //encapTLV field
    dsa_req->tlv_sf->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	memset(dsa_req->tlv_sf->encapTLV, 0, sizeof(struct tlv_info));
    curp = dsa_req->tlv_sf->encapTLV;

    count = 0;
    while(count < dsa_req->tlv_sf->length)
    {
        curp->type = payload[cur_p++];
        count++;
        if(payload[cur_p]>>7)
        {
            switch(payload[cur_p] & 0x7f)
            {
                case 1:
                    curp->length = payload[++cur_p];
                    break;
                default :
                    FLOG_ERROR("dsa_req->tlv_sf->encapTLV->length is wrong a!");
                    return -1;
            }
            count += 2;

        }
        else
        {
            curp->length = payload[cur_p];
            count++;
        }

        if(curp->type != 3)//not class name
        {
            switch(curp->length)
            {
                case 1:
                    tmp1 = (unsigned char *)malloc(sizeof(unsigned char));
                    *tmp1 =  (unsigned char)payload[++cur_p];
                    curp->value  = tmp1;
                    count++;
                    break;
                case 2:
                    cur_p++;
                    tmp2 = (u_int16_t *)malloc(sizeof(u_int16_t));
                    *tmp2 =  (((u_int16_t)payload[cur_p])<<8) + ((u_int16_t)payload[cur_p+1]);
                    cur_p++;
                    curp->value  = tmp2;

                    count += 2;
                    break;
                case 4:
                    cur_p++;
                    tmp4 = (int *)malloc(sizeof(int));

                    *tmp4 = (((int)payload[cur_p])<<24) + (((int)payload[cur_p+1])<<16 )+(((int)payload[cur_p+2])<<8 )+(int)payload[cur_p+3];
                    cur_p+=3;

                    curp->value  = tmp4;

                    count += 4;
                    break;
                default:
                    FLOG_ERROR("dsa_req->tlv_sf->encapTLV->value is wrong b!");
                    return -1;
            }
        }
        else
        {
            curp->value = (unsigned char *)malloc(curp->length * sizeof(unsigned char));
            memcpy(curp->value, payload+cur_p+1, curp->length);

            cur_p = cur_p + curp->length;

            count += curp->length;
        }
        if(count< dsa_req->tlv_sf->length)
        {
            curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			memset(curp->next, 0, sizeof(struct tlv_info));
            curp = curp->next;
            cur_p++;
        }
        else
        {
            curp->next = NULL;
        }
    }

	return 0;
}

int parse_dsc_req (unsigned char *payload, int mm_len, dsc_req_msg *dsc_req)//in,in,out
{
    int tlv_length_nums;
    int cur_p = 0;
    int count;
    struct tlv_info *curp;
    unsigned char * tmp1;
    u_int16_t* tmp2;
    int *tmp4;

	if (payload[cur_p] != DSC_REQ)
	{
		FLOG_ERROR("Error in parse_dsc_req: MM_type in Input is not DSA_REQ");
		return -1;
	}

	dsc_req->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsc_req->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;


	//tlv field
	dsc_req->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsc_req->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));
	dsc_req->tlv_sf->type = payload[cur_p++];
    if(payload[cur_p]>>7)//cur_p = 4
    {
        tlv_length_nums = payload[cur_p] & 0x7f;
        switch(tlv_length_nums)
        {
            case 1:
                dsc_req->tlv_sf->length = (int)payload[++cur_p];
                break;
            case 2:
                cur_p++;
                dsc_req->tlv_sf->length = (((int)payload[cur_p])<<8) +
                                           (int)payload[cur_p+1];
                cur_p++;
                break;
            case 3:
                cur_p++;
                dsc_req->tlv_sf->length = (((int)payload[cur_p])<<16) +
                                           (((int)payload[cur_p+1])<<8) +
                                           (int)payload[cur_p+2];
                cur_p+=2;
                break;
            case 4:
                cur_p++;
                dsc_req->tlv_sf->length = (((int)payload[cur_p])<<24) +
                                            (((int)payload[cur_p+1])<<16) +
                                             (((int)payload[cur_p+2])<<8) +
                                              (int)payload[cur_p+3];
                cur_p+=3;
                break;
            default:
                FLOG_ERROR("dsc_req->tlv_sf->length is wrong!");
                return -1;
        }

    }
    else
    {
        dsc_req->tlv_sf->length = (int)payload[cur_p];
    }
    cur_p++;

    if(mm_len != dsc_req->tlv_sf->length + cur_p)
    {
        FLOG_ERROR("parse nums error!");
        return -1;
    }
    //encapTLV field
    dsc_req->tlv_sf->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	memset(dsc_req->tlv_sf->encapTLV, 0, sizeof(struct tlv_info));
    curp = dsc_req->tlv_sf->encapTLV;

    count = 0;
    while(count < dsc_req->tlv_sf->length)
    {
        curp->type = payload[cur_p++];
        count++;
        if(payload[cur_p]>>7)
        {
            switch(payload[cur_p] & 0x7f)
            {
                case 1:
                    curp->length = payload[++cur_p];
                    break;
                default :
                    FLOG_ERROR("dsc_req->tlv_sf->encapTLV->length is wrong a!");
                    return -1;
            }
            count += 2;

        }
        else
        {
            curp->length = payload[cur_p];
            count++;
        }

        if(curp->type != 3)//not class name
        {
            switch(curp->length)
            {
                case 1:
                    tmp1 = (unsigned char *)malloc(sizeof(unsigned char));
                    *tmp1 =  (unsigned char)payload[++cur_p];
                    curp->value  = tmp1;
                    count++;
                    break;
                case 2:
                    cur_p++;
                    tmp2 = (u_int16_t *)malloc(sizeof(u_int16_t));
                    *tmp2 =  (((u_int16_t)payload[cur_p])<<8) + ((u_int16_t)payload[cur_p+1]);
                    cur_p++;
                    curp->value  = tmp2;

                    count += 2;
                    break;
                case 4:
                    cur_p++;
                    tmp4 = (int *)malloc(sizeof(int));

                    *tmp4 = (((int)payload[cur_p])<<24) + (((int)payload[cur_p+1])<<16 )+(((int)payload[cur_p+2])<<8 )+(int)payload[cur_p+3];
                    cur_p+=3;

                    curp->value  = tmp4;

                    count += 4;
                    break;
                default:
                    FLOG_ERROR("dsc_req->tlv_sf->encapTLV->value is wrong b!");
                    return -1;
            }
        }
        else
        {
            curp->value = (unsigned char *)malloc(curp->length * sizeof(unsigned char));
            memcpy(curp->value, payload+cur_p+1, curp->length);

            cur_p = cur_p + curp->length;

            count += curp->length;
        }
        if(count< dsc_req->tlv_sf->length)
        {
            curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			memset(curp->next, 0, sizeof(struct tlv_info));
            curp = curp->next;
            cur_p++;
        }
        else
        {
            curp->next = NULL;
        }
    }

	dsc_req->sf_id = 0;
	/* to find out the service flow id in the dsc request */
	curp = dsc_req->tlv_sf->encapTLV;
	while (curp != NULL)
	{
		switch(curp->type)
		{
			case 1:		/* service flow id */
				dsc_req->sf_id = *((int*)(curp->value));
				curp = NULL;
				break;
			default:
				curp = curp->next;
				break;
		}
	}

	return 0;
}

int free_dsa_req(dsa_req_msg* dsa_req)
{
    struct tlv_info * curp, *temp;
    curp = dsa_req->tlv_sf->encapTLV;
    while(curp!=NULL)
    {
       temp = curp;
       free(temp->value);
       curp=curp->next;
       free(temp);
    }
    free(dsa_req->tlv_sf);
    free(dsa_req);
    return 0;
}

int free_dsc_req(dsc_req_msg* dsc_req)
{
    struct tlv_info * curp, *temp;
    curp = dsc_req->tlv_sf->encapTLV;
    while(curp!=NULL)
    {
       temp = curp;
       free(temp->value);
       curp=curp->next;
       free(temp);
    }
    free(dsc_req->tlv_sf);
    free(dsc_req);
    return 0;
}

int init_dsa_rsp(dsa_req_msg* dsa_req, serviceflow* sf, int cfm_code, dsa_rsp_msg* dsa_rsp)//in,in,out
{
    int length = 0, cid = -1, sfid = -1;
#ifdef SS_TX
#ifdef SS_RX
    int first_TLV_set = FALSE;
#endif
#endif
    struct tlv_info *curp, *curpp;

	assert(dsa_req != NULL);
	assert(sf != NULL);
	assert(dsa_rsp != NULL);

    dsa_rsp->mgm_msg_type = DSA_RSP;
    dsa_rsp->trans_id = dsa_req->trans_id;

    dsa_rsp->cfm_code = cfm_code;

	// If DSA-REQ is not successful, then return, no TLVs needed
	if (cfm_code != CC_SUCCESS) 
	{
		dsa_rsp->tlv_sf = NULL;
		return 0;
	} 
    dsa_rsp->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsa_rsp->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));

    dsa_rsp->tlv_sf->type = dsa_req->tlv_sf->type;

#ifndef SS_RX
	int ss_num;

	ss_num = param_MAX_VALID_BASIC_CID  - BASIC_CID_MIN_VALUE;
	// For successful DSAs, BS will include an SFID and CID (for active/admitted flows)
	switch(sf->schedule_type)
	{
		case SERVICE_UGS:
			if (dsa_rsp->tlv_sf->type == 145) //UL
			{
				cid =  UL_UGS_CID_MIN_VALUE + ss_num; 
			}
			else if (dsa_rsp->tlv_sf->type == 146) //DL
			{
				cid = UGS_CID_MIN_VALUE + ss_num;
			}
			break;
		case SERVICE_ertPS:
			cid = ERTPS_CID_MIN_VALUE + ss_num;
			break;		
		case SERVICE_rtPS:
			cid = RTPS_CID_MIN_VALUE + ss_num;
			break;
		case SERVICE_nrtPS:
			cid = NRTPS_CID_MIN_VALUE + ss_num;
			break;
		case SERVICE_BE:
			cid = BE_CID_MIN_VALUE + ss_num;
			break;
		default:
			FLOG_ERROR("Error in init_dsa_rsp: Unknown Scheduling Type\n");
			exit(-1);
	}
	// Currently SFID = CID. TODO: Add separate logic for active/admitted flows
	sfid = cid;
	sf->sfid = sfid;
	sf->cid = cid;
#else
	sfid = sf->sfid;
	cid = sf->cid;
#endif

	// Now add corresponding TLVs in DSA-RSP
    curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));//redefine malloc containing detect
    curp->type = 1;//sfid
    curp->length = 4;

    int *tmp1 = (int*)malloc(curp->length);
    *tmp1 = sfid;
    curp->value  = tmp1;

    dsa_rsp->tlv_sf->encapTLV = curp;

    length = length + curp->length+ 2;

    //cid
    //this field needs to be noted
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 2;//CID
    curp->length = 2;


    u_int16_t *tmp2 = (u_int16_t*)malloc(curp->length);
    *tmp2 = cid;
    curp->value = tmp2;

    length = length + curp->length+ 2;


    curpp = dsa_req->tlv_sf->encapTLV;
	
	assert(curpp != NULL);

	// copy the unmodified TLVs from DSA-REQ 
	// We need a generic logic which doesn't assume an order amongst TLVs
	// nor assumes the presence/absence of certain TLVs
	while (curpp != NULL)
	{		
		switch(curpp->type)
		{
			case 1:
				/* skip, because it has been padded */
				break;
			case 2:
				/* skip, because it has been padded */
				break;
			case 3: //service class name
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 3;//service class name

				curp->length = curpp->length;
				curp->value = (unsigned char*)malloc(curp->length);
				memcpy(curp->value, curpp->value, curp->length);
				length = length + curp->length + 2;
				if(curp->length == 128)
				length += 1;
				break;
			case 5:  //QoS parameters set type
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 5;//provisioned, admitted or active set


				curp->length = 1;

				unsigned char *tmp3 = (unsigned char*)malloc(curp->length);
				*tmp3 = *(unsigned char*)curpp->value;//maybe not the same
				curp->value = tmp3;


				length = length + curp->length+ 2;
				break;

			case 6:  //traffic priority parameters
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 6;//traffic priority parameters

				curp->length = 1;
				unsigned char * tmp4 = (unsigned char*)malloc(curp->length);
				*tmp4 = *(unsigned char*)curpp->value;
				curp->value = tmp4;
				length = length + curp->length+ 2;
				break;
			case 7:  //maximum sustained rate
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 7;//maximum sustained rate

				curp->length = 4;
				int * tmp5 = (int*)malloc(curp->length);
				*tmp5 = *(int *)curpp->value;
				curp->value = tmp5;

				length = length + curp->length+ 2;
				break;
			case 8:  //maximum burst size
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 8;//maximum burst size

				curp->length = 4;

				int * tmp6 = (int*)malloc(curp->length);
				*tmp6 = *(int *)curpp->value;
				curp->value = tmp6;

				length = length + curp->length+ 2;
				break;
			case 9:      //minimum reserved rate
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 9;//minimum reserved rate

				curp->length = 4;

				int* tmp7 = (int*)malloc(curp->length);
				*tmp7 = *(int *)curpp->value;
				curp->value = tmp7;

				length = length + curp->length+ 2;
				break;
			case 11:      //UL grant scheduling type
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 11;//UL grant scheduling type

				curp->length = 1;
				unsigned char* tmp8 = (unsigned char*)malloc(curp->length);
				*tmp8 = *(unsigned char*)curpp->value;
				curp->value = tmp8;

				length = length + curp->length+ 2;
				break;
			case 12:      //request/transmission policy
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 12;//request/transmission policy

				curp->length = 1;
				unsigned char* tmp9 = (unsigned char*)malloc(curp->length);
				*tmp9 = *(unsigned char*)curpp->value;
				curp->value = tmp9;

				length = length + curp->length+ 2;
				break;
			case 13:    //tolerated jitter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 13;//tolerated jitter

				curp->length = 4;
				int* tmp10 = (int*)malloc(curp->length);
				*tmp10 = *(int *)curpp->value;
				curp->value = tmp10;

				length = length + curp->length+ 2;
				break;
			case 14:  //maximum latency
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 14;//maximum latency

				curp->length = 4;
				int* tmp11 = (int*)malloc(curp->length);
				*tmp11 = *(int *)curpp->value;
				curp->value = tmp11;

				curp->next =NULL;

				length = length + curp->length+ 2;
				break;
			case 20:	//Unsolicited Grant Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 20;
				
				curp->length = 2;
			    short* tmp12 = (short *)malloc(curp->length);
			    *tmp12 = *(u_int16_t *)curpp->value;
			    curp->value = tmp12;
				length = length + curp->length + 2;
				break;
			case 21:	//Unsolicited Polling Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 21;
				
				curp->length = 2;
			    short* tmp13 = (short *)malloc(curp->length);
			    *tmp13 = *(u_int16_t *)curpp->value;
			    curp->value = tmp13;
				length = length + curp->length + 2;
				break;
			case 26:	//SDU Inter-Arrival Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 26;
				
				curp->length = 2;
			    short* tmp14 = (short *)malloc(curp->length);
			    *tmp14 = *(u_int16_t *)curpp->value;
			    curp->value = tmp14;
				length = length + curp->length + 2;
				break;
			default:
				FLOG_ERROR("Error in init_dsa_rsp: Unrecognized TLV in input dsa_req, type = %d\n", curpp->type);
		}
		curpp = curpp->next;
	}
 
	curp->next = NULL;
	dsa_rsp->tlv_sf->length = length;

    return 0;
}

int init_dsc_rsp(dsc_req_msg* dsc_req, serviceflow* sf, int cfm_code, dsc_rsp_msg* dsc_rsp)//in,in,out
{
    int length = 0, cid = -1, sfid = -1;
#ifdef SS_TX
#ifdef SS_RX
    int first_TLV_set = FALSE;
#endif
#endif
    struct tlv_info *curp, *curpp;

	assert(dsc_req != NULL);
	assert(sf != NULL);
	assert(dsc_rsp != NULL);

    dsc_rsp->mgm_msg_type = DSC_RSP;
    dsc_rsp->trans_id = dsc_req->trans_id;

    dsc_rsp->cfm_code = cfm_code;

	// If DSC-REQ is not successful, then return, no TLVs needed
	if (cfm_code != CC_SUCCESS) 
	{
		dsc_rsp->tlv_sf = NULL;
		return 0;
	} 
    dsc_rsp->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsc_rsp->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));

    dsc_rsp->tlv_sf->type = dsc_req->tlv_sf->type;

	sfid = sf->sfid;
	cid = sf->cid;

	// Now add corresponding TLVs in DSC-RSP
    curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));//redefine malloc containing detect
    curp->type = 1;//sfid
    curp->length = 4;

    int *tmp1 = (int*)malloc(curp->length);
    *tmp1 = sfid;
    curp->value  = tmp1;

    dsc_rsp->tlv_sf->encapTLV = curp;

    length = length + curp->length+ 2;

    //cid
    //this field needs to be noted
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 2;//CID
    curp->length = 2;


    u_int16_t *tmp2 = (u_int16_t*)malloc(curp->length);
    *tmp2 = cid;
    curp->value = tmp2;

    length = length + curp->length+ 2;


    curpp = dsc_req->tlv_sf->encapTLV;
	
	assert(curpp != NULL);

	// copy the unmodified TLVs from DSC-REQ 
	// We need a generic logic which doesn't assume an order amongst TLVs
	// nor assumes the presence/absence of certain TLVs
	while (curpp != NULL)
	{		
		switch(curpp->type)
		{
			case 1:
				/* skip, because it has been padded */
				break;
			case 2:
				/* skip, because it has been padded */
				break;
			case 3: //service class name
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 3;//service class name

				curp->length = curpp->length;
				curp->value = (unsigned char*)malloc(curp->length);
				memcpy(curp->value, curpp->value, curp->length);
				length = length + curp->length + 2;
				if(curp->length == 128)
				length += 1;
				break;
			case 5:  //QoS parameters set type
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 5;//provisioned, admitted or active set


				curp->length = 1;

				unsigned char *tmp3 = (unsigned char*)malloc(curp->length);
				*tmp3 = *(unsigned char*)curpp->value;//maybe not the same
				curp->value = tmp3;


				length = length + curp->length+ 2;
				break;

			case 6:  //traffic priority parameters
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 6;//traffic priority parameters

				curp->length = 1;
				unsigned char * tmp4 = (unsigned char*)malloc(curp->length);
				*tmp4 = *(unsigned char*)curpp->value;
				curp->value = tmp4;
				length = length + curp->length+ 2;
				break;
			case 7:  //maximum sustained rate
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 7;//maximum sustained rate

				curp->length = 4;
				int * tmp5 = (int*)malloc(curp->length);
				*tmp5 = *(int *)curpp->value;
				curp->value = tmp5;

				length = length + curp->length+ 2;
				break;
			case 8:  //maximum burst size
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 8;//maximum burst size

				curp->length = 4;

				int * tmp6 = (int*)malloc(curp->length);
				*tmp6 = *(int *)curpp->value;
				curp->value = tmp6;

				length = length + curp->length+ 2;
				break;
			case 9:      //minimum reserved rate
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 9;//minimum reserved rate

				curp->length = 4;

				int* tmp7 = (int*)malloc(curp->length);
				*tmp7 = *(int *)curpp->value;
				curp->value = tmp7;

				length = length + curp->length+ 2;
				break;
			case 11:      //UL grant scheduling type
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 11;//UL grant scheduling type

				curp->length = 1;
				unsigned char* tmp8 = (unsigned char*)malloc(curp->length);
				*tmp8 = *(unsigned char*)curpp->value;
				curp->value = tmp8;

				length = length + curp->length+ 2;
				break;
			case 12:      //request/transmission policy
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 12;//request/transmission policy

				curp->length = 1;
				unsigned char* tmp9 = (unsigned char*)malloc(curp->length);
				*tmp9 = *(unsigned char*)curpp->value;
				curp->value = tmp9;

				length = length + curp->length+ 2;
				break;
			case 13:    //tolerated jitter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 13;//tolerated jitter

				curp->length = 4;
				int* tmp10 = (int*)malloc(curp->length);
				*tmp10 = *(int *)curpp->value;
				curp->value = tmp10;

				length = length + curp->length+ 2;
				break;
			case 14:  //maximum latency
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp = curp->next;
				curp->type = 14;//maximum latency

				curp->length = 4;
				int* tmp11 = (int*)malloc(curp->length);
				*tmp11 = *(int *)curpp->value;
				curp->value = tmp11;

				curp->next =NULL;

				length = length + curp->length+ 2;
				break;
			case 20:	//Unsolicited Grant Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 20;
				
				curp->length = 2;
			    short* tmp12 = (short *)malloc(curp->length);
			    *tmp12 = *(u_int16_t *)curpp->value;
			    curp->value = tmp12;
				length = length + curp->length + 2;
				break;
			case 21:	//Unsolicited Polling Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 21;
				
				curp->length = 2;
			    short* tmp13 = (short *)malloc(curp->length);
			    *tmp13 = *(u_int16_t *)curpp->value;
			    curp->value = tmp13;
				length = length + curp->length + 2;
				break;
			case 26:	//SDU Inter-Arrival Interval parameter
				curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			    curp = curp->next;
			    curp->type = 26;
				
				curp->length = 2;
			    short* tmp14 = (short *)malloc(curp->length);
			    *tmp14 = *(u_int16_t *)curpp->value;
			    curp->value = tmp14;
				length = length + curp->length + 2;
				break;
			default:
				FLOG_ERROR("Error in init_dsc_rsp: Unrecognized TLV in input dsc_req, type = %d\n", curpp->type);
		}
		curpp = curpp->next;
	}
 
	curp->next = NULL;
	dsc_rsp->tlv_sf->length = length;

    return 0;
}

int build_dsa_rsp (dsa_rsp_msg* dsa_rsp, unsigned char* payload, int* length)//in,out,out
{

    unsigned char* cur_p;
    struct tlv_info *curp;


    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsa_rsp->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsa_rsp->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsa_rsp->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsa_rsp->cfm_code);
    ( *length )++;

	// If the add was not successful, there will be no more TLVs
	if (dsa_rsp->tlv_sf == NULL) {return 0;}

    // TLV field
    cur_p++; //type
    ( *cur_p ) = ( unsigned char ) (dsa_rsp->tlv_sf->type);
    ( *length )++;

    cur_p++; //length
    if(dsa_rsp->tlv_sf->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(dsa_rsp->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_rsp->tlv_sf->length <= 0xff)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length);
        (*length)++;

    }
    else if(dsa_rsp->tlv_sf->length <= 0xffff)
    {
        (*cur_p) = 0x82;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_rsp->tlv_sf->length <= 0xffffff)
    {
        (*cur_p) = 0x83;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length);
        (*length)++;
    }
    else
    {
        (*cur_p) = 0x84;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 24);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_rsp->tlv_sf->length);
        (*length)++;
    }

    curp = dsa_rsp->tlv_sf->encapTLV;
	assert(curp != NULL);

	while(curp != NULL)
	{
		cur_p++;
		( *cur_p ) = ( unsigned char ) (curp->type);
		( *length )++;
		switch(curp->type)
		{
			case 1: //sfid
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value));
				( *length )++;

				break;
			case 2: //cid
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value));
				( *length )++;
				break;
			case 3: //service class name
				cur_p++;
				if(curp->length <= 0x7f)
				{
					(*cur_p) = (unsigned char)(curp->length);
					(*length)++;
				}
				else if(curp->length == 0x80)
				{
					(*cur_p) = 0x81;
					(*length)++;
					cur_p ++;
					*cur_p = (unsigned char)(curp->length);
					(*length)++;

				}
				else
					FLOG_ERROR("service class name must be less than 128!");

				cur_p++;
				memcpy(cur_p, curp->value, curp->length);
				*length = *length + curp->length;

				cur_p = cur_p + curp->length - 1;

				break;
			case 5: //QoS set parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 6: //traffic priority parameters
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 7: //maximum sustained rate
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 8: //maximum burst size
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 9: //minimum reserved rate
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 11: //UL grant scheduling type
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
								break;
			case 12: //Request/Transmission policy
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 13: //Tolerate jitter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;

				break;
			case 14: //Max Latency
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 20: //Unsolicited Grant Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 21: //Unsolicited Polling Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 26: //SDU Inter-Arrival Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			default:
				FLOG_ERROR("Error in build_dsa_rsp: Unsupported TLV type: %d\n", curp->type);
		}
		curp = curp->next;
	}

    return 0;
}

int build_dsc_rsp (dsc_rsp_msg* dsc_rsp, unsigned char* payload, int* length)//in,out,out
{
    unsigned char* cur_p;
    struct tlv_info *curp;

    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsc_rsp->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsc_rsp->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsc_rsp->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsc_rsp->cfm_code);
    ( *length )++;

	// If the add was not successful, there will be no more TLVs
	if (dsc_rsp->tlv_sf == NULL) {return 0;}

    // TLV field
    cur_p++; //type
    ( *cur_p ) = ( unsigned char ) (dsc_rsp->tlv_sf->type);
    ( *length )++;

    cur_p++; //length
    if(dsc_rsp->tlv_sf->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(dsc_rsp->tlv_sf->length);
        (*length)++;
    }
    else if(dsc_rsp->tlv_sf->length <= 0xff)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length);
        (*length)++;

    }
    else if(dsc_rsp->tlv_sf->length <= 0xffff)
    {
        (*cur_p) = 0x82;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length);
        (*length)++;
    }
    else if(dsc_rsp->tlv_sf->length <= 0xffffff)
    {
        (*cur_p) = 0x83;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length);
        (*length)++;
    }
    else
    {
        (*cur_p) = 0x84;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 24);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsc_rsp->tlv_sf->length);
        (*length)++;
    }

    curp = dsc_rsp->tlv_sf->encapTLV;
	assert(curp != NULL);

	while(curp != NULL)
	{
		cur_p++;
		( *cur_p ) = ( unsigned char ) (curp->type);
		( *length )++;
		switch(curp->type)
		{
			case 1: //sfid
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int*)(curp->value));
				( *length )++;

				break;
			case 2: //cid
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value));
				( *length )++;
				break;
			case 3: //service class name
				cur_p++;
				if(curp->length <= 0x7f)
				{
					(*cur_p) = (unsigned char)(curp->length);
					(*length)++;
				}
				else if(curp->length == 0x80)
				{
					(*cur_p) = 0x81;
					(*length)++;
					cur_p ++;
					*cur_p = (unsigned char)(curp->length);
					(*length)++;

				}
				else
					FLOG_ERROR("service class name must be less than 128!");

				cur_p++;
				memcpy(cur_p, curp->value, curp->length);
				*length = *length + curp->length;

				cur_p = cur_p + curp->length - 1;

				break;
			case 5: //QoS set parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 6: //traffic priority parameters
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 7: //maximum sustained rate
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 8: //maximum burst size
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 9: //minimum reserved rate
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 11: //UL grant scheduling type
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
								break;
			case 12: //Request/Transmission policy
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
				( *length )++;
				break;
			case 13: //Tolerate jitter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;

				break;
			case 14: //Max Latency
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 20: //Unsolicited Grant Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 21: //Unsolicited Polling Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			case 26: //SDU Inter-Arrival Interval parameter
				cur_p++;
				( *cur_p ) = ( unsigned char ) (curp->length);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
				( *length )++;

				cur_p++;
				( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
				( *length )++;
				break;
			default:
				FLOG_ERROR("Error in build_dsc_rsp: Unsupported TLV type\n");
		}
		curp = curp->next;
	}

    return 0;
}


int parse_dsa_rsp(unsigned char *payload, int mm_len, dsa_rsp_msg *dsa_rsp)//in,in,out
{
    int tlv_length_nums;
	// TODO: Replace cur_p with cur_idx or similar name. This is not a pointer
	// hence violates coding guideline, & is confusing
    int cur_p = 0;
    int count;
    struct tlv_info *curp;
    unsigned char * tmp1;
    u_int16_t* tmp2;
    int *tmp4;


	if (payload[cur_p] != DSA_RSP)
	{
		FLOG_ERROR("Error in parse_dsa_rsp: MM_type in Input is not DSA_REQ");
		return -1;
	}
	dsa_rsp->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsa_rsp->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;

    dsa_rsp->cfm_code = payload[cur_p++];
	if (mm_len <= 4)
	{
		dsa_rsp->tlv_sf = NULL;
		return 0;
	}
	//tlv field
	dsa_rsp->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsa_rsp->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));
	dsa_rsp->tlv_sf->type = payload[cur_p++];
    if(payload[cur_p]>>7)//cur_p = 4. If length > 127
    {
		// TODO: Verify this logic
        tlv_length_nums = payload[cur_p] & 0x7f;
        switch(tlv_length_nums)
        {
            case 1:
                dsa_rsp->tlv_sf->length = (int)payload[++cur_p];
                break;
            case 2:
                cur_p++;
                dsa_rsp->tlv_sf->length = (((int)payload[cur_p])<<8) +
                                           (int)payload[cur_p+1];
                cur_p++;
                break;
            case 3:
                cur_p++;
                dsa_rsp->tlv_sf->length = (((int)payload[cur_p])<<16) +
                                           (((int)payload[cur_p+1])<<8) +
                                           (int)payload[cur_p+2];
                cur_p+=2;
                break;
            case 4:
                cur_p++;
                dsa_rsp->tlv_sf->length = (((int)payload[cur_p])<<24) +
                                            (((int)payload[cur_p+1])<<16) +
                                             (((int)payload[cur_p+2])<<8) +
                                              (int)payload[cur_p+3];
                cur_p+=3;
                break;
            default:
                FLOG_ERROR("dsa_rsp->tlv_sf->length is wrong! %d\n", tlv_length_nums);
                return -1;
        }

    }
    else
    {
        dsa_rsp->tlv_sf->length = (int)payload[cur_p];
    }
    cur_p++;
    if(mm_len != dsa_rsp->tlv_sf->length + cur_p)
    {
        FLOG_ERROR("In parse_dsa_rsp, parse nums error! mm_len: %d, other: %d\n", mm_len,  dsa_rsp->tlv_sf->length + cur_p);
        return -1;
    }
    //encapTLV field
    dsa_rsp->tlv_sf->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	memset(dsa_rsp->tlv_sf->encapTLV, 0, sizeof(struct tlv_info));
    curp = dsa_rsp->tlv_sf->encapTLV;

    count = 0;
    while(count < dsa_rsp->tlv_sf->length)
    {
        curp->type = payload[cur_p++];
        count++;
        if(payload[cur_p]>>7)
        {
            switch(payload[cur_p] & 0x7f)
            {
                case 1:
					// TODO: Verify this logic. shdn't it be payload[cur_p] << 8 + payload[++cur_p]?
                    curp->length = payload[++cur_p];
                    break;
                default :
                    FLOG_ERROR("dsa_rsp->tlv_sf->encapTLV->length is wrong a!");
                    return -1;
            }
            count += 2;
        }
        else
        {
            curp->length = payload[cur_p];
            count++;
        }

        if(curp->type != 3)//not class name
        {
            switch(curp->length)
            {
                case 1:
                    tmp1 = (unsigned char *)malloc(sizeof(unsigned char));
                    *tmp1 =  (unsigned char)payload[++cur_p];
                    curp->value  = tmp1;
                    count++;
                    break;
                case 2:
                    cur_p++;
                    tmp2 = (u_int16_t *)malloc(sizeof(u_int16_t));
                    *tmp2 =  (((u_int16_t)payload[cur_p])<<8) + ((u_int16_t)payload[cur_p+1]);
                    cur_p++;
                    curp->value  = tmp2;

                    count += 2;
                    break;
                case 4:
                    cur_p++;
                    tmp4 = (int *)malloc(sizeof(int));

                    *tmp4 = (((int)payload[cur_p])<<24) + (((int)payload[cur_p+1])<<16 )+(((int)payload[cur_p+2])<<8 )+(int)payload[cur_p+3];
                    cur_p+=3;

                    curp->value  = tmp4;

                    count += 4;
                    break;
                default:
                    FLOG_ERROR("dsa_rsp->tlv_sf->encapTLV->value is wrong b!");
                    return -1;
            }
        }
        else
        {
            curp->value = (unsigned char *)malloc(curp->length * sizeof(unsigned char));
            memcpy(curp->value, payload+cur_p+1, curp->length);

            cur_p = cur_p + curp->length;

            count += curp->length;
        }
        if(count< dsa_rsp->tlv_sf->length)
        {
            curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
            curp = curp->next;
            cur_p++;
        }
        else
        {
            curp->next = NULL;
        }
    }

	return 0;
}

int parse_dsc_rsp(unsigned char *payload, int mm_len, dsc_rsp_msg *dsc_rsp)//in,in,out
{
    int tlv_length_nums;
	// TODO: Replace cur_p with cur_idx or similar name. This is not a pointer
	// hence violates coding guideline, & is confusing
    int cur_p = 0;
    int count;
    struct tlv_info *curp;
    unsigned char * tmp1;
    u_int16_t* tmp2;
    int *tmp4;


	if (payload[cur_p] != DSC_RSP)
	{
		FLOG_ERROR("Error in parse_dsc_rsp: MM_type in Input is not DSA_REQ");
		return -1;
	}
	dsc_rsp->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsc_rsp->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;

    dsc_rsp->cfm_code = payload[cur_p++];
	if (mm_len <= 4)
	{
		dsc_rsp->tlv_sf = NULL;
		return 0;
	}
	//tlv field
	dsc_rsp->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	memset(dsc_rsp->tlv_sf, 0, sizeof(struct tlv_sf_mgmt_encoding));
	dsc_rsp->tlv_sf->type = payload[cur_p++];
    if(payload[cur_p]>>7)//cur_p = 4. If length > 127
    {
		// TODO: Verify this logic
        tlv_length_nums = payload[cur_p] & 0x7f;
        switch(tlv_length_nums)
        {
            case 1:
                dsc_rsp->tlv_sf->length = (int)payload[++cur_p];
                break;
            case 2:
                cur_p++;
                dsc_rsp->tlv_sf->length = (((int)payload[cur_p])<<8) +
                                           (int)payload[cur_p+1];
                cur_p++;
                break;
            case 3:
                cur_p++;
                dsc_rsp->tlv_sf->length = (((int)payload[cur_p])<<16) +
                                           (((int)payload[cur_p+1])<<8) +
                                           (int)payload[cur_p+2];
                cur_p+=2;
                break;
            case 4:
                cur_p++;
                dsc_rsp->tlv_sf->length = (((int)payload[cur_p])<<24) +
                                            (((int)payload[cur_p+1])<<16) +
                                             (((int)payload[cur_p+2])<<8) +
                                              (int)payload[cur_p+3];
                cur_p+=3;
                break;
            default:
                FLOG_ERROR("dsc_rsp->tlv_sf->length is wrong! %d\n", tlv_length_nums);
                return -1;
        }

    }
    else
    {
        dsc_rsp->tlv_sf->length = (int)payload[cur_p];
    }
    cur_p++;
    if(mm_len != dsc_rsp->tlv_sf->length + cur_p)
    {
        FLOG_ERROR("In parse_dsc_rsp, parse nums error! mm_len: %d, other: %d\n", mm_len,  dsc_rsp->tlv_sf->length + cur_p);
        return -1;
    }
    //encapTLV field
    dsc_rsp->tlv_sf->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	memset(dsc_rsp->tlv_sf->encapTLV, 0, sizeof(struct tlv_info));
    curp = dsc_rsp->tlv_sf->encapTLV;

    count = 0;
    while(count < dsc_rsp->tlv_sf->length)
    {
        curp->type = payload[cur_p++];
        count++;
        if(payload[cur_p]>>7)
        {
            switch(payload[cur_p] & 0x7f)
            {
                case 1:
					// TODO: Verify this logic. shdn't it be payload[cur_p] << 8 + payload[++cur_p]?
                    curp->length = payload[++cur_p];
                    break;
                default :
                    FLOG_ERROR("dsc_rsp->tlv_sf->encapTLV->length is wrong a!");
                    return -1;
            }
            count += 2;
        }
        else
        {
            curp->length = payload[cur_p];
            count++;
        }

        if(curp->type != 3)//not class name
        {
            switch(curp->length)
            {
                case 1:
                    tmp1 = (unsigned char *)malloc(sizeof(unsigned char));
                    *tmp1 =  (unsigned char)payload[++cur_p];
                    curp->value  = tmp1;
                    count++;
                    break;
                case 2:
                    cur_p++;
                    tmp2 = (u_int16_t *)malloc(sizeof(u_int16_t));
                    *tmp2 =  (((u_int16_t)payload[cur_p])<<8) + ((u_int16_t)payload[cur_p+1]);
                    cur_p++;
                    curp->value  = tmp2;

                    count += 2;
                    break;
                case 4:
                    cur_p++;
                    tmp4 = (int *)malloc(sizeof(int));

                    *tmp4 = (((int)payload[cur_p])<<24) + (((int)payload[cur_p+1])<<16 )+(((int)payload[cur_p+2])<<8 )+(int)payload[cur_p+3];
                    cur_p+=3;

                    curp->value  = tmp4;

                    count += 4;
                    break;
                default:
                    FLOG_ERROR("dsc_rsp->tlv_sf->encapTLV->value is wrong b!");
                    return -1;
            }
        }
        else
        {
            curp->value = (unsigned char *)malloc(curp->length * sizeof(unsigned char));
            memcpy(curp->value, payload+cur_p+1, curp->length);

            cur_p = cur_p + curp->length;

            count += curp->length;
        }
        if(count< dsc_rsp->tlv_sf->length)
        {
            curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
            curp = curp->next;
            cur_p++;
        }
        else
        {
            curp->next = NULL;
        }
    }

	return 0;
}

int free_dsa_rsp(dsa_rsp_msg* dsa_rsp)
{
    struct tlv_info * curp, *temp;
	if (dsa_rsp->tlv_sf != NULL)
	{
		curp = dsa_rsp->tlv_sf->encapTLV;
		while(curp!=NULL)
		{
			temp = curp;
			free(temp->value);
			curp=curp->next;
			free(temp);
		}
		free(dsa_rsp->tlv_sf);
	}
    free(dsa_rsp);
    return 0;
}

int free_dsc_rsp(dsc_rsp_msg* dsc_rsp)
{
    struct tlv_info * curp, *temp;
	if (dsc_rsp->tlv_sf != NULL)
	{
		curp = dsc_rsp->tlv_sf->encapTLV;
		while(curp!=NULL)
		{
			temp = curp;
			free(temp->value);
			curp=curp->next;
			free(temp);
		}
		free(dsc_rsp->tlv_sf);
	}
    free(dsc_rsp);
    return 0;
}

int init_dsa_ack(dsa_rsp_msg* dsa_rsp, dsa_ack_msg* dsa_ack)//in,out
{

    dsa_ack->mgm_msg_type = 13;//create msg dsaREQ
    dsa_ack->trans_id = dsa_rsp->trans_id;

    dsa_ack->cfm_code = dsa_rsp->cfm_code;
	dsa_ack->tlv_sf = NULL;
	
	// Ref Sec 12.4.2.1.1: No TLVs besides HMAC Tuples shall be reported back
	// in DSA-ACK messages for The Wimax-OFDMA profiles. HMAC tuple not supported presently 
	#if 0 
    dsa_ack->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));


    dsa_ack->tlv_sf->type = dsa_rsp->tlv_sf->type;


    length = 0;


    struct tlv_info *curp, *curpp;
    curpp = dsa_rsp->tlv_sf->encapTLV;
    //sfid
    curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));//redefine malloc containing detect
    curp->type = 1;//sfid
    curp->length = 4;

    int *tmp1 = (int*)malloc(curp->length);
    *tmp1 = *(int *)curpp->value;
    curp->value  = tmp1;

    dsa_ack->tlv_sf->encapTLV = curp;

    length = length + curp->length+ 2;

    //cid
    //this field needs to be noted
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 2;//CID
    curp->length = 2;


    u_int16_t *tmp2 = (u_int16_t*)malloc(curp->length);
    *tmp2 = *(u_int16_t*)curpp->value;
    curp->value = tmp2;

    length = length + curp->length+ 2;

    //service class name
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 3;//service class name

    curp->length = curpp->length;


    curp->value = (unsigned char*)malloc(curp->length);
    memcpy(curp->value, curpp->value, curp->length);
    length = length + curp->length + 2;
    if(curp->length == 128)
    length += 1;
//    temp = curp->length;
//    while(temp>0)
//    {
//        * curp->value++ = * sf->service_class_name++;
//        temp--;
//    }



    //QoS parameters set type
    //pls look table 600 on page 1347
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 5;//provisioned, admitted or active set


    curp->length = 1;

    unsigned char *tmp3 = (unsigned char*)malloc(curp->length);
    *tmp3 = *(unsigned char*)curpp->value;//maybe not the same
    curp->value = tmp3;


    length = length + curp->length+ 2;

   //6,7,8,9, 11,12,13,14are QoS parameters
   //traffic priority parameters
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 6;//traffic priority parameters

    curp->length = 1;
    unsigned char * tmp4 = (unsigned char*)malloc(curp->length);
    *tmp4 = *(unsigned char*)curpp->value;
    curp->value = tmp4;


    length = length + curp->length+ 2;

    //maximum sustained rate
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 7;//maximum sustained rate


    curp->length = 4;
    int * tmp5 = (int*)malloc(curp->length);
    *tmp5 = *(int *)curpp->value;
    curp->value = tmp5;


    length = length + curp->length+ 2;

    //maximum burst size
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 8;//maximum burst size


    curp->length = 4;

    int * tmp6 = (int*)malloc(curp->length);
    *tmp6 = *(int *)curpp->value;
    curp->value = tmp6;


    length = length + curp->length+ 2;

    //minimum reserved rate
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 9;//minimum reserved rate


    curp->length = 4;

    int* tmp7 = (int*)malloc(curp->length);
    *tmp7 = *(int *)curpp->value;
    curp->value = tmp7;



    length = length + curp->length+ 2;

    //UL grant scheduling type
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 11;//UL grant scheduling type


    curp->length = 1;
    unsigned char* tmp8 = (unsigned char*)malloc(curp->length);
    *tmp8 = *(unsigned char*)curpp->value;
    curp->value = tmp8;



    length = length + curp->length+ 2;

    //request/transmission policy
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 12;//request/transmission policy


    curp->length = 1;
    unsigned char* tmp9 = (unsigned char*)malloc(curp->length);
    *tmp9 = *(unsigned char*)curpp->value;
    curp->value = tmp9;



    length = length + curp->length+ 2;

    //tolerated jitter
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 13;//tolerated jitter


    curp->length = 4;
    int* tmp10 = (int*)malloc(curp->length);
    *tmp10 = *(int *)curpp->value;
    curp->value = tmp10;



    length = length + curp->length+ 2;

    //maximum latency
    curpp = curpp->next;
    curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = curp->next;
    curp->type = 14;//maximum latency

    curp->length = 4;
    int* tmp11 = (int*)malloc(curp->length);
    *tmp11 = *(int *)curpp->value;
    curp->value = tmp11;

    curp->next =NULL;



    length = length + curp->length+ 2;



    dsa_ack->tlv_sf->length = length;
	#endif
    return 0;
}

int init_dsc_ack(dsa_rsp_msg* dsa_rsp, dsc_ack_msg* dsc_ack)//in,out
{
    dsc_ack->mgm_msg_type = DSC_ACK;//create msg dsaREQ
    dsc_ack->trans_id = dsa_rsp->trans_id;

    dsc_ack->cfm_code = dsa_rsp->cfm_code;
	dsc_ack->tlv_sf = NULL;

	return 0;
}

int build_dsa_ack(dsa_ack_msg* dsa_ack, unsigned char* payload, int* length)//in,out,out
{

    unsigned char* cur_p;

    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsa_ack->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsa_ack->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsa_ack->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsa_ack->cfm_code);
    ( *length )++;

	// Ref Sec 12.4.2.1.1: No TLVs besides HMAC Tuples shall be reported back
	// in DSA-ACK messages for The Wimax-OFDMA profiles. HMAC tuple not supported presently
	#if 0
    // TLV field
    cur_p++; //type
    ( *cur_p ) = ( unsigned char ) (dsa_ack->tlv_sf->type);
    ( *length )++;

    cur_p++; //length
    if(dsa_ack->tlv_sf->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(dsa_ack->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_ack->tlv_sf->length <= 0xff)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length);
        (*length)++;

    }
    else if(dsa_ack->tlv_sf->length <= 0xffff)
    {
        (*cur_p) = 0x82;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length);
        (*length)++;
    }
    else if(dsa_ack->tlv_sf->length <= 0xffffff)
    {
        (*cur_p) = 0x83;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length);
        (*length)++;
    }
    else
    {
        (*cur_p) = 0x84;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 24);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 16);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length >> 8);
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(dsa_ack->tlv_sf->length);
        (*length)++;
    }

    curp = dsa_ack->tlv_sf->encapTLV;
    //sfid
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int*)(curp->value));
    ( *length )++;

    curp = curp->next;
    //CID
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(u_int16_t *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //service class name
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    if(curp->length <= 0x7f)
    {
        (*cur_p) = (unsigned char)(curp->length);
        (*length)++;
    }
    else if(curp->length == 0x80)
    {
        (*cur_p) = 0x81;
        (*length)++;
        cur_p ++;
        *cur_p = (unsigned char)(curp->length);
        (*length)++;

    }
    else
        FLOG_ERROR("service class name must be less than 128!");

    cur_p++;
    memcpy(cur_p, curp->value, curp->length);
    *length = *length + curp->length;

    cur_p = cur_p + curp->length - 1;

    curp=curp->next;

    //QoS set parameter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;

    //traffic priority parameters
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum sustained rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum burst size
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //minimum reserved rate
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;


    curp = curp->next;
    //UL grant scheduling type
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //request/transmission policy
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(unsigned char *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //tolerated jitter
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    curp = curp->next;
    //maximum latency
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->type);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (curp->length);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 24);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 16);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value) >> 8);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (*(int *)(curp->value));
    ( *length )++;

    //build dsa_req end
    curp = curp->next;
    if(curp!=NULL)
    {
        FLOG_WARNING("In build_dsa_ack: Discarding extra TLVs!\n");
        return -1;
    }
	#endif
    return 0;
}

int build_dsc_ack(dsc_ack_msg* dsc_ack, unsigned char* payload, int* length)//in,out,out
{
    unsigned char* cur_p;

    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsc_ack->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsc_ack->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsc_ack->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsc_ack->cfm_code);
    ( *length )++;
    
    return 0;
}

int parse_dsc_ack(unsigned char *payload, int mm_len, dsc_ack_msg *dsc_ack)//in,in,out
{
    int cur_p = 0;

	if (payload[cur_p] != DSC_ACK)
	{
		FLOG_ERROR("Error in parse_dsa_rsp: MM_type in Input is not DSC_ACK");
		return -1;
	}
	dsc_ack->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsc_ack->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;

    dsc_ack->cfm_code = payload[cur_p++];
	dsc_ack->tlv_sf = NULL;

	return 0;
}

int parse_dsa_ack(unsigned char *payload, int mm_len, dsa_ack_msg *dsa_ack)//in,in,out
{
    int cur_p = 0;


	if (payload[cur_p] != DSA_ACK)
	{
		FLOG_ERROR("Error in parse_dsa_rsp: MM_type in Input is not DSA_ACK");
		return -1;
	}
	dsa_ack->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsa_ack->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      ((u_int16_t)payload[cur_p+1]);
    cur_p+=2;

    dsa_ack->cfm_code = payload[cur_p++];
	dsa_ack->tlv_sf = NULL;

	// Ref Sec 12.4.2.1.1: No TLVs besides HMAC Tuples shall be reported back
	// in DSA-ACK messages for The Wimax-OFDMA profiles. HMAC tuple not supported presently
	#if 0
	//tlv field
	dsa_ack->tlv_sf = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	dsa_ack->tlv_sf->type = payload[cur_p++];
    if(payload[cur_p]>>7)//cur_p = 4
    {
        tlv_length_nums = payload[cur_p] & 0x7f;
        switch(tlv_length_nums)
        {
            case 1:
                dsa_ack->tlv_sf->length = (int)payload[++cur_p];
                break;
            case 2:
                cur_p++;
                dsa_ack->tlv_sf->length = (((int)payload[cur_p])<<8) +
                                           (int)payload[cur_p+1];
                cur_p++;
                break;
            case 3:
                cur_p++;
                dsa_ack->tlv_sf->length = (((int)payload[cur_p])<<16) +
                                           (((int)payload[cur_p+1])<<8) +
                                           (int)payload[cur_p+2];
                cur_p+=2;
                break;
            case 4:
                cur_p++;
                dsa_ack->tlv_sf->length = (((int)payload[cur_p])<<24) +
                                            (((int)payload[cur_p+1])<<16) +
                                             (((int)payload[cur_p+2])<<8) +
                                              (int)payload[cur_p+3];
                cur_p+=3;
                break;
            default:
                printf("dsa_ack->tlv_sf->length is wrong!");
                return -1;
        }

    }
    else
    {
        dsa_ack->tlv_sf->length = (int)payload[cur_p];
    }
    cur_p++;

    if(mm_len != dsa_ack->tlv_sf->length + cur_p)
    {
        printf("parse nums error!");
        return -1;
    }
    //encapTLV field
    dsa_ack->tlv_sf->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));
    curp = dsa_ack->tlv_sf->encapTLV;

    count = 0;
    while(count < dsa_ack->tlv_sf->length)
    {
        curp->type = payload[cur_p++];
        count++;
        if(payload[cur_p]>>7)
        {
            switch(payload[cur_p] & 0x7f)
            {
                case 1:
                    curp->length = payload[++cur_p];
                    break;
                default :
                    printf("dsa_ack->tlv_sf->encapTLV->length is wrong a!");
                    return -1;
            }
            count += 2;

        }
        else
        {
            curp->length = payload[cur_p];
            count++;
        }

        if(curp->type != 3)//not class name
        {
            switch(curp->length)
            {
                case 1:
                    tmp1 = (unsigned char *)malloc(sizeof(unsigned char));
                    *tmp1 =  (unsigned char)payload[++cur_p];
                    curp->value  = tmp1;
                    count++;
                    break;
                case 2:
                    cur_p++;
                    tmp2 = (u_int16_t *)malloc(sizeof(u_int16_t));
                    *tmp2 =  (((u_int16_t)payload[cur_p])<<8) + ((u_int16_t)payload[cur_p+1]);
                    cur_p++;
                    curp->value  = tmp2;

                    count += 2;
                    break;
                case 4:
                    cur_p++;
                    tmp4 = (int *)malloc(sizeof(int));

                    *tmp4 = (((int)payload[cur_p])<<24) + (((int)payload[cur_p+1])<<16 )+(((int)payload[cur_p+2])<<8 )+(int)payload[cur_p+3];
                    cur_p+=3;

                    curp->value  = tmp4;

                    count += 4;
                    break;
                default:
                    printf("dsa_ack->tlv_sf->encapTLV->value is wrong b!");
                    return -1;
            }
        }
        else
        {
            curp->value = (unsigned char *)malloc(curp->length * sizeof(unsigned char));
            memcpy(curp->value, payload+cur_p+1, curp->length);

            cur_p = cur_p + curp->length;

            count += curp->length;
        }
        if(count< dsa_ack->tlv_sf->length)
        {
            curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
            curp = curp->next;
            cur_p++;
        }
        else
        {
            curp->next = NULL;
        }
    }
	#endif
	return 0;
}

int free_dsa_ack(dsa_ack_msg* dsa_ack)
{
    struct tlv_info * curp, *temp;
	if(dsa_ack != NULL) 
	{
		if(dsa_ack->tlv_sf != NULL)
		{
			curp = dsa_ack->tlv_sf->encapTLV;
			while(curp!=NULL)
			{
				temp = curp;
				free(temp->value);
				curp=curp->next;
				free(temp);
			}
			free(dsa_ack->tlv_sf);
		}
		free(dsa_ack);
	}
    return 0;
}

int free_dsc_ack(dsc_ack_msg* dsc_ack)
{
    struct tlv_info * curp, *temp;
	if(dsc_ack != NULL) 
	{
		if(dsc_ack->tlv_sf != NULL)
		{
			curp = dsc_ack->tlv_sf->encapTLV;
			while(curp!=NULL)
			{
				temp = curp;
				free(temp->value);
				curp=curp->next;
				free(temp);
			}
			free(dsc_ack->tlv_sf);
		}
		free(dsc_ack);
	}
    return 0;
}


int init_dsx_rvd(dsa_req_msg* dsa_req, dsx_rvd_msg *dsx_rvd)//in,out
{
    dsx_rvd->mgm_msg_type = 30;//create msg dsarvd
    dsx_rvd->trans_id = dsa_req->trans_id;

    //cfm_code needs to be added after init
    dsx_rvd->cfm_code = 18;//initiate 18
    return 0;
}
int build_dsx_rvd(dsx_rvd_msg* dsx_rvd, unsigned char* payload, int* length)//in,out,out
{

    unsigned char* cur_p;


    *length = 0;
    // management message type
    cur_p = payload ;
    ( *cur_p ) = ( unsigned char ) (dsx_rvd->mgm_msg_type);
    ( *length )++;

    // trans_id
    cur_p++;
    ( *cur_p ) = ( unsigned char ) ((dsx_rvd->trans_id >> 8) & 0x00ff);
    ( *length )++;

    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsx_rvd->trans_id  & 0x00ff);
    ( *length )++;

    // cc field
    cur_p++;
    ( *cur_p ) = ( unsigned char ) (dsx_rvd->cfm_code);
    ( *length )++;

    return 0;
}
int parse_dsx_rvd (unsigned char *payload, int mm_len, dsx_rvd_msg *dsx_rvd)//in,in,out
{
    int cur_p = 0;

	if (payload[cur_p] != DSX_RVD)
	{
		FLOG_ERROR("Error in parse_dsa_req: MM_type in Input is not dsx_rvd");
		return -1;
	}
	dsx_rvd->mgm_msg_type = payload[cur_p];
	cur_p++;
	dsx_rvd->trans_id = (((u_int16_t)payload[cur_p])<<8) +
	                      (((u_int16_t)payload[cur_p+1]));
    cur_p += 2;

    dsx_rvd->cfm_code = payload[cur_p++];

    if(mm_len != cur_p)
    {
        FLOG_ERROR("parse nums error!");
        return -1;
    }
	return 0;
}

/*
int print_dsa_msg(void *ds_msg, int type)
{
	dsa_req_msg *dsa_req = NULL;
	dsa_rsp_msg *dsa_rsp = NULL;
	dsa_ack_msg *dsa_ack = NULL;
	dsx_rvd_msg *dsx_rvd = NULL;
	switch(type)
	{
		case DSA_REQ:
			dsa_req = (dsa_req_msg*)ds_msg;
			printf("DSA-REQ: trans_id: %d\n", dsa_req->trans_id);
			print_tlv_sf(dsa_req->tlv_sf);
			break;
		case DSA_RSP:
			dsa_rsp = (dsa_rsp_msg*)ds_msg;
			printf("DSA-RSP: trans_id: %d, CFM code:%d\n", dsa_rsp->trans_id, dsa_rsp->cfm_code);
			print_tlv_sf(dsa_rsp->tlv_sf);
			break;
		case DSX_RVD:
			dsx_rvd = (dsx_rvd_msg*)ds_msg;
			printf("DSX-RVD: trans_id: %d, CFM code:%d\n", dsx_rvd->trans_id, dsx_rvd->cfm_code);
			break;
		case DSA_ACK:
			dsa_ack = (dsa_ack_msg*)ds_msg;
			printf("DSA-ACK: trans_id: %d, CFM code:%d\n", dsa_ack->trans_id, dsa_ack->cfm_code);
			print_tlv_sf(dsa_ack->tlv_sf);
			break;
		default:
			printf("Can't print this message type");
			break;
	}
	return 0;
}

int print_tlv_sf(struct tlv_sf_mgmt_encoding *tlv_sf)
{
	assert(tlv_sf != NULL);
	printf("Type: %d, Overall Length: %d\n", tlv_sf->type, tlv_sf->length);
	struct tlv_info *tlv = tlv_sf->encapTLV;
	int count = 0;
	while(tlv != NULL)
	{
		if (tlv->type == 3) //service class name
		{
			printf("\nTLV no. %d, Type: %d, Length: %d, Value: %s", ++count, tlv->type, tlv->length, (char*)tlv->value);
		}
		else
		{
			printf("\nTLV no. %d, Type: %d, Length: %d, Value: %d", ++count, tlv->type, tlv->length, *((int*)tlv->value));
		}
		tlv = tlv->next;
	}
	return 0;
}*/
