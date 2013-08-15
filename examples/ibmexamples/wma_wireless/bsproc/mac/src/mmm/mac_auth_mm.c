
//This file contains functions that initialize PKM_REQ and PKM_RSP messages. (Their names start with init_")

//build_* functions are used to construct a Management message from the internal PKM_MSG format. The output of the build_* functions can be enqueued to the MAC transmission queue for on-air transmission.

//parse_* functions are used at the receiver. They do the reverse of build_*. They reconstruct pkm messages from the transmitted payload and give back structures of the type pkm_msg*.

//There are 9 functions here. For each of Auth_request (PKM_REQ), Auth_reply and Auth_reject, there is an init, build and parse function respectively.

#include "mac_auth.h"
//#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <mac.h>
#include "ul_mgt_msg_queue.h"
#include "flog.h"
#include "dl_exp_params.h"

static int pkm_id;
int init_pkm_req(struct ss_security_info* ss_details, struct pkm_msg *pkm_req_msg)
{
	int length;

	pkm_req_msg->mgm_msg_type = 9; //Management type of PKM_REQ
	pkm_req_msg->code = 4; //Auth_request code
	if (ss_details->resends_left == NUM_PKM_REQ_RETRIES)
	{
		pkm_req_msg->pkm_id = pkm_id;
		ss_details->latest_pkm_id = pkm_id;
		pkm_id = (pkm_id + 1)%256;
	}
	else
	{
		pkm_req_msg->pkm_id = (pkm_id - 1)%256;
		

	}
	//if (ss_details->resends_left == NUM_PKM_REQ_RETRIES) pkm_req_msg->pkm_id = pkm_id; //SS-specific PKM Id
	//else pkm_req_msg->pkm_id = (pkm_id - 1)%256;

//Internal Book-keeping
	//ss_details->latest_pkm_id = pkm_id;

//Updating for next use.
	//if (ss_details->resends_left == NUM_PKM_REQ_RETRIES)	pkm_id = (pkm_id + 1)%256; //Updating SS's local PKM Id
	
	pkm_req_msg->tlv_pkm = (struct tlv_sf_mgmt_encoding*) malloc(sizeof(struct tlv_sf_mgmt_encoding));

	pkm_req_msg->tlv_pkm->type = 145; //Denotes TLV of an Uplink Message	
	
	length = 0;

	//Auth_request should have 3 TLV type - SS Certificate, Security Capabilities and SAID. Refer to Section 11.9 of WiMAX Standard Draft Rev5-D2
	struct tlv_info* curp;

	curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	
	pkm_req_msg->tlv_pkm->encapTLV = curp;

	//Encoding SS Certificate first.
	curp->type = 18; //18 is the type for SS Certificate.

	curp->length = ss_details->cert_file_length;
	//curp->length = sizeof(X509); //This value is 104 (< 128). So one byte is sufficient for X509.
	u_char *tmp0 = (u_char*)malloc(curp->length);

	memcpy(tmp0,ss_details->ss_cert_file, curp->length);
	//memcpy(tmp0,ss_details->ss_cert,sizeof(X509));
	//X509_print_fp(stdout, ss_details->ss_cert);
	curp->value =  tmp0;
	//printf("init : Curp length %d %d\n",curp->length,ss_details->cert_file_length);
	if (curp->length <= 127)
	{
		length = length + curp->length + 2;
	}
	else if(curp->length <= 255)
	{
		length = length + curp->length + 3;
	}
	else if (curp->length <= 65535)
	{
		length = length + curp->length + 4;
	}
	else
	{
		FLOG_ERROR("Certificate file is unexpectedly long! \n");
	}
	//Encoding Security Capabilities.

	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;
	curp->type = 19;

	curp->length = ss_details->num_suites_available*5 + 2;
	struct tlv_info* tmp1_2;
	struct tlv_info* tmp1 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp->value = tmp1;
	tmp1->type = 21;
	tmp1->length = ss_details->num_suites_available*5; // There are only 10 possible suites in WiMAX. So this can be atmost 50.
	tmp1_2 = tmp1;
	tmp1 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	tmp1_2->value = tmp1;
		
	struct crypto_suite* tmp2 = ss_details->suite_list;
	int tmp3 = ss_details->num_suites_available;
	u_char* tmp_char;
	while (tmp2 != NULL)
	{
		if (tmp3 == 0) FLOG_DEBUG("Wrong Suite list counter. \n");//Sanity Check
		tmp1->type = 20; //Cryptographic Suite TLV
		tmp1->length = 3;
		tmp_char = (u_char*)malloc(tmp1->length);
		
		(*tmp_char) = tmp2->data_encryption_algo;
		(*(tmp_char+1)) = tmp2->data_auth_algo;
		(*(tmp_char+2)) = tmp2->tek_encryption_algo;
		tmp1->value = tmp_char;
		if (tmp2->next != NULL)		
		{
			tmp1->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			tmp1 = tmp1->next;
		}
		else
		{
			tmp1->next = NULL;
		}
		tmp2 = tmp2->next;
		tmp3--;
	}
	
	length += curp->length + 2;

	//Encoding SAID
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;
	curp->type = 12;//TLV Type for SAID
	curp->length = 2;//TLV Length for SAID
	u_int16_t* tmp4 = (u_int16_t*)malloc(curp->length);
	*tmp4 = ss_details->said;
	curp->value = tmp4;

	length = length + curp->length + 2;

	curp->next = NULL;
	pkm_req_msg->tlv_pkm->length = length;

	return 0;
}

