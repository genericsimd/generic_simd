/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: q_container.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Apr.2009       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _Q_CONTAINER_
#define _Q_CONTAINER_
#include <stdlib.h>

//definitions for Q container
typedef struct qcontainer q_container;
struct qcontainer {
  q_container* prev; //prev ptr in q
  q_container* next; // next ptr in q
  void* data; //data of the list element
  size_t len; //length of the list element
  int data_type; //type of the data
  unsigned long long key;
};

#endif //ifndef _Q_CONTAINER_
