/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: uart_util.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-Aug 2011       Created                                         Chang, Jianjun

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _UART_UTIL_H_
#define _UART_UTIL_H_

int init_uart(void);
int release_uart(void);
int write_uart(char * buf, int len);

#endif