int build_pkm_req(struct pkm_msg* pkm_req, u_char* payload, int* length)
{

//This function takes the pkm_msg type and creates a unsigned char payload out of it.
	u_char* cur_p;
	struct tlv_info* curp;
	struct tlv_info* tmp1;
	*length = 0;
	//u_char* tmp_char;
	u_int16_t* tmp_16;

	//Management message type	
	cur_p = payload;
	(*cur_p) = (u_char) (pkm_req->mgm_msg_type);
	(*length)++;


	//Code
	cur_p++;
	(*cur_p) = (u_char)(pkm_req->code);
	(*length)++;

	//PKM ID
	cur_p++;

	(*cur_p) = pkm_req->pkm_id;
	(*length)++;
	
	cur_p++;	
	//TLV Fields encoding
	(*cur_p) = pkm_req->tlv_pkm->type;
	(*length)++;

	cur_p++;
	
	//length
	//If the length is less thn 128, one byte of length field is enough. If not, it has to be more, as defined in Chapter 11 of WiMAX standard.
	if (pkm_req->tlv_pkm->length <=0x7f)
	{
		//Length <= 127
		(*cur_p) = pkm_req->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_req->tlv_pkm->length <=0xff)
	{
		//128<= length <= 255
		(*cur_p) = 0x81;
		(*length)++;
		cur_p++;
		(*cur_p) = pkm_req->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_req->tlv_pkm->length <= 0xffff)
	{
		//255 <= length <= 65535

		(*cur_p) = 0x82;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = (pkm_req->tlv_pkm->length & 0xff);
		(*length)++;
	}
	else if (pkm_req->tlv_pkm->length <= 0xffffff)
	{
		(*cur_p) = 0x83;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length ) &0xff);
		(*length)++;
	}
	else
	{
		(*cur_p) = 0x84;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 24) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_req->tlv_pkm->length ) &0xff);
		(*length)++;
	}	
	curp = pkm_req->tlv_pkm->encapTLV;
	cur_p++;
	//Have encoded TLV Length and type. Now onto the TLV Info structure.
	assert(curp != NULL); //Sanity Check

	while (curp != NULL)
	{
		(*cur_p) = curp->type;
		(*length)++;
		cur_p++;
		switch(curp->type)
		{
			//If TEK is introduced ever, add more cases here and accommodate further TLVs
			case 19:
				//19 is TLV Security Capabilities - Its a compound TLV
				(*cur_p) = curp->length; //Length of the compound TLV
				(*length)++;
				cur_p++;

				//This processes the Crypto-suite-list sub-TLV.
				tmp1 = (struct tlv_info*)curp->value;
				(*cur_p) = tmp1->type;
				(*length)++;
				cur_p++;

				(*cur_p) = tmp1->length;	
				(*length)++;
				cur_p++;

				//Now to process each Crypto-suite TLV LL, created in the init function.
				tmp1 = (struct tlv_info*)tmp1->value;
				while (tmp1!= NULL)
				{
					//Until there is a crypto-suite in the list, keep encoding it.
					(*cur_p) = tmp1->type;cur_p++;
					(*cur_p) = tmp1->length;cur_p++;
					(*length) += 2;

					memcpy(cur_p, tmp1->value,tmp1->length);
					(*length)+=tmp1->length;
					cur_p += tmp1->length;		
					tmp1 = tmp1->next;
				}
				break;
					
			case 18:
				//Encoding SS Cert.
				if (curp->length <= 127)
				{
					(*cur_p) = (u_char)curp->length;
					(*length)++;
					cur_p++;
				
				}
				if (curp->length > 127 && curp->length <= 255)
				{
					(*cur_p) = 0x81;
					(*length)++;
					cur_p++;

					(*cur_p) = curp->length;
					(*length)++;
					cur_p++;
				}
				else if (curp->length > 255 && curp->length <= 65535)
				{
					(*cur_p) = 0x82;
					(*length)++;
					cur_p++;
					
					(*cur_p) = ((curp->length >> 8) &0xff);
					(*length)++;
					cur_p++;
					(*cur_p) = (curp->length & 0xff);
					(*length)++;
					cur_p++;
					
				}
				else
				{
					FLOG_ERROR("Unexpectedly long Certificate %d\n",curp->length);
				}
/*
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
*/
				memcpy(cur_p,curp->value,curp->length);
				*length = *length + curp->length;
				
				cur_p = cur_p+curp->length ;

				break;

			case 12:
				//SAID
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
				tmp_16 = (u_int16_t*)curp->value;
				(*cur_p) = (u_char) (((*tmp_16) >>8) &0xff);
				(*length)++;
				cur_p++;
				(*cur_p) = (u_char) ((*tmp_16) &0xff);
				(*length)++;
				cur_p++;
		
				break;
				
			default : FLOG_ERROR("Unexpected TLV Type in PKM REQ\n");break;

		}
		curp = curp->next;
	}	
		
	
	return 0;
}
int parse_pkm_req(u_char* payload, int length, struct pkm_msg* pkm_req)
{

//This function is to be used in the receiver to convert the transmitted MM payload back to pkm_msg format.

	int cur_idx =0;int i;
	int tlv_length_nums;
	struct tlv_info* curp;
	struct tlv_info* tmp6;
	u_int16_t* tmp1;
	u_char* tmp2;
	struct tlv_info* tmp3;
	int num_suites;
	int count = 0;

	//Sanity check
	if (payload[cur_idx] != PKM_REQ)
	{
		FLOG_ERROR("Parse pkm req: Message inputted is not of type PKM_REQ\n");
		return -1;
	}

//Mgm type, code and pkm id being re-constructed.
	pkm_req->mgm_msg_type = payload[cur_idx];
	cur_idx++;

	pkm_req->code = payload[cur_idx];
	cur_idx++;

	pkm_req->pkm_id = payload[cur_idx];
	cur_idx++;

//Now, to reconstruct the TLV encodings.
	pkm_req->tlv_pkm =  (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	pkm_req->tlv_pkm->type = payload[cur_idx++];
	//printf("TLV PKM Type %d\n",pkm_req->tlv_pkm->type);

//Judging the length of the TLV.
	if (payload[cur_idx]>>7)
	{
		tlv_length_nums = payload[cur_idx] & 0x7f;//The number of bytes into which length fied was encoded.
		switch(tlv_length_nums)
		{

			case 1:
				pkm_req->tlv_pkm->length = (int)payload[++cur_idx];
				break;
			case 2:
				cur_idx++;
				pkm_req->tlv_pkm->length = ((int)payload[cur_idx] <<8) + (int)payload[cur_idx+1];
				cur_idx++;
				break;
			case 3:
				cur_idx++;
				pkm_req->tlv_pkm->length = ((int)payload[cur_idx] <<16) + ((int)payload[cur_idx+1]<<8) + (int)payload[cur_idx+2];
				cur_idx+=2;
				break;
			case 4:
				cur_idx++;	
				pkm_req->tlv_pkm->length = ((int)payload[cur_idx] <<24) + ((int)payload[cur_idx+1]<<16) + ((int)payload[cur_idx+2]<<8) + (int)payload[cur_idx+3];
				cur_idx+=3;
				break;
			default :
				FLOG_ERROR("Wrong length for TLV_PKM in PKM REQ %d\n",tlv_length_nums);
				return -1;
		}
	}
	else
	{
		pkm_req->tlv_pkm->length = (int)payload[cur_idx];
	}
	cur_idx++;

	//printf("Total TLV Length %d\n",pkm_req->tlv_pkm->length);	
//Initializing TLV Datastructures
	pkm_req->tlv_pkm->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));	
	memset(pkm_req->tlv_pkm->encapTLV,0,sizeof(struct tlv_info));
	curp = pkm_req->tlv_pkm->encapTLV;

	count = 0;
	while (count < pkm_req->tlv_pkm->length)
	{
		//Use the TLV Length as an indication to decide whether we have parsed the MM fully.
		
		//printf("\nCount is %d\n",count);
		curp->type = payload[cur_idx++];
		count++;
		if (payload[cur_idx]>>7 )
		{
			if (curp->type != 18)
			{
				FLOG_ERROR("Size more than 127 is unexpected here. Type is %d\n",curp->type);
				return -1;
			}
			else
			{
				if (payload[cur_idx] == 0x81)
				{
					cur_idx++;
					count++;
					curp->length = payload[cur_idx];
					cur_idx++;
					count++;
					
				}
				else if (payload[cur_idx] == 0x82)
				{
					cur_idx++;
					count++;
					curp->length = ((u_int32_t)payload[cur_idx] <<8) + ((u_int32_t)payload[cur_idx+1]);
					printf("Length is %d\n",curp->length);	
					cur_idx += 2;
					count += 2;
				}
				else
				{
					FLOG_ERROR("Unexpectedly long Certificate\n");	
				}	

			}
		}
		else
		{
			curp->length = payload[cur_idx];
			count++;
			cur_idx++;
		}
		printf("Parse_PKM_REQ: Type and Length %d %d\n",curp->type,curp->length);
		switch(curp->type)
		{
			case 12:
				//SAID TLV
				tmp1 = (u_int16_t*)malloc(sizeof(u_int16_t));
				*tmp1 = ((u_int16_t)payload[cur_idx]<<8) + (u_int16_t)payload[cur_idx + 1];
				cur_idx += 2;
				curp->value = tmp1;
				//printf("SAID is %d\n",*tmp1);
				count += 2;
				break;

			case 18:
				//SS Certificate.
			
				//Certificate file is of variable size.
	
				tmp2 = (u_char*)malloc(curp->length);
				memcpy(tmp2, payload+cur_idx, curp->length);
				curp->value = tmp2;
				cur_idx += curp->length;
				count += curp->length;
				//X509_print_fp(stdout,(X509*)tmp2);
				break; 

			case 19:
				//Crypto suite being re-constructed.
				tmp3 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp->value = tmp3;
				tmp3->type = payload[cur_idx++];
				tmp3->length = payload[cur_idx++];
				count += 2;

				num_suites = tmp3->length/5;//Each suite is 3 bytes long. Add 2 bytes for T and L. So each suite description is 5 bytes. So num. suties can be found by dividing by 5.
				//printf("Num suites %d\n",num_suites);
				tmp3 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				tmp6 = (struct tlv_info*)curp->value;
				tmp6->value = tmp3;
				for (i=0;i<num_suites;i++)
				{
					tmp3->type = payload[cur_idx++];
					tmp3->length = payload[cur_idx++];
					tmp2 = (u_char*)malloc(tmp3->length);
					memcpy(tmp2,payload+ cur_idx,tmp3->length);
					tmp3->value = tmp2;
					count += 5;
					cur_idx+=tmp3->length;
					//printf("%x %x %x\n",(*tmp2),(*(tmp2+1)),(*(tmp2+2)));
					if (i != num_suites-1)	
					{
						tmp3->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
						tmp3 = tmp3->next;
					}
					else tmp3->next = NULL;
				}
				break;
					
			default : 
				FLOG_ERROR("Unexpected TLV type when parsing PKM_REQ %d\n",curp->type);		
				return -1;


		}
	
		//Create space for next TLV if needed.

		if (count < pkm_req->tlv_pkm->length)
		{
			curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			curp = curp->next;
		}
		else
		{
			curp->next = NULL;
		}
	}
			
	return 0;
}

	
int init_auth_reject(u_int8_t error_code,  struct pkm_msg* pkm_req_msg, u_char* display_string, int string_len, struct pkm_msg* pkm_rsp_msg)
{
	FLOG_DEBUG("Initalizing auth reject message\n");
	pkm_rsp_msg->mgm_msg_type = 10; //Management message type for PKM_RSP
	pkm_rsp_msg->code = 6;	//Auth_reject code
	pkm_rsp_msg->pkm_id = pkm_req_msg->pkm_id; //Send back with same PKM ID as the REQ

	pkm_rsp_msg->tlv_pkm = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	pkm_rsp_msg->tlv_pkm->type = 146; //TLV Code denoting Downlink
	
//Auth Reject should have TLVs corresponding to Error code, and Display string. (See Sec 11.9 of WiMAX Rev5-D2)

	int length = 0;		

	struct tlv_info* curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	pkm_rsp_msg->tlv_pkm->encapTLV = curp;

	//Encoding Error Code
	curp->type = 16; //Error code TLV Type
	curp->length = 1;
	u_int8_t* tmp1 = (u_int8_t*)malloc(curp->length);
	(*tmp1) = error_code;	
	curp->value = tmp1;

	length += curp->length + 2;
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;


	//Encoding Display string
	curp->type = 6;//Display string TLV Type
	if (string_len+1 <=127)
	{
		curp->length = string_len + 1;
		length = length + curp->length + 2;
	}
	else if (string_len+1 ==128)
	{
		curp->length = 128;
	length = length + curp->length + 3;
	}
	else
	{
		//Display string max size is 128 bytes.
		FLOG_ERROR("Service class name must be less than 128\n");
	}
	
	curp->value = (u_char*)malloc(string_len + 1);
	memcpy(curp->value,display_string,curp->length);

	curp->next = NULL;
	pkm_rsp_msg->tlv_pkm->length = length;

	return 0;
}


