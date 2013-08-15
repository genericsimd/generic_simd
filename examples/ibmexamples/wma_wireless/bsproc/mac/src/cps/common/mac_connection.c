/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_connection.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "mac_connection.h"
#include "mac_serviceflow.h"
#include "mac_auth.h"
#include "dl_exp_params.h"

//Global pointer to list of connections
connection* connection_list_head=NULL;
connection** conn_array;

int get_total_rsvd_rate()
{
	int sum = 0;
	connection* conn = connection_list_head;
	while (conn != NULL)
	{
		if (conn->sf != NULL)
		{
			sum += conn->sf->min_reserved_traffic_rate;
		}
#ifdef BR_ENABLE
		else
		{
			sum += conn->min_reserved_traffic_rate;
		}
#endif
		conn = conn->next;
	}
	return sum;
}

int get_total_ul_rate()
{
        int sum = 0;
        connection* conn = connection_list_head;
        while (conn != NULL)
        {
                if (conn->sf != NULL)
                {
			if (conn->sf->sf_direction == UL)
			{
                        	sum += conn->sf->min_reserved_traffic_rate;
			}
                }
#ifdef BR_ENABLE
                else
                {
                        sum += conn->min_reserved_traffic_rate;
                }
#endif
                conn = conn->next;
        }
        return sum;
}

int get_total_ul_slot()
{
        int sum = 0;
	int slot_num;
	int ss_index;
	ModulCodingType	ul_type;
	ModulCodingType dl_type;

        connection* conn = connection_list_head;
        while (conn != NULL)
        {
                if (conn->owner == NULL)
                {
			conn = conn->next;
                	continue;
                }

		ss_index = conn->owner->ss_index;
		get_ss_mcs(ss_index, &dl_type, &ul_type);
                if (conn->sf != NULL)
                {
                        if (conn->sf->sf_direction == UL)
                        {
				slot_num = ceil((float)(conn->sf->min_reserved_traffic_rate * 8)/\
							(bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT)); 
                                sum += slot_num;
                        }
                }
#ifdef BR_ENABLE
                else
                {
			slot_num = ceil((float)(conn->min_reserved_traffic_rate * 8)/\
                                                        (bits_per_car[ul_type] * UL_DATA_CAR_PER_SLOT));
                        sum += slot_num;
                }
#endif
                conn = conn->next;
        }
        return sum;
}

int get_total_dl_slot()
{
        int sum = 0;
        int slot_num;
        int ss_index;
        ModulCodingType ul_type;
        ModulCodingType dl_type;

        connection* conn = connection_list_head;
        while (conn != NULL)
        {
                if (conn->owner == NULL)
                {
			conn = conn->next;
                        continue;
                }

                ss_index = conn->owner->ss_index;
                get_ss_mcs(ss_index, &dl_type, &ul_type);
                if (conn->sf != NULL)
                {
                        if (conn->sf->sf_direction == DL)
                        {
                                slot_num = ceil((float)(conn->sf->min_reserved_traffic_rate * 8)/\
                                                        (bits_per_car[dl_type] * DL_DATA_CAR_PER_SLOT));
                                sum += slot_num;
                        }
                }
#ifdef BR_ENABLE
                else
                {
                        slot_num = ceil((float)(conn->min_reserved_traffic_rate * 8)/\
                                                        (bits_per_car[dl_type] * DL_DATA_CAR_PER_SLOT));
                        sum += slot_num;
                }
#endif
                conn = conn->next;
        }
        return sum;
}

int get_total_dl_rate()
{
        int sum = 0;
        connection* conn = connection_list_head;
        while (conn != NULL)
        {
                if (conn->sf != NULL)
                {
			if (conn->sf->sf_direction == DL)
			{
                        	printf("cid = %d min_reserved_traffic_rate = %d\n", conn->sf->cid, conn->sf->min_reserved_traffic_rate);
                        	sum += conn->sf->min_reserved_traffic_rate;
			}
                }
#ifdef BR_ENABLE
                else
                {
                        sum += conn->min_reserved_traffic_rate;
                }
#endif
                conn = conn->next;
        }
        return sum;
}


int is_fix_sdu_length(int cid, u_int8_t* is_fixed_sdu){

  //assign by default to 0
  (*is_fixed_sdu)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_fixed_sdu) = con->is_fixed_macsdu_length; 
  }
  return(0);

}

