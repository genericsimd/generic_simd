//This file contains functions related to the Authentication State Machine.
//It has a function, which will run as a separate thread, that processes received PKM messages. This function also updates the linked list called ss_security_list, which holds data about all existing SAs in the device. Also, this function sets timers as needed, using existing timer infrastructure in the MAC. key_expiry_timeout_func takes care of expiry.


#include "mac_auth.h"
#include "debug.h"
#include "thread_sync.h"
#include "app_timer.h"
#include "flog.h"
#include "dl_exp_params.h"

extern int threads_alive;
int ss_reauth_after_auth_reject(int said)
{
	//SAID is the WMB's Basic CID. Enter that as argument.
	struct ss_security_info* my_security_info = find_sa_from_said(said);
	if (my_security_info == NULL)
	{
		FLOG_ERROR("Unable to find SA for Reauthorization after Auth Reject\n");
		return -1;
	}
	reauth_func_ss((void*)my_security_info);		
	FLOG_INFO("Trying to re-authorize with WMA after Auth reject was received\n");
	return 0;
}
void clear_ss_security_list()
{
	struct ss_security_info* temp = ss_security_list;
	struct crypto_suite* temp2,*temp3;
	pthread_mutex_lock(&(security_list_lock));
	while (temp != NULL)
	{
		if (temp->suite_list)
		{
			temp2 = temp->suite_list;
			while (temp2 != NULL)
			{	
				temp3 = temp2;
				temp2 = temp2->next;
				free(temp3);
			}
		}
		if (temp->reauth_timer != NULL) 
		{
			app_timer_delete(temp->reauth_timer);
			temp->reauth_timer = NULL;
		}
		if (temp->key_timeout_timer != NULL) 
		{
			app_timer_delete(temp->key_timeout_timer);
			temp->key_timeout_timer = NULL;
		}
		if (temp->pkm_resend_timer != NULL) 
		{
			app_timer_delete(temp->pkm_resend_timer);
			temp->pkm_resend_timer = NULL;
		}
		if (temp->timer_node != NULL) 
		{
			app_timer_delete(temp->timer_node);
			temp->timer_node = NULL;
		}
		if (temp->transport_conns_timer != NULL) 
		{
			app_timer_delete(temp->transport_conns_timer);
			temp->transport_conns_timer = NULL;
		}
		ss_security_list = ss_security_list->next;
		free(temp->ss_cert_file);
		if (temp->ss_cert)X509_free(temp->ss_cert);
		free(temp);
		temp = ss_security_list;

	}
	pthread_mutex_unlock(&(security_list_lock));

//	FLOG_INFO("Security List freed up\n");

}
int ss_init_authorization(bs_ss_info* ss_info)
{
	FLOG_INFO("Beginning SS Authorization.\n");
	u_char* temp_cert;
	FILE *fp = 0;	
	char temp_file_name[200];
	memset(temp_file_name,0,200);
	strcat(temp_file_name,"/home/IoTCA/certs/");
	//create a ss_security_info node.
	struct ss_security_info* sa_info = (struct ss_security_info*)malloc(sizeof(struct ss_security_info));
	memset(sa_info,0,sizeof(struct ss_security_info));
	assert(ss_security_list == NULL);
	ss_security_list = sa_info;
	
	//Get and store cert, said etc.
	sa_info->ss_cert_file = (u_char*)malloc(MAX_CERT_FILE_LENGTH);
	char string_mac_addr[48];
	u_int64_t mac_addr = 0;
	int ii = 0;
	for (ii = 0; ii < 6; ii++)
	{
		mac_addr += (((u_int64_t)param_MY_MAC[ii])<<(5-ii)*8);
	}
	sprintf(string_mac_addr, "%llu.crt",mac_addr);
	//sprintf(string_mac_addr, "%llu.crt",ss_info->mac_addr);
	FLOG_INFO("SS_Authorization started for : mac addr of device from bs_ss_info %llu\n",ss_info->mac_addr);
	//temp_cert = CMA_Get_Certificate(string_mac_addr);
	strcat(temp_file_name, string_mac_addr);
	fp = fopen(temp_file_name,"r");
	if (fp == 0)
	{
		FLOG_ERROR("Unable to Read Device Certificate for MAC Address %llu\n",ss_info->mac_addr);
		return -1;
	}
	fseek(fp,0,SEEK_END);
	sa_info->cert_file_length = ftell(fp);
	rewind(fp);
	fread(sa_info->ss_cert_file, 1, sa_info->cert_file_length, fp);
	fclose(fp);

/*
	if (temp_cert == NULL) 
	{
		FLOG_ERROR("Unable to Read Device Certificate for MAC Address %lld\n",ss_info->mac_addr);
		return -1;
	}
	memcpy(sa_info->ss_cert, temp_cert,sizeof(X509));
*/
	
	sa_info->said = ss_info->basic_cid; 
	sa_info->num_suites_available = 1;

	sa_info->suite_list = (struct crypto_suite*)malloc(sizeof(struct crypto_suite));
	sa_info->suite_list->next = NULL;
	sa_info->suite_list->data_encryption_algo = 0x01;	
	sa_info->suite_list->tek_encryption_algo = 0x00;	
	sa_info->suite_list->data_auth_algo = 0x00;

	sa_info->first_flag = 0;
	sa_info->current_seq_no = 0%4; 
	pthread_mutex_init(&(sa_info->sa_lock),NULL);
	sa_info->resends_left = NUM_PKM_REQ_RETRIES;
	//send pkm req.
	struct pkm_msg* pkm_req = malloc(sizeof(struct pkm_msg));
	init_pkm_req(sa_info, pkm_req);
	u_char* payload = (u_char*)malloc(MAX_PKM_REQ_LEN);
	int length;
	build_pkm_req(pkm_req, payload, &length);
	u_char* payload_real = mac_sdu_malloc(length,5);
	memcpy(payload_real, payload, length);
	free(payload);
/*
//Freeing up PKM Msg
	struct tlv_info* curp = pkm_req->tlv_pkm->encapTLV;
	while (curp != NULL)
	{
		free(curp->value);
		curp = curp->next;
	}
	free(pkm_req->tlv_pkm);
*/
	free_pkm_req(pkm_req);
	free(pkm_req);
	
	int primary_cid = PRIMARY_CID_MIN_VALUE + (sa_info->said - BASIC_CID_MIN_VALUE);
	FLOG_INFO("Enqueueing PKM REQ to primary cid %d\n",primary_cid);
	enqueue_transport_sdu_queue(dl_sdu_queue, primary_cid, length, payload_real);
	//Set PKM REQ Retry Timer.
	long long int current_time = readtsc();
	long long int firing_time = current_time + PKM_RESEND_WAIT_DURATION;
	app_timer_add(firing_time, &pkm_resend_func_ss, NULL, NULL, 0, 0, (void**)&(sa_info->pkm_resend_timer), (void*)sa_info);
	sa_info->current_sa_status = AUTH_WAIT;
	FLOG_INFO("Exitting SS Init Authorization call. Waiting for Auth Response.\n");
	return 0;
}
struct ss_security_info* find_sa_from_said(u_int16_t said)
{
	struct ss_security_info* tmp_list = ss_security_list;
	pthread_mutex_lock(&(security_list_lock));
	while (tmp_list != NULL)
	{
		if(tmp_list->said == said) 
		{
			pthread_mutex_unlock(&(security_list_lock));
			return tmp_list;
		}
		tmp_list = tmp_list->next;
	}	
	pthread_mutex_unlock(&(security_list_lock));
	return tmp_list;	
}

