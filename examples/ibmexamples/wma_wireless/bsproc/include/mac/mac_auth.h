#include <stdio.h>
#include "mac.h"
#include "mac_qos_mm.h"
#include <openssl/x509.h>
#include "CMAInterface.h"
#include "StoreInterface.h"

#define PKM_RESEND_WAIT_DURATION 1000000LL //in usec
#define NUM_PKM_REQ_RETRIES 3//3 PKM Retries allowed.

//The commented values below should be enabled when PKM Is integrated with FOAK.The currently used values are dummy and really short. They are being set only for purposes of Security team demo.
//#define KEY_LIFE_DURATION 86400000000LL //in usec
//#define GRACE_TIME 14400000000LL //in usec
//#define DELTA_ADD 100000LL//in usec
//#define KEY_LIFE_DURATION 30000000LL //in usec
//#define GRACE_TIME  15000000LL//in usec
//#define DELTA_ADD 5000000LL

#define MAX_PKM_RSP_LEN 1000
#define MAX_PKM_REQ_LEN 1000

#define PERM_AUTH_REJECT 1
#define AUTH_WAIT 2
#define AUTHORIZED 0
#define AUTH_REQUEST 4
#define AUTH_REPLY 5
#define AUTH_REJECT 6


//#define PKM_RESEND_TEST
//#define AUTH_REJECT_TEST
//#define TRANSITION_TEST
//#define PKM_TEST
#define MAX_CERT_FILE_LENGTH 1500

struct ss_security_info* ss_security_list;
pthread_mutex_t security_list_lock;

struct crypto_suite {
u_char data_encryption_algo;
u_char data_auth_algo;
u_char tek_encryption_algo; //Since we will not use this, we can set to 0.
struct crypto_suite* next;
};
/*
struct ss_rsa_info {
X509 ss_cert;
u_char ss_private_key[128];//assuming that private key is 128 bits
};
*/

struct ss_security_info {

X509* ss_cert;
u_char* ss_cert_file;
int cert_file_length;
int num_suites_available;
struct crypto_suite* suite_list;
struct crypto_suite suite_used;
u_char sa_type;
u_int16_t said;
u_char akey[4][KEYLEN]; //64-bit key for 4 sequence numbers.
struct timeval key_gen_time[4]; //Key lifetime for keys that are alive. Set to 0 for inactive sequence numbers 
u_int8_t current_seq_no; //The seq number of the key currently being used for encryption.
int current_sa_status;//State of the Auth.SM in which SA is.
u_char  latest_pkm_id;	//Latest PKM ID used with a SA
int error_code;
char display_string[128];
//Used in SS.
//#ifdef SS_TX
	int first_flag;
	app_timer *reauth_timer;
	app_timer *key_timeout_timer;
	app_timer *pkm_resend_timer;
	int resends_left;
//#else
	app_timer* timer_node;
//#endif
app_timer* transport_conns_timer;
pthread_mutex_t sa_lock;
struct ss_security_info* next;
};

struct pkm_msg {
u_char mgm_msg_type;//9 or 10.
u_char code; //Defined in Table 50 of WiMAX Std doc
u_char pkm_id;
struct tlv_sf_mgmt_encoding *tlv_pkm;
};

void* pkm_msg_handler();

int pkm_resend_func_ss(void *node_pointer);

int mac_des_crypto(unsigned char* data, int datalen, unsigned char* ivec, unsigned char* key, unsigned char* output, int enc);

int init_pkm_req(struct ss_security_info* ss_details, struct pkm_msg *pkm_req_msg);

int build_pkm_req(struct pkm_msg* pkm_req, u_char* payload, int* length);

int parse_pkm_req(u_char* payload, int length, struct pkm_msg* pkm_req);

int init_auth_reply(struct pkm_msg* pkm_req_msg, struct ss_security_info* ss_details,  struct pkm_msg* pkm_rsp_msg);

int build_auth_reply(struct pkm_msg* pkm_rsp, u_char* payload, int* length);

int init_auth_reject(u_int8_t error_code,  struct pkm_msg* pkm_req_msg, u_char* display_string, int string_len, struct pkm_msg* pkm_rsp_msg);

int build_auth_reject (struct pkm_msg* pkm_rsp, u_char* payload, int* length);

int parse_auth_reply(u_char* payload, int length, struct pkm_msg* pkm_rsp);

int parse_auth_reject(u_char* payload, int length, struct pkm_msg* pkm_rsp);

struct ss_security_info* find_sa_from_said(u_int16_t said);

int simulate_pkm_req_to_bs(int bcid);

struct ss_security_info*  update_sa_from_auth_reply(struct pkm_msg* pkm_rsp);

struct ss_security_info*  update_sa_from_auth_reject(struct pkm_msg* pkm_rsp);

void clear_ss_security_list();

int ss_reauth_after_perm_reject(int said);

int ss_reauth_after_auth_reject(int said);