int get_sdu_size(int cid, int* sdu_size){

  //assign by default to 0
  (*sdu_size)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*sdu_size) = con->macsdu_size; 
  }
  return(0);
}

int get_pdu_size(int cid, int* pdu_size){

  //assign by default to 0
  (*pdu_size)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*pdu_size) = con->macpdu_size; 
  }
  return(0);
}

int get_current_seq_no(int cid, int* seq_no){

  //assign by default to 0
  (*seq_no)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*seq_no) = con->current_seq_no; 
  }
  return(0);

}

int set_current_seq_no(int cid, int seq_no){

  connection* con=find_connection(cid);
  if(con!=NULL) {
    con->current_seq_no = seq_no;
  }
  return(0);

}

int get_modulo(int cid, int* modulo){

  //assign by default to 0
  (*modulo)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*modulo) = con->modulo; 
  }
  return(0);

}

int is_mgt_con(int cid, u_int8_t* is_mgmt){

  //assign by default to 0
  (*is_mgmt)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
            if (con->con_type == CONN_DATA)
            {
                (*is_mgmt) = 0;
            }else {
                (*is_mgmt) = 1;
            } 
  }
  return(0);

}

int is_frag_enabled(int cid, u_int8_t* is_frag){

  //assign by default to 0
  (*is_frag)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_frag) = con->is_frag_enabled; 
  }
  return(0);

}

int is_pack_enabled(int cid, u_int8_t* is_pack){

  //assign by default to 0
  (*is_pack)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_pack) = con->is_pack_enabled; 
  }
  return(0);

}

int is_crc_enabled(int cid, u_int8_t* is_crc){

  //assign by default to 0
  (*is_crc)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_crc) = con->is_crc_included; 
  }
  return(0);

}
/*
void get_encryption_key(int cid, u_char *key,int key_length)
{

  connection *con = find_connection(cid);
  if (con!=NULL) memcpy(key, con->key,key_length);
}
*/
int get_encryption_key(int cid, u_char* key, int key_length)
{
	int my_basic_cid;
	struct ss_security_info* my_security_info;
	int aksn;
	
#ifdef TRANSITION_TEST
	int trans_aksn;
	unsigned long long int trans_lifetime;
	unsigned long long int current_time = 0;
#endif
	
	get_basic_cid(cid, &my_basic_cid);
	my_security_info = find_sa_from_said(my_basic_cid);
	
	pthread_mutex_lock(&(my_security_info->sa_lock));

	if (my_security_info == NULL || my_security_info->current_sa_status == PERM_AUTH_REJECT) 
	{
		pthread_mutex_unlock(&(my_security_info->sa_lock));
		return -1;
	}
	
	aksn = my_security_info->current_seq_no;

#ifdef TRANSITION_TEST

	#ifdef SS_TX
		trans_aksn = (aksn - 1)%4;
	#else
		trans_aksn = (aksn + 1)%4;
	#endif
	trans_lifetime = my_security_info->key_gen_time[trans_aksn].tv_sec*1000000LL + my_security_info->key_gen_time[trans_aksn].tv_usec;
	current_time = readtsc();
	printf("Keygentime %llu, current time %llu, Difference %llu\n",trans_lifetime + param_KEY_LIFE_DURATION, current_time, trans_lifetime + param_KEY_LIFE_DURATION - current_time);
	if (trans_lifetime + param_KEY_LIFE_DURATION > current_time) aksn = trans_aksn;
#endif 
	
	if (my_security_info != NULL) memcpy(key,my_security_info->akey[aksn],key_length);
	pthread_mutex_unlock(&(my_security_info->sa_lock));
	return 0;
	
}
int get_decryption_key(int cid, u_char* key, int key_length, int seq_no)
{
	int my_basic_cid;
	struct ss_security_info* my_security_info;
	if ((seq_no %4) != seq_no)
	{
		FLOG_ERROR("Sequence number to get_decryption_key must be modulo 4\n");
		return -1;
	}
	get_basic_cid(cid, &my_basic_cid);
	my_security_info = find_sa_from_said(my_basic_cid);
	pthread_mutex_lock(&(my_security_info->sa_lock));
	if (my_security_info == NULL || my_security_info->current_sa_status == PERM_AUTH_REJECT) 
	{
		if (my_security_info->current_sa_status == PERM_AUTH_REJECT) printf("PAR\n");
		pthread_mutex_unlock(&(my_security_info->sa_lock));
		return -1;
	}
	
	if (my_security_info != NULL) memcpy(key,my_security_info->akey[seq_no],key_length);
	pthread_mutex_unlock(&(my_security_info->sa_lock));
	return 0;
	
}

