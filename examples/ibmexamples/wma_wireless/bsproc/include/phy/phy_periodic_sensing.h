/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_perodical_sensing.h

   Function: Header file for the phy_perodical_sensing()

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "phy_ul_rx_interface.h"

#define AVG_POWER_HISTORY 3

struct phy_ps_node
{
    int flag;
    float expected_pwr;
    float noise_figure;

    float * avg_pwr;

    struct phy_ps_node * next;
};

struct phy_ps_hdr
{
    pthread_mutex_t mutex;
    struct phy_ps_node * next;
};


extern struct phy_ps_hdr phy_ps_history;

int32_t spectrum_per_scan(struct phy_ul_rx_syspara *para,
                          float *input_r,
                          float *input_i,
                          float *power_output);

int32_t per_dts_update(struct phy_ul_rx_syspara *para,
                       float *input,
                       struct phy_dts_info *dtspara);

int phy_ps_init_history();
int phy_ps_deinit_history();
struct phy_ps_node * phy_ps_get_current();
int phy_ps_update();
struct phy_ps_node * phy_ps_get_history(int index);

/*
struct perodical_scan_result
{
    float ps_power[1024];
    float ps_thd;
    char active_band[21];
    float reserved_bit[4];
};
*/