int build_auth_reject (struct pkm_msg* pkm_rsp, u_char* payload, int* length)
{
//This function converts the pkm_msg format of auth-reject into a unsigned char* format, which can be transmitted on air.

	u_char* cur_p;
	struct tlv_info* curp;

	*length = 0;

	//Management message type	
	cur_p = payload;
	(*cur_p) = (u_char) (pkm_rsp->mgm_msg_type);
	(*length)++;


	//Code
	cur_p++;
	(*cur_p) = pkm_rsp->code;
	(*length)++;

	//PKM ID
	cur_p++;
	(*cur_p) = pkm_rsp->pkm_id;
	(*length)++;

	cur_p++;	
	//TLV Fields

	(*cur_p) = pkm_rsp->tlv_pkm->type;
	(*length)++;

	cur_p++;
	
	//length

	if (pkm_rsp->tlv_pkm->length <=0x7f)
	{
		//length <=127
		(*cur_p) = pkm_rsp->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <=0xff)
	{
		//128<=length<=255
		(*cur_p) = 0x81;
		(*length)++;
		cur_p++;
		(*cur_p) = pkm_rsp->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <= 0xffff)
	{
		//256 <= length <= 65535
		(*cur_p) = 0x82;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = (pkm_rsp->tlv_pkm->length & 0xff);
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <= 0xffffff)
	{
		(*cur_p) = 0x83;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length ) &0xff);
		(*length)++;
	}
	else
	{
		(*cur_p) = 0x84;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 24) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length ) &0xff);
		(*length)++;
	}	
	curp = pkm_rsp->tlv_pkm->encapTLV;
	cur_p++;
	//Have encoded TLV Length and type. Now onto the TLV Info structure.
	assert(curp != NULL);

	while (curp != NULL)
	{
		(*cur_p) = (u_char) curp->type;
		(*length)++;
		cur_p++;
		switch(curp->type)
		{
			case 6:
				//Display String	
				if (curp->length <=0x7f)
				{
					(*cur_p) = (u_char) (curp->length);
					(*length)++;
				}
				else if (curp->length == 0x80)
				{
					(*cur_p) = 0x81;
					(*length)++;
					cur_p++;
					*cur_p = (u_char) (curp->length);
					(*length)++;

				}	
				else 
				{
					FLOG_ERROR("Display string can be atmost 128 bytes\n");exit(-1);	
				}	
				cur_p++;
				memcpy(cur_p,curp->value,curp->length);
				*length = *length + curp->length;
				cur_p = cur_p + curp->length;
				break;
	
			case 16:
				//Error-code encoding.
				(*cur_p) = (u_char) (curp->length);
				(*length)++;
				cur_p++;
				(*cur_p) = *((u_char*) (curp->value));
				(*length)++;
				cur_p++;
				break;	
			default : FLOG_ERROR("Unexpected TLV type in Auth_Reject\n");
		}
		curp = curp->next;

	}
	return 0;
}


