/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dlmap_builder.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DLMAP_BUILDER_H
#define _DLMAP_BUILDER_H

#include <math.h>
#include <sys/types.h>
#include "mac_amc.h"
#include "dlmap.h"
#include "phy_params.h"
#include "memmgmt.h"
#include "mac_header.h"
#include "dl_exp_params.h"

#define NUM_MAX_DLMAP_IE_PER_SS 3

int dl_map_builder(int* dl_ss_allocation, amc_info* amc_info_header, dl_map_msg* dl_map, int* dlmap_length,int num_dl_subch);

int build_full_band_rect_dlmap_ie(int dl_ss_allocation, dl_map_ie *ie, int* cur_ofdma_symbol_index, int* cur_slot_index, int cid, int diuc, int *dlmap_len,int num_dl_subch, int num_dl_slots_in_freq);

int build_partial_band_dlmap_ie(int dl_ss_allocation, dl_map_ie* ie, int* cur_ofdma_symbol_index, int* cur_slot_index, int cid, int diuc, int *dlmap_len,int num_dl_slots_in_freq);

int init_dlmap_ie(dl_map_ie **ie_ptr, int diuc_code);

int init_normal_diuc_ie(normal_diuc_ie* normal_ie, u_int16_t cid);

int init_extended_ie(extended_diuc_ie **ie_ptr, int extd_diuc);

int init_stc_zone_ie(extended_diuc_ie *ie, BOOL fhdc_flag, stc_matrix_type matrix_ind, int stc_zone_start);

int print_dlmap(dl_map_msg *dl_map);

int dump_dlmap(FILE * fp_dlmap, dl_map_msg *dl_map);


int init_layer_attri(layer_attri *la, int cid, int layer_index, int diuc, int rci);

int init_region_attri(region_attri *ra, stc_matrix_type matrix_ind);
#endif
