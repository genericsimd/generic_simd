/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _TEST_UTILITY_H_
#define _TEST_UTILITY_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "phy_ul_rx_common.h"
#include "phy_ul_rx.h"
#include "phy_ul_rx_interface.h"
#include "adapter_bs_ul_map_interface.h"



FILE* open_ul_file(char *p_file_name);

void close_ul_file(FILE *fp);
int32_t dump_ulrx_procblk(FILE *fp, struct block_data_ie *block);

void dump_ulrx_ffloat(FILE*fp, float *p_buf, unsigned int number);
void dump_ulrx_uinteger(FILE*fp, unsigned int *p_buf, unsigned int number);
void dump_ulrx_integer(FILE*fp, int *p_buf, unsigned int number);
void dump_ulrx_uchar(FILE*fp, unsigned char *p_buf, unsigned int number);
void dump_ulrx_char(FILE*fp, char *p_buf, unsigned int number);

int dump_phy_ul_rx_ffloat(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_ul_rx_uinteger(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_ul_rx_uchar(int flag, char * name, FILE * fd, int len, void *p_buf);
int dump_phy_ul_rx_activeband(int flag, char * name, FILE * fd, int len, void *p_buf);



#endif