int key_expiry_timeout_func_ss(void *node_pointer)
{
	
	struct ss_security_info* node_ptr = (struct ss_security_info*)node_pointer;
	FLOG_INFO("Entered Key Expiry Timeout Function in  SS. SAID %d\n",node_ptr->said);
#ifdef PKM_TEST
	printf("Entered Key Expiry Timeout Function in  SS. SAID %d\n",node_ptr->said);
#endif
	node_ptr->key_timeout_timer = NULL;

	//Should I check if csn + 1 has life  ?		

	node_ptr->current_sa_status = PERM_AUTH_REJECT;
	node_ptr->latest_pkm_id = 0;
	return 0;
}
int pkm_resend_func_ss(void *node_pointer)
{
	FLOG_INFO("Entered PKM REQ Resend Function\n");

	struct ss_security_info* node_ptr = (struct ss_security_info*)node_pointer;
	node_ptr->pkm_resend_timer = NULL;
	u_char* payload;	
	u_char* payload_real;
	int payload_length;
	if (node_ptr->resends_left > 0)
	{
		(node_ptr->resends_left)--;

		//Need to resend Re-auth Request.	
		struct pkm_msg* pkm_req = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
		init_pkm_req(node_ptr, pkm_req);
		payload = (u_char*)malloc(MAX_PKM_REQ_LEN);	
		build_pkm_req(pkm_req,payload, &payload_length);
	
		payload_real = (u_char*)mac_sdu_malloc(payload_length, 5);
		memcpy(payload_real, payload, payload_length);
		free(payload);
/*
//Freeing up pkm msg
		if (pkm_req->tlv_pkm)
		{
			struct tlv_info* curp = pkm_req->tlv_pkm->encapTLV;
			while (curp != NULL)
			{
				free(curp->value);
				curp = curp->next;
			}
			free(pkm_req->tlv_pkm);
		}
*/
		free_pkm_req(pkm_req);
		free(pkm_req);

		//Enqueue this PKM_REQ.
		int primary_cid = node_ptr->said - BASIC_CID_MIN_VALUE + PRIMARY_CID_MIN_VALUE;
		enqueue_transport_sdu_queue(dl_sdu_queue,primary_cid, payload_length, payload_real);

		//Set a timer within which PKM_RSP must be received. If not received, do a resend with a counter.

	
		long long int current_time = readtsc();
		long long int firing_time = current_time + PKM_RESEND_WAIT_DURATION;
		//node_ptr->resends_left = NUM_PKM_REQ_RETRIES;
		FLOG_DEBUG("Adding App Timer for PKM Resend Timeout. Timer Duration %d\n",firing_time - current_time);
		app_timer_add(firing_time, &pkm_resend_func_ss, NULL, NULL, 0, 0, (void**)&(node_ptr->pkm_resend_timer), (void*)node_ptr);
		node_ptr->current_sa_status = AUTH_WAIT;

	}	
	else
	{
		//Set state to PERM_AUTH_REJECT
		node_ptr->current_sa_status = PERM_AUTH_REJECT;
		node_ptr->latest_pkm_id = 0; 
		FLOG_INFO("No PKM RSP after repeated requests. Entering Perm Auth Reject mode and staying silent.\n");

	}

	return 0;
}
int reauth_func_ss(void* node_pointer)
{
	FLOG_DEBUG("Entering Reauthentication Func\n");
#ifdef PKM_TEST
	printf("Entering Reauthentication Func\n");
#endif
	long long int current_time;
	long long int firing_time;
	struct ss_security_info* node_ptr = (struct ss_security_info*)node_pointer;
	if (node_ptr == NULL) return -1;
	node_ptr->reauth_timer = NULL;

	u_char* payload;	
	u_char* payload_real;
	int payload_length;
	//Need to send out a Re-auth Request.	
	struct pkm_msg* pkm_req = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
	init_pkm_req(node_ptr, pkm_req);
	//printf("In Reauth Func SS\n");
	//X509_print_fp(stdout,node_ptr->ss_cert);
	payload = (u_char*)malloc(MAX_PKM_REQ_LEN);	
	build_pkm_req(pkm_req,payload, &payload_length);
	
	payload_real = (u_char*)mac_sdu_malloc(payload_length, 5);
	memcpy(payload_real, payload, payload_length);
	free(payload);

	//Enqueue this PKM_REQ.
	int primary_cid = node_ptr->said - BASIC_CID_MIN_VALUE + PRIMARY_CID_MIN_VALUE;
	enqueue_transport_sdu_queue(dl_sdu_queue,primary_cid, payload_length, payload_real);
	FLOG_INFO("PKM_REQ Transmitted \n");
//Free up the PKM Msg
/*
	if (pkm_req->tlv_pkm)
	{
		struct tlv_info* curp = pkm_req->tlv_pkm->encapTLV;
		while (curp != NULL)
		{
			free(curp->value);
			curp = curp->next;
		}
		free(pkm_req->tlv_pkm);
	}
*/
	free_pkm_req(pkm_req);
	free(pkm_req);

	//Set a timer within which PKM_RSP must be received. If not received, do a resend with a counter.
	
	current_time = readtsc();
	firing_time = current_time + PKM_RESEND_WAIT_DURATION;
	node_ptr->resends_left = NUM_PKM_REQ_RETRIES;
	if (node_ptr->pkm_resend_timer != NULL)
	{
		app_timer_delete(node_ptr->pkm_resend_timer);
		node_ptr->pkm_resend_timer = NULL;
	}
#ifdef SS_TX
	FLOG_DEBUG("Setting PKM Resend timer in Resend function. Gap is %d\n",firing_time - current_time);
	app_timer_add(firing_time, &pkm_resend_func_ss, NULL, NULL, 0, 0, (void**)&(node_ptr->pkm_resend_timer), (void*)node_ptr);
#endif
	node_ptr->current_sa_status = AUTH_WAIT;
	FLOG_DEBUG("Finished Reauthorization function for ss\n");
#ifdef PKM_TEST
	printf("Finished Reauth_func_ss\n");
#endif
	return 0;
}
int key_expiry_timeout_func(void* node_pointer)
{
	FLOG_DEBUG("Entered key expiry timeout function in BS\n");
	//printf("Entered key expiry timeout function in BS\n");
	struct ss_security_info* node_ptr = (struct ss_security_info*)node_pointer;
	if (node_ptr == NULL) return -1;
	node_ptr->timer_node = NULL;

	//printf("Continuing key expiry timeout function in BS\n");
	unsigned long long l1,l2;
	int i;
	long long int current_time;
	pthread_mutex_lock(&(node_ptr->sa_lock));

	int csn = node_ptr->current_seq_no;
	struct timeval t1 = node_ptr->key_gen_time[csn];
	l1 = t1.tv_sec*1000000LL + t1.tv_usec;
	t1 = node_ptr->key_gen_time[(csn+1)%4];
	l2 = t1.tv_sec*1000000LL + t1.tv_usec;
	gettimeofday(&t1,NULL);
	current_time = t1.tv_sec*1000000LL + t1.tv_usec;
	long long int ct2 = current_time + param_KEY_DELTA_ADD;
	if (l2 > l1)
	{
		//This will happen if there has been a PKM_REQ and new key has been sent, but transition has not happened yet.
		FLOG_DEBUG("PKM_REQ received, but not transitioned to new key yet. Transitioning now.\n");
		//Nulling the current seq number's key gen time.
		node_ptr->key_gen_time[node_ptr->current_seq_no].tv_sec = (long int )(ct2/1000000LL);
		node_ptr->key_gen_time[node_ptr->current_seq_no].tv_usec = ct2 - node_ptr->key_gen_time[node_ptr->current_seq_no].tv_sec*1000000LL;

		//Updating seq no.
		node_ptr->current_seq_no = (node_ptr->current_seq_no + 1)%4;
		
	}
	else
	{
		if (l1 + param_KEY_LIFE_DURATION <= (t1.tv_sec*1000000LL + t1.tv_usec))
		{
			FLOG_INFO("Key expiry in BS. No new keys requested. Going to Perm Auth Reject state\n");
			//Key expiry. Set status to perm_auth_reject
			//remove_sa_node(node_ptr);
			 node_ptr->current_sa_status = PERM_AUTH_REJECT;
			for (i=0;i<4;i++)
			{
				node_ptr->key_gen_time[i].tv_sec = 0;
				node_ptr->key_gen_time[i].tv_usec = 0;
			}	
		
		}
		else
		{
			//PKM Happened and key got toggled.
			//So seq no. does not need updates and SA can stay.
			//Nothing to be done.
		}
	
	}

	pthread_mutex_unlock(&(node_ptr->sa_lock));
	return 0;
}
int init_transport_conns_for_device(void *node_ptr)
{
	struct ss_security_info* my_security_info = (struct ss_security_info*)node_ptr;
	int bcid = my_security_info->said;
	my_security_info->transport_conns_timer = NULL;
	FLOG_INFO("Initalizing transport connections for SAID %d\n",bcid);
#ifdef PKM_TEST
	printf("Initalizing transport connections for SAID %d\n",bcid);
#endif
	//add_transport_conns(bcid);
	return 0;
}

