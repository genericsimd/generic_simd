#ifndef _WIT_MONITOR_TOOL_MONITOR_FEATURE_H_
#define _WIT_MONITOR_TOOL_MONITOR_FEATURE_H_

#include "../witm_types.h"

#define CON_POINT_NUM       840
#define AMP_POINT_NUM       1024
#define PSD_POINT_NUM       256

#define WITM_BER_NUM        6
#define WITM_MAX_BER_POINT  13

#define UL_SLOT_NUM         3
#define DL_SLOT_NUM         2

#define MAX_SLOT_NUM        3

/// Definitions of TX side data block
struct wmtf_tx_block
{
    float con_x[CON_POINT_NUM * MAX_SLOT_NUM];
    float con_y[CON_POINT_NUM * MAX_SLOT_NUM];
    float amp_x[AMP_POINT_NUM * MAX_SLOT_NUM];
    float amp_y[AMP_POINT_NUM * MAX_SLOT_NUM];
};

/// Definitions of RX side data block
struct wmtf_rx_block
{
	float con_bef_x[CON_POINT_NUM * MAX_SLOT_NUM];
	float con_bef_y[CON_POINT_NUM * MAX_SLOT_NUM];
	float con_aft_x[CON_POINT_NUM * MAX_SLOT_NUM];
	float con_aft_y[CON_POINT_NUM * MAX_SLOT_NUM];
};

/**
 * Definitions of Ber block
 * In two-dimentional mode:
 *     snr[index of Ber Figure][index of point]
 * Ber Figure number : 6
 * Max Ber Point of each figure: 12
 * 	   current[index of Ber Figure]: the latest data of ber
 * 	   range from 1 to 12
 */
struct wmtf_ber
{
	int snr[WITM_BER_NUM][WITM_MAX_BER_POINT];
	float ber[WITM_BER_NUM][WITM_MAX_BER_POINT];

	int current[WITM_BER_NUM];
};

/**
 * Difinitions of Ber Nine Figures data block
 * In two-dimension mode
 *     con_x, con_y, ofdma_x, ofdma_y in DownLink Mode ( two groups )
 *     con_bef_x, con_bef_y, con_aft_x, con_aft_y in UpLink Mode ( three groups )
 */
struct wmtf_link_block
{
	float con_x[WITM_BER_NUM][CON_POINT_NUM * DL_SLOT_NUM];
	float con_y[WITM_BER_NUM][CON_POINT_NUM * DL_SLOT_NUM];
	float ofdma_x[WITM_BER_NUM][AMP_POINT_NUM * DL_SLOT_NUM];
	float ofdma_y[WITM_BER_NUM][AMP_POINT_NUM * DL_SLOT_NUM];

	float con_bef_x[WITM_BER_NUM][CON_POINT_NUM * UL_SLOT_NUM];
	float con_bef_y[WITM_BER_NUM][CON_POINT_NUM * UL_SLOT_NUM];
	float con_aft_x[WITM_BER_NUM][CON_POINT_NUM * UL_SLOT_NUM];
	float con_aft_y[WITM_BER_NUM][CON_POINT_NUM * UL_SLOT_NUM];
};

/// Definitions of Ranging info features
struct wmtf_ranging
{
	float status;
    float toff;
    float frac_foff;
    float foff;
};

/// Definitions of Sync info features
struct wmtf_sync
{
	float sync_cor;
	int sync_flag;
	int toff;
	float norm_frac_foff;
	float frac_foff;
	int norm_int_foff;
	float int_foff;
};

/// TX side data block
extern const struct wmtf_tx_block *g_wmtfr_tx_block;
extern struct wmtf_tx_block *g_wmtfw_tx_block;
/// RX side data block
extern const struct wmtf_rx_block *g_wmtfr_rx_block;
extern struct wmtf_rx_block *g_wmtfw_rx_block;
/// Ranging feaures
extern const struct wmtf_ranging *g_wmtfr_ranging;
extern struct wmtf_ranging *g_wmtfw_ranging;
/// Sync Info features
extern const struct wmtf_sync *g_wmtfr_sync;
extern struct wmtf_sync *g_wmtfw_sync;
/// Ber data block
extern const struct wmtf_ber *g_wmtfr_ber;
extern struct wmtf_ber *g_wmtfw_ber;
/// Ber Nine Figures data block
extern const struct wmtf_link_block *g_wmtfr_link_block;
extern struct wmtf_link_block *g_wmtfw_link_block;

#endif /*_WIT_MONITOR_TOOL_MONITOR_FEATURE_H_*/