int parse_auth_reject(u_char* payload, int length, struct pkm_msg* pkm_rsp)
{
	
	int cur_idx =0;
	int tlv_length_nums;
	struct tlv_info* curp;
	//u_int16_t* tmp1;
	u_char* tmp2;

	if (payload[cur_idx] != PKM_RSP)
	{
		FLOG_ERROR("In parse pkm rsp: Message is not PKM_RSP\n");
		return -1;
	}

//Re-constructing Management msg type, code ,pkm id.
	pkm_rsp->mgm_msg_type = payload[cur_idx];
	cur_idx++;

	pkm_rsp->code = payload[cur_idx];
	cur_idx++;

	pkm_rsp->pkm_id = payload[cur_idx];
	cur_idx++;
	
	//printf("Mgm type %d code %d pkm id %d\n",pkm_rsp->mgm_msg_type, pkm_rsp->code, pkm_rsp->pkm_id);

//Initialize structures to put TLVs in.
	pkm_rsp->tlv_pkm =  (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	pkm_rsp->tlv_pkm->type = payload[cur_idx++];

//Decoding TLV length. Ref. Chapter 11 on length encodings.
	if (payload[cur_idx]>>7)
	{
		tlv_length_nums = payload[cur_idx] & 0x7f;
		switch(tlv_length_nums)
		{

			case 1:
				pkm_rsp->tlv_pkm->length = (int)payload[++cur_idx];
				break;
			case 2:
				cur_idx++;
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<8) + (int)payload[cur_idx+1];
				cur_idx++;
				break;
			case 3:
				cur_idx++;
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<16) + ((int)payload[cur_idx+1]<<8) + (int)payload[cur_idx+2];
				cur_idx+=2;
				break;
			case 4:
				cur_idx++;	
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<24) + ((int)payload[cur_idx+1]<<16) + ((int)payload[cur_idx+2]<<8) + (int)payload[cur_idx+3];
				cur_idx+=3;
				break;
			default :
				FLOG_ERROR("Wrong length for TLV_PKM in PKM RSP %d\n",tlv_length_nums);
				return -1;
		}
	}
	else
	{
		pkm_rsp->tlv_pkm->length = (int)payload[cur_idx];
	}
	cur_idx++;
