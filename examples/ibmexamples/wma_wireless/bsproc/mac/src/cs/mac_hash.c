/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: mac_hash.c

Change Activity:

Date    	Description of Change        	By
----------------------------------------------------------
01-Oct-2008 	     Created		   Malolan Chetlur

----------------------------------------------------------
*/

#include "mac_hash.h"
#include "mac.h"
#include "debug.h"
#include "flog.h"

BOOL hashtable[MAX_CIDS];

void ht_init() {
  FLOG_DEBUG("Calling hash_init() ...");
  int ii=0;
  for(ii=0; ii<MAX_CIDS; ii++) {
    hashtable[ii]=FALSE;
  }
}

BOOL ht_is_value_present(int value) {
  return ((hashtable[value]==TRUE) ? TRUE : FALSE);
}

int ht_add_value(int value) {
  hashtable[value]=TRUE;
  return value;
}

int ht_get_key(int value) {
  return value;
}
