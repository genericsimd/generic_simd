/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_config.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Zhen Bo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_CONFIG_H__
#define __MAC_CONFIG_H__

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

typedef unsigned char u_char;


#ifdef _DEBUG_TRACE_
#define DPRINTF printf
#else
#define DPRINTF(fmt, ...) {}
#endif

#define ERROR_TRACE printf
#define CONFIG_COUNT 256

#pragma pack(push)
#pragma pack(1)

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_int8_t  value;
}tlv8;

typedef struct  {
    u_int8_t  type;
    u_int8_t  length;
    u_int16_t value:9;
}tlv9;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_int16_t value:13;
}tlv13;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_int16_t value:16;
}tlv16;
	
typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char    value[3];
}tlv24;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_int32_t value;
}tlv32;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char    value[5];
}tlv40;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char    value[6];
}tlv48;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char value[8];
}tlv64;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char   value[9];
}tlv72;

typedef struct {
    u_int8_t  type;
    u_int8_t  length;
    u_char   value[13];
}tlv104;

#pragma pack(pop)
#endif