struct ss_security_info*  update_sa_from_pkm_req(struct pkm_msg* pkm_req)
{
	//This function will take a pkm_req, and use it to update the relevant SA.
	X509* ss_cert;
	u_char* temp_cert_file;int temp_cert_file_length=0;
	struct crypto_suite* ss_suite_list = NULL;
	struct crypto_suite* tmp3,*next_suite;
	u_int16_t *said_ptr;
	int num_suites_reported;
	//u_char pkm_id;
	struct tlv_info* tmp1 = pkm_req->tlv_pkm->encapTLV;
	struct tlv_info* tmp2;
	u_char* tmp4;
	while (tmp1 != NULL)
	{
		switch(tmp1->type)
		{
			case 18: temp_cert_file = (u_char*)tmp1->value;
				 temp_cert_file_length = tmp1->length; 
				 break;
			case 19:
				 
				tmp2 = (struct tlv_info*) tmp1->value;		
				if (tmp2->type != 21) FLOG_ERROR("Unexpected type. Expecting Suite list\n");
				num_suites_reported = (tmp2->length)/5;//Each Suite TLV is 5 bytes = 3 bytes of value + 2 for T and L. 
				tmp2 = (struct tlv_info*)tmp2->value;
				
				if (tmp2 != NULL) 
				{
					tmp3 = (struct crypto_suite*) malloc(sizeof(struct crypto_suite));
					ss_suite_list = tmp3;
				}
				while (tmp2 != NULL)
				{
					if (tmp2->type != 20) FLOG_ERROR("Unexpected type. Expected Crypto suite only\n");
					tmp4 = (u_char*)tmp2->value;
					tmp3->data_encryption_algo = tmp4[0];	
					tmp3->data_auth_algo = tmp4[1];	
					tmp3->tek_encryption_algo = tmp4[2];
					//printf("%d %d %d\n",tmp4[0],tmp4[1],tmp4[2]);	
					tmp2 = tmp2->next;
					if (tmp2 == NULL)tmp3->next = NULL;
					else {
						tmp3->next = (struct crypto_suite*) malloc(sizeof(struct crypto_suite));	
						tmp3 = tmp3->next;
					    }
				}
				//tmp3 = NULL;
				break;	
			case 12: said_ptr = (u_int16_t*)tmp1->value;
				 break;
			default : FLOG_ERROR("Unexpected TLV Type in PKM REQ message. Ignoring message\n"); 
			printf("Unexpected TLV Type in PKM REQ message. Ignoring message\n"); break;


		}

		tmp1 = tmp1->next;
	}

