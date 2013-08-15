/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_reassembly.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_ul_reassembly.h"
#include "debug.h"
#include "dl_exp_params.h"
#include "queue_util.h"

#include "mac_auth.h"
extern int threads_alive;
extern int param_NUM_ATTACHED_PROCS;
extern pthread_mutex_t all_thread_mutex;

ul_con_thrd_args con_thread_arg[UL_CON_THREAD_NUM];
ul_con_thrd_info* con_processing_thrd=NULL;

pthread_mutex_t process_ul_con_thread_mutex;
pthread_cond_t child_con_threads_done;
//No of active ul con threads
int total_ul_con_threads;

//No. of con in current fram
int max_valid_ul_con;

//Should the burst threads be alive

void* ul_con_reassembly(void* b_args)
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
#ifdef __ENCRYPT__
    //int dummy;
    //char key[KEYLEN+1] = {102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,'\0'};
#endif

#ifdef INTEGRATION_TEST
        long long int myid=(long long int)b_args;
#else
        int myid=(int)b_args;
#endif
    ul_con_thrd_args* ba;
    ul_con_thrd_info* b_p_t;
    int i=0;
    pdu_cid_queue* con_pduq;

    while(threads_alive)
    {

        b_p_t=&(con_processing_thrd[myid]);

        // lock burst mutex
		pthread_cleanup_push((void*)pthread_mutex_unlock, (void*)&(b_p_t->con_mutex));
        pthread_mutex_lock(&(b_p_t->con_mutex));
        while(b_p_t->resume_status!= -1) {
            // wait for signal from main cps thread
           pthread_cond_wait (&(b_p_t->ready_to_process), &(b_p_t->con_mutex));
        }

        FLOG_DEBUG( "UL_MULTI: start processing reassembly myid is: %d --total connection: %d\n", myid, total_ul_con_threads);
        //printf("UL_MULTI: start processing p&f --total connection: %d\n", total_ul_con_threads);

        for(i = 0; i < max_valid_ul_con; i++)
        {
            if( ( i % param_NUM_ATTACHED_PROCS ) == myid)
            {
                //this connection is my responsibility to process
                //do processing
                ba = &(con_thread_arg[i]);
                con_pduq = ba->pdu_list;
                while(con_pduq)
                {
pthread_mutex_lock(&all_thread_mutex);


		   reassembly_per_con(ba->frame_num, con_pduq, ba->sduq, ba->fragq, ba->brq, ba->ul_msgq);


pthread_mutex_unlock(&all_thread_mutex);
               con_pduq = con_pduq->next;
                }
            }
        }

        // lock cps thread mutex
		pthread_cleanup_push((void*)pthread_mutex_unlock, (void*)&(process_ul_con_thread_mutex));
        pthread_mutex_lock(&(process_ul_con_thread_mutex));
        // signal cps thread
        total_ul_con_threads--;
        FLOG_DEBUG( "UL_MULTI: finished processing reassembly myid is: %d --total con: %d\n", myid,total_ul_con_threads);
        //printf("UL_MULTI: finished processing p&f --total con: %d\n", total_ul_con_threads);
        if(total_ul_con_threads==0) {
            FLOG_DEBUG( "UL_MULTI: all threads done reassembly myid is:%d -- total con:  %d\n", myid,total_ul_con_threads);
            //printf("UL_MULTI: all threads done p&f: %d\n", total_ul_con_threads);
            pthread_cond_signal(&child_con_threads_done);
        }
        // unlock cps thread
        pthread_mutex_unlock(&(process_ul_con_thread_mutex));
		pthread_cleanup_pop(0);

        b_p_t->resume_status=0;
        // unlock burst mutex
        pthread_mutex_unlock(&(b_p_t->con_mutex));
		pthread_cleanup_pop(0);
    }
    pthread_exit((void*)0);
}


void init_ul_con_threads(void)
{
#ifdef INTEGRATION_TEST
        long long int i = 0;
#else
        int i = 0;
#endif
    int rc=0;

    //initialize the mutexes and cond variables for bursts burst_thread_arg
    // and global variables
    total_ul_con_threads=0;
    max_valid_ul_con=0;
    pthread_mutex_init(&(process_ul_con_thread_mutex), NULL);
    pthread_cond_init(&(child_con_threads_done), NULL);
    for(i=0; i<UL_CON_THREAD_NUM; i++) {
        con_thread_arg[i].frame_num = -1;
        con_thread_arg[i].pdu_list=NULL;
        con_thread_arg[i].sduq=NULL;
        con_thread_arg[i].fragq=NULL;
        con_thread_arg[i].brq=NULL;
        con_thread_arg[i].ul_msgq=NULL;
        con_thread_arg[i].status=0;
    }

    //after initialization -- start all the threads
    con_processing_thrd = (ul_con_thrd_info*) malloc( sizeof(ul_con_thrd_info) * param_NUM_ATTACHED_PROCS );

    for(i = 0; i < param_NUM_ATTACHED_PROCS; i++)
    {
        //printf("******************initate a new ul connection thread\n");
        pthread_mutex_init(&(con_processing_thrd[i].con_mutex), NULL);
        pthread_cond_init(&(con_processing_thrd[i].ready_to_process), NULL);
        con_processing_thrd[i].resume_status=0;

        rc=pthread_create(&(con_processing_thrd[i].con_thrd), NULL, ul_con_reassembly, (void*) i);
        if(rc) {
            FLOG_ERROR("Unable to Spawn threads for Burst functions\n");
        }
    }
}

