/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_header.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Zhen Bo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_HEADER_H__
#define __MAC_HEADER_H__

#include "mac_config.h"

#define GENERIC_MAC_HEADER_LEN 6
#define MAC_CRC_LEN 4
#define MAX_FCH_LEN 24
#define BYTES_PER_SLOT 6

#define UN_FRAGMENTED 0x00
#define LAST_FRAG 0x01
#define FIRST_FRAG 0x02
#define CONTINUING_FRAG 0x03

#define EXTEND_FRAG_SUBHEADER_LEN 2
#define FRAG_SUBHEADER_LEN 1
#define EXTEND_PACK_SUBHEADER_LEN 3
#define PACK_SUBHEADER_LEN 2


// extended subheader type

//UL
#define MIMO_MODE_FEEDBACK_ESH_TYPE 0
#define UL_TX_POWER_REPORT_ESH_TYPE 1
#define MINI_FEEDBACK_ESH_TYPE 2
#define PDU_SN_SHORT_ESH_TYPE 3
#define PDU_SN_LONG_ESH_TYPE 4

#define GRANT_MANAGEMENT_SUBHEADER_LEN 2
#define MIMO_MODE_FEEDBACK_ESH_LEN 1
#define UL_TX_POWER_REPORT_ESH_LEN 1
#define MINI_FEEDBACK_ESH_LEN 2
#define PDU_SN_SHORT_ESH_LEN 1
#define PDU_SN_LONG_ESH_LEN 2


// the generic mac header for wach PDU is fixed in length, 6 bytes, from 6.3.2

typedef struct {
    u_int8_t ht; // header type, shall be set to o, 0= generic header  or mac management message, 1=mac header without payload and crc
    u_int8_t ec; // encryption control, 1=payload is encrypted, 0 = no encyrption 
    u_int8_t type; // indicates the subheaders and special payload types present
    u_int8_t esf; // extended subheader field, indicates that extended subheaders is present
    u_int8_t ci; //CRC indicator, 1=crc included, 0=no crc
    u_int8_t eks; // encryption key sequence, the index of the TEK and IV used to encrypt the payload
    u_int8_t rsv; // shall be set to zero
    u_int16_t len ; //the length (bytes) of the pdu including mac header and crc
    u_int16_t cid ; //
    u_int8_t hcs; // header check sequence, detect errors in mac header, calculated according to the first five bytes
}generic_mac_hdr ;

// Fragment subheader

typedef struct {
    u_int8_t    fc; // indicates the fragmentation state of the payload
    u_int16_t    fsn; // sequence number of current mac sdu fragment.
    u_int8_t    rsv; // shall be set to zero
}frag_sub_hdr;

// packing subheader

typedef struct {
    u_int8_t    fc; // indicates the fragmentation state of the payload
    u_int16_t    fsn; // indicates the sequence number of the current SDU fragment, increment by one for each fragment, including unfragmented SDUs and unpacked SDUs
    u_int16_t    length;//length of the SDU fragments in bytes including the PSH
}pack_sub_hdr;


#endif

