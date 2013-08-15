/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_utility.c

   Function: Utility used for PHY DL processing

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "adapter_bs_dl_interface_data.h"
#include "phy_dl_utility.h"
#include "phy_dl_tx_interface.h"
#include "flog.h"

unsigned char if_newfile(unsigned int frame_index, unsigned int symbol_offset)
{
    unsigned char new_file;
    if( (frame_index == 0) && (symbol_offset <= 1) ) /* The first symbol is preamble */
        new_file = 1;
    else
        new_file = 0;
    return new_file;
}

FILE* phy_openfile(unsigned char new_file_flag, char *p_file_name)
{
    FILE *fp;
    if (new_file_flag == 1)
    	fp = fopen(p_file_name, "w+t");
    else
    	fp = fopen(p_file_name, "a+t");
    if(fp == NULL) 
    {
        FLOG_ERROR("Open file %s failed", p_file_name);
        return fp;
    }
    return fp;
}

void phy_closefile(FILE *fp)
{
	  if(fp != NULL)
	  {
	  	fclose(fp);
	  	fp = NULL;
	  }
}

void dl_dump_slotsymbol_header(FILE *fp, struct phy_dl_slotsymbol *p_symbolslot)
{
   	
    fprintf(fp,"------------------------ phy_symboleslot ---------------------------\n");
    fprintf(fp, "is_broadcast = %d\n",p_symbolslot->is_broadcast);
    fprintf(fp, "dl_subframe_end_flag = %d\n",p_symbolslot->dl_subframe_end_flag);
    fprintf(fp, "mimo_mode = %d\n",p_symbolslot->mimo_mode);
    fprintf(fp, "symboloffset = %d\n",p_symbolslot->symboloffset);
    fprintf(fp, "slotlength = %d\n",p_symbolslot->slotlength);
    fprintf(fp, "frame_index = %d\n",p_symbolslot->frame_index);
    fprintf(fp,"--------------------------------------------------------------------\n");

}

void dl_dump_slotsymbol_slot(FILE *fp, struct phy_dl_slot *p_slot)
{
    unsigned int i = 0;
    fprintf(fp, "code_id = %d\n",p_slot->code_id);
    fprintf(fp, "bytelength = %d\n",p_slot->bytelength);
    fprintf(fp, "repetition_coding_indication = %d\n",p_slot->repetition_coding_indication);
    fprintf(fp, "is_broadcast = %d\n",p_slot->is_broadcast);
    fprintf(fp, "coding_type = %d\n",p_slot->coding_type);
    fprintf(fp, "mimo_mode = %d\n",p_slot->mimo_mode);
    fprintf(fp, "block_id = %d\n",p_slot->block_id);
    fprintf(fp, "unused_flag = %d\n",p_slot->unused_flag);
    for(; i < p_slot->bytelength; i++) 
    {
        fprintf(fp,"%x",p_slot->payload[i]);
    }
    fprintf(fp, "\n");
}

void dl_dump_slotsymbol(FILE *fp, struct phy_dl_slotsymbol *phybuffer, unsigned int number)
{
    static unsigned int ntimes=0;
    if (number != 1) 
    {
    	  printf("dump_slotsymbol: Only can dump one by one\n");
    	  exit(1);
    }
    if(phybuffer == NULL)
        return;
    struct phy_dl_slot *p_slot = phybuffer->slot_header;
    fprintf(fp,"\n----------------------the %dth to record---------\n",ntimes);
    dl_dump_slotsymbol_header(fp,phybuffer);
    int icount = 0;
    while(p_slot != NULL) 
    {
        fprintf(fp,"--------------------the %d slot----------------\n",icount);
        dl_dump_slotsymbol_slot(fp,p_slot);
        fprintf(fp,"------------------------------------------------------\n");
        p_slot = p_slot->next;
        icount++;
    }
    ntimes++;
}

void dl_dump_oneslot_data(FILE *fp, struct phy_dl_slot *p_slot, unsigned int number)
{
    unsigned int i;
    if (number != 1) 
    {
    	  FLOG_DEBUG("dump_one_slotdata: Only can dump one by one\n");
    	  exit(1);
    }
    for(i=0; i < p_slot->bytelength; i++) 
    {
        fprintf(fp,"%d\n",p_slot->payload[i]);
    }
}

