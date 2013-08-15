/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_demuxofdmsym.h

   Function: Header file for phy_ul_demuxofdmsym()

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#ifndef  _PHY_UL_DEMUXOFDMSYM_H_ 

#define  _PHY_UL_DEMUXOFDMSYM_H_ 



#include "phy_ul_rx_interface.h"

int32_t phy_ul_demuxofdmsym(struct phy_ul_rx_syspara *para,
                            const float *muxofdm_data_r,
                            const float *muxofdm_data_i,
                            const u_int32_t *datasubcar,
                            const u_int32_t *rotation_posindex,
                            const u_int32_t *slotsym_num,
                            float *demuxofdmsym_r,
                            float *demuxofdmsym_i);

int32_t phy_ul_demuxpilot(struct phy_ul_rx_syspara *para,
                          const float *muxofdm_data_r,
                          const float *muxofdm_data_i,
                          const u_int32_t *pilot_allocation,
                          const u_int32_t *rotation_posindex,
                          const u_int32_t *slotsym_num,
                          float *demuxpilot_r,
                          float *demuxpilot_i);

#endif

