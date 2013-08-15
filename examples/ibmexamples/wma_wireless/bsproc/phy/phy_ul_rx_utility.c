/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */




#include <string.h>
#include <memory.h> 
#include "phy_ul_rx_utility.h"
#include "adapter_bs_ul_map_interface.h"
#include "flog.h"


FILE* open_ul_file(char *p_file_name)
{
    FILE *fp;
    fp = fopen(p_file_name, "a+");
    if(fp == NULL)
    {
        FLOG_WARNING("open dump file failed \n");
        return fp;
    }
    return SUCCESS_CODE;
}

void close_ul_file(FILE *fp)
{
    fclose(fp);
}


int dump_ulrx_datablk(FILE *fp, struct block_data_ie *block)
{
    fprintf(fp, "code_id = %d \n",block->uiuc);
    fprintf(fp, "subchannel_offset= %d\n", block->subchannel_offset);
    fprintf(fp, "subchannel_num= %d\n", block->subchannel_num);
    fprintf(fp, "symbol_offset= %d\n", block->ofdma_symbol_offset);
    fprintf(fp, "repetition_code = %d \n",block->repetition_coding_indication);
    fprintf(fp, "coding_type = %d \n",block->coding_type);
    fprintf(fp, "mimo_mode = %d\n",block->mimo_mode);
    return 0;
}

void dump_ulrx_ffloat(FILE*fp, float *p_buf, unsigned int number)
{
    unsigned int count;
    for (count=0; count<number; count++)
    {
        fprintf(fp, "%.6f\n", p_buf[count]);
    }

    return;

} 


void dump_ulrx_uinteger(FILE*fp, unsigned int *p_buf, unsigned int number)
{
    unsigned int count;
    for (count=0; count<number; count++)
    {
        fprintf(fp, "%d\n", p_buf[count]);
    }

    return;
} 


void dump_ulrx_integer(FILE*fp, int *p_buf, unsigned int number)
{
    unsigned int count;
    for (count=0; count<number; count++)
    {
        fprintf(fp, "%d\n", p_buf[count]);
    }

    return;
} 


void dump_ulrx_uchar(FILE*fp, unsigned char *p_buf, unsigned int number)
{
    unsigned int count;
    for (count=0; count<number; count++)
    {
        fprintf(fp, "%d\n", p_buf[count]);
    }

    return;	  
} 


void dump_ulrx_char(FILE*fp, char *p_buf, unsigned int number)
{
    unsigned int count;
    for (count=0; count<number; count++)
    {
        fprintf(fp, "%d\n", p_buf[count]);
    }

    return;
} 


/**----------------------------------------------------------------------------
 *    Function:    dump_phy_ul_rx_xxx
 *
 *    Description: Callback functions for integration dump framework.
 *
 *    Parameters:
 *       Input  -  [int flag] flag 
 *       	-  [char * name] file name 
 *       	-  [FILE *fd] file handle to put the dump data in 
 *       	-  [int len] data length 
 *       	-  [void *p_buf] data buffer 
 *
 *    Return Value:
 *       0       Success
 *       others  Error
 *    Notes:
 *       None
 *-------------------------------------------------------------------
 * LOG END TAG zYx                                                            */
int dump_phy_ul_rx_ffloat(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dump_ulrx_ffloat(fd,  p_buf, (unsigned int) len);
	
	return 0;
}

int dump_phy_ul_rx_uinteger(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dump_ulrx_uinteger(fd,  p_buf, (unsigned int) len);
	
	return 0;
}



int dump_phy_ul_rx_uchar(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dump_ulrx_uchar(fd,  p_buf, (unsigned int) len);
	
	return 0;
}

int dump_phy_ul_rx_activeband(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;
    (void)	len;

    struct phy_dts_info *dtspara = (struct phy_dts_info *)p_buf;
    int mm = 0;

    for (mm =0; mm<21; mm++)
    {
        fprintf(fd, "dts->active_band[%d] = %d\n", mm, dtspara->active_band[mm]);
    }
						
    fprintf(fd, "dts->dl_unused_subch = %d\n", dtspara->dl_unused_subch);
    fprintf(fd, "dts->ul_unused_subch = %d\n", dtspara->ul_unused_subch);
	
	return 0;
}



