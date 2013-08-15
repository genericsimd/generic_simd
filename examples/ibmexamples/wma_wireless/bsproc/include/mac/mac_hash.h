/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_hash.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_HASH_
#define _MAC_HASH_

#include "mac.h"

//simple hash table for now
extern BOOL hashtable[MAX_CIDS];

extern void ht_init();
extern BOOL ht_is_value_present(int value);
extern int ht_add_value(int value);
extern int ht_get_key(int value);

#endif