void release_ul_con_threads()
{
	int i = 0;
    for(i = 0; i < param_NUM_ATTACHED_PROCS; i++)
    {
        pthread_cancel(con_processing_thrd[i].con_thrd);
        pthread_mutex_destroy(&(con_processing_thrd[i].con_mutex));
        pthread_cond_destroy(&(con_processing_thrd[i].ready_to_process));

    }
	free(con_processing_thrd);
	//pthread_mutex_destroy(&(process_ul_con_thread_mutex));
	//pthread_cond_destroy(&(child_con_threads_done));
}

void release_thrd_pdu_cid_queue()
{
    int i;
    pdu_cid_queue* cur_cidq;
    pdu_cid_queue* pre_cidq;
    for (i=0; i< UL_CON_THREAD_NUM; i++)
    {
        if (con_thread_arg[i].pdu_list)
        {
            cur_cidq = con_thread_arg[i].pdu_list;
            while (cur_cidq)
            {
                pre_cidq = cur_cidq;
                cur_cidq = cur_cidq->next;
                free(pre_cidq);
                
            }
            con_thread_arg[i].pdu_list = NULL;
        }
    }
}

int get_ceil(double original)
{
    int later = (int)original;
    if (later != original)
    {
        later = later + 1;
    }
    return later;
}

int is_within_win(int win_start, int win_size, int cur_block, int modulo)
{

    int win_end = (win_start+win_size) % modulo;
    if (win_end > win_start)
    {
        if (cur_block >=win_start && cur_block <win_end)
        {
            return 1;
        }
        else 
        {
            return 0;
        }

    }else
    {
        if ((cur_block >= win_start && cur_block < modulo) || (cur_block < win_end))
        {
            return 1;
        }
        else 
        {
            return 0;
        }
    }

    return 0;
}
#ifdef SIMULATE_ARQ_LOSS
	#define LOSS_PROB 0.05 
#endif

