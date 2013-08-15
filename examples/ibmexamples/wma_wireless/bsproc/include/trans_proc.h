/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: trans_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __TRANS_PROC_H_
#define __TRANS_PROC_H_

struct trans_carrier_config
{
    int carrier_enable;
    int carrier_freq;
    int carrier_bw;
    int carrier_pwr;
};

struct trans_contorl_config
{
    unsigned int rru_id;
    unsigned int bbu_id;
    unsigned short src_port;
    unsigned short dst_port;
    unsigned int src_ip;
    unsigned int dst_ip;
    unsigned int rru_nic_mask;
    char rru_nic_if[16];
    unsigned short bbu_tcp_port;
    unsigned int agent_ip;
    unsigned short agent_server_port;
    unsigned int agent_heartbeat_timeout;
    unsigned int rru_broadcast_ip;
    unsigned short monitor_server_port;
    char my_mac[6];    
    char rru_mac[6];
    int rru_byte_order;
    int enable_gps;
    int rru_output_power;
    int chan1_pa_enable;
    int chan2_pa_enable;
    int is_rxfrm_len;
    int rru_rx_len;
    int rru_tx_len;
    int rru_ttg;
    int rru_rtg;
    int enable_agc;
    int chan1_rx_pgc;
    int chan2_rx_pgc;
    struct trans_carrier_config carrier_info[4];
};

extern struct trans_contorl_config g_trans_control_param;

int init_trans (char * device_id, char * agent_id);
int close_trans (void);

int connect_rrh(void);
int close_rrh(void);

int connect_agent(void);
int wait_agent(void);
int close_agent(void);


#endif /* __TRANS_PROC_H_ */
