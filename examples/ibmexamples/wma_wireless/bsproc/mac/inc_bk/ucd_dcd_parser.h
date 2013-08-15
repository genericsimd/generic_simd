/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ucd_dcd_parser.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _UCD_DCD_PARSER_H_
#define _UCD_DCD_PARSER_H_

#include "ucd.h"
#include "dcd.h"


extern int parse_ucd(u_char* payload, int length, ucd_msg* ucd);  // meomry for ucd should be allocated before calling this function
extern int parse_dcd(u_char* payload, int length, dcd_msg* dcd);  // meomry for dcd should be allocated before calling this function
extern int print_ucd_msg(ucd_msg *ucd);
extern int print_dcd_msg(dcd_msg *dcd);


#endif
