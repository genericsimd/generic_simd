/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_assistfunc.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 10-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _MAC_ASSISTFUNC_H_
#define _MAC_ASSISTFUNC_H_

typedef unsigned char BYTE;
#include "adapter_config.h"

#ifdef DEBUG_
#define ERROR_RETURN(p,value,msg)         assert(p == value)
#else
#define ERROR_RETURN(p,value,msg)         if(p == value){ERROR_TRACE("%s\n",msg);return -1;}         
#endif


#ifdef DEBUG_
#define ERROR_RETURN_NOT(p,value,msg)     assert(p != value)
#else
#define ERROR_RETURN_NOT(p,value,msg)     if(p != value){ERROR_TRACE("%s\n",msg);return -1;}
#endif


#ifdef VERIFY
#define VERIFY_ERROR(p,value,msg)         if(p == value){ERROR_TRACE("%s\n",msg);return -1;}
#else  
#define VERIFY_ERROR(p,value,msg)                    
#endif

#define CREATELINKNODE(phead,pcurrentnode,pnextnode)             if(phead == NULL)\
                                                                 {\
                                                                     phead = pnextnode;\
                                                                 }\
                                                                 if(pcurrentnode != NULL)\
                                                                    pcurrentnode->next = pnextnode;\
                                                                 pcurrentnode = pnextnode;

//for gcc   
#ifndef __cplusplus
#define OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#else /* C++ */
#define OFFSETOF(TYPE, MEMBER) (reinterpret_cast <size_t> \
    (&reinterpret_cast <char &>(static_cast <TYPE *> (0)->MEMBER)))
#endif /* C++ */

BYTE*  mallocnode(int  usize);




#endif

