/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: memmgmt.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MEM_MGMT_
#define _MEM_MGMT_

#include <stdlib.h>
#include "mac.h"
#include "logical_packet.h"

#define MMM_CLASS_TYPE 5
#ifdef LOG_METRICS
typedef struct {
  void* sdu_start_location;
  short class_type;
} sdu_mem_tag;
#endif
extern void* mac_malloc(size_t num_bytes);
extern void mac_free(size_t num_bytes, void* mem);

extern void *WiMAX_mac_malloc(size_t);
extern void WiMAX_mac_free(void *);
extern void *WiMAX_mac_calloc(int, size_t);

//The following functions should be used to
//allocate and free mac transport sdu's  only

//currently class_type is unused; used for future extension
extern void* mac_sdu_malloc(size_t num_bytes, short class_type);
extern int mac_sdu_free(void* mem, size_t num_bytes, fragment_type ftype);

extern int app_malloc(void** ptr, size_t  len);
extern int app_realloc(void** ptr, size_t  len);
extern int app_free(void* ptr, size_t len);

#endif
