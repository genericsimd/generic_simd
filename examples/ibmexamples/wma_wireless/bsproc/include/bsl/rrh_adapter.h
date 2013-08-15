#ifndef __RRH_ADAPTER_H__
#define __RRH_ADAPTER_H__


typedef struct _RRH_adapter_t
{
    int type;
    int rrh_id;

    int      send_channel_num;
    int      rec_channel_num;
    double rx_center_freq;;
    int rx_bitrate;
    double tx_center_freq;
    int tx_bitrate;
    double rx_gain;
    double tx_ampl;
 
    unsigned int b_r;
    unsigned int b_w;
   
    void * rrh_priv;
} RRH_adapter_t;


typedef struct _RRH_handler_t
{
    int (* init)(RRH_adapter_t *);
    int (* deinit)(RRH_adapter_t *);
    int (* rxstart)(RRH_adapter_t *);
    int (* rxstop)(RRH_adapter_t *);
    int (* txstart)(RRH_adapter_t *);
    int (* txstop)(RRH_adapter_t *);
    unsigned int (* read)(RRH_adapter_t *, void *, unsigned int);
    unsigned int (* write)(RRH_adapter_t *, void *, unsigned int);
    unsigned int (* read_2)(RRH_adapter_t *, void *, void *,unsigned int);
    unsigned int (* write_2)(RRH_adapter_t *, void *, void *,unsigned int);
    int (* config)(RRH_adapter_t *, int, void *);
} RRH_handler_t;


#define RRH_TYPE_NUM  2
extern RRH_handler_t RRH_handler[RRH_TYPE_NUM];


int RRH_adapter_create(RRH_adapter_t ** adapter, int type, int rrh_id);
int RRH_adapter_destroy(RRH_adapter_t * adapter);
int RRH_adapter_rxstart(RRH_adapter_t * adapter);
int RRH_adapter_rxstop(RRH_adapter_t * adapter);
int RRH_adapter_txstart(RRH_adapter_t * adapter);
int RRH_adapter_txstop(RRH_adapter_t * adapter);
int RRH_adapter_config(RRH_adapter_t * adapter, int param, void * value);
unsigned int RRH_adapter_read(RRH_adapter_t * adapter, void * buffer, unsigned int bufsize);
unsigned int RRH_adapter_write(RRH_adapter_t * adapter, void * buffer, unsigned int bufsize);

unsigned int RRH_adapter_read_2(RRH_adapter_t * adapter, void * buffer1,void * buffer2, unsigned int bufsize);
unsigned int RRH_adapter_write_2(RRH_adapter_t * adapter, void * buffer1,void * buffer2, unsigned int bufsize);

#endif // __RRH_ADAPTER_H__