//Length Encoded.

	//printf("PKM RSP TLV Length %d\n",pkm_rsp->tlv_pkm->length);	
	pkm_rsp->tlv_pkm->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));	
	memset(pkm_rsp->tlv_pkm->encapTLV,0,sizeof(struct tlv_info));
	curp = pkm_rsp->tlv_pkm->encapTLV;

	int count = 0;
	while (count < pkm_rsp->tlv_pkm->length)
	{
		//Using count to find when message is fully parsed.
		curp->type = payload[cur_idx++];
		count++;
		if (payload[cur_idx]>>7)
		{
			//Only display string can have this. And its length is 128 bytes. So only one extra byte of length.
			if (payload[cur_idx] != 0x81) 
			{
				FLOG_ERROR("Not an expected TLV length in Auth reject\n");
				return -1;
			}
			curp->length = payload[++cur_idx];
			count+=2;
			cur_idx++;
			
		}
		else
		{
			curp->length = payload[cur_idx];
			count++;
			cur_idx++;
		}
		//printf("Type length %d %d\n",curp->type, curp->length);
		switch(curp->type)
		{
			case 6:
				//Display String.
				tmp2 = (u_char*)malloc(curp->length);
				memcpy(tmp2,payload+cur_idx,curp->length);
				cur_idx += curp->length;
				curp->value = tmp2;
				count += curp->length;
				//printf("%s\n",tmp2);
				break;

			case 16:
				//Error code.
				tmp2 = (u_char*)malloc(curp->length);
				*tmp2 = payload[cur_idx];
				curp->value = tmp2;
				count++; cur_idx++;
				//printf("%d\n",*tmp2);
				break;
			default : FLOG_ERROR("Unexpected TLV Type in Auth Reject\n");
		}

		if (count < pkm_rsp->tlv_pkm->length)
		{
			curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			curp = curp->next;
		}
		else
		{
			curp->next = NULL;
		}
	}
	return 0;
}
int init_auth_reply(struct pkm_msg* pkm_req_msg, struct ss_security_info* ss_details,  struct pkm_msg* pkm_rsp_msg)
{
	int bufsize;
	struct tlv_info* tmp1;
	u_int16_t *tmp_16;
	u_char* tmp_char;

	pkm_rsp_msg->mgm_msg_type = 10; //PKM_RSP Mgmt Msg type
	pkm_rsp_msg->code = 5; //Auth Reply code.
	pkm_rsp_msg->pkm_id = pkm_req_msg->pkm_id;//Same PKM Id as REQ.

	
	pkm_rsp_msg->tlv_pkm = (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	pkm_rsp_msg->tlv_pkm->type = 146; //Downlink TLV
	
	int length = 0;		

	struct tlv_info* curp = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	int next_seqno;
	pkm_rsp_msg->tlv_pkm->encapTLV = curp;

//Auth_reply should contain TLVs corresponding to Auth-Key (RSA Encrypted), key lifetime, AK Seq No, SA Descriptor. SA Descriptor is a compound TLV which contains 1) SAID 2) SA-Type 3) Crypto suite used. 

	//Encoding  AUTH-key
	curp->type = 7;//Auth Key TLV Type
	curp->length = 128; //Because RSA output is 128 bytes or 1024 bits

	length = length + curp->length + 3;

	u_char* enc_key = (u_char*)malloc(128); //Assuming RSA Modulus of 1024 bits (which is 128 bytes).

	//Getting Public Key from Certificate using Standard Openssl Functions. Then doing RSA Encryption of the AK using this public key.
	next_seqno = (ss_details->current_seq_no + 1)%4;
	EVP_PKEY * pkey = X509_get_pubkey(ss_details->ss_cert);
	//X509_print_fp(stdout,ss_details->ss_cert);
	if (pkey == NULL) printf("Pkey is null\n");
	//usleep(100);
	RSA* rsa_public_key = EVP_PKEY_get1_RSA(pkey);
	//RSA* rsa_public_key = pkey->pkey.rsa;
	bufsize = RSA_public_encrypt(KEYLEN,ss_details->akey[next_seqno],enc_key,rsa_public_key,RSA_PKCS1_OAEP_PADDING);
	if (bufsize == -1)
	{
		FLOG_ERROR("RSA Encryption of key failed.\n");
		pkm_rsp_msg = NULL;
		return -1;
	}
 	RSA_free(rsa_public_key);	
	EVP_PKEY_free(pkey);
/*
	int i;
	for (i=0;i<64;i++)
	{
		printf("%d\t",enc_key[i]);
	}
*/
	curp->value = enc_key;
	
	//Encoding key lifetime
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;
	curp->type = 9;// TLV Type for Key Lifetime
	curp->length = 4;//4 bytes. Value to be sent in seconds. So this is enough.
	u_int32_t *kl_ptr = (u_int32_t*)malloc(curp->length);
	*kl_ptr = (u_int32_t) (param_KEY_LIFE_DURATION/1000000);
	curp->value = kl_ptr;

	length = length + curp->length + 2;
	//Encoding AK Seq Number
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;
	curp->type = 10;//AK Seq Number TLV Type
	curp->length = 1;

	length = length +  curp->length + 2;

	u_int8_t *aksn_ptr = (u_int8_t*)malloc(curp->length);
	*(aksn_ptr) = next_seqno;//ss_details->current_seq_no;
	curp->value = aksn_ptr;

	//Encoding SA Descriptor
	curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	curp = curp->next;
	curp->type = 23; //SA Descriptor TLV Type

	curp->length = 0; 
	curp->value = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	tmp1 = curp->value;
	
	//Encoding Compound Value 1 : SAID
	tmp1->type = 12;//SAID TLV Type
	tmp1->length = 2;
	tmp_16 = (u_int16_t*)malloc(tmp1->length);
	*(tmp_16) = ss_details->said;
	tmp1->value = tmp_16;
	curp->length+= tmp1->length + 2; 
	tmp1->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	tmp1 = tmp1->next;
	
	//Compound Value 2: SA-Type
	tmp1->type = 24; //SA-Type TLV Type
	tmp1->length = 1;
	tmp_char = (u_char*)malloc(tmp1->length);
	(*tmp_char) = ss_details->sa_type;
	tmp1->value = tmp_char;
	curp->length += tmp1->length + 2;	
	tmp1->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
	tmp1 = tmp1->next;

	//Compound Value 3: SA Service Type is ignored. So far only primary SAIDs are in our scope.
	//Compound Value 4: Cryptographic-Suite
	tmp1->type = 20; //This is the crypto suite that BS picks for use from SS's suite list. Hence 				only one suite.
	tmp1->length = 3;
	tmp_char = (u_char*) malloc(3);
	(*tmp_char) = ss_details->suite_used.data_encryption_algo;
	(*(tmp_char+1))= ss_details->suite_used.data_auth_algo;
	(*(tmp_char+2)) = ss_details->suite_used.tek_encryption_algo;
	tmp1->value = tmp_char;
	curp->length += tmp1->length + 2;
	tmp1->next = NULL; 

 
	length = length + curp->length + 2;



	curp->next = NULL;
	
	pkm_rsp_msg->tlv_pkm->length = length;
	//ERR_remove_state();
	//EVP_cleanup();
	//CRYPTO_cleanup_all_ex_data();
	return 0;
}

int build_auth_reply(struct pkm_msg* pkm_rsp, u_char* payload, int* length)
{
//This function converts the pkm_msg format of auth-reply into a unsigned char* format, which can be transmitted on air.

	u_char* cur_p;
	struct tlv_info* curp;
	u_int32_t* tmp_32;
	u_int16_t* tmp_16;
	*length = 0;
	struct tlv_info* tmp_info;
	u_char* tmp_char;

	//Management message type	
	cur_p = payload;

	(*cur_p) = (u_char) (pkm_rsp->mgm_msg_type);
	(*length)++;


	//Code
	cur_p++;

	(*cur_p) = pkm_rsp->code;
	(*length)++;

	//PKM ID
	cur_p++;

	(*cur_p) = pkm_rsp->pkm_id;
	(*length)++;
		
	cur_p++;
	//TLV Fields

	(*cur_p) = pkm_rsp->tlv_pkm->type;
	(*length)++;

	cur_p++;
	
	//length
	//Encoding the length field as defined in Chapter 11 of WiMAX std, according to the value it takes.
	if (pkm_rsp->tlv_pkm->length <=0x7f)
	{
		//length<=127
		(*cur_p) = pkm_rsp->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <=0xff)
	{
		//128 <= length <= 255
		(*cur_p) = 0x81;
		(*length)++;
		cur_p++;
		(*cur_p) = pkm_rsp->tlv_pkm->length;
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <= 0xffff)
	{
		// 256 <= length <= 65535
		(*cur_p) = 0x82;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = (pkm_rsp->tlv_pkm->length & 0xff);
		(*length)++;
	}
	else if (pkm_rsp->tlv_pkm->length <= 0xffffff)
	{
		(*cur_p) = 0x83;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length ) &0xff);
		(*length)++;
	}
	else
	{
		(*cur_p) = 0x84;
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 24) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 16) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length >> 8) &0xff);
		(*length)++;
		cur_p++;
		(*cur_p) = ((pkm_rsp->tlv_pkm->length ) &0xff);
		(*length)++;
	}	
	curp = pkm_rsp->tlv_pkm->encapTLV;
	cur_p++;
	//Have encoded TLV Length and type. Now onto the TLV Info structure.
	assert(curp != NULL);