	struct ss_security_info* tmp_list = ss_security_list;
	struct ss_security_info* my_security_info = NULL;
	while (tmp_list != NULL)
	{
		if (tmp_list->said == *(said_ptr))  
		{
			my_security_info = tmp_list;
			break;
		}
		tmp_list = tmp_list->next;
	}
#ifdef SS_TX
	if (my_security_info == NULL) printf("Something wrong. In Loopback, security info is NULL\n");
#else
	long long int current_time;
	long long int firing_time;
	if (my_security_info == NULL)
	{


		//This is the first time a PKM_REQ is made for this SAID.
		my_security_info = (struct ss_security_info*)malloc(sizeof(struct ss_security_info));
		//Do inits to all zeros, esp. to make sure the pointers are all NULL.
		memset(my_security_info,0,sizeof(struct ss_security_info));
		my_security_info->ss_cert_file = (u_char*)malloc(MAX_CERT_FILE_LENGTH);
		my_security_info->cert_file_length = 0;
		my_security_info->first_flag = 0;
		my_security_info->next = ss_security_list;
		ss_security_list = my_security_info;

		//my_security_info->ss_cert = NULL;
	
		pthread_mutex_init(&(my_security_info->sa_lock),NULL);
		my_security_info->resends_left = NUM_PKM_REQ_RETRIES;
		FLOG_DEBUG("Created SA node in list ss_security_list\n");
	
	}
	else
	{
#ifdef INTEGRATION_TEST
		//if (my_security_info->latest_pkm_id >= pkm_req->pkm_id)
		if(0)
		{
#else
		//if (my_security_info->latest_pkm_id > pkm_req->pkm_id)
		if (0)
		{
#endif
//if PKM_REQ is received in SS_RX, it means loopback testing.
			FLOG_ERROR("PKM_IDs dont match %d %d\n",my_security_info->latest_pkm_id, pkm_req->pkm_id);
			//This is a Resend of PKM_REQ
			//Free crypto list here
			tmp3 = ss_suite_list;
			while (tmp3 != NULL)
			{
				next_suite = tmp3->next;
				free(tmp3);
				tmp3 = next_suite;	

			} 
			return NULL;
			//If NULL is returned, this PKM is no longer processed.
		}
		//if not this is a re-auth attempt. No new allocation needed. Simply update data.
		//Since we are going to plug in ss_suite_list into SA, free the older suite_list
		tmp3 = my_security_info->suite_list;
		while (tmp3 != NULL)
		{
			next_suite = tmp3->next;
			free(tmp3);
			tmp3 = next_suite;	

		} 

	}	
	//Whether new Auth req or re-auth attempt, update SA.
	
	memcpy(my_security_info->ss_cert_file,temp_cert_file,temp_cert_file_length);
	my_security_info->cert_file_length = temp_cert_file_length;
	my_security_info->num_suites_available = num_suites_reported; 			
	my_security_info->suite_list = ss_suite_list;
	my_security_info->said = *(said_ptr);
		
	my_security_info->latest_pkm_id = pkm_req->pkm_id;
#endif
	return my_security_info;
}
#ifdef SS_TX
struct ss_security_info*  update_sa_from_auth_reject(struct pkm_msg* pkm_rsp)
{
	struct ss_security_info* tmp_list = ss_security_list;
	struct ss_security_info* my_security_info = NULL;
	struct tlv_info* tmp1;
	
	while (tmp_list != NULL)
	{
		if (pkm_rsp->pkm_id == tmp_list->latest_pkm_id)
		{
			my_security_info = tmp_list;	
			break;
		}		
		tmp_list = tmp_list->next;	

	}
	if (my_security_info == NULL)
	{
		//SA is not present but PKM_RSP is given to it.
		FLOG_ERROR("No SA Found for this PKM Id\n");
		return NULL;
	}
	//Removing any PKM_RESEND_TIMER that is present.
	if (my_security_info->pkm_resend_timer != NULL)
	{
		app_timer_delete(my_security_info->pkm_resend_timer);
		my_security_info->pkm_resend_timer = NULL;
	}
	//Updating Status
	my_security_info->current_sa_status = PERM_AUTH_REJECT;	
	tmp1 = pkm_rsp->tlv_pkm->encapTLV;
	memcpy(my_security_info->display_string,tmp1->value, tmp1->length);
	tmp1 = tmp1->next;
	my_security_info->error_code = *((u_char*)tmp1->value);
	FLOG_INFO("Authentication Attempt was Rejected.%s\n",my_security_info->display_string);
	//printf("Authentication Attempt was Rejected.%s\n",my_security_info->display_string);

//Remove any timers present.
//This prevents automatic re-auths.
	if (my_security_info->timer_node != NULL)
	{
		app_timer_delete(my_security_info->timer_node);
		my_security_info->timer_node = NULL;	
	}

	
	return my_security_info; 
}

struct ss_security_info*  update_sa_from_auth_reply(struct pkm_msg* pkm_rsp)
{
	int i;
	struct ss_security_info* tmp_list = ss_security_list;
	struct ss_security_info* my_security_info = NULL;
	struct tlv_info* tmp1;
	u_char* akey[128]; 
	int bufsize;	
	u_char* tmp_char;u_char aksn;
	u_int16_t* tmp_16;
	u_int32_t* tmp_32;u_int32_t key_lifetime;