void dl_dump_slots_data(FILE *fp, struct phy_dl_slotsymbol *phybuffer, unsigned int number)
{
    unsigned int i;
    unsigned int count = 0;
    if (number != 1) 
    {
    	  FLOG_DEBUG("dump_slot_bench: Only can dump one by one\n");
    	  exit(1);
    }
    struct phy_dl_slot *p_slot = phybuffer->slot_header;
    while(p_slot != NULL) 
    {
        for(i=0; i < p_slot->bytelength; i++) 
        {
            fprintf(fp,"%d\n",p_slot->payload[i]);
        }
        p_slot = p_slot->next;;
        count++;
    }
}

void dl_dump_rrusymbol(FILE *fp, struct phy_dl_rru_symbol *rru_symbol, unsigned int number)
{
    unsigned int sample_count;
    float *sample_i, *sample_q;

    if (number != 1) 
    {
    	  FLOG_DEBUG("dump_rrusymbol: Only can dump one by one\n");
    	  exit(1);
    }
    sample_i = rru_symbol->symbol_i;
    sample_q = rru_symbol->symbol_q;
    fprintf(fp, "\n\nframe_index=%d\n", rru_symbol->frame_index);
    fprintf(fp, "symbol_offset=%d\n", rru_symbol->symbol_offset);
    fprintf(fp, "symbol_num=%d\n", rru_symbol->symbol_num);
    fprintf(fp, "symbol_len=%d\n", rru_symbol->symbol_len);
    for (sample_count=0; sample_count<rru_symbol->symbol_num*rru_symbol->symbol_len; sample_count++) 
    {
         fprintf(fp,
                "sample%d: (%.6f, %.6f)\n",
                sample_count,
                sample_i[sample_count],
                sample_q[sample_count]);
    }
}

void dl_dump_ffloat(FILE*fp, float *p_buf, unsigned int number)
{
	  unsigned int count;
	  for (count=0; count<number; count++)
	  {
  	    fprintf(fp, "%.6f\n", p_buf[count]);
  	}
	  return;
	  
} 

void dl_dump_uinteger(FILE*fp, unsigned int *p_buf, unsigned int number)
{
	  unsigned int count;
	  for (count=0; count<number; count++)
	  {
  	    fprintf(fp, "%d\n", p_buf[count]);
  	}
	  return;
} 

void dl_dump_integer(FILE*fp, int *p_buf, unsigned int number)
{
	  unsigned int count;
	  for (count=0; count<number; count++)
	  {
  	    fprintf(fp, "%d\n", p_buf[count]);
  	}
	  return;
	  
} 

void dl_dump_uchar(FILE*fp, unsigned char *p_buf, unsigned int number)
{
	  unsigned int count;
	  for (count=0; count<number; count++)
	  {
  	    fprintf(fp, "%d\n", p_buf[count]);
  	}
	  return;
	  
} 

void dl_dump_char(FILE*fp, char *p_buf, unsigned int number)
{
	  unsigned int count;
	  for (count=0; count<number; count++)
	  {
  	    fprintf(fp, "%d\n", p_buf[count]);
  	}
	  return;
	  
}

/**----------------------------------------------------------------------------
 *    Function:    dump_phy_dl_xxx
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

int dump_phy_dl_ffloat(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_ffloat(fd, (float *)p_buf, len);
	return 0;
}

int dump_phy_dl_oneslot_data(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_oneslot_data(fd,  (struct phy_dl_slot *)p_buf, len);
	
	return 0;
}

int dump_phy_dl_uinteger(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_uinteger(fd,  p_buf, (unsigned int) len);
	
	return 0;
}



int dump_phy_dl_uchar(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;


	dl_dump_uchar(fd,  p_buf, (unsigned int) len);
	
	return 0;
}


int dump_phy_dl_slotsymbol(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_slotsymbol(fd,  (struct phy_dl_slotsymbol *)p_buf, len);
	
	return 0;
}


int dump_phy_dl_slots_data(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_slots_data(fd,  (struct phy_dl_slotsymbol *)p_buf, len);
	
	return 0;
}


int dump_phy_dl_rrusymbol(int flag, char * name, FILE * fd, int len, void *p_buf)
{
    (void)	name;
    (void)	flag;

	dl_dump_rrusymbol(fd,  (struct phy_dl_rru_symbol *)p_buf, len);
	
	return 0;
}



