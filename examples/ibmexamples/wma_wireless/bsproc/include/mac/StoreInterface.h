/******************************************************************************
 (C) Copyright IBM Corp. 2011 All Rights Reserved
     IoT Key Management
     Contact: jipeng@cn.ibm.com
*******************************************************************************/

#ifndef _STORE_INTERFACE_H_
#define _STORE_INTERFACE_H_

#if defined(__cplusplus)
extern "C" {
#endif
	
#include <stdio.h>
#include <stdlib.h>

#include <openssl/ssl.h>
#include <openssl/bio.h> 
#include <openssl/conf.h> 
//#include <openssl/err.h> 
#include <openssl/pem.h> 
#include <openssl/x509.h>
#include "Def.h"

/*
 *	The method is to get content of private key for the given device from key file stored in Key Store
 *  @deviceIdentity, always MAC address of the device, e.g. "112233445566"
 *  @pwd4KeyFile, password to open private key file
 *  @return, address of EVP_PKEY instance for successful reading, otherwise return NULL
 */
EVP_PKEY* CMA_Get_PrivateKey(const char* deviceIdentity, const char* pwd4KeyFile);

/*
 *	The method is to get content of X.509 certificate for the given device from certificate file stored in Key Store
 *  @deviceIdentity, always MAC address of the device, e.g. "112233445566"
 *  @return, address of X509 certificate instance for successful reading, otherwise return NULL
 */
X509* CMA_Get_Certificate(const char* deviceIdentity);


#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif	/* _STORE_INTERFACE_H_ */
