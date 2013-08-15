/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: classifier.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CLASSIFIER_
#define _CLASSIFIER_
#include <stdlib.h>

#define NUM_CLSFY_FUNCTIONS 100

typedef struct clsfr classifier;
struct clsfr {
  int classifier_indx;  //index into the array of function pointers 
                        // containing classification code
  void* cls_arg; // memory pointing to list of arguments for classification fn
  size_t arg_len; // length of arugment memory
  int priority; // priority of the classifier
  classifier* next; // pointer to the nest classifier
};

typedef struct {
  u_int8_t* PHSM; //PHS Mask
  u_char* PHSF; //PHS Fields
  int PHSS; // PHS size
  int PHSV; //flag to check if PHS verification to be performed
  u_int8_t PHSI;//PHS index
} PHS;


typedef struct {
  int cid; //  connection id
  int cid_status; // status of the connection
  classifier* classification_list; //list of classification rules to be applied

  //PHS is optional; currently only necessary structures are defined
  //Will be fully implemented in later phases
  u_int8_t PHS_enabled;
  PHS* phs_rule; //PHS rule to be applied for this connection
} connection_classifier_info;


typedef int (*cls_func_ptr)(void* mem, size_t len, cs_sdu_header*sdu);

extern connection_classifier_info** ipv4_conn_clsfr_array;
extern cls_func_ptr* ipv4_f_ptr;

#endif
