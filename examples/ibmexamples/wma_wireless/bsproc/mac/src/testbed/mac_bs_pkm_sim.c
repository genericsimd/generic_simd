#include <stdio.h>
#include "mac.h"
#include "mac_auth.h"
#include "StoreInterface.h"
#include "mac_sdu_queue.h"
#include "bs_ss_info.h"
#include "mac_bs_pkm_sim.h"

int simulate_pkm_req_to_bs(int bcid)
{

	int i;
	struct ss_security_info temp_info;
	int macadd;u_char string_macadd[30];struct pkm_msg pkm_req;
	u_char* payload;
	u_char* payload_real;
	int length; int primary_cid; FILE *fp = 0;
	char temp_file_name[200];
		memset(temp_file_name,0,200);
		strcat(temp_file_name, "/home/IoTCA/certs/");
		i = bcid;
		memset(&temp_info,0,sizeof(struct ss_security_info));
		temp_info.said = i;
		temp_info.resends_left = NUM_PKM_REQ_RETRIES;
		temp_info.num_suites_available = 1;
		temp_info.suite_list = (struct crypto_suite*) malloc(sizeof(struct crypto_suite));
		temp_info.suite_list->data_encryption_algo = 0x01;
		temp_info.suite_list->data_auth_algo = 0x0;
		temp_info.suite_list->tek_encryption_algo = 0x0;
		temp_info.suite_list->next = NULL;
		macadd = get_macaddr_from_basic_cid(i);
		sprintf(string_macadd,"%lld.crt",macadd);
		FLOG_INFO(" Starting Security Association for MAC Address %lld.crt\n",macadd);
		strcat(temp_file_name,string_macadd);
		printf("%s",temp_file_name);
		fp = fopen(temp_file_name,"r");
		if (fp == 0)
		{
			FLOG_ERROR("Unable to read device cert in simulate_pkm_req_to_bs\n");
			return -1;
		}
		fseek(fp,0,SEEK_END);
		temp_info.cert_file_length = ftell(fp);
		printf("Certificate file length %d\n",temp_info.cert_file_length);
		rewind(fp);
		temp_info.ss_cert_file = (u_char*)malloc(MAX_CERT_FILE_LENGTH);
		fread(temp_info.ss_cert_file, 1,temp_info.cert_file_length, fp);
		fclose(fp);
		//temp_info.ss_cert = CMA_Get_Certificate(string_macadd);
/*		
		if (temp_info.ss_cert_file == NULL)
		{
			printf("Could not load Certificate in simulate_pkm_req_to_bs\n");
			return -1;
		}
*/
		temp_info.latest_pkm_id = 0;
		
		init_pkm_req(&temp_info, &pkm_req);
		payload = (u_char*)malloc(1000);
		build_pkm_req(&pkm_req, payload, &length);
		payload_real = (u_char*)mac_sdu_malloc(length,5);
		memcpy(payload_real, payload, length);
		free(payload);
		
		primary_cid = PRIMARY_CID_MIN_VALUE + i -  BASIC_CID_MIN_VALUE;

		enqueue_transport_sdu_queue(dl_sdu_queue, primary_cid, length, payload_real);

		free(temp_info.suite_list);
		
}