// defragmentation and depacking, reassemble the sdu
int reassembly_per_con(unsigned int frame_num, pdu_cid_queue* pdu_list, sdu_queue* sduq, frag_queue* fragq, ul_br_queue* brq, mgt_msg_queue *ul_msgq){

    logical_element* list_head = pdu_list->head;
    logical_element* cur_pdu;
    logical_element* lep;

    int ret;
    int  is_gmsh_present, is_psh_present, is_fsh_present, is_arq_feedback_present, is_extend;
    int esh_group_len, left_len;
    int fixed_sdu_len;
    u_char* current_pos;
    u_char* newcontent;
    int esh_type;
    generic_mac_hdr* mac_hdr;
    pack_sub_hdr* psh;
    frag_sub_hdr* fsh;
    int payload_len, fc, fsn;
    logical_packet* sdu_pkt;
    int hdr_len, length, msg_type;
    u_int8_t is_mgtcon;
    int block_num, end_block, current_block;
    u_int8_t is_arq, is_frag, is_pack,  is_order, is_fixed_sdu;
    int arq_block_size, rx_win_size, rx_win_start, modulo;
    int i;
    int crc_len;
    int sdu_num;
    int mimo_mode_type = MIMO_MODE_FEEDBACK_ESH_TYPE;
    int ul_tx_power_type = UL_TX_POWER_REPORT_ESH_TYPE;
    int mini_feedback_type = MINI_FEEDBACK_ESH_TYPE;
    int cid = pdu_list->cid;
    int residue_len;

    struct queue_msg cs_msg;
    cs_msg.my_type = csforw_en_id;

    struct ss_security_info* my_security_info;
    long long int current_time, key_expiry_time;
    struct timeval temp_keygen_time;
    int my_basic_cid;
    int is_encrypted=0;
	
    is_encrypt_enabled(cid,(u_int8_t*)&is_encrypted);
    if (is_encrypted == 1)
    {
    	if (cid >= UGS_CID_MIN_VALUE && cid <= BE_CID_MAX_VALUE)
    	{
		get_basic_cid(cid,&my_basic_cid);       
        	my_security_info = find_sa_from_said(my_basic_cid);
        	if (my_security_info == NULL) {FLOG_ERROR("Security Info not found in reassembly_per_con\n");exit(-1);}
    	}
    }

    if (list_head == NULL)
    {
        return 0;
    }
    
    // get the connection info related with this PDU
    
	if (BROADCAST_CID == cid)
	{
		is_mgtcon = 1;
		is_arq = 0;
		is_frag = 0;
		is_pack = 0;
		is_fixed_sdu = 0;
		modulo = 0;
	}
	else
	{
		is_arq_enabled(cid, &is_arq);
		is_frag_enabled(cid, &is_frag);
		is_pack_enabled(cid, &is_pack);
		is_mgt_con(cid, &is_mgtcon);
		is_fix_sdu_length(cid, &is_fixed_sdu);
		get_modulo(cid, &modulo); 
	}
    crc_len = MAC_CRC_LEN;

    if (is_fixed_sdu)
    {
       get_sdu_size(cid, &fixed_sdu_len); 
    }
  
    if (is_arq)
    {
        is_arq_order_preserved(cid, &is_order);
        get_rx_win_size(cid, &rx_win_size);
        get_arq_block_size(cid, &arq_block_size);
        
    }
    
    cur_pdu = list_head;
    mac_hdr = (generic_mac_hdr*) malloc(sizeof(generic_mac_hdr));
    memset(mac_hdr, 0, sizeof(generic_mac_hdr));

    if (is_pack)
    {
        psh = (pack_sub_hdr*) malloc(sizeof(pack_sub_hdr));
        memset(psh, 0, sizeof(pack_sub_hdr));
    }
    else
    {
        psh = NULL;
    }

    if (is_frag || is_arq)
    {
        fsh = (frag_sub_hdr*) malloc(sizeof(frag_sub_hdr));
        memset(fsh, 0, sizeof(frag_sub_hdr));
    }
    else
    {
        fsh = NULL;
    }

    while (cur_pdu!=NULL)
    {
        current_pos = cur_pdu->data;
        // first try to find the extended subheader and grant-management subheader, put them into the management message queue
        parse_gmh(current_pos, mac_hdr, &hdr_len);

	is_encrypt_enabled(cid,(u_int8_t*)&is_encrypted);
	if (is_encrypted == 1)
	{
		pthread_mutex_lock(&(my_security_info->sa_lock));
		if (mac_hdr->eks != my_security_info->current_seq_no)
		{
			FLOG_DEBUG("AKSN in MAC header %d is different from current sequence number %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#ifdef PKM_TEST
			printf("AKSN in MAC header %d is different from current sequence number %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#endif
#ifdef SS_TX
			if (mac_hdr->eks != (u_int8_t)((int)my_security_info->current_seq_no - 1)%4)
			{
			//Discard PDU. Unexpected CSN.
				cur_pdu = cur_pdu->next;
				FLOG_WARNING("Unexpected Sequence number in PDU header. Ignoring PDU. Received AKSN was %d, current sequence number is %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#ifdef PKM_TEST
				printf("Unexpected Sequence number in PDU header. Ignoring PDU. Received AKSN was %d, current sequence number is %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#endif
				pthread_mutex_unlock(&(my_security_info->sa_lock));
				printf("Continue1\n");
				continue;
			}
			else
			{
				current_time = readtsc();
				temp_keygen_time = my_security_info->key_gen_time[mac_hdr->eks];
				key_expiry_time = temp_keygen_time.tv_sec*1000000LL + temp_keygen_time.tv_usec + param_KEY_LIFE_DURATION; 
				if (current_time < key_expiry_time) 
				{
				//Nulling Key Gen time of this CSN to prevent another update.
				//my_security_info->key_gen_time[mac_hdr->eks].tv_sec = 0;	
				//my_security_info->key_gen_time[mac_hdr->eks].tv_usec = 0;

				//Updating CSN.
					//my_security_info->current_seq_no = mac_hdr->eks;	


				}
				else
				{			
					//Discard PDU. Unexpected CSN.
					cur_pdu = cur_pdu->next;
					FLOG_WARNING("MAC Header AKSN is expired even though it is CSN - 1. Ignoring PDU.\n");
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					continue;
				}

			}	
#else
			if (mac_hdr->eks != (my_security_info->current_seq_no + 1)%4)
			{
				current_time = readtsc();
				temp_keygen_time = my_security_info->key_gen_time[mac_hdr->eks];
				key_expiry_time = temp_keygen_time.tv_sec*1000000LL + temp_keygen_time.tv_usec;
				if ((mac_hdr->eks == (u_int8_t)((int)my_security_info->current_seq_no - 1 )%4) && ((long long unsigned int)key_expiry_time >= (long long unsigned int)current_time) )
				{
					//Do nothing.
				}
				else
				{
					//Discard PDU. This is an unexpected CSN.
					cur_pdu = cur_pdu->next;
					FLOG_WARNING("Unexpected Sequence number in PDU header. Ignoring PDU. Received AKSN was %d, current sequence number is %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#ifdef PKM_TEST
				printf("Unexpected Sequence number in PDU header. Ignoring PDU. Received AKSN was %d, current sequence number is %d\n",mac_hdr->eks, my_security_info->current_seq_no);
#endif
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					printf("Continue3\n");
					continue;
				}
			}
			else
			{
				current_time = readtsc();
			
				temp_keygen_time = my_security_info->key_gen_time[mac_hdr->eks];
				key_expiry_time = temp_keygen_time.tv_sec*1000000LL + temp_keygen_time.tv_usec + param_KEY_LIFE_DURATION; 
				if (current_time < key_expiry_time) 
				{
					//This is the only valid case. Means its the first time (CSN+1)%4 is used.
					//Update CSN locally, and reset the old CSN Key lifetime
					current_time += param_KEY_DELTA_ADD;
					my_security_info->key_gen_time[my_security_info->current_seq_no].tv_sec = (long int)(current_time/1000000LL);
					my_security_info->key_gen_time[my_security_info->current_seq_no].tv_usec = current_time - my_security_info->key_gen_time[my_security_info->current_seq_no].tv_sec*1000000LL;	
					//printf("Updating KET as %lu %lu\n", my_security_info->key_gen_time[my_security_info->current_seq_no].tv_sec, my_security_info->key_gen_time[my_security_info->current_seq_no].tv_usec);
					my_security_info->current_seq_no = mac_hdr->eks;
				
				}
				else
				{
					//Again, this is an unexpected CSN. Discard PDU.
					cur_pdu = cur_pdu->next;
					FLOG_WARNING("MAC Header AKSN is expired even though it is CSN + 1. Ignoring PDU.\n");
#ifdef PKM_TEST
					printf("Expired key used to encrypt PDU. Ignoring PDU\n");
#endif
					pthread_mutex_unlock(&(my_security_info->sa_lock));
					continue;
				}
	
			}		
#endif


		}
		else
		{
			//This is the current seqno
#ifdef PKM_TEST
			printf("Current seq no only %d\n",mac_hdr->eks);
#endif
			FLOG_DEBUG("Sequence number in PDU %d\n",mac_hdr->eks);
#ifdef SS_TX
			//If this is SS, the older ak life should be reset now. Older key no longer to be entertained.
			my_security_info->key_gen_time[(my_security_info->current_seq_no -1 )%4].tv_sec = 0;
			my_security_info->key_gen_time[(my_security_info->current_seq_no -1 )%4].tv_usec = 0;

#endif
		}
		pthread_mutex_unlock(&(my_security_info->sa_lock));
	}

        left_len = mac_hdr->len;
        // the pdu length substract the generic header and crc checksum
        if (mac_hdr->ci)
        {
            left_len -= hdr_len;
            left_len -= crc_len;
        }
        else
        {
            left_len -=hdr_len;
        }
        current_pos += hdr_len;
        
        // now try to check whether there are any extended subheaders
	//u_char *temppos = current_pos;
        if (mac_hdr->esf)
        {
            esh_group_len = (u_int8_t)(current_pos[0])-1;
            current_pos++;
            while(esh_group_len >0)
            {
                esh_type = (int) (current_pos[0]);
                current_pos++;
                esh_group_len--;
                if (esh_type == mimo_mode_type)
                {
                    msg_type = MIMO_MODE_FEEDBACK_EXTENDED_SUBHEADER;
                    length = MIMO_MODE_FEEDBACK_ESH_LEN;
                }

                if (esh_type == ul_tx_power_type)
                {
                    msg_type = UL_TX_POWER_REPORT_EXTENDED_SUBHEADER;
                    length = UL_TX_POWER_REPORT_ESH_LEN;
                }

                if (esh_type == mini_feedback_type)
                {
                    msg_type = MINI_FEEDBACK_EXTENDED_SUBHEADER;
                    length = MINI_FEEDBACK_ESH_LEN;
                }

                enqueue_ul_mgt_msg(ul_msgq, frame_num, cid, msg_type, length, (void*)current_pos);
                current_pos += length;
                esh_group_len-=length;
            }
            left_len -= esh_group_len;
        }

        is_gmsh_present = (mac_hdr->type)&0x01;

        if (is_gmsh_present)
        {
            // process the grant management message in the uplink
            msg_type = GRANT_MANAGEMENT_SUBHEADER;
            length = GRANT_MANAGEMENT_SUBHEADER_LEN;
            enqueue_br_queue(brq, frame_num, cid, msg_type, length, (void*)current_pos);
            current_pos += length;
            left_len -= length;
        }

        // check the packing subheader
        is_psh_present = (mac_hdr->type)&0x02;
        // secondlyly, check the fragmentation subheader
        is_fsh_present = (mac_hdr->type)&0x04;
        // third,  check the extended type
        is_extend = (mac_hdr->type)&0x08;
        // fourthly, check if the arq feedback payload is contained
        is_arq_feedback_present = (mac_hdr->type)&0x10;
        if (is_fsh_present) 
        {
              	parse_fsh(current_pos, fsh, is_extend, &hdr_len);
		current_pos += hdr_len;
		left_len -= hdr_len;
        }

	if (is_encrypted==1)
	{
		u_char key[KEYLEN+1];

  		ret = get_decryption_key(cid,key,KEYLEN,mac_hdr->eks);	  	
		if (ret == -1)
		{
			//SA is in PERM_AUTH_REJECT state. Discard PDU.
			cur_pdu = cur_pdu->next;
			printf("Could not read private key during Decryption. Ignoring PDU.\n");
			continue;

		}
		u_char* temp;unsigned long int templen;
		unsigned char tempiv[IVLEN+1];
		memcpy(tempiv ,current_pos , IVLEN);

#ifdef ENCRYPT_TEST	
		FLOG_DEBUG("PRINTING IVs\n");
		int mmi;
		for (mmi=0;mmi<IVLEN;mmi++)
		{
			FLOG_DEBUG("%d\n",tempiv[mmi]);
		}
	
#endif

		temp = (u_char*)malloc(sizeof(u_char)*(left_len -IVLEN-TAGLEN));
 		int ret_var=0;
 		ret_var = mac_des_crypto(current_pos+IVLEN,left_len - IVLEN,tempiv,key,temp,0);//0 for decrypt
		templen = left_len - IVLEN;
		//ret_var = AES_CCM_Decrypt(tempiv,IVLEN,key,KEYLEN,NULL,0,current_pos+IVLEN,left_len-IVLEN,temp,&templen,TAGLEN);		
		if (ret_var == 0 ) 
		{
			FLOG_DEBUG("Decryption Failed. Likely that decryption key was wrong. So I am ignoring this PDU contents\n");
	 		cur_pdu=cur_pdu->next;	
			continue;
		}
		left_len -= (IVLEN+TAGLEN);
		memcpy(current_pos,temp,left_len);	
		memset(current_pos+left_len,0,IVLEN+TAGLEN);
#ifdef ENCRYPT_TEST
		FLOG_DEBUG("Printinf Decoded Data in UL \n");
		for (mmi=0;mmi<left_len;mmi++)
		{
			FLOG_DEBUG("%d\t",temp[mmi]);
		}	

		FLOG_DEBUG("\n");
#endif

 		free(temp);	

	}
	if (is_fsh_present) {current_pos -= hdr_len;left_len += hdr_len;}
#ifdef ENCRYPT_TEST
   	
	FLOG_DEBUG("AT UL : CID %d PDU LEN %d\n\n",cid, left_len);	

#endif

#ifdef SIMULATE_ARQ_LOSS 
                                    	float aa = rand(); aa = aa/RAND_MAX;
					if (aa < LOSS_PROB && cid > UGS_CID_MIN_VALUE && cid < BE_CID_MAX_VALUE) 
					{
						usleep(1000000);
					}
#endif
        if (is_psh_present || is_fsh_present)
        {

            while (left_len>0)
            {
                if (is_psh_present)
                {
                    parse_psh(current_pos, psh, is_extend, &hdr_len);
                    payload_len = psh->length;
                    fc = psh->fc;
                    fsn = psh->fsn;
                }
		else
		{
			parse_fsh(current_pos, fsh, is_extend, &hdr_len);
			fc = fsh->fc;
			fsn = fsh->fsn;
		}
                switch(fc)
                {
                    case UN_FRAGMENTED:
                        // first check whether this contains the arq feedback payload
                        if (is_arq_feedback_present )
                        {
                            // tell the arq_queue about the arq feedback
                            msg_type = ARQ_FEEDBACK_IE;
                            length = payload_len - hdr_len;
							FLOG_DEBUG("Lengths: length %d payload len %d hdr len %d\n",length, payload_len, hdr_len);
							u_int8_t* msg_payload = (u_int8_t*)malloc(length);
							memcpy(msg_payload, current_pos + hdr_len,length); 
							ARQ_feedback_message *tmp = (ARQ_feedback_message*)msg_payload;
							if (tmp->mgmt_msg_type == ARQ_FEEDBACK) parse_and_process_feedback_msg(tmp);
							else if (tmp->mgmt_msg_type == ARQ_DISCARD)process_discard_msg((ARQ_discard_message*)tmp,fragq);
							else if (tmp->mgmt_msg_type == ARQ_RESET) {process_reset_msg((ARQ_reset_message*)tmp,fragq);}
				   		    else FLOG_DEBUG("Unknown ARQ Mgmt Message\n");
                            //enqueue_ul_mgt_msg(ul_msgq, frame_num, mac_hdr->cid, msg_type, length,(current_pos + hdr_len));
                            left_len -= payload_len;
                           
                        }
                        if (is_arq)
                        {
                            if (is_psh_present)
                            {
                                block_num = get_ceil((double)( payload_len-hdr_len ) /(double)arq_block_size);
                                residue_len =  payload_len-hdr_len - (block_num -1)* arq_block_size;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                block_num = get_ceil((double)(left_len-hdr_len) /(double)arq_block_size);
                                residue_len =  left_len-hdr_len - (block_num-1) * arq_block_size;
                            }
                            current_block = fsn-1;
                            end_block = (current_block + block_num) % modulo;
                            for (i = 0; i< block_num; i++)
                            {
                                current_block = (current_block + 1) % modulo;
                                // first check the arq engine about the windwo state
                                rx_win_start = ARQ_get_rx_window_start(cid);
                                if (is_within_win(rx_win_start, rx_win_size, current_block, modulo))
                                {
                                    // insert the arq block into the frag queue
                                    lep = (logical_element*) malloc(sizeof(logical_element));
                                    memset(lep, 0, sizeof(logical_element));
                                    lep->type = ARQ_BLOCK;
                                    lep->data = current_pos+hdr_len+i*arq_block_size;
                                    if (current_block == end_block)
                                    {
                                        if (current_block == fsn)
                                        {
                                            lep->blk_type = NO_FRAGMENTATION;
                                        }
                                        else
                                        {
                                            lep->blk_type = LAST_FRAGMENT;
                                        }
                                        lep->length = residue_len;
                                    }
                                    else if (current_block == fsn)
                                    {   
                                        lep->blk_type = FIRST_FRAGMENT;
                                        lep->length = arq_block_size;
                                    }
                                    else
                                    {
                                        lep->blk_type = CONTINUING_FRAGMENT;
                                        lep->length = arq_block_size;
                                    }
                                    lep->start_bsn = current_block;
                                    enqueue_arq_fragq(fragq, frame_num, cid, lep, is_order, rx_win_start, rx_win_size, modulo, is_mgtcon, sduq);
						
				    				FLOG_DEBUG("Notifying for block %d in connection %d\n",current_block,cid);
				    				notify_received_blocks(cid, current_block,current_block);
                                    continue;
                                }
				else
				{
					//FLOG_DEBUG("In UL: received an out of window block %d\n",current_block);
				}		
                            }
                            
                            // tell the arq_queue about the arrival of these block
                        }
                        else
                        {
                            // this sdu can be parsed and directly put it into the ul_sduq, since it is not fragmented
                            if (is_psh_present)
                            {
                                length = payload_len-hdr_len;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                // if fragmentation is present, there should be only one fragments in a mac pdu
                                length =left_len-hdr_len;
                            }
                            if (is_mgtcon)
                            {
                                msg_type = UNFRAG_MANAGEMENT_MESSAGE;
                                enqueue_ul_mgt_msg(ul_msgq, frame_num, mac_hdr->cid, msg_type, length, (current_pos+hdr_len));
                            }
                            else
                            {
                                sdu_pkt = (logical_packet*) malloc(sizeof(logical_packet));
                                memset(sdu_pkt, 0, sizeof(logical_packet));
                                lep = (logical_element*) malloc(sizeof(logical_element));
                                memset(lep, 0, sizeof(logical_element));
                                // memory copy the payload of logical element
                                newcontent = (u_char *) malloc(length);
                                memcpy(newcontent, current_pos+hdr_len, length);
                                lep->data = newcontent;
                                lep->length = length;
                                lep->type = MAC_SDU;
                                lep->next = NULL;
                                sdu_pkt->element_head = lep;
                                sdu_pkt->length = lep->length;
                                sdu_pkt->next = NULL;
                                sdu_pkt->cid = mac_hdr->cid;
                                enqueue_sduq(sduq, mac_hdr->cid, sdu_pkt);

                                cs_msg.len = mac_hdr->cid;

                                if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                                    sizeof(struct queue_msg)) == -1)
                                {
                                    FLOG_ERROR("enqueue error\n");
                                }

                            }           
                	    }
                        break;
                    case FIRST_FRAGMENT:

                        if (is_arq)
                        {
                            if (is_psh_present)
                            {
                                block_num = get_ceil((double)( payload_len-hdr_len ) /(double) arq_block_size);
                                left_len -= payload_len;
                            }
                            else 
                            {
                                block_num = get_ceil((double)(left_len-hdr_len) /(double)arq_block_size);
                            }
                            current_block = fsn-1;
                            for (i = 0; i< block_num; i++)
                            {
                                current_block = (current_block + 1) % modulo;
                                // first check the arq engine about the windwo state
                                rx_win_start = ARQ_get_rx_window_start(cid);
                                if (is_within_win(rx_win_start, rx_win_size, current_block, modulo))
                                {
                                    // insert the arq block into the frag queue
                                    lep = (logical_element*) malloc(sizeof(logical_element));
                                    memset(lep, 0, sizeof(logical_element));
                                    lep->data = current_pos+hdr_len+i*arq_block_size;
                                    lep->length = arq_block_size;
                                    lep->type = ARQ_BLOCK;
                                    if (current_block == fsn)
                                    {
                                        lep->blk_type = FIRST_FRAGMENT;
                                    }
                                    else 
                                    {
                                        lep->blk_type = CONTINUING_FRAGMENT;
                                    }
                                    lep->start_bsn = current_block;
                                    enqueue_arq_fragq(fragq, frame_num, cid, lep, is_order, rx_win_start, rx_win_size, modulo, is_mgtcon, sduq);
				    				FLOG_DEBUG("Notifying for block %d in connection %d\n",current_block,cid);
				    				notify_received_blocks(cid, current_block, current_block);
				}
                                else 
                                {
                                    break;
                                }
                            }
                        }
                        else 
                        {
                            lep = (logical_element*) malloc(sizeof(logical_element));
                            memset(lep, 0, sizeof(logical_element));
                            lep->data = current_pos+hdr_len;
                            if (is_psh_present)
                            {
                                lep->length = payload_len-hdr_len;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                lep->length =left_len-hdr_len;
                            }
                            lep->start_bsn = fsn;
                            lep->type = MAC_SDU_FRAG;
                            lep->blk_type = FIRST_FRAGMENT;
                            enqueue_fragq(fragq, frame_num, cid, lep, modulo, is_mgtcon, sduq);
                        }
                        break;
                    case CONTINUING_FRAGMENT:
                        // there only could be fragment header here
                        if (is_arq)
                        {
                            if (is_psh_present)
                            {
                                block_num = get_ceil((double)( payload_len-hdr_len ) /(double) arq_block_size);
                                left_len -= payload_len;
                            }
                            else 
                            {
                                block_num = get_ceil((double)(left_len-hdr_len) /(double)arq_block_size);
                            }
                            
                            current_block = fsn-1;
                            for (i = 0; i< block_num; i++)
                            {
                                current_block = (current_block + 1) % modulo;
                                // first check the arq engine about the windwo state
                                rx_win_start = ARQ_get_rx_window_start(cid);
                                if (is_within_win(rx_win_start, rx_win_size, current_block, modulo))
                                {
                                    // insert the arq block into the frag queue
                                    lep = (logical_element*) malloc(sizeof(logical_element));
                                    memset(lep, 0, sizeof(logical_element));
                                    lep->data = current_pos+hdr_len+i*arq_block_size;
                                    lep->length = arq_block_size;
                                    lep->type = ARQ_BLOCK;
                                    lep->blk_type = CONTINUING_FRAGMENT;
                                    lep->start_bsn = current_block;
                                    enqueue_arq_fragq(fragq, frame_num, cid, lep, is_order, rx_win_start, rx_win_size, modulo, is_mgtcon, sduq);
				    				FLOG_DEBUG("Notifying for block %d in connection %d\n",current_block,cid);
				    				notify_received_blocks(cid, current_block, current_block);
				}
                                else 
                                {
                                    break;
                                }
                            }
                            // tell the arq_queue about the arrival of these block
                        }
                        else 
                        {
                            lep = (logical_element*) malloc(sizeof(logical_element));
                            memset(lep, 0, sizeof(logical_element));
                            lep->data = current_pos+hdr_len;
                            if (is_psh_present)
                            {
                                lep->length = payload_len-hdr_len;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                lep->length =left_len-hdr_len;
                            }
                            lep->start_bsn = fsn;
                            lep->type = MAC_SDU_FRAG;
                            lep->blk_type = CONTINUING_FRAGMENT;
                            enqueue_fragq(fragq, frame_num, cid, lep, modulo, is_mgtcon, sduq);
                        }
                        break;
                    case LAST_FRAGMENT:
                        // this is a last fragment
                        if (is_arq)
                        {
                            if (is_psh_present)
                            {
                                block_num = get_ceil((double)( payload_len-hdr_len ) /(double) arq_block_size);
                                residue_len =  payload_len-hdr_len - (block_num -1)* arq_block_size;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                block_num = get_ceil((double)(left_len-hdr_len) /(double)arq_block_size);
                                residue_len =  left_len-hdr_len - (block_num -1)* arq_block_size;
                            }
                            current_block = fsn-1;
                            end_block = (current_block + block_num) % modulo;
                            for (i = 0; i< block_num; i++)
                            {
                                current_block = (current_block + 1) % modulo;
                                // first check the arq engine about the windwo state
                                rx_win_start = ARQ_get_rx_window_start(cid);
                                if (is_within_win(rx_win_start, rx_win_size, current_block, modulo))
                                {
                                    // insert the arq block into the frag queue
                                    lep = (logical_element*) malloc(sizeof(logical_element));
                                    memset(lep, 0, sizeof(logical_element));
                                    lep->data = current_pos+hdr_len+i*arq_block_size;
                                    lep->type = ARQ_BLOCK;
                                    if (current_block == end_block)
                                    {
                                        lep->blk_type = LAST_FRAGMENT;
                                        lep->length = residue_len;
                                    }
                                    else 
                                    {
                                        lep->blk_type = CONTINUING_FRAGMENT;
                                        lep->length = arq_block_size;
                                    }
                                    lep->start_bsn = current_block;
                                    enqueue_arq_fragq(fragq, frame_num, cid, lep, is_order, rx_win_start, rx_win_size, modulo, is_mgtcon, sduq);
				    				FLOG_DEBUG("Notifying for block %d in connection %d\n",current_block,cid);
				    				notify_received_blocks(cid, current_block, current_block);
				}
                                else 
                                {
                                        break;
                                }
                            }
                            
                            // tell the arq_queue about the arrival of these block
                        }
                        else 
                        {
                            lep = (logical_element*) malloc(sizeof(logical_element));
                            memset(lep, 0, sizeof(logical_element));
                            lep->data = current_pos+hdr_len;
                            if (is_psh_present)
                            {
                                lep->length = payload_len-hdr_len;
                                left_len -= payload_len;
                            }
                            else 
                            {
                                lep->length =left_len-hdr_len;
                            }
                            lep->start_bsn = fsn;
                            lep->type = MAC_SDU_FRAG;
                            lep->blk_type = LAST_FRAGMENT;
                            enqueue_fragq(fragq, frame_num, cid, lep, modulo, is_mgtcon, sduq);
                        }

                        break;
                    default:
                    // print error and return
                        break;
                }
                if (is_psh_present)
                {
                    current_pos += psh->length;
                }
                else 
                {
                    current_pos = NULL;
                    left_len = 0;
                }
            }

        }
        else
        {
            // no fragmentation, no packing
            if (is_arq_feedback_present )
            {
                // tell the arq_queue about the arq feedback
                msg_type = ARQ_FEEDBACK_IE;
                length = left_len;
	  u_int8_t* msg_payload = (u_int8_t*)malloc(length);
	  memcpy(msg_payload, current_pos,length); 
	   ARQ_feedback_message *tmp = (ARQ_feedback_message*)msg_payload;
	   if (tmp->mgmt_msg_type == ARQ_FEEDBACK) {parse_and_process_feedback_msg(tmp);}
	   else if (tmp->mgmt_msg_type == ARQ_DISCARD)process_discard_msg((ARQ_discard_message*)tmp,fragq);
	   else if (tmp->mgmt_msg_type == ARQ_RESET) {process_reset_msg((ARQ_reset_message*)tmp,fragq);}
	   else FLOG_DEBUG("Unknown ARQ Mgmt Message\n");
          //      enqueue_ul_mgt_msg(ul_msgq, frame_num, mac_hdr->cid, msg_type, length, (void*)(current_pos));
	   free(msg_payload);
                left_len = 0;
            }
            else
            {
                if (is_mgtcon)
                {
                    msg_type = UNFRAG_MANAGEMENT_MESSAGE;
                    length = left_len;
                    enqueue_ul_mgt_msg(ul_msgq, frame_num, mac_hdr->cid, msg_type, length, (void*)(current_pos));
                }
                else
                {
                    if (is_fixed_sdu)
                    {
                        sdu_num = (left_len) / fixed_sdu_len;
                        for (i=0; i< sdu_num; i++)
                        {
                           sdu_pkt = (logical_packet*) malloc(sizeof(logical_packet));
                           memset(sdu_pkt, 0, sizeof(logical_packet));
                           lep = (logical_element*) malloc( sizeof(logical_element));
                           memset(lep, 0, sizeof(logical_element));
                           lep->length = fixed_sdu_len;
                           newcontent = (u_char *) malloc(fixed_sdu_len);
                           memcpy(newcontent, current_pos, fixed_sdu_len);
                           lep->data = newcontent;
                           lep->type = MAC_SDU;
                           lep->next = NULL;
                           sdu_pkt->element_head = lep;
                           sdu_pkt->length = lep->length;
                           sdu_pkt->next = NULL;
                           sdu_pkt->cid = mac_hdr->cid;
                           enqueue_sduq(sduq, mac_hdr->cid, sdu_pkt);

                           cs_msg.len = mac_hdr->cid;

                           if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                               sizeof(struct queue_msg)) == -1)
                           {
                               FLOG_ERROR("enqueue error\n");
                           }

                           current_pos += fixed_sdu_len;
                        }
                    }
                    else
                    {
                        sdu_pkt = (logical_packet*) malloc(sizeof(logical_packet));
                        memset(sdu_pkt, 0, sizeof(sdu_pkt));
                        lep = (logical_element*) malloc( sizeof(logical_element));
                        memset(lep, 0, sizeof(logical_element));
                        lep->length = left_len;
                        newcontent = (u_char *) malloc(left_len);
                        memcpy(newcontent, current_pos, left_len);
                        lep->data = newcontent;
                        lep->type = MAC_SDU;
                        lep->next = NULL;
                        sdu_pkt->element_head = lep;
                        sdu_pkt->length = lep->length;
                        sdu_pkt->next = NULL;
                        sdu_pkt->cid = mac_hdr->cid;
                        enqueue_sduq(sduq, mac_hdr->cid, sdu_pkt);

                        cs_msg.len = mac_hdr->cid;

                        if ( wmrt_enqueue( csforw_en_id, &cs_msg,
                             sizeof(struct queue_msg)) == -1)
                        {
                            FLOG_ERROR("enqueue error\n");
                        }

                    }
                }  
                left_len = 0;
            }
        }

	      // now move to the next pdu
        cur_pdu = cur_pdu->next;
    }

    free(mac_hdr);
    mac_hdr = NULL;

    if (fsh)
    {
        free(fsh);
        fsh = NULL;
    }
    if (psh)
    {
        free(psh);
        psh = NULL;
    }

    //release_pducidq(pdu_list);
    pdu_list->is_processed = 1;
    return 0;
}

int reassembly(pdu_frame_queue* pduqlist, sdu_queue* sduq, frag_queue* fragq, ul_br_queue* brq, mgt_msg_queue *ul_msgq)
{
    int i=0;
    int step;
    int con_in_last_block;
    int j=0;
    int k=0;

    pdu_cid_queue* copied_pducidq_header;
    pdu_cid_queue* pre_copied_pducidq;
    int is_residue_processed = 0;

    if (pduqlist == NULL)
    {
        return 0;
    }

    pdu_cid_queue* con_pduq = pduqlist->pdu_cid_q_head;

    if (con_pduq == NULL)
    {
        return 0;
    }

    step = pduqlist->num_cids / UL_CON_THREAD_NUM;
    con_in_last_block = pduqlist->num_cids % UL_CON_THREAD_NUM;

    if (con_in_last_block>0)
    {
         is_residue_processed = 0;
    }
    else
    {
         is_residue_processed = 1;
    }

    FLOG_DEBUG( "UL_MULTI: STARTING CPS --total connection %d: \n", total_ul_con_threads);
    //printf( "UL_MULTI: STARTING CPS --total connection %d: \n", total_ul_con_threads);
    // lock cps thread mutex
    pthread_mutex_lock(&(process_ul_con_thread_mutex));

    if (step ==0)
    {
        max_valid_ul_con=pduqlist->num_cids;
    }
    else
    {
        max_valid_ul_con=UL_CON_THREAD_NUM;
    }

    // if step ==0, then, allocate one in each UL_CON_THREAD_NUM
    // if step >0, then allocate multiple one in each UL_CON_THREAD_NUM

    while(i<pduqlist->num_cids)
    {
	      // initialize the thread args
        con_thread_arg[k].frame_num=pduqlist->frame_no;
        con_thread_arg[k].sduq=sduq;
        con_thread_arg[k].fragq = fragq;
        con_thread_arg[k].brq =brq;
        con_thread_arg[k].ul_msgq = ul_msgq;
        con_thread_arg[k].status=-1;

        pre_copied_pducidq = NULL;

        for (j = 0; j < step; j++)
        {
            // malloc a new thread
            copied_pducidq_header = (pdu_cid_queue*) malloc(sizeof(pdu_cid_queue));
            memcpy(copied_pducidq_header, con_pduq, sizeof(pdu_cid_queue));

            if (pre_copied_pducidq == NULL )
            {
                con_thread_arg[k].pdu_list = copied_pducidq_header;
            }
            else
            {
                pre_copied_pducidq->next = copied_pducidq_header;
            }

            pre_copied_pducidq = copied_pducidq_header;
            con_pduq = con_pduq->next;
            i++;
        }

        if (!is_residue_processed)
        {
            if (con_in_last_block > 0)
            {
                // malloc a new thread
                copied_pducidq_header = (pdu_cid_queue*) malloc(sizeof(pdu_cid_queue));
                memcpy(copied_pducidq_header, con_pduq, sizeof(pdu_cid_queue));

                if (pre_copied_pducidq == NULL )
                {
                    con_thread_arg[k].pdu_list = copied_pducidq_header;
                }
                else
                {
                    pre_copied_pducidq->next = copied_pducidq_header;
                }

                pre_copied_pducidq = copied_pducidq_header;
                i++;
                con_in_last_block--;
                con_pduq = con_pduq->next;
            }
            else
            {
                is_residue_processed = 1;
            }
        }
        pre_copied_pducidq->next = NULL;
        k++;
    }

    //Invoke the burst threads
    for(i = 0; i < param_NUM_ATTACHED_PROCS; i++)
    {
        pthread_mutex_lock(&(con_processing_thrd[i].con_mutex));
        con_processing_thrd[i].resume_status=-1;
        // signal child burst threads
        pthread_cond_signal(&(con_processing_thrd[i].ready_to_process));
        total_ul_con_threads++;
        pthread_mutex_unlock(&(con_processing_thrd[i].con_mutex));
        //printf("MULTI: signal to process reassembly --total connection: %d \n", total_ul_con_threads);
        FLOG_DEBUG( "UL_MULTI: signal to process reassembly --total connection: %d \n", total_ul_con_threads);
    }

    // lock cps thread mutex
    //pthread_mutex_lock(&(process_cps_thread_mutex));
    while(total_ul_con_threads>0)
    {
        // wait for signal from last completing child
        pthread_cond_wait(&child_con_threads_done, &(process_ul_con_thread_mutex));
    }

    // unlock cps thread mutex
    pthread_mutex_unlock(&(process_ul_con_thread_mutex));
    FLOG_DEBUG( "UL_MULTI: DONE reassembly --total connection: %d\n", total_ul_con_threads);

    // release the pointer to the pdu_cid_queue in each thread
    release_thrd_pdu_cid_queue();

    return 0;
}