	while (tmp_list != NULL)
	{
		if (pkm_rsp->pkm_id == tmp_list->latest_pkm_id)
		{
			my_security_info = tmp_list;	
			break;
		}		
		tmp_list = tmp_list->next;	

	}
	if (my_security_info == NULL)
	{
		//SA is not present but PKM_RSP is given to it.
		FLOG_ERROR("No SA Found for this PKM Id\n");
		return NULL;
	}
	
	//Updating SA.
	//Removing any PKM_RESEND_TIMER that is present.
	if (my_security_info->pkm_resend_timer != NULL)
	{
		app_timer_delete(my_security_info->pkm_resend_timer);
		my_security_info->pkm_resend_timer = NULL;
	}

	//RSA Modulated key.
	tmp1 = pkm_rsp->tlv_pkm->encapTLV;
	tmp_char = (u_char*)tmp1->value;
	if (tmp1->length != 128) {FLOG_ERROR("Unexpected RSA encrypted key size in PKM_RSP\n");exit(-1);}
	memcpy(akey,tmp_char,tmp1->length);

	//Key Lifetime.
	tmp1 = tmp1->next;
	if (tmp1->length != 4) {FLOG_ERROR("Unexpected length for key lifetime in PKM_RSP\n");exit(-1);}
	tmp_32 = (u_int32_t*) tmp1->value;	
	key_lifetime = *(tmp_32);	
	//AK Seq No.
	tmp1 = tmp1->next;
	tmp_char = (u_char*)tmp1->value;
	aksn = (*tmp_char);	

	tmp1 = tmp1->next;
	tmp1 = tmp1->value;
	
	tmp_16 = (u_int16_t*)tmp1->value;
	my_security_info->said = (*tmp_16);
	//printf("said %d\n",my_security_info->said);	
	tmp1 = tmp1->next;
	tmp_char = (u_char*)tmp1->value;
	my_security_info->sa_type = (*tmp_char);
	//printf("SA Type %d\n",(*tmp_char));

	tmp1 = tmp1->next;
	tmp_char = (u_char*)tmp1->value;
	//printf("C Length %d\n",tmp1->length);
	usleep(10);
	my_security_info->suite_used.data_encryption_algo = (*tmp_char);
	my_security_info->suite_used.data_auth_algo = (*(tmp_char+1));
	my_security_info->suite_used.tek_encryption_algo = (*(tmp_char+2));

	pthread_mutex_lock(&(my_security_info->sa_lock));
	if (aksn != (my_security_info->current_seq_no+1)%4 ) 
	{
		//The next seq number must be +1%4.
		if (my_security_info->first_flag == 1)FLOG_DEBUG("Unexpected Sequence number in PKM_RSP\n");
		//else my_security_info->first_flag = 1;
	} 	
	my_security_info->current_seq_no = aksn;
	pthread_mutex_unlock(&(my_security_info->sa_lock));
	
	//RSA Decrypt the AK and store.

	char string_mac_addr[48];
	u_int64_t mac_addr = 0;
	int ii = 0;
	for (ii = 0; ii < 6; ii++)
	{
		mac_addr += (((u_int64_t)param_MY_MAC[ii])<<(5-ii)*8);
	}
	sprintf(string_mac_addr, "%llu",mac_addr);
	//sprintf(string_mac_addr, "%llu",mac_addr);
	//u_int64_t mac_addr = get_macaddr_from_basic_cid(my_security_info->said);
#ifdef PKM_TEST
	printf("%s\n",string_mac_addr);
#endif
	EVP_PKEY* private_key = CMA_Get_PrivateKey(string_mac_addr, "pwd4test");		
	if (private_key == NULL) 
	{
		FLOG_ERROR("Could not read Private key for SA. Aborting Processing PKM_RSP. MAC add is %s\n",string_mac_addr);
		return NULL;

	}
	RSA* rsa_private_key = EVP_PKEY_get1_RSA(private_key);
#ifdef ENCRYPT_TEST
	for (i=0;i<64;i++)
	{
		printf("%d\t",((u_char*)akey)[i]);
	}
#endif
	pthread_mutex_lock(&(my_security_info->sa_lock));
	bufsize = RSA_private_decrypt(RSA_size(rsa_private_key), akey,my_security_info->akey[aksn], rsa_private_key, RSA_PKCS1_OAEP_PADDING);
	pthread_mutex_unlock(&(my_security_info->sa_lock));
	//bufsize = RSA_private_decrypt(64, akey,tempp, rsa_private_key, RSA_PKCS1_OAEP_PADDING);
	if (bufsize == -1)
	{
		FLOG_ERROR("RSA Decryption of AK failed for SAID %d\n",my_security_info->said);
		return NULL;
	}
#ifdef ENCRYPT_TEST
	printf("\n Decrypted AK \n");
	for (i =0 ;i< KEYLEN;i++)
	{
		printf("%d \t ",my_security_info->akey[aksn][i]);
	}
	printf("\n");
#endif
	//Store key lifetime.
	pthread_mutex_lock(&(my_security_info->sa_lock));
	gettimeofday(&my_security_info->key_gen_time[aksn], NULL);
	pthread_mutex_unlock(&(my_security_info->sa_lock));

	if (my_security_info->reauth_timer != NULL) 
	{
		app_timer_delete(my_security_info->reauth_timer);
		my_security_info->reauth_timer = NULL;
	}
	if (my_security_info->key_timeout_timer != NULL)
	{
		app_timer_delete(my_security_info->key_timeout_timer);
		my_security_info->key_timeout_timer = NULL;
	}
	//Set timer for expiry
	long long int current_time = readtsc();
	long long int firing_time = current_time + (long long int )key_lifetime*1000000LL;
#ifdef PKM_TEST
	printf("Key lifetime %d\n",key_lifetime);
#endif
//One timer to start reauth sometime before key expires. Another timer to act upon key expiry, which might happen if the reauth attempt failed due to some reason.
	app_timer_add(firing_time - param_KEY_GRACE_TIME, &reauth_func_ss, NULL, NULL, 0, 0, (void**)&(my_security_info->reauth_timer), (void*)my_security_info);
	app_timer_add(firing_time, &key_expiry_timeout_func_ss,	NULL, NULL, 0, 0, (void**)&(my_security_info->key_timeout_timer),(void*)my_security_info);
	my_security_info->current_sa_status = AUTHORIZED;

