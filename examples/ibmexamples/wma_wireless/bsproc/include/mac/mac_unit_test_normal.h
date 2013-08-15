/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_unit_test_normal.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Zhen Bo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_UNIT_TEST_H__
#define __MAC_UNIT_TEST_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mac_config.h"
#include "mac_headermsg_builder.h"
#include "mac_headermsg_parser.h"
#include "mac_header.h"
#include "mac_ul_pdu_parser.h"
#include "mac_ul_pdu_queue.h"
#include "mac_dl_concatenation.h"
#include "mac_ul_reassembly.h"
#include "mac_ul_frag_queue.h"
#include "mac_sdu_queue.h"
#include "mac_hcs.h"
#include "mac_dl_frag_pack.h"
#include "mac_crc.h"
#include "mac_connection.h"
#include "mac_subframe_queue.h"
#include "mac_cps.h"
#include "br_queue.h"
int main(int argc,char *argv[]);

int testgmh();

int testfsh();

int testpsh();

int testframeprefix();

int testconcat_parse();

int testcrc();

int testhcs();

int testfrag_pack_assembe_fixsdu();

int testfrag_pack_assembe_fragonly();

int testfrag_pack_assembe_packonly();

int testfrag_pack_assembe_fragpack();

int testfrag_pack_assembe_arqonly();

int testfrag_pack_assembe_arqfrag();

int testfrag_pack_assembe_arqpack();

int testfrag_pack_assembe_arqfragpack();

int testcps_framework();

int testbrqueue();

int testfrag_pack_fragments_fragonly();

int testfrag_pack_fragments_fragpack();

int testfrag_pack_fragments_arqfragpack();

int testfrag_pack_arqretrans_arqfragpack();

int testfrag_pack_arqretrans_arqpack();

int testfrag_pack_arqretrans_arqfrag();

int testfrag_pack_arqretrans_arqonly();

int testreassembly_arqretrans_arqfragpack();

#endif

