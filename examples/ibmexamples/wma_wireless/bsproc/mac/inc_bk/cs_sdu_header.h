/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: cs_sdu_header.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CS_SDU_HEADER_
#define _CS_SDU_HEADER_
#include <stdlib.h>

typedef struct {
  void* upper_layer_sdu; //SDU from the upper network layer
  size_t sdu_len; // length of SDU from upper network layer
  void* hdr_supd_sdu; // pointer to upper layer sdu with header suppressed
  size_t supd_sdu_len; // length of header suppressed sdu
  u_int8_t classified; //variable to indicate sdu is classified
  u_int8_t phs_enabled; // flag to indicate if this sdu allows PHS
  u_int8_t hdr_supd; //variable to indicate sdu hdr is suppressed
  int cid; // cid of the sdu (after classification
} cs_sdu_header;

#endif
