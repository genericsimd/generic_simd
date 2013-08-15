/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: logical_packet.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _LOGICAL_PACKET_
#define _LOGICAL_PACKET_

#include "mac.h"

//Enum definiing the fragment type in a PDU
typedef enum { NO_FRAGMENTATION=0,
	       LAST_FRAGMENT=1,
	       FIRST_FRAGMENT=2,
	       CONTINUING_FRAGMENT=3 } fragment_type;

// Enum defining the type of object pointed by logical_element data 
// within a logical_packet
typedef enum { MAC_GENERIC_HEADER=0,
		    FRAG_SUBHEADER=1,
		    EXTEND_FRAG_SUBHEADER=2,
		    PACK_SUBHEADER=3,
		    EXTEND_PACK_SUBHEADER=4,
		    MAC_SDU=5,
		    ARQ_BLOCK=6,
		    MAC_SDU_FRAG = 7,
		    MAC_PDU=8,
		    MAC_MGT_MESSAGE=9,
		    DL_MAP_IE=10,
		    UL_MAP_IE=11,
		    EXTEND_SUBHEADER=12} element_type;


// logical_element contains pointer to physical data and length of the data
// In the case of an SDU, it also contains the fragment type and block sequence number (BSN)

typedef struct logicalelement logical_element;

struct logicalelement{
  element_type  type; // header, arq block, fragmented sdu, sdu
  fragment_type blk_type;
  size_t length;
  u_char *data;
  int start_bsn; //used in the case of ARQ
  logical_element *next;
};

// logical_packet containing logical_element 
// A list of logical_packet can be formed using the next, prev pointers

typedef struct logicalpacket logical_packet;

struct logicalpacket{
  int cid;
  int length;
  logical_element *element_head;
  logical_packet *next;  
  logical_packet *prev;
};

extern logical_packet* logical_packet_init(int cid, size_t num_bytes, void* mem, element_type et);

extern void logical_packet_finalize(logical_packet* lp);
extern void logical_element_finalize(logical_element* le);

#endif