	if (my_security_info->first_flag == 0)
	{
		//This is the first time PKM_RSP is received.  Key has been created. 
		//Can open up UGS connections.
		my_security_info->first_flag = 1;
//#ifndef INTEGRATION_TEST
		current_time = readtsc();
		
		firing_time = current_time + ((long long int)my_security_info->resends_left)*PKM_RESEND_WAIT_DURATION;	
		app_timer_add(firing_time, &init_transport_conns_for_device,	NULL, NULL, 0, 0, (void**)&(my_security_info->transport_conns_timer),(void*)my_security_info);
		//add_transport_conns(my_security_info->said);	
//#endif

	}
	return my_security_info;
}
#endif
int remove_sa_node(struct ss_security_info* node)
{
	//This function is used to remove a node from a SA.
	if (node == NULL) return -1;
	FLOG_DEBUG("Entering Remove SA Node Function\n");
	if (ss_security_list == NULL) 
	{
		FLOG_DEBUG("Empty SS List in remove_sa_node\n");
		return -1;
	}

	pthread_mutex_lock(&(security_list_lock));

	struct ss_security_info* tmp = ss_security_list;
	struct ss_security_info* tmp_prev = NULL;
//Searching for node.
	while (tmp != NULL)
	{
		if (tmp == node) break;
		tmp_prev = tmp;
		tmp = tmp->next;
	}
//Verifying that node was found.
	if (tmp != node)
	{
		FLOG_DEBUG("Node not found in remove_sa_node\b");
		pthread_mutex_unlock(&(security_list_lock));
		return -1;
	}
//Disconnecting node from list.
	if (tmp_prev == NULL)
	{
		ss_security_list = ss_security_list->next;
	}
	else
	{
		tmp_prev->next = tmp->next;
	}
	//printf("Verifying correctness\n");
	//printf("node said %d\n",tmp->said);
//Freeing node up.
	struct crypto_suite* tmp1 = node->suite_list;
	struct crypto_suite* tmp2;
	while (tmp1 != NULL)
	{	
		tmp2 = tmp1->next;
		free(tmp1);
		tmp1 = tmp2;
	}
		
	free(node->ss_cert_file);
	X509_free(node->ss_cert);
//Removing Timers
		if (node->reauth_timer != NULL) 
		{
			app_timer_delete(node->reauth_timer);
			node->reauth_timer = NULL;
		}
		if (node->key_timeout_timer != NULL) 
		{
			app_timer_delete(node->key_timeout_timer);
			node->key_timeout_timer = NULL;
		}
		if (node->pkm_resend_timer != NULL) 
		{
			app_timer_delete(node->pkm_resend_timer);
			node->pkm_resend_timer = NULL;
		}
		if (node->timer_node != NULL) 
		{
			app_timer_delete(node->timer_node);
			node->timer_node = NULL;
		}
		if (node->transport_conns_timer != NULL) 
		{
			app_timer_delete(node->transport_conns_timer);
			node->transport_conns_timer = NULL;
		}
	free(node);

	pthread_mutex_unlock(&(security_list_lock));
	return 0;
}
void* pkm_msg_handler()
{
	//Handles PKM MSGs.
	struct ss_security_info*  ret_sa, *my_security_info;
	int ret,flag,i,primary_cid;
	struct pkm_msg* pkm_req;
	struct pkm_msg* pkm_rsp;
	int next_seq_no;
	int payload_length;
	u_char* pkm_rsp_payload, *payload;
	struct crypto_suite* tmp1;
	struct crypto_suite bs_suite;
	unsigned long long firing_time;
	unsigned long long current_time;
	char* temp_disp_string;
	struct tlv_info* curp;
	struct timeval t1;
	X509* formatted_cert;	
	bs_suite.data_encryption_algo = 0x01;
	bs_suite.data_auth_algo = 0x00;
	bs_suite.tek_encryption_algo = 0x00;


    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
	while(can_sync_continue())
	{
		mgt_msg *mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[PKM_MMM_INDEX]);
		if (mm == NULL) break;	
		if (mm == NULL) 
		{
			FLOG_ERROR("Received PKM Management Message pointer is NULL\n");
			break;
		}	
		switch(mm->msg_type)
		{
			case PKM_REQ:
				FLOG_INFO("Received PKM_REQ Message in PKM_MSG_Handler\n");
#ifdef PKM_TEST
				printf("Received PKM_REQ Message in PKM_MSG_Handler\n");
#endif
				#ifdef SS_TX
				#ifdef SS_RX
					#ifdef INTEGRATION_TEST
						FLOG_ERROR("PKM_REQ Unexpected in SS Mode\n");break;
					#endif
				#endif
				#endif
				pkm_req = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
				parse_pkm_req(mm->data,mm->length,pkm_req);
				if (pkm_req->code != AUTH_REQUEST) 
				{
					FLOG_ERROR("Unexpected PKM_REQ Code. Only Auth_request expected\n");
					return NULL;
				}
				
				my_security_info = update_sa_from_pkm_req(pkm_req);	
				if (my_security_info == NULL)
				{
					FLOG_ERROR("PKM Request Message TLVs were not properly formatted, or PKM ID of the message was unexpected.\n");
#ifdef PKM_TEST
					printf("PKM Request Message TLVs were not properly formatted, or PKM ID of the message was unexpected.\n");
#endif
					//If PKM ID does not match, it might mean a resend and would not need any more processing to be done.
					break;
				}
				//Verify Certificate
				//X509_print_fp(stdout, my_security_info->ss_cert);
				printf("Input length to stream reader is %d\n",my_security_info->cert_file_length);	
				formatted_cert = CMA_Get_Certificate_From_Stream(my_security_info->ss_cert_file, my_security_info->cert_file_length);	
				if (formatted_cert == NULL)
				{
					FLOG_ERROR("Could not convert Cert file for X509\n");
				}
				//ret = CMA_Verify_Certificate(my_security_info->ss_cert);
				ret = CMA_Verify_Certificate(formatted_cert);
				if (my_security_info->ss_cert)
				{
					X509_free(my_security_info->ss_cert);
				}
				my_security_info->ss_cert = formatted_cert;
				
#ifdef AUTH_REJECT_TEST
				ret = -1;	
#endif	
				temp_disp_string = CMA_GetError(ret);
				FLOG_INFO("Verification results %d. meaning %s\n",ret,temp_disp_string);
#ifdef PKM_TEST
				printf("Verification results %d. meaning %s\n",ret,temp_disp_string);
#endif
				memset(my_security_info->display_string,0,128);	
				memcpy(my_security_info->display_string, temp_disp_string, strlen(temp_disp_string));
				my_security_info->error_code = ret;
				free(temp_disp_string);	
				tmp1 = my_security_info->suite_list;

				//Verify that SS crypto suite list has an entry matching BS capabilities.
				flag = 0;
				while (tmp1 != NULL)
				{
					if (tmp1->data_encryption_algo == bs_suite.data_encryption_algo && tmp1->data_auth_algo == bs_suite.data_auth_algo && tmp1->tek_encryption_algo == bs_suite.tek_encryption_algo) flag = 1;
#ifdef PKM_TEST
					printf("Cryptosuite %d %d %d.BS Suite %d %d %d\n",tmp1->data_encryption_algo, tmp1->data_auth_algo, tmp1->tek_encryption_algo,bs_suite.data_encryption_algo, bs_suite.data_auth_algo, bs_suite.tek_encryption_algo);
#endif
					tmp1 = tmp1->next;
				}
				if (flag == 0)
				{
					FLOG_DEBUG("Could not find a matching Suite between SS Suites and BS Suite. Hence cannot initiate SA. Auth Reject\n"); 
				}
				//printf("Ret %d Flag %d\n",ret, flag);	
				if (ret == 1 && flag != 0)
				{
#ifndef SS_TX
					if (my_security_info->first_flag == 0)
					{
						current_time = readtsc();
						firing_time = current_time + NUM_PKM_REQ_RETRIES*PKM_RESEND_WAIT_DURATION;	
						app_timer_add(firing_time, &init_transport_conns_for_device, NULL, NULL, 0, 0, (void**)&(my_security_info->transport_conns_timer), (void*)my_security_info);
						my_security_info->first_flag = 1;
					}
#endif
				//Fill in other SA details.
					my_security_info->sa_type = 0;//assuming only primary said
					next_seq_no = (my_security_info->current_seq_no + 1)%4;
					//printf("Next Sequence number %d\n",next_seq_no);
					
				//For this seq number, create a key, store key
					gettimeofday(&t1, NULL);
					srand(t1.tv_usec*t1.tv_sec);
					pthread_mutex_lock(&(my_security_info->sa_lock));
					for (i=0;i<KEYLEN;i++)
					{
						my_security_info->akey[next_seq_no][i] = rand()%255;
						//printf("%d\t",my_security_info->akey[next_seq_no][i]);
					}
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					my_security_info->current_sa_status = AUTHORIZED;
					
					
					my_security_info->suite_used.data_encryption_algo = 0x01;
					my_security_info->suite_used.data_auth_algo = 0x00;
					my_security_info->suite_used.tek_encryption_algo = 0x00;

					//Create an Auth Reply
					pkm_rsp = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
					init_auth_reply(pkm_req, my_security_info, pkm_rsp);	
					pkm_rsp_payload = (u_char*)malloc(MAX_PKM_RSP_LEN);
					build_auth_reply(pkm_rsp,pkm_rsp_payload,&payload_length);
					payload = (u_char*)mac_sdu_malloc(payload_length,5);
					memcpy(payload,pkm_rsp_payload,payload_length);
					free(pkm_rsp_payload);
					
					free_pkm_rsp(pkm_rsp);
					free(pkm_rsp);
//Freeing allocs in pkm_req
/*
					if (pkm_rsp != NULL && pkm_rsp->tlv_pkm != NULL)
					{
						struct tlv_info  *tmp_tlv_info = pkm_rsp->tlv_pkm->encapTLV;
						struct tlv_info* prev_tlv_info;
						while (tmp_tlv_info != NULL)
						{
							prev_tlv_info = tmp_tlv_info;
							free(prev_tlv_info);
							tmp_tlv_info = tmp_tlv_info->next;
							
						}
						free(pkm_rsp->tlv_pkm);
					}
*/
#ifdef SS_TX
//Loopback testing only. Internal var
				struct timeval temptime = my_security_info->key_gen_time[next_seq_no];
#endif
	
					pthread_mutex_lock(&(my_security_info->sa_lock));
					gettimeofday(&(my_security_info->key_gen_time[next_seq_no]),NULL);
					//printf("\nNext seq no keygentime %llu %llu\n", readtsc(),my_security_info->key_gen_time[next_seq_no].tv_sec*1000000LL + my_security_info->key_gen_time[next_seq_no].tv_usec);
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					current_time = readtsc();
					firing_time = current_time + (long long int)param_KEY_LIFE_DURATION;	
					primary_cid = PRIMARY_CID_MIN_VALUE + (my_security_info->said - BASIC_CID_MIN_VALUE);
#ifndef PKM_RESEND_TEST			
					FLOG_INFO("Transmitting Auth Reply in response to PKM REQ.\n");		
					//printf("Transmitting Auth Reply in response to PKM REQ.\n");		
					enqueue_transport_sdu_queue(dl_sdu_queue, primary_cid, payload_length, payload);
#else
					mac_sdu_free(payload, payload_length, 5);
#endif
					//Enqueue Timer.
#ifndef SS_TX
#ifndef INTEGRATION_TEST
					app_timer_add(firing_time - param_KEY_GRACE_TIME, &reauth_func_ss, NULL, NULL, 0, 0, (void**)&(my_security_info->reauth_timer), (void*)my_security_info);
#endif
					app_timer_add(firing_time, &key_expiry_timeout_func, NULL, NULL, 0, 0, (void**)&(my_security_info->timer_node), (void*)my_security_info);
#endif

//If this is the first PKM_REQ, need to start transport conns for this guy.

#ifdef SS_TX
#ifdef SS_RX
//This will hold true only if loopback testing.
//We do not want the updates to security_info here during loopback testing. Updates should happen only when PKM_RSP is received. We had to update my_security_info anyway since the template for init_pkm_rsp uses it. Now negating those chances.

					pthread_mutex_lock(&(my_security_info->sa_lock));
					for (i=0;i<KEYLEN;i++)
					{
						my_security_info->akey[next_seq_no][i] = 0;
					}
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					my_security_info->current_sa_status = AUTH_WAIT;
					memset(&(my_security_info->suite_used),0,sizeof(struct crypto_suite));
					pthread_mutex_lock(&(my_security_info->sa_lock));
					my_security_info->key_gen_time[next_seq_no] = temptime;	
					pthread_mutex_unlock(&(my_security_info->sa_lock));

#endif
#endif
				}
				else
				{
					//Either Certificate did not verify properly or Crypto Suite did not match with BS Capabilities.
					pkm_rsp = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
					init_auth_reject(6, pkm_req,my_security_info->display_string, strlen(my_security_info->display_string), pkm_rsp);
					pkm_rsp_payload = (u_char*)malloc(MAX_PKM_RSP_LEN);
				 	build_auth_reject(pkm_rsp, pkm_rsp_payload, &payload_length);
					payload = (u_char*)mac_sdu_malloc(payload_length,5);
					memcpy(payload,pkm_rsp_payload,payload_length);
					free(pkm_rsp_payload);
//get basic cid to primary_cid;
					primary_cid = PRIMARY_CID_MIN_VALUE + (my_security_info->said - BASIC_CID_MIN_VALUE);
					//Send Auth Reject
#ifndef PKM_RESEND_TEST
					FLOG_INFO("Sending Auth Reject\n");
					enqueue_transport_sdu_queue(dl_sdu_queue, primary_cid, payload_length, payload);
#else
					mac_sdu_free(payload, payload_length, 5);
#endif
#ifndef SS_TX
					my_security_info->current_sa_status = PERM_AUTH_REJECT;
//Remove sa info
					//remove_sa_node(my_security_info);
#endif
				
				}
/*
//Freeing up PKM_REQ and PKM_RSP
				if (pkm_req->tlv_pkm)
				{
					curp = pkm_req->tlv_pkm->encapTLV;
					while (curp != NULL)
					{
						free(curp->value);
						curp = curp->next;
					}
					free(pkm_req->tlv_pkm);
				}
				free(pkm_req);
				if (pkm_rsp->tlv_pkm == NULL) FLOG_DEBUG("Unexpected. TLV PKM Is null\n");
					curp = pkm_rsp->tlv_pkm->encapTLV;
					while (curp != NULL)
					{
						free(curp->value);
						curp = curp->next;
					}
					free(pkm_rsp->tlv_pkm);
				free(pkm_rsp);
//Finished freeing up PKM Message structures
*/
				free_pkm_req(pkm_req);
				free(pkm_req);

				//printf("At break\n");
				//X509_print_fp(stdout, my_security_info->ss_cert);
				break;
				
			case PKM_RSP:
				FLOG_INFO("Received PKM RSP Message in PKM_MSG_Handler\n");
#ifdef PKM_TEST
				printf("Received PKM RSP Message in PKM_MSG_Handler\n");
#endif
				#ifndef SS_TX
				#ifndef SS_RX
					#ifdef INTEGRATION_TEST
					FLOG_ERROR("PKM_RSP Unexpected in BS Mode\n");
					#endif
					break;
				#endif
				#endif
				pkm_rsp = (struct pkm_msg*)malloc(sizeof(struct pkm_msg));
				switch(((u_char*)(mm->data))[1])
				{
					case AUTH_REPLY:
						parse_auth_reply(mm->data,mm->length,pkm_rsp);
						my_security_info = update_sa_from_auth_reply(pkm_rsp);
						if (my_security_info == NULL)
						{
							FLOG_ERROR("Update SA From Auth Reply Failed. Likely causes are : Did not find SA with the given PKM ID, or RSA Private Key could not be loaded, or RSA Private decryption failed with the loaded key.\n");
#ifdef PKM_TEST
							printf("Update SA From Auth Reply Failed. Likely causes are : Did not find SA with the given PKM ID, or RSA Private Key could not be loaded, or RSA Private decryption failed with the loaded key.\n");
#endif
							exit(-1);
						}
						else
						{
							my_security_info->resends_left = NUM_PKM_REQ_RETRIES;
						}
//TO DO : Free up PKM_RSP
						break;
					case AUTH_REJECT:
						parse_auth_reject(mm->data,mm->length,pkm_rsp);
						my_security_info = update_sa_from_auth_reject(pkm_rsp);
						if (my_security_info == NULL)
						{
							FLOG_ERROR("SA Not Found. Something wrong in the  PKM_RSP Message received\n");
							exit(-1);
						}
//TO DO: Free up PKM_RSP
						if (pkm_rsp->tlv_pkm)
						{
							curp = pkm_rsp->tlv_pkm->encapTLV;
							while (curp != NULL)
							{
								free(curp->value);
								curp = curp->next;
							}
							free(pkm_rsp->tlv_pkm);
						}
						free(pkm_rsp);
						break;
					default: 
						FLOG_ERROR("Unexpected PKM_RSP Code. Auth reply or reject  expected\n");
						return NULL;
				}
//Freeing PKM_RSP structure	
				free_pkm_rsp(pkm_rsp);
				free(pkm_rsp);	
				break;	
			default:
				FLOG_ERROR("Unknown PKM MSG type. Message code can be either PKM_REQ or PKM_RSP\n");return NULL;

		}
		//CRYPTO_cleanup_all_ex_data();
		//EVP_cleanup();
		if (mm->data) free(mm->data);
		if (mm)free(mm);
		mm = NULL;
	}
	pthread_exit((void*)0);
	return NULL;
}
	