//int i;
	while (curp != NULL)
	{
		(*cur_p) = curp->type;
		(*length)++;
		cur_p++;
		switch(curp->type)
		{
			case 7:
				//RSA Encrypted AK.
				//assuming that RSA encryption is 1024 bit
				if (curp->length != 0x80) 
				{
					FLOG_ERROR("RSA Modulus is assumed to be atmost 1024-bits but that is not the length assigned here. This is an error.\n");exit(-1);
				}
				(*cur_p) = 0x81;
				(*length)++;
				cur_p++;
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
				memcpy(cur_p,curp->value,curp->length);
				*length = *length + curp->length;
				cur_p = cur_p+curp->length ;
				break;
			case 9:
				//Key Lifetime in seconds.
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
				tmp_32 = (u_int32_t*)curp->value;
				(*cur_p) = (u_char) (((*tmp_32) >>24)&0xff);
				(*length)++;
				cur_p++;
				(*cur_p) = (u_char) (((*tmp_32) >>16)&0xff);
				(*length)++;
				cur_p++;
				(*cur_p) = (u_char) (((*tmp_32) >>8)&0xff);
				(*length)++;
				cur_p++;
				(*cur_p) = (u_char) ((*tmp_32)&0xff);
				(*length)++;
				cur_p++;
				break;
				
			case 10:
				//Key Sequence number in use.
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
				tmp_char = (u_char*)curp->value;
				(*cur_p) = (u_char) (*tmp_char);
				(*length)++;
				cur_p++;
				//(*cur_p) = (u_char) ((*tmp_16)&0xff);
				//(*length)++;
				//cur_p++;
				break;
			case 23:
				//SA _descriptor. This is a compound TLV containing SAID, SA-type and Crypto-suite chosen by BS to be used.
				(*cur_p) = curp->length;
				(*length)++;
				cur_p++;
				tmp_info = (struct tlv_info*)curp->value;
			
				//Encoding SAID
				(*cur_p) = tmp_info->type; //Type of the sub_TLV: SAID
				(*length)++;
				cur_p++;
				(*cur_p) = tmp_info->length;//Length of sub_TLV
				(*length)++;
				cur_p++;
				tmp_16 = (u_int16_t*)tmp_info->value; //SAID is 16-bitds.
				(*cur_p) = (u_char) (((*tmp_16) >>0x08) &0xff);
				(*length)++;
				cur_p++;
				(*cur_p) = (u_char) ((*tmp_16)&0xff);
				(*length)++;
				cur_p++;

				tmp_info = tmp_info->next;

				//Encoding SA-Type
				(*cur_p) = tmp_info->type;
				(*length)++;
				cur_p++;
				(*cur_p) = tmp_info->length;
				(*length)++;
				cur_p++;
				tmp_char = (u_char*)tmp_info->value;
				(*cur_p) = (*tmp_char);
				(*length)++;		
				cur_p++;
				tmp_info = tmp_info->next;

				//Encoding Crypto-suite used
				(*cur_p) = tmp_info->type;
				(*length)++;
				cur_p++;
				(*cur_p) = tmp_info->length;
				(*length)++;
				cur_p++;
				tmp_char = (u_char*)tmp_info->value;
				//printf("%d\n",*tmp_char);
				memcpy(cur_p,tmp_char,tmp_info->length);
				cur_p += tmp_info->length;
				(*length) += tmp_info->length;
				break;
			default: FLOG_ERROR("Invalid TLV Type for Auth_reply\n");

		}	
		curp = curp->next;
	}	
	
	return 0;
}
int free_pkm_rsp(struct pkm_msg* pkm_rsp)
{
	struct tlv_info* curp, *pcurp, *tcurp,*tpcurp;
	if (pkm_rsp == NULL) return 0;
	if (pkm_rsp->tlv_pkm == NULL) return 0;
	curp = pkm_rsp->tlv_pkm->encapTLV;
	while (curp != NULL)
	{
		pcurp = curp;
		curp = curp->next;
		if (pcurp->type == 23)
		{
			tcurp = (struct tlv_info*)pcurp->value;
			while (tcurp != NULL)
			{
				tpcurp = tcurp;
				tcurp = tcurp->next;
				free(tpcurp->value);
				free(tpcurp);
			}
			free(pcurp);
					
		}
		else
		{
			free(pcurp->value);
			free(pcurp);
		}
	}
	free(pkm_rsp->tlv_pkm);
}
int free_pkm_req(struct pkm_msg* pkm_req)
{
	struct tlv_info* curp, *pcurp, *tcurp,*tpcurp;
	if (pkm_req == NULL) return 0;
	if (pkm_req->tlv_pkm == NULL) return 0;
	curp = pkm_req->tlv_pkm->encapTLV;
	while (curp != NULL)
	{
		pcurp = curp;
		curp = curp->next;
		if (pcurp->type == 19)
		{

			tcurp = (struct tlv_info*)pcurp->value;
			if (tcurp == NULL) exit(0); 
			tcurp = (struct tlv_info*)tcurp->value;
			while (tcurp != NULL)
			{
				tpcurp = tcurp;
				tcurp = tcurp->next;
				free(tpcurp->value);
				free(tpcurp);
			}
			free(pcurp->value);
			free(pcurp);
					
		}
		else
		{
			free(pcurp->value);
			free(pcurp);
		}
	}
	free(pkm_req->tlv_pkm);
}