int is_encrypt_enabled(int cid, u_int8_t* is_encrypt){

  //assign by default to 0
  (*is_encrypt)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_encrypt) = con->is_encrypt_enabled; 
  }
  return(0);

}

int is_arq_enabled(int cid, u_int8_t* is_arq){

  //assign by default to 0
  (*is_arq)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_arq) = con->is_arq_enabled; 
  }
  return(0);

}

int get_rx_win_size(int cid, int* win_size){

  //assign by default to 0
  (*win_size)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*win_size) = con->arq->arq_window_size; 
  }
  return(0);

}

int get_arq_block_size(int cid, int* block_size){

  //assign by default to 0
  (*block_size)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*block_size) = con->arq_block_size; 
  }
  return(0);

}

int is_arq_order_preserved(int cid, u_int8_t* is_order){

  //assign by default to 0
  (*is_order)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*is_order) = con->arq->is_order_preserved; 
  }
  return(0);

}

int get_fsn_size(int cid, int* fsn_size){

  //assign by default to 0
  (*fsn_size)=0;
  connection* con=find_connection(cid);
  if(con!=NULL) {
    (*fsn_size) = con->fsn_size; 
  }
  return(0);

}


// for CRL testing usage

int add_connection(int cid, u_int8_t is_arq, connection** con){
    connection* con_head;
    connection* prev;
    u_int8_t is_insert;
    (*con) = (connection*) malloc(sizeof(connection));
    (*con) ->cid = cid;
    (*con) ->next = NULL;
    if (is_arq)
    {
        initialize_arq(cid, &((*con) ->arq));
    }
    else
    {
        (*con)->arq = NULL;
    }
    con_head = connection_list_head;
    prev = NULL;
    if (con_head == NULL)
    {
        connection_list_head = (*con);
    }
    else
    {
        is_insert = 0;
        while(con_head)
        {
            if (con_head->cid < cid)
            {
                prev = con_head;
                con_head = con_head->next;
            }
            else
            {
                if (prev)
                {
                    prev->next = (*con);
                    (*con) ->next = con_head;
                }
                else 
                {
                    (*con) ->next = connection_list_head;
                    connection_list_head = (*con);
                }
                is_insert = 1;
                break;
            }
        }

        if (!is_insert)
        {
            // insert the connection at the end of this queue
            prev->next = (*con) ;
        }
        
    }
    return 0;
    
}

int delete_connection(int cid){
    connection* con;
    connection* prev;
    con = connection_list_head;
    prev = NULL;
    if(conn_array != NULL)
	{
		conn_array[cid] =  NULL;
    }
    while (con)
    {
        if (con->cid == cid)
        {
            if (prev)
            {
                prev->next = con->next;
                
            }
            else 
            {
                connection_list_head = con->next;
            }
			if(con->arq != NULL) {free(con->arq);}
            free(con);
            break;
        }
        else 
        {
            prev = con;
            con = con->next;
        }

    }
    return 0;
}

int modify_connection(){
    return 0;
}


int get_connection(int cid, connection** conn){
    connection* con;
    con = connection_list_head;

    while (con)
    {
        if (con->cid == cid)
        {
            (*conn) = con;
            break;
        }
        else
        {
            con = con->next;
        }
    }
    return 0;
}

int release_connection_queue(connection* connection_list){
    connection* con;
    connection* prev;
    con = connection_list_head;
    prev = NULL;
    while (con)
    {
        prev = con;
        con = con->next;
        if (prev->is_arq_enabled)
        {
            free(prev->arq);
        }
        free(prev);
    }
    connection_list_head = NULL;
    return 0;
}

int get_rx_win_start(int cid, int* win_start){
    connection* con;
    get_connection(cid, &con);

    (*win_start) = con->arq->rx_window_start;

    return 0;
}


BOOL is_conn_frag_enabled(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->is_frag_enabled);
  }
  else {
    return(FALSE); //error connection not found
  }
}

BOOL is_conn_pack_enabled(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->is_pack_enabled);
  }
  else {
    return(FALSE); //error connection not found
  }
}

BOOL is_conn_arq_enabled(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->is_arq_enabled);
  }
  else {
    return(FALSE); //error connection not found
  }
}

int get_mac_pdu_size(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->macpdu_size); 
  }
  else {
    return(0); //error connection not found
  }
}

