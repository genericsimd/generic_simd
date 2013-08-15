/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_shift_bits.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 2-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _MAC_SHIFT_BITS_
#define _MAC_SHIFT_BITS_
#include "adapter_config.h"

#define PERBITINBYTE        8
#define LSB                 0x01



int bitshuff(u_int8_t *firstdata,u_int8_t *getdata,int mode,u_int8_t MASK,int bitlength);
int bitshuff_lowbit_to_highbit(u_int8_t *firstdata,u_int8_t *getdata,u_int8_t MASK,int bitlength);
int bitshuff_highbit_to_lowbit(u_int8_t *firstdata,u_int8_t *getdata,u_int8_t MASK,int bitlength);

/*      offset
*  |--------|-----|
*  |        |     |
*  |________|_____|  
*create by changjj
*if mode >=0  then first move pdata1 from low bits to pdata2 high bits,else from pdata1 high to pdata1 low bits 
*/
int bitmove(u_int8_t *pdata1,u_int8_t *pdata2,int mode,int bitoffset,int ibytecount);
int bitcombinate(u_int8_t data1,u_int8_t data2,int ioffset,u_int8_t *presultdata);

int get_bit_data_put_in_byte(u_int8_t *pdata,int ioffset,int ibitlength,int ibytelength,u_int8_t *presultdata);
//int macro_get_bit_data_put_in_byte(u_int8_t *pdata,int ioffset,int ibitlength,int ibytelength,u_int8_t *presultdata);

int bit_to_byte_f(u_int8_t *pdata,int ioffset,int ibitlength,int ibytelength,u_int8_t *presultdata);
int bit_to_byte_f_2buf(u_int8_t *pdata,int ioffset,int ibitlength,int ibytelength,u_int8_t *presultdata_odd,u_int8_t *presultdata_even);

int bits_to_byte(u_int8_t *pdata,int ibitlength,u_int8_t *pdestdata,int *ibitstartoffset,int * ibyteoffset);
int bits_to_byte_2(u_int8_t *pdata1,u_int8_t *p_data2, int ibitlength,u_int8_t *pdestdata,int *ibitstartoffset,int * ibyteoffset);
#endif