int parse_auth_reply(u_char* payload, int length, struct pkm_msg* pkm_rsp)
{
//This function is to be used in the receiver to convert the transmitted MM payload back to pkm_msg format.
	//int i;
	int cur_idx =0;
	int tlv_length_nums;
	struct tlv_info* curp;
	u_int16_t* tmp1;
	u_char* tmp2;
	u_int32_t* tmp3;
	struct tlv_info *tmp5;

//Sanity check.
	if (payload[cur_idx] != PKM_RSP)
	{
		FLOG_ERROR("In Parse pkm rsp: Message is not PKM_RSP\n");
		return -1;
	}


//Reconstructing mgm msg type, code, pkm id.
	pkm_rsp->mgm_msg_type = payload[cur_idx];
	cur_idx++;
	pkm_rsp->code = payload[cur_idx];
	cur_idx++;
	pkm_rsp->pkm_id = payload[cur_idx];
	cur_idx++;
	pkm_rsp->tlv_pkm =  (struct tlv_sf_mgmt_encoding*)malloc(sizeof(struct tlv_sf_mgmt_encoding));
	pkm_rsp->tlv_pkm->type = payload[cur_idx++];

//Decoding Length.
	if (payload[cur_idx]>>7)
	{
		tlv_length_nums = payload[cur_idx] & 0x7f;
		//Number of bytes into which length is encoded.
		switch(tlv_length_nums)
		{
			case 1:
				pkm_rsp->tlv_pkm->length = (int)payload[++cur_idx];
				break;
			case 2:
				cur_idx++;
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<8) + (int)payload[cur_idx+1];
				cur_idx++;
				break;
			case 3:
				cur_idx++;
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<16) + ((int)payload[cur_idx+1]<<8) + (int)payload[cur_idx+2];
				cur_idx+=2;
				break;
			case 4:
				cur_idx++;	
				pkm_rsp->tlv_pkm->length = ((int)payload[cur_idx] <<24) + ((int)payload[cur_idx+1]<<16) + ((int)payload[cur_idx+2]<<8) + (int)payload[cur_idx+3];
				cur_idx+=3;
				break;
			default :
				FLOG_ERROR("Wrong length for TLV_PKM in PKM RSP %d\n",tlv_length_nums);
				return -1;
		}
	}
	else
	{
		pkm_rsp->tlv_pkm->length = (int)payload[cur_idx];
	}
