/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2012

   All Rights Reserved.

   File Name: mac_br_api.h

   Change Activity:

   Date                      Description of Change                   By
   -----------      --------------------- 		--------
   22-MAY.2012		Created                          	Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_SF_API_H_
#define _MAC_SF_API_H_

#include <stdint.h>
#include "mac.h"

struct br_req
{
	u_int16_t	type;
	u_int16_t	cid;
	u_int32_t	bw_len;
};
#define BR_REQ	(15U)

#endif
