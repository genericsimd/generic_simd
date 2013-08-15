/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.   


   File Name: phy_dl_utility.h

   Function:  Define the data structure and APIs for physical layer interface.

   Change Activity:

   Date             Description of Change                          By
   -------------    ---------------------                          -----------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __PHY_DL_UTILITY_H__
#define __PHY_DL_UTILITY_H__

#include <stdlib.h>
#include <stdio.h>

#include "adapter_bs_dl_interface_data.h"
#include "phy_dl_tx_common.h"
#include "phy_dl_tx_interface.h"

unsigned char if_newfile(unsigned int frame_index, unsigned int symbol_offset);
FILE* phy_openfile(unsigned char new_file_flag, char *p_file_name);
void phy_closefile(FILE *fp);
void dl_dump_slotsymbol_header(FILE *fp, struct phy_dl_slotsymbol *p_symbolslot);
void dl_dump_slotsymbol_slot(FILE *fp, struct phy_dl_slot *p_slot);
void dl_dump_slotsymbol(FILE *fp, struct phy_dl_slotsymbol *phybuffer, unsigned int number);
void dl_dump_oneslot_data(FILE *fp, struct phy_dl_slot *p_slot, unsigned int number);
void dl_dump_slots_data(FILE *fp, struct phy_dl_slotsymbol *phybuffer, unsigned int number);
void dl_dump_rrusymbol(FILE *fp, struct phy_dl_rru_symbol *rru_symbol, unsigned int number);
void dl_dump_ffloat(FILE*fp, float *p_buf, unsigned int number);
void dl_dump_uinteger(FILE*fp, unsigned int *p_buf, unsigned int number);
void dl_dump_integer(FILE*fp, int *p_buf, unsigned int number);
void dl_dump_uchar(FILE*fp, unsigned char *p_buf, unsigned int number);
void dl_dump_char(FILE*fp, char *p_buf, unsigned int number);

int dump_phy_dl_ffloat(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_oneslot_data(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_uinteger(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_uchar(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_slotsymbol(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_slots_data(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_dl_rrusymbol(int flag, char * name, FILE * fd, int len, void *p_buf);



#endif 