//Length Decoded.

	cur_idx++;
	//printf("Auth Reply Mgm Msg type %d Code %d PKM Id %d\n",pkm_rsp->mgm_msg_type, pkm_rsp->code, pkm_rsp->pkm_id);	

	pkm_rsp->tlv_pkm->encapTLV = (struct tlv_info*)malloc(sizeof(struct tlv_info));	
	memset(pkm_rsp->tlv_pkm->encapTLV,0,sizeof(struct tlv_info));
	curp = pkm_rsp->tlv_pkm->encapTLV;

	int count = 0;
	while (count < pkm_rsp->tlv_pkm->length)
	{
		//printf("Count is %d\n",count);
		curp->type = payload[cur_idx++];
		count++;
		if (payload[cur_idx]>>7)
		{
			if (payload[cur_idx] != 0x81) 
			{
				return -1;
			}
			curp->length = payload[++cur_idx];
			count+=2;
			cur_idx++;
			
		}
		else
		{
			curp->length = payload[cur_idx];
			count++;
			cur_idx++;
		}
		//printf("Type and Length %d %d\n",curp->type, curp->length);
		switch(curp->type)
		{
			case 7:
				//AK
				tmp2 = (u_char*)malloc(curp->length);
				memcpy(tmp2,payload+cur_idx,curp->length);
				cur_idx += curp->length;
				curp->value = tmp2;
				count += curp->length;
				break;
			case 9:
				//Key lifetime in seconds.
				tmp3 = (u_int32_t*)malloc(curp->length);
				*tmp3 = ((u_int32_t)payload[cur_idx] << 24) + ((u_int32_t)payload[cur_idx+1] << 16) + ((u_int32_t)payload[cur_idx+2] << 8) + (u_int32_t)payload[cur_idx+3] ;
				curp->value = tmp3;
				count+=4; cur_idx+=4;
				break;
			case 10:
				//Seq number.
				tmp2 = (u_char*)malloc(curp->length);
				*tmp2 = (u_char)payload[cur_idx];
				curp->value = tmp2;
				cur_idx++;
				count++;
				break;
			case 23:
				//SA Descriptor Compound TLV
				
					//tmp4 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
					//tmp4->type = payload[cur_idx++];
					//count++;
					//tmp4->length = payload[cur_idx++];
					//count++;
					//tmp4->value = tmp5;

				//SAID TLV In this Compound TLV

				tmp5 = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				curp->value = tmp5;
				tmp5->type = payload[cur_idx++];
				count++;
				tmp5->length = payload[cur_idx++];
				count++;
				tmp1 = (u_int16_t*)malloc(tmp5->length);
				(*tmp1) = ((u_int16_t)payload[cur_idx] <<8) + (u_int16_t)payload[cur_idx+1];
				tmp5->value = tmp1;
				//printf("Case 23: SAID T  L V %d %d %d\n",tmp5->type, tmp5->length,*tmp1);
				cur_idx+=2;
				count+=2;
				
			//SA-Type Sub-TLV.	
				tmp5->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				tmp5 = tmp5->next;
				tmp5->type = payload[cur_idx++];
				tmp5->length = payload[cur_idx++];
				tmp2 = (u_char*)malloc(tmp5->length);
				(*tmp2) = payload[cur_idx++];	
				tmp5->value = tmp2;
				count+=3;
				//printf("Case 23: SA-Type T L V %d %d %d\n",tmp5->type, tmp5->length, *tmp2);

			//Crypto-suite used.
				tmp5->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
				tmp5 = tmp5->next;
				tmp5->type = payload[cur_idx++];
				tmp5->length = payload[cur_idx++];
				//printf("Crypto suite length %d\n",tmp5->length);
				tmp2 = (u_char*)malloc(tmp5->length);
				memcpy(tmp2,payload+cur_idx,tmp5->length);
				tmp5->value = tmp2;
				//printf("%d\n",*((u_char*)tmp5->value));
				count+=5;
				cur_idx +=3;
				tmp5->next = NULL;
				//printf("Case 23: Suite Used : %d %d %d\n",*tmp2,*(tmp2+1),*(tmp2+2));
				break;
					
			default : FLOG_ERROR("Unexpected TLV Type in Auth Reply %d\n",curp->type);
		}

		//Allocate more space if there are more tlvs.
		if (count < pkm_rsp->tlv_pkm->length)
		{
			curp->next = (struct tlv_info*)malloc(sizeof(struct tlv_info));
			curp = curp->next;
		}
		else
		{
			curp->next = NULL;
		}
	}

//Temp test
/*
	curp = pkm_rsp->tlv_pkm->encapTLV;
	struct tlv_info* curp2;
	while (curp != NULL)
	{
		printf("Type %d Length %d\n",curp->type, curp->length);
		if (curp->type == 23)
		{
			curp2 = (struct tlv_info*)curp->value;
			while (curp2 != NULL)
			{
				printf("Sub TLV Type %d Length %d\n",curp2->type, curp2->length);
				curp2 = curp2->next;
			}
		}
		curp = curp->next;

	}
*/
	return 0;
}
