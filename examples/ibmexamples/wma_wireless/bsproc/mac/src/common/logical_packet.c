/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: logical_packet.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "logical_packet.h"
#include "memmgmt.h"


//initialize logical element 
logical_element* logical_element_init(size_t num_bytes, void* mem, element_type et) {
  logical_element* le=(logical_element*) mac_malloc(sizeof(logical_element));
  if (le == NULL)
  {
    FLOG_FATAL("Malloc failed in logical_element_init\n");
  }
  le->type=et;
  //no fragmentation intially
  le->blk_type=NO_FRAGMENTATION;
  le->length=num_bytes;
  le->data=mem;
  //starting block number for this sdu fragment
  le->start_bsn=0;
  le->next=NULL;
  return le;
}

//finalize logical element
// free the memory used by logical element structure
void logical_element_finalize(logical_element* le) {
  mac_free(sizeof(logical_element), le);
}

//initialize the logical packet
logical_packet* logical_packet_init(int cid, size_t num_bytes, void* mem, element_type et) {

  logical_packet* lp= (logical_packet*)mac_malloc(sizeof(logical_packet));
  if (lp == NULL)
  {
    FLOG_FATAL("Malloc failed in logical_packet_init\n");
  }
  lp->cid=cid;
  lp->length=num_bytes;
  lp->element_head=logical_element_init(num_bytes, mem, et);
  lp->next=NULL;
  lp->prev=NULL;
  return lp;
}

//finalize logical packet
// free the memory used by logical packet structure
void logical_packet_finalize(logical_packet* lp) {
  mac_free(sizeof(logical_packet), lp);
}
