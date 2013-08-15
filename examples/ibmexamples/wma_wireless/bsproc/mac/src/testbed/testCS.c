/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: testCS.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "CS.h"
#include "classifier.h"
#include "cs_sdu_header.h"
#include "memmgmt.h"

void testCS(void) {

  //initialize conn clsfr array
  connection_classifier_info** conn_clsfr_array=ipv4_conn_clsfr_array;
  conn_clsfr_array[0]= (connection_classifier_info*) mac_malloc(sizeof(connection_classifier_info));
  conn_clsfr_array[0]->cid=150;
  conn_clsfr_array[0]->cid_status=1;
  conn_clsfr_array[0]->classification_list= (classifier*) mac_malloc(sizeof(classifier));
  classifier* clsfr=conn_clsfr_array[0]->classification_list;
  clsfr->classifier_indx=5;
  clsfr->cls_arg=mac_malloc(sizeof(u_char)*8);
  memset(clsfr->cls_arg, 0, 8);
  clsfr->arg_len=8;
  clsfr->priority=10;
  clsfr->next=NULL;
  conn_clsfr_array[0]->PHS_enabled=0;
  conn_clsfr_array[0]->phs_rule=NULL;

  //create payload
  cs_sdu_header* sdu_hdr=(cs_sdu_header*) mac_malloc(sizeof(cs_sdu_header));
  sdu_hdr->upper_layer_sdu= (void*) mac_sdu_malloc(200, 0);
  sdu_hdr->sdu_len=200;
  sdu_hdr->hdr_supd_sdu=NULL;
  sdu_hdr->supd_sdu_len=0;
  sdu_hdr->classified=0;
  sdu_hdr->phs_enabled=0;
  sdu_hdr->hdr_supd=0;

  //call classify
  IPV4_CS_DL(sdu_hdr);
}
