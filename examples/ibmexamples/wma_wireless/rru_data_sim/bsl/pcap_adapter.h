#ifndef __PCAP_ADAPTER_H__
#define __PCAP_ADAPTER_H__


#include <pcap.h>


#define PCAP_SNAP_LEN_MAX  65535
#define PCAP_FILTER_MAX       1024
#define PCAP_ERRORBUF_MAX  1024

typedef enum _PCAP_PARAM_T
{
    PCAP_SET_FILTER  = 0,
    PCAP_SET_NIC = 1
} PCAP_PARAM_T;


typedef struct _PCAP_adapter_t
{
    pcap_t * pcap;
    char * error_buf;
    char * filter;
    char nic[20];
} PCAP_adapter_t;


int PCAP_adapter_create(PCAP_adapter_t ** adapter, int pcap_id);
int PCAP_adapter_destroy(PCAP_adapter_t * adapter);
int PCAP_adapter_start(PCAP_adapter_t * adapter);
int PCAP_adapter_stop(PCAP_adapter_t * adapter);
int PCAP_adapter_config(PCAP_adapter_t * adapter, int param, void * value, int size);
unsigned int PCAP_adapter_read(PCAP_adapter_t * adapter, void * buffer, unsigned int bufsize);
unsigned int PCAP_adapter_write(PCAP_adapter_t * adapter, void * buffer, unsigned int bufsize);


#endif // __PCAP_ADAPTER_H__
