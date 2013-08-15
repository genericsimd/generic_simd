/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_PROC_H_
#define __MAC_PROC_H_

int mac_bs_process (void);
int mac_bs_release (void);

int resync_mac(void);

/* need basic SDU queue operation API for CS layer */

/* need basic APIs for connection information insert and check */

/* need basic APIs for Stat. information check */

#endif /* __MAC_H_ */