int get_blk_size(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->arq_block_size); 
  }
  else {
    return(0); //error connection not found
  }
}

BOOL is_fragmentation_enabled(int cid) {
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    return(conn->is_frag_enabled);
  }
  else {
    return(FALSE); //error connection not found
  }
}

BOOL is_be_cid(int cid) {
  if((cid>=BE_CID_MIN_VALUE) && (cid<=BE_CID_MAX_VALUE)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}


BOOL is_ugs_cid(int cid) {
  if((cid>=UGS_CID_MIN_VALUE) && (cid<=UGS_CID_MAX_VALUE)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//This function returns the connection element for the 
// associated cid
//Currently does a linear search of the global connection list maintained
//This will be modified to perform optimized search (later)
connection* find_connection(int cid) {
  if(!conn_array){
    return(NULL);
  }
  return(conn_array[cid]);
  //connection* conn=connection_list_head;
  //while(conn!=NULL) {
  //  if(conn->cid==cid) {
  //    return conn;
  //  }
  //  conn=conn->next;
  //}
  //return NULL;
}

//for a cid returns the service_flow associated with it
//This is called for non basic cids only
//The basic cids are identified by the value of cids
//Basic cid values fall within a range -- for instance 1-200
int get_service_flow(int cid, serviceflow** sflow) {
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    *sflow=conn->sf;
    return 0; //no error
  }
  else {
    *sflow=NULL;
    return -1; //error connection not found
  }
}

//for a cid returns the scheduling type
int get_scheduling_type(int cid, SchedulingType* svc_flow_type) {
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    if(conn->sf == NULL){  // for Basic and Primary cids
          return(-1);
    }
    *svc_flow_type=conn->sf->schedule_type;
    return 0; //no error
  }
  else {
    *svc_flow_type=SERVICE_UGS; //assign UGS by default when cid isnot known
    return -1; //error connection not found
  }
}


//for a cid returns its bs_ss_info
int get_bs_ss_info(int cid, bs_ss_info** ss_info) {
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    *ss_info=conn->owner;
    return 0; //no error
  }
  else {
    *ss_info=NULL;
    return -1; //error connection not found
  }
}

//for a cid returns its basic cid
//returns the basic cid associated with the bs_ss_info for any cid
int get_basic_cid(int cid, int* basic_cid) {
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    *basic_cid=conn->owner->basic_cid;
    return 0; //no error
  }
  else {
    *basic_cid=0;
    FLOG_ERROR("BCID not found");
    return -1; //error connection not found
  }
}

//for a cid returns its primary cid
//returns the primary cid associated with the bs_ss_info for any cid
int get_primary_cid(int cid, int* primary_cid) {
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    *primary_cid=conn->owner->primary_cid;
    return 0; //no error
  }
  else {
    *primary_cid=0;
    FLOG_ERROR("Primary cid not found");
    return -1; //error connection not found
  }
}

int get_ss_index(int cid, int* ss_index)
{
  //first find the connection element for the cid
  connection* conn=find_connection(cid);
  if(conn!=NULL) {
    *ss_index=conn->owner->ss_index;
    return 0; //no error
  }
  else {
    *ss_index=0;
    //FLOG_ERROR("Cid not found");
    return -1; //error connection not found
  }
}

int add_conection(connection* conn) {
  conn->next=connection_list_head;
  connection_list_head=conn;
  if(conn_array != NULL){
    conn_array[conn->cid] =  conn;
  }
  else{
    FLOG_ERROR("conn created without creating conn_array!");
  }
  return 0;
}

int associate_connection_with_svc_flow(connection* conn, int sfid) {
  assert(conn->owner!=NULL);
  serviceflow* sflow=conn->owner->sf_list_head;
  while(sflow!=NULL) {
    int id=sflow->sfid;
    if(id==sfid) {
      conn->sf=sflow;
      sflow->cid=conn->cid;
//      printf("Associate: cid:%d sfid:%d sflow:%x conn:%x ssinfo:%x\n",
//	     conn->cid, sfid, sflow, conn, conn->owner);
      return 0;
    }
    sflow=sflow->next;
  }
  return -1;

}


int get_cid_from_sfid(int sfid)
{

	connection * conn = connection_list_head;
		
	while (conn!=NULL)
	{
		if (conn->sf == NULL) {conn=conn->next;continue;}
		if (conn->sf->sfid == sfid) 
		{
			return conn->cid;
		}
		conn = conn->next;
	}
	return -1;
}

